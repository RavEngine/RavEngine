
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


struct SkinnedMeshPart : public MeshPart {
    std::vector<VertexWeights> vertexWeights;
};

template<bool isSkinned>
std::variant<MeshPart, SkinnedMeshPart> LoadMesh(const std::filesystem::path& path, std::optional<std::string_view> meshName, float scaleFactor) {
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


    Mesh_t mesh;
    uint32_t index_base = 0;
    for (int i = 0; i < meshCount; i++) {

        try{
            auto mp = AIMesh2MeshPart(meshName.has_value() ? scene->mMeshes[meshNode->mMeshes[i]] : scene->mMeshes[i], matrix4(scaleFactor));
            for (auto& index : mp.indices) {
                index += index_base;            // renumber indices
            }

            mesh.vertices.insert(mesh.vertices.end(), mp.vertices.begin(), mp.vertices.end());
            mesh.indices.insert(mesh.indices.end(), mp.indices.begin(), mp.indices.end());
            index_base += mp.vertices.size();
        }
        catch(const std::runtime_error& err){
            FATAL(err.what());
        }
    }
    
    // optimize mesh

    std::vector<uint32_t> remap(mesh.indices.size()); // allocate temporary memory for the remap table
    size_t vertex_count = meshopt_generateVertexRemap(remap.data(), mesh.indices.data(), mesh.indices.size(), mesh.vertices.data(), mesh.vertices.size(), sizeof(vertex_t));
    
    meshopt_remapIndexBuffer(mesh.indices.data(),mesh.indices.data(),mesh.indices.size(),remap.data());
    meshopt_remapVertexBuffer(mesh.vertices.data(),mesh.vertices.data(), mesh.vertices.size(), sizeof(vertex_t), remap.data());
    
    meshopt_optimizeVertexCache(mesh.indices.data(), mesh.indices.data(), mesh.indices.size(), mesh.vertices.size());
    meshopt_optimizeOverdraw(mesh.indices.data(), mesh.indices.data(), mesh.indices.size(), &mesh.vertices[0].position.x, mesh.vertices.size(), sizeof(vertex_t), 1.05f);
    
    auto indcpy = mesh.indices;
    meshopt_optimizeVertexFetchRemap(remap.data(), mesh.indices.data(), mesh.indices.size(), mesh.vertices.size());

    meshopt_remapIndexBuffer(mesh.indices.data(), indcpy.data(), mesh.indices.size(), remap.data());
    meshopt_remapVertexBuffer(mesh.vertices.data(), mesh.vertices.data(), mesh.vertices.size(), sizeof(vertex_t), remap.data());


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
           .numVertices = uint32_t(mesh.vertices.size()),
           .numIndicies = uint32_t(mesh.indices.size()),
           .attributes = uint8_t(isSkinned ? SerializedMeshDataHeader::SkinnedMeshBit : 0)
        };

        // write header
        out.write(reinterpret_cast<const char*>(&header), sizeof(header));

        // vertices and indices
        out.write(reinterpret_cast<const char*>(mesh.vertices.data()), mesh.vertices.size() * sizeof(mesh.vertices[0]));
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
    
    auto mesh = isSkinned ? LoadMesh<true>(infile, fmn_opt, scaleFactor) : LoadMesh<false>(infile, fmn_opt, scaleFactor);

    inputFile.replace_extension("");
    const auto outfileName = inputFile.filename().string() + ".rvem";

    SerializeMeshPart(outputDir / outfileName, mesh);

    return 0;
}


