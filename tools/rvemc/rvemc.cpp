
#include <cxxopts.hpp>
#include <fmt/format.h>
#include <filesystem>
#include <simdjson.h>
#include <iostream>
#include <fstream>
#include "Skeleton.hpp"
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>
#include <assimp/mesh.h>
#include "Mesh.hpp"
#include <variant>
#include "CaseAnalysis.hpp"
#include <RavEngine/ImportLib.hpp>
#include <meshoptimizer.h>
#include <numeric>
#include <glm/gtc/type_ptr.hpp>

using namespace std;
using namespace RavEngine;

#define FATAL(reason) {std::cerr << "rvemc error: " << reason << std::endl; std::exit(1);}
#define ASSERT(cond, str) {if (!(cond)) FATAL(str)}

static constexpr auto assimp_flags = aiProcess_CalcTangentSpace |
aiProcess_GenSmoothNormals |
aiProcess_FlipUVs |
aiProcess_JoinIdenticalVertices |
aiProcess_ImproveCacheLocality |
aiProcess_LimitBoneWeights |
aiProcess_RemoveRedundantMaterials |
aiProcess_SplitLargeMeshes |
aiProcess_Triangulate |
aiProcess_GenUVCoords |
aiProcess_SortByPType |
//aiProcess_FindDegenerates               |
aiProcess_FindInstances |
aiProcess_ValidateDataStructure |
aiProcess_OptimizeMeshes |
aiProcess_FindInvalidData;

#if defined __APPLE__ || __STDC_VERSION__ >= 199901L    //check for C99
#define stackarray(name, type, size) type name[size]    //prefer C VLA on supported systems
#else
#define stackarray(name, type, size) type* name = (type*)alloca(sizeof(type) * size) //warning: alloca may not be supported in the future
#endif

struct SkinnedMeshPart : public MeshPart {
    std::vector<VertexWeights> vertexWeights;
};

