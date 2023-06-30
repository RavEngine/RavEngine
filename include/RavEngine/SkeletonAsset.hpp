#pragma once
#include <ozz/animation/runtime/skeleton.h>
#include <ozz/animation/offline/skeleton_builder.h>
#include <string>
#include "DataStructures.hpp"
#include "mathtypes.hpp"
#include <RGL/Types.hpp>


namespace RavEngine {
class SkeletonAsset{
	ozz::unique_ptr<ozz::animation::Skeleton> skeleton;
	RGLBufferPtr bindpose, boneHierarchy;

    RavEngine::Vector<glm::mat4> bindposes;
public:
	SkeletonAsset(const std::string& path);
	~SkeletonAsset();
	
	
	/**
	 Get the internal skeleton. For internal use only.
	 */
	const auto& GetSkeleton() const{
		return skeleton;
	}
	
	/**
	 @return bindposes for use in a shader
	 */
    inline const auto GetBindposeHandle() const{
		return bindpose;
	}
	
	/**
	 @return bindposes for use in software
	 */
    inline const auto GetBindposes() const{
		return bindposes;
	}
	
	/**
	 @return the bone hierarchy information. This is a linear buffer where each bone is represented by one entry in the buffer.
	 The value at each index is the index of the bone's parent. If the value is -1, then the bone has no parent
	 */
    inline const auto GetBoneHierarchy() const{
		return boneHierarchy;
	}
	
	/**
	 @param boneName name of the bone to find
	 @return True if the skeleton has a bone by the name, false if not
	 */
	bool HasBone(const std::string& boneName) const;
};
}
