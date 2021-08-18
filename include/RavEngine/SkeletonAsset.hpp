#pragma once
#include <ozz/animation/runtime/skeleton.h>
#include <ozz/animation/offline/skeleton_builder.h>
#include <string>
#include <bgfx/bgfx.h>
#include "mathtypes.hpp"
#include <vector>

namespace RavEngine {
class SkeletonAsset{
	ozz::unique_ptr<ozz::animation::Skeleton> skeleton;
	bgfx::VertexBufferHandle bindpose = BGFX_INVALID_HANDLE;
	bgfx::VertexBufferHandle boneHierarchy = BGFX_INVALID_HANDLE;
	
	std::vector<glm::mat4> bindposes;
public:
	SkeletonAsset(const std::string& path);
	
	~SkeletonAsset(){
		bgfx::destroy(bindpose);
		bgfx::destroy(boneHierarchy);
	}
	
	/**
	 Get the internal skeleton. For internal use only.
	 */
	const decltype(skeleton)& GetSkeleton() const{
		return skeleton;
	}
	
	/**
	 @return bindposes for use in a shader
	 */
	inline const decltype(bindpose) GetBindposeHandle() const{
		return bindpose;
	}
	
	/**
	 @return bindposes for use in software
	 */
	inline const decltype(bindposes) GetBindposes() const{
		return bindposes;
	}
	
	/**
	 @return the bone hierarchy information. This is a linear buffer where each bone is represented by one entry in the buffer.
	 The value at each index is the index of the bone's parent. If the value is -1, then the bone has no parent
	 */
	inline const decltype(boneHierarchy) GetBoneHierarchy() const{
		return boneHierarchy;
	}
};
}