template<bool isSkinned>
std::variant<MeshPart, SkinnedMeshPart> LoadMesh(const std::filesystem::path& path, std::optional<std::string_view> meshName, float scaleFactor, bool bakeHierarchy) {
    const aiScene* scene = aiImportFile(path.string().c_str(), assimp_flags);


    if (!scene) {
        FATAL(fmt::format("Cannot load from filesystem: {}", aiGetErrorString()));
    }

    aiNode* meshNode = scene->mRootNode;
    uint32_t meshCount = scene->mNumMeshes;

    using Mesh_t = typename std::conditional<isSkinned, SkinnedMeshPart, MeshPart>::type;
    
    if (meshName.has_value()) {
        meshNode = scene->mRootNode->FindNode(aiString(std::string(meshName.value())));
        if (meshNode == nullptr) {
            FATAL(fmt::format("No mesh with name \"{}\" in scene {}", meshName.value(), path.string()));
        }
        meshCount = meshNode->mNumMeshes;
    }

    auto getWorldMatrix = [](const aiNode* node) -> matrix4 {
        // figure out size
        uint16_t depth = 0;
        for (aiNode* p = node->mParent; p != nullptr; p = p->mParent) {
            ++depth;
        }
        stackarray(transforms, aiMatrix4x4, depth);

        uint16_t tmp = 0;
        for (aiNode* p = node->mParent; p != nullptr; p = p->mParent) {
            transforms[tmp] = p->mTransformation;
            ++tmp;
        }

        aiMatrix4x4 mat;	//identity
        if (depth > 0) {
            for (int i = depth - 1; i >= 0; --i) {
                mat *= transforms[i];
            }
        }
        mat *= node->mTransformation;

        // row major -> column major
        mat = mat.Transpose();
        matrix4 ret;
        std::memcpy(glm::value_ptr(ret), mat[0], sizeof(ret));
        return ret;
    };

    Mesh_t mesh;
    uint32_t index_base = 0;
    for (int i = 0; i < meshCount; i++) {

        try{
            matrix4 worldTransform{ 1 };
            if (bakeHierarchy) {
                worldTransform = getWorldMatrix(meshNode);
            }
            worldTransform *= matrix4(scaleFactor);

            auto mp = AIMesh2MeshPart(meshName.has_value() ? scene->mMeshes[meshNode->mMeshes[i]] : scene->mMeshes[i], worldTransform);
            for (auto& index : mp.indices) {
                index += index_base;            // renumber indices
            }

            mesh.positions.insert(mesh.positions.end(), mp.positions.begin(), mp.positions.end());
            mesh.normals.insert(mesh.normals.end(), mp.normals.begin(), mp.normals.end());
            mesh.tangents.insert(mesh.tangents.end(), mp.positions.begin(), mp.positions.end());
            mesh.bitangents.insert(mesh.bitangents.end(), mp.bitangents.begin(), mp.bitangents.end());
            mesh.uv0.insert(mesh.uv0.end(), mp.uv0.begin(), mp.uv0.end());
            mesh.lightmapUVs.insert(mesh.lightmapUVs.end(), mp.lightmapUVs.begin(), mp.lightmapUVs.end());
            mesh.indices.insert(mesh.indices.end(), mp.indices.begin(), mp.indices.end());
            index_base += mp.NumVerts();
        }
        catch(const std::runtime_error& err){
            FATAL(err.what());
        }
    }
    
    // optimize mesh

    std::vector<uint32_t> remap(mesh.indices.size()); // allocate temporary memory for the remap table
    std::iota(std::begin(remap), std::end(remap), 0);
#if 0
    size_t vertex_count = meshopt_generateVertexRemap(remap.data(), mesh.indices.data(), mesh.indices.size(), mesh.positions.data(), mesh.NumVerts(), sizeof(VertexPosition_t));

    meshopt_remapIndexBuffer(mesh.indices.data(),mesh.indices.data(),mesh.indices.size(),remap.data());

    meshopt_remapVertexBuffer(mesh.positions.data(),mesh.positions.data(), mesh.NumVerts(), sizeof(VertexPosition_t), remap.data());
    
    meshopt_optimizeVertexCache(mesh.indices.data(), mesh.indices.data(), mesh.indices.size(), mesh.NumVerts());
    meshopt_optimizeOverdraw(mesh.indices.data(), mesh.indices.data(), mesh.indices.size(), &mesh.positions[0].x, mesh.NumVerts(), sizeof(VertexPosition_t), 1.05f);
    
    auto indcpy = mesh.indices;
    meshopt_optimizeVertexFetchRemap(remap.data(), mesh.indices.data(), mesh.indices.size(), mesh.NumVerts());

    meshopt_remapIndexBuffer(mesh.indices.data(), indcpy.data(), mesh.indices.size(), remap.data());

    meshopt_remapVertexBuffer(mesh.positions.data(), mesh.positions.data(), mesh.NumVerts(), sizeof(VertexPosition_t), remap.data());
    meshopt_remapVertexBuffer(mesh.normals.data(), mesh.normals.data(), mesh.NumVerts(), sizeof(VertexNormal_t), remap.data());
    meshopt_remapVertexBuffer(mesh.tangents.data(), mesh.tangents.data(), mesh.NumVerts(), sizeof(VertexTangent_t), remap.data());
    meshopt_remapVertexBuffer(mesh.bitangents.data(), mesh.bitangents.data(), mesh.NumVerts(), sizeof(VertexBitangent_t), remap.data());
    meshopt_remapVertexBuffer(mesh.uv0.data(), mesh.uv0.data(), mesh.NumVerts(), sizeof(VertexUV_t), remap.data());
    if (mesh.lightmapUVs.size() > 0){
        meshopt_remapVertexBuffer(mesh.lightmapUVs.data(), mesh.lightmapUVs.data(), mesh.NumVerts(), sizeof(VertexUV_t), remap.data());
    }
#endif

    decltype(SkinnedMeshPart::vertexWeights) weightsgpu;
    if constexpr (isSkinned) {
        // load skin data

        RavEngine::Vector<VertexWeights::vweights> allweights;
        {
            uint32_t numverts = 0;
            for (int i = 0; i < scene->mNumMeshes; i++) {
                numverts += scene->mMeshes[i]->mNumVertices;
            }

            allweights.resize(numverts);
        }

        uint16_t current_offset = 0;

        auto allbones = NameToBone(scene);
        auto sk = CreateSkeleton(allbones);
        auto serialized = FlattenSkeleton(sk);

        auto calcMesh = [&](const aiMesh* mesh) {
            for (int i = 0; i < mesh->mNumBones; i++) {
                auto bone = mesh->mBones[i];
                VertexWeights::vweights weights;
                //find this bone in the skeleton to determine joint index
                auto idx = serialized.IndexForBoneName({ bone->mName.C_Str(), bone->mName.length});

                //copy (index + current_offset) and influence
                for (int j = 0; j < bone->mNumWeights; j++) {
                    auto weightval = bone->mWeights[j];

                    allweights[remap[weightval.mVertexId] + current_offset].weights.push_back({ VertexWeights::vweights::vw{idx,weightval.mWeight} });
                    //allweights[weightval.mVertexId + current_offset].weights.push_back({ VertexWeights::vweights::vw{idx,weightval.mWeight} });
                }

            }
            };

        //go through mesh and pull out weights
        for (int i = 0; i < scene->mNumMeshes; i++) {
            auto mesh = scene->mMeshes[i];

            calcMesh(mesh);
            current_offset += mesh->mNumVertices;
        }

        //make gpu version
        weightsgpu.reserve(allweights.size());
        std::memset(weightsgpu.data(), 0, weightsgpu.size() * sizeof(weightsgpu[0]));

        for (const auto& weights : allweights) {
            VertexWeights w;
            uint8_t i = 0;
            for (const auto& weight : weights.weights) {
                w.w[i].influence = weight.influence;
                w.w[i].joint_idx = weight.joint_idx;

                i++;
            }
            weightsgpu.push_back(w);
        }
        mesh.vertexWeights = std::move(weightsgpu);
    }
    aiReleaseImport(scene);

    return mesh;
}

