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
#include "Animation.hpp"

using namespace RavEngine;
using namespace std;

#define FATAL(reason) {std::cerr << "rveac error: " << reason << std::endl; std::exit(1);}
#define ASSERT(cond, str) {if (!(cond)) FATAL(str)}

JointAnimation LoadAnimation(const std::filesystem::path& path) {
	const aiScene* scene = aiImportFile(path.string().c_str(),
		aiProcess_ImproveCacheLocality |
		aiProcess_ValidateDataStructure |
		aiProcess_FindInvalidData);

	if (!scene) {
		FATAL(fmt::format("Cannot load: {}", aiGetErrorString()));
	}

	ASSERT(scene->HasAnimations(), "Scene must have animations");
	JointAnimation raw_animation;

	auto& animations = scene->mAnimations;

	//find the longest animation, this is the length of the raw anim

	//assume the first animation is the animation to use
	auto anim = scene->mAnimations[0];
	float tps = anim->mTicksPerSecond; 
	raw_animation.duration = anim->mDuration;   // ticks! keys from assimp are also in ticks
    raw_animation.ticksPerSecond = tps;
    
	float duration_seconds = anim->mDuration / tps;

	auto create_keyframe = [&](const aiNodeAnim* channel, JointAnimationTrack& track) {

		//translate
		for (int i = 0; i < channel->mNumPositionKeys; i++) {
			auto key = channel->mPositionKeys[i];
			track.translations.push_back({ {key.mValue.x,key.mValue.y,key.mValue.z}, static_cast<float>(key.mTime) });
		}

		//rotate
		for (int i = 0; i < channel->mNumRotationKeys; i++) {
			auto key = channel->mRotationKeys[i];
			track.rotations.push_back({{key.mValue.w, key.mValue.x,key.mValue.y,key.mValue.z}, static_cast<float>(key.mTime)});
		}

		//scale
		for (int i = 0; i < channel->mNumScalingKeys; i++) {
			auto key = channel->mScalingKeys[i];
			track.scales.push_back({{key.mValue.x,key.mValue.y,key.mValue.z}, static_cast<float>(key.mTime) });
		}
	};

	auto allbones = NameToBone(scene);
	auto sk = CreateSkeleton(allbones);
	auto serialized = FlattenSkeleton(sk);

	// populate the tracks
	raw_animation.tracks.resize(allbones.bones.size());
	raw_animation.name = string_view(anim->mName.C_Str());
	uint32_t num_loaded = 0;
	for (int i = 0; i < anim->mNumChannels; i++) {
		auto channel = anim->mChannels[i];
		auto& name = channel->mNodeName;
		auto bone_index = serialized.IndexForBoneName({ name.C_Str(), name.length});

		if (bone_index == serialized.allBones.size()) {
			continue;
		}
		num_loaded++;
	
		//populate the keyframes per track
		create_keyframe(channel, raw_animation.tracks[bone_index]);

	}

	ASSERT(num_loaded > 0, "No animations were loaded for this skeleton. This can be caused by naming differences if the animation is a different file type than the skeleton.");

	//free afterward
	aiReleaseImport(scene);

	return raw_animation;
}

void SerializeAnim(const std::filesystem::path& outfile, const JointAnimation& anim) {
	constexpr auto maxLen = std::numeric_limits<decltype(SerializedJointAnimationHeader::nameLength)>::max();
	ASSERT(anim.name.size() <= maxLen, "Animation's name is too long!");

	ofstream out(outfile, std::ios::binary);
	if (!out) {
		FATAL(fmt::format("Could not open {} for writing", outfile.string()));
	}

	SerializedJointAnimationHeader header{
		.duration = anim.duration,
        .ticksPerSecond = anim.ticksPerSecond,
		.numTracks = uint32_t(anim.tracks.size()),
		.nameLength = uint16_t(anim.name.size())
	};

	// write header
	out.write(reinterpret_cast<char*>(&header), sizeof(header));

	// write name
	out.write(anim.name.data(), anim.name.size());

	// write tracks
	for (const auto& track : anim.tracks) {
		SerializedJointAnimationTrackHeader header{
			.numTranslations = uint32_t(track.translations.size()),
			.numRotations = uint32_t(track.rotations.size()),
			.numScales = uint32_t(track.scales.size())
		};
		// write header for this track
		out.write(reinterpret_cast<char*>(&header), sizeof(header));

		auto writeBuffer = [&out](auto&& data) {
			out.write(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(data[0]));
		};
		
		writeBuffer(track.translations);
		writeBuffer(track.rotations);
		writeBuffer(track.scales);
	}
}

int main(int argc, char** argv) {

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

	auto anim = LoadAnimation(infile);

	inputFile.replace_extension("");
	const auto outfileName = inputFile.filename().string() + ".rvea";

	SerializeAnim(outputDir / outfileName, anim);

	return 0;
}
