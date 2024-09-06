
#include <cxxopts.hpp>
#include <fmt/format.h>
#include <filesystem>
#include <simdjson.h>
#include <iostream>
#include <fstream>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>
#include <assimp/mesh.h>
#include "Mesh.hpp"

using namespace std;
using namespace RavEngine;

#define FATAL(reason) {std::cerr << "rvemc error: " << reason << std::endl; std::exit(1);}
#define ASSERT(cond, str) {if (!cond) FATAL(str)}

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

MeshPart LoadMesh(const std::filesystem::path& path, std::optional<std::string_view> meshName, float scaleFactor) {
    const aiScene* scene = aiImportFile(path.string().c_str(), assimp_flags);

    if (!scene) {
        FATAL(fmt::format("Cannot load from filesystem: {}", aiGetErrorString()));
    }

    aiNode* meshNode = scene->mRootNode;
    uint32_t meshCount = scene->mNumMeshes;
    
    if (meshName.has_value()) {
        meshNode = scene->mRootNode->FindNode(aiString(std::string(meshName.value())));
        if (meshNode == nullptr) {
            FATAL(fmt::format("No mesh with name \"{}\" in scene {}", meshName.value(), path.string()));
        }
        meshCount = meshNode->mNumMeshes;
    }


    MeshPart mesh;
    uint32_t index_base = 0;
    for (int i = 0; i < meshCount; i++) {

        auto mp = AIMesh2MeshPart(meshName.has_value() ? scene->mMeshes[meshNode->mMeshes[i]] : scene->mMeshes[i], matrix4(scaleFactor));
        for (auto& index : mp.indices) {
            index += index_base;            // renumber indices
        }

        mesh.vertices.insert(mesh.vertices.end(), mp.vertices.begin(), mp.vertices.end());
        mesh.indices.insert(mesh.indices.end(), mp.indices.begin(), mp.indices.end());
        index_base += mp.vertices.size();
    }

    aiReleaseImport(scene);

    return mesh;
}

void SerializeMeshPart(const std::filesystem::path& outfile, const MeshPart& mesh) {
    std::filesystem::create_directories(outfile.parent_path());		// make all the folders necessary

    SerializedMeshDataHeader header{
        .numVertices = uint32_t(mesh.vertices.size()),
        .numIndicies = uint32_t(mesh.indices.size())
    };

    ofstream out(outfile, std::ios::binary);
    if (!out) {
        FATAL(fmt::format("Could not open {} for writing",outfile.string()));
    }

    // write header
    out.write(reinterpret_cast<const char*>(&header), sizeof(header));

    // write vertices
    for (const auto& vert : mesh.vertices) {
        out.write(reinterpret_cast<const char*>(&vert), sizeof(vert));
    }

    // write indices
    for (const auto& ind : mesh.indices) {
        out.write(reinterpret_cast<const char*>(&ind), sizeof(ind));
    }
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
    
    auto mesh = LoadMesh(infile, fmn_opt, scaleFactor);

    inputFile.replace_extension("");
    const auto outfileName = inputFile.filename().string() + ".rvem";

    SerializeMeshPart(outputDir / outfileName, mesh);
}