void SerializeMeshPart(const std::filesystem::path& outfile, const std::variant<MeshPart,SkinnedMeshPart>& mesh) {
    std::filesystem::create_directories(outfile.parent_path());		// make all the folders necessary

    bool isSkinned = false;
    std::visit(CaseAnalysis{
        [&isSkinned](const MeshPart& mesh) {
            isSkinned = false;
        }, 
        [&isSkinned](const SkinnedMeshPart& mesh) {
            isSkinned = true;
        }}, mesh);

    // common code for skinned and non-skinned meshes
    ofstream out(outfile, std::ios::binary);
    if (!out) {
        FATAL(fmt::format("Could not open {} for writing", outfile.string()));
    }

    std::visit([&out,&isSkinned](const MeshPart& mesh) {
        
        SerializedMeshDataHeader header{
           .numVertices = uint32_t(mesh.positions.size()),  // these are all the same
           .numIndicies = uint32_t(mesh.indices.size()),
           .attributes = uint8_t(isSkinned ? SerializedMeshDataHeader::SkinnedMeshBit : 0)
        };
        header.attributes |= SerializedMeshDataHeader::hasPositionsBit;
        header.attributes |= SerializedMeshDataHeader::hasNormalsBit;
        header.attributes |= SerializedMeshDataHeader::hasTangentsBit;
        header.attributes |= SerializedMeshDataHeader::hasBitangentsBit;
        header.attributes |= SerializedMeshDataHeader::hasUV0Bit;
        if (mesh.lightmapUVs.size() > 0) {
            header.attributes |= SerializedMeshDataHeader::hasLightmapUVBit;
        }

        // write header
        out.write(reinterpret_cast<const char*>(&header), sizeof(header));

        // vertices and indices
        out.write(reinterpret_cast<const char*>(mesh.positions.data()), mesh.positions.size() * sizeof(mesh.positions[0]));
        out.write(reinterpret_cast<const char*>(mesh.normals.data()), mesh.normals.size() * sizeof(mesh.normals[0]));
        out.write(reinterpret_cast<const char*>(mesh.tangents.data()), mesh.tangents.size() * sizeof(mesh.tangents[0]));
        out.write(reinterpret_cast<const char*>(mesh.bitangents.data()), mesh.bitangents.size() * sizeof(mesh.bitangents[0]));
        out.write(reinterpret_cast<const char*>(mesh.uv0.data()), mesh.uv0.size() * sizeof(mesh.uv0[0]));

        if (mesh.lightmapUVs.size() > 0) {
            out.write(reinterpret_cast<const char*>(mesh.lightmapUVs.data()), mesh.lightmapUVs.size() * sizeof(mesh.uv0[0]));
        }

        out.write(reinterpret_cast<const char*>(mesh.indices.data()), mesh.indices.size() * sizeof(mesh.indices[0]));


    }, mesh);
   
    // executed only for skinned meshes
    std::visit(CaseAnalysis{
        [](const MeshPart& mesh) {},        // no-op
        [&out](const SkinnedMeshPart& mesh) {
            out.write(reinterpret_cast<const char*>(mesh.vertexWeights.data()), mesh.vertexWeights.size() * sizeof(decltype(mesh.vertexWeights)::value_type));
        }}, mesh);
}

