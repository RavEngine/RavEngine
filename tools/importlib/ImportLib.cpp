#include "Skeleton.hpp"
#include <stdexcept>
#include <assimp/scene.h>
#include <fmt/format.h>
#include "Mesh.hpp"

using namespace std;

void ASSERT(bool cond, const std::string_view msg){
    if (!cond){
        throw std::runtime_error(std::string(msg));
    }
}

namespace RavEngine {

    MeshPart AIMesh2MeshPart(const aiMesh* mesh, const matrix4& scalemat)
    {
        MeshPart mp;
        //mp.indices.mode = indexBufferWidth;

        mp.indices.reserve(mesh->mNumFaces * 3);
        mp.vertices.reserve(mesh->mNumVertices);
        for (int vi = 0; vi < mesh->mNumVertices; vi++) {
            auto vert = mesh->mVertices[vi];
            vector4 scaled(vert.x, vert.y, vert.z, 1);

            scaled = scalemat * scaled;

            ASSERT(mesh->mTangents, "Mesh does not have tangents!");
            ASSERT(mesh->mBitangents, "Mesh does not have bitangents!");

            auto normal = mesh->mNormals[vi];
            auto tangent = mesh->mTangents[vi];
            auto bitangent = mesh->mBitangents[vi];

            //does mesh have uvs?
            float uvs[2] = { 0 };
            if (mesh->mTextureCoords[0]) {
                uvs[0] = mesh->mTextureCoords[0][vi].x;
                uvs[1] = mesh->mTextureCoords[0][vi].y;
            }

            mp.vertices.push_back(VertexNormalUV{
                .position = {static_cast<float>(scaled.x),static_cast<float>(scaled.y),static_cast<float>(scaled.z)},
                .normal = {normal.x,normal.y,normal.z},
                .tangent = {tangent.x, tangent.y, tangent.z},
                .bitangent = {bitangent.x,bitangent.y,bitangent.z},
                .uv = {uvs[0],uvs[1]}
                });
        }

        for (int ii = 0; ii < mesh->mNumFaces; ii++) {
            //alert if encounters a degenerate triangle
            if (mesh->mFaces[ii].mNumIndices != 3) {
                throw runtime_error("Cannot load model: Degenerate triangle (Num indices = " + to_string(mesh->mFaces[ii].mNumIndices) + ")");
            }

            mp.indices.push_back(mesh->mFaces[ii].mIndices[0]);
            mp.indices.push_back(mesh->mFaces[ii].mIndices[1]);
            mp.indices.push_back(mesh->mFaces[ii].mIndices[2]);

        }
        return mp;
    }


    void ASSERT(bool cond, const std::string& msg) {
        if (!cond) {
            throw std::runtime_error(msg);
        }
    }

    uint16_t SerializedSkeleton::IndexForBoneName(const std::string_view name) {
        auto it = std::find_if(allBones.begin(), allBones.end(), [&name](const Bone& bone) {
            return bone.name == name;
        });
        return std::distance(allBones.begin(), it);
    }

    SerializedSkeleton FlattenSkeleton(const SkeletonData& skeleton) {
        SerializedSkeleton serialized;
        UnorderedMap<std::string_view, uint16_t> nameToOffset;
        // flatten skeleton

        auto recurseSkeleton = [&serialized, &nameToOffset](const SkeletonData::Bone& bone, auto&& fn) -> void {
            serialized.allBones.push_back({bone.transform, bone.name});
            const auto myIdx = serialized.allBones.size() - 1;
            nameToOffset[bone.name] = myIdx;

            // recurse children
            for (const auto& child : bone.children) {
                fn(child, fn);
            }

            };
        recurseSkeleton(skeleton.root, recurseSkeleton);

        auto recurseSkeletonForChildren = [&serialized, &nameToOffset](const SkeletonData::Bone& bone, auto&& fn) -> void {
            // populate children map
            serialized.childrenMap.push_back({});
            auto& childrenList = serialized.childrenMap.back();
            for (const auto& child : bone.children) {
                auto idx = nameToOffset.at(child.name);
                childrenList.push_back(idx);
            }

            // recurse children
            for (const auto& child : bone.children) {
                fn(child, fn);
            }
            };
        recurseSkeletonForChildren(skeleton.root, recurseSkeletonForChildren);

        return serialized;
    }

	RavEngine::NameToBoneResult RavEngine::NameToBone(const aiScene* scene)
	{
        // create hashset of the bones list to determine quickly if a scene node is a relevant bone
        UnorderedMap<std::string_view, aiBone*> bones;

        for (int i = 0; i < scene->mNumMeshes; i++) {
            auto mesh = scene->mMeshes[i];	//assume the first mesh is the mesh to use (TODO: filter the objects)
            if (!mesh->HasBones()) {
                continue;
            }

            for (int i = 0; i < mesh->mNumBones; i++) {
                bones.insert(make_pair(string_view(mesh->mBones[i]->mName.C_Str()), mesh->mBones[i]));
            }
        }
        ASSERT(bones.size() > 0, "Scene does not contain bones!");

        //find root bone
        //pick a bone, and recurse up its parent hierarchy
        //TODO: this is very inefficient, optimize
        aiNode* rootBone = nullptr;
        for (const auto& name : bones) {
            auto node = scene->mRootNode->FindNode(aiString(name.first.data()));

            //if the node has all of the bones in its sub-hierarchy (findnode) then it is the root node for the skeleton
            while (node->mParent != NULL) {
                int found_bones = 0;
                for (const auto& bone : bones) {
                    if (node->FindNode(aiString(bone.first.data()))) {
                        found_bones++;
                    }
                }
                if (found_bones == bones.size()) {
                    rootBone = node;
                    goto found_bone;
                }
                else {
                    node = node->mParent;
                }
            }

        }
    found_bone:
        return { bones, rootBone };
	}

    SkeletonData CreateSkeleton(const NameToBoneResult& unpackedSkeleton)
    {
        auto& bones = unpackedSkeleton.bones;
        auto& rootBone = unpackedSkeleton.rootBone;
        // we will now bone this mesh
        SkeletonData skeleton;

        //recurse the root node and get all of the bones

        //construct skeleton hierarchy
        auto recurse_bone = [&bones](SkeletonData::Bone& ozzbone, aiNode* node) -> void {
            auto recurse_impl = [&bones](SkeletonData::Bone& ozzbone, aiNode* node, auto& recursive_call) -> void {
                // create its transformation
                aiVector3t<float> scale, position;
                aiQuaterniont<float> rotation;
                node->mTransformation.Decompose(scale, rotation, position);
                ozzbone.transform.translation = { position.x, position.y, position.z };
                ozzbone.transform.scale = { scale.x, scale.y, scale.z };
                ozzbone.transform.rotation = { rotation.w , rotation.x, rotation.y, rotation.z};

                ozzbone.name = string_view(node->mName.C_Str());

                for (int i = 0; i < node->mNumChildren; i++) {
                    //is this a relevant bone?
                    auto childnode = node->mChildren[i];
                    if (bones.contains(string_view(childnode->mName.C_Str()))) {

                        //create a new bone
                        auto& newbone = ozzbone.children.emplace_back();

                        //construct all the child bones for this bone
                        recursive_call(newbone, childnode, recursive_call);
                    }
                }
                };
            recurse_impl(ozzbone, node, recurse_impl);
            };

        recurse_bone(skeleton.root, rootBone);
        return skeleton;
    }

}
