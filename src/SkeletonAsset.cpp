#include "SkeletonAsset.hpp"
#include <ozz/animation/runtime/skeleton.h>
#include <ozz/animation/offline/raw_skeleton.h>
#include "App.hpp"
#include "Debug.hpp"
#include <ozz/base/io/stream.h>
#include <ozz/base/io/archive.h>
#include "Filesystem.hpp"
#include <ozz/base/memory/unique_ptr.h>
#include <ozz/base/span.h>
#include <ozz/base/maths/soa_transform.h>
#include <ozz/animation/runtime/local_to_model_job.h>
#include "VirtualFileSystem.hpp"
#include <glm/gtc/type_ptr.hpp>
#include "Skeleton.hpp"
#if !RVE_SERVER
    #include "RenderEngine.hpp"
#endif

using namespace RavEngine;
using namespace std;

SkeletonData DeserializeSkeleton(std::span<uint8_t> binaryData) {
	uint8_t* fp = binaryData.data();

	SerializedSkeletonDataHeader header = *reinterpret_cast<decltype(header)*>(fp);
	fp += sizeof(header);

	// check header
	if (strncmp(header.header.data(), "rves", sizeof("rves") - 1) != 0) {
		Debug::Fatal("Header does not match, data is not a mesh!");
	}

	SerializedSkeleton deserialized;
	deserialized.allBones.reserve(header.numBones);
	deserialized.childrenMap.reserve(header.numBones);

	// Get bone transforms
	for (uint32_t i = 0; i < header.numBones; i++) {
		BoneTransform transform = *reinterpret_cast<decltype(transform)*>(fp);
		fp += sizeof(transform);
        deserialized.allBones.push_back({transform});
	}
	// get bone names
	for (uint32_t i = 0; i < header.numBones; i++) {
		uint16_t length = *reinterpret_cast<decltype(length)*>(fp);
		fp += sizeof(length);
		auto& name = deserialized.allBones[i].name;
		name.reserve(length);
		for (int i = 0; i < length; i++) {
			name += (char(*fp));
			fp++;
		}
	}
	// get bone children
	for (uint32_t i = 0; i < header.numBones; i++) {
		uint16_t numchildren = *reinterpret_cast<decltype(numchildren)*>(fp);
		fp += sizeof(numchildren);

		auto& vec = deserialized.childrenMap.emplace_back();
		vec.reserve(numchildren);
		for (int i = 0; i < numchildren; i++) {
			uint16_t childIdx = *reinterpret_cast<decltype(childIdx)*>(fp);
			fp += sizeof(childIdx);
			vec.push_back(childIdx);
		}
	}

	auto deserializeBone = [&deserialized](SkeletonData::Bone& bone, uint32_t index, auto&& fn) -> void {
		auto& boneData = deserialized.allBones[index];
		auto& childData = deserialized.childrenMap[index];

		bone.name = boneData.name;
		bone.transform = boneData.transform;

		bone.children.reserve(childData.size());
		for (const auto childIdx : childData) {
			auto& childbone = bone.children.emplace_back();
			fn(childbone, childIdx, fn);
		}
	};

	SkeletonData data;
	// we know the root bone is the first one
	deserializeBone(data.root,0, deserializeBone);

	return data;
}

SkeletonAsset::SkeletonAsset(const std::string& str){
	auto path = Format("skeletons/{}.rves",str);
	
	if(GetApp()->GetResources().Exists(path.c_str())){
		
		auto data = GetApp()->GetResources().FileContentsAt(path.c_str());

		auto skeletonData = DeserializeSkeleton(data);

		//recurse the root node and get all of the bones
		ozz::animation::offline::RawSkeleton raw_skeleton;
		raw_skeleton.roots.resize(1);
		auto& root = raw_skeleton.roots[0];

		auto convertBone = [](decltype(root) dest, const SkeletonData::Bone& source, auto&& fn) -> void {
			dest.name = source.name;
			dest.transform.translation = { source.transform.translation.x,source.transform.translation.y,source.transform.translation.z };
			dest.transform.scale = { source.transform.scale.x,source.transform.scale.y,source.transform.scale.z };
			dest.transform.rotation = { source.transform.rotation.x,source.transform.rotation.y,source.transform.rotation.z, source.transform.rotation.w };

			dest.children.reserve(source.children.size());
			for (const auto& child : source.children) {
				auto& newDest = dest.children.emplace_back();
				fn(newDest, child, fn);
			}
		};
		convertBone(root, skeletonData.root, convertBone);

		//convert into a runtime-optimized skeleton
		Debug::Assert(raw_skeleton.Validate(), "Skeleton validation failed");

		ozz::animation::offline::SkeletonBuilder skbuilder;
		ozz::unique_ptr<ozz::animation::Skeleton> sk = skbuilder(raw_skeleton);
		skeleton = std::move(sk);
	}
	else{
		Debug::Fatal("No skeleton at {}",path);
	}

	bindposes.resize(skeleton->joint_names().size());
	stackarray(bindpose_ozz, ozz::math::Float4x4, skeleton->joint_names().size());
	
	//convert from local space to model space
	ozz::animation::LocalToModelJob job;
	job.skeleton = skeleton.get();
	job.input = ozz::span(skeleton->joint_rest_poses());
	job.output = ozz::span(bindpose_ozz, skeleton->joint_names().size_bytes());
	
	Debug::Assert(job.Run(), "Bindpose extraction failed");
	
	//convert to format understandble by GPU
	float matrix[16];
	for(int i = 0; i < skeleton->joint_names().size(); i++){
		auto& t = bindpose_ozz[i];
		for(int r = 0; r < 4; r++){
			float result[4];
			std::memcpy(result,t.cols + r,sizeof(t.cols[r]));
			//_mm_store_ps(result,p.cols[r]);
			std::memcpy(matrix + r*4,result,sizeof(result));
		}
		//inverse here because shader needs the inverse bindpose
		bindposes[i] = glm::inverse(glm::make_mat4(matrix));
	}
	
	assert(bindposes.size() * sizeof(bindposes[0]) < numeric_limits<uint32_t>::max());
#if !RVE_SERVER
	bindpose = GetApp()->GetDevice()->CreateBuffer({
		uint32_t(bindposes.size()),
		{.StorageBuffer = true},
		sizeof(bindposes[0]),
		RGL::BufferAccess::Private
	});
	bindpose->SetBufferData({ bindposes.data(), bindposes.size() * sizeof(bindposes[0]) });
	
	// populate hierarchy
	auto parents = skeleton->joint_parents();
	

	boneHierarchy = GetApp()->GetDevice()->CreateBuffer({
		uint32_t(parents.size()),
		{.StorageBuffer = true},
		sizeof(parents[0]),
		RGL::BufferAccess::Private
	});
	boneHierarchy->SetBufferData({parents.data(), parents.size() * sizeof(parents[0])});
#endif
}

RavEngine::SkeletonAsset::~SkeletonAsset()
{
#if !RVE_SERVER
	auto& gcbuffers = GetApp()->GetRenderEngine().gcBuffers;
	gcbuffers.enqueue(bindpose);
	gcbuffers.enqueue(boneHierarchy);
#endif
}


bool SkeletonAsset::HasBone(const std::string_view boneName) const{
    return IndexForBone(boneName).has_value();
}

std::optional<uint16_t> SkeletonAsset::IndexForBone(const std::string_view boneName) const{
    for (int i = 0; i < skeleton->num_joints(); i++) {
        if (strncmp(skeleton->joint_names()[i], boneName.data(), boneName.size()) == 0) {
            return i;
        }
    }
    return std::nullopt;
};