int main(int argc, char** argv){
    cxxopts::Options options("rvemc", "RavEngine Mesh Compiler");
    options.add_options()
        ("f,file", "Input file path", cxxopts::value<std::filesystem::path>())
        ("o,output", "Ouptut file path", cxxopts::value<std::filesystem::path>())
        ("h,help", "Show help menu")
        ;
    
    auto args = options.parse(argc, argv);

    if (args["help"].as<bool>()) {
        cout << options.help() << endl;
        return 0;
    }

    std::filesystem::path inputFile;
    try {
        inputFile = args["file"].as<decltype(inputFile)>();
    }
    catch (exception& e) {
        FATAL("no input file")
    }
    std::filesystem::path outputDir;
    try {
        outputDir = args["output"].as<decltype(outputDir)>();
    }
    catch (exception& e) {
        FATAL("no output file")
    }
    
    simdjson::ondemand::parser parser;

    auto json = simdjson::padded_string::load(inputFile.string());
    simdjson::ondemand::document doc = parser.iterate(json);

    const auto json_dir = inputFile.parent_path();

    auto infile = json_dir / std::string_view(doc["file"]);

    double scaleFactor;
    auto err = doc["scale"].get(scaleFactor);
    if (err) {
        scaleFactor = 1;
    }

    std::string_view filteredMeshName;
    std::optional<string_view> fmn_opt;
    err = doc["mesh"].get(filteredMeshName);
    if (!err) {
        fmn_opt.emplace(filteredMeshName);
    }

    bool isSkinned = false;
    std::string_view typeStr;
    err = doc["type"].get(typeStr);
    if (!err) {
        isSkinned = (typeStr == "skinned");
    }

    bool bakeHierarchy = false;
    err = doc["bake_transform"].get(bakeHierarchy);
    if (err) {
        bakeHierarchy = false;
    }
    
    auto mesh = isSkinned ? LoadMesh<true>(infile, fmn_opt, scaleFactor, bakeHierarchy) : LoadMesh<false>(infile, fmn_opt, scaleFactor, bakeHierarchy);

    inputFile.replace_extension("");
    const auto outfileName = inputFile.filename().string() + ".rvem";

    SerializeMeshPart(outputDir / outfileName, mesh);

    return 0;
}


