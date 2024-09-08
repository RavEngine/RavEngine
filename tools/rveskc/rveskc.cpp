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

using namespace std;
using namespace RavEngine;

#define FATAL(reason) {std::cerr << "rveskc error: " << reason << std::endl; std::exit(1);}
#define ASSERT(cond, str) {if (!(cond)) FATAL(str)}

SkeletonData LoadSkeleton(const std::filesystem::path& infile) {
    const aiScene* scene = aiImportFile(infile.string().c_str(),
        aiProcess_ImproveCacheLocality |
        aiProcess_ValidateDataStructure |
        aiProcess_FindInvalidData);

    if (!scene) {
        FATAL(fmt::format("Cannot load: {}", aiGetErrorString()));
    }

    auto unpackedSkeleton = NameToBone(scene);
    auto& rootBone = unpackedSkeleton.rootBone;
    auto& bones = unpackedSkeleton.bones;

    ASSERT(rootBone != nullptr, "Could not find root bone");

    auto skeleton = CreateSkeleton(unpackedSkeleton);

    //free afterward
    aiReleaseImport(scene);

    return skeleton;
}

void SerializeSkeleton(const std::filesystem::path& outfile, const SkeletonData& skeleton) {
    std::filesystem::create_directories(outfile.parent_path());		// make all the folders necessary

    auto serialized = FlattenSkeleton(skeleton);

    SerializedSkeletonDataHeader header{
        .numBones = uint32_t(serialized.allBones.size())
    };

    ofstream out(outfile, std::ios::binary);
    if (!out) {
        FATAL(fmt::format("Could not open {} for writing", outfile.string()));
    }

    // write header
    out.write(reinterpret_cast<const char*>(&header), sizeof(header));

    // write bone transform table
    for (const auto& bone : serialized.allBones) {
        out.write(reinterpret_cast<const char*>(& bone.transform), sizeof(bone.transform));
    }
    // write the bone name table
    for (const auto& bone : serialized.allBones) {
        // bone name
        uint16_t len = bone.name.length();
        out.write(reinterpret_cast<const char*>(&len), sizeof(len));
        out.write(bone.name.data(), bone.name.size());
    }
    // write the bone children table
    for (const auto& childrenList : serialized.childrenMap) {
        uint16_t numChildren = childrenList.size();
        out.write(reinterpret_cast<const char*>(&numChildren), sizeof(numChildren));
        out.write(reinterpret_cast<const char*>(childrenList.data()), childrenList.size() * sizeof(childrenList[0]));
    }
}

int main(int argc, char** argv) {
    cxxopts::Options options("rveskc", "RavEngine Skeleton Compiler");
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

    auto skeleton = LoadSkeleton(infile);

    inputFile.replace_extension("");
    const auto outfileName = inputFile.filename().string() + ".rves";

    SerializeSkeleton(outputDir / outfileName, skeleton);

    return 0;
}
