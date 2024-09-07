#include "Skeleton.hpp"
#include <stdexcept>
#include <assimp/scene.h>
#include <fmt/format.h>

using namespace std;

namespace RavEngine {

    void ASSERT(bool cond, const std::string& msg) {
        if (!cond) {
            throw std::runtime_error(msg);
        }
    }

	UnorderedMap<std::string, uint16_t> FlattenSkeleton(aiNode* rootBone)
	{


		return UnorderedMap<std::string, uint16_t>();
	}

	NameToBoneResult NameToBone(const aiScene* scene)
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

}