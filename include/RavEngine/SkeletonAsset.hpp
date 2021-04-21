#pragma once
#include <ozz/animation/runtime/skeleton.h>
#include <ozz/animation/offline/skeleton_builder.h>
#include <string>
#include <bgfx/bgfx.h>

namespace RavEngine {
class SkeletonAsset{
	ozz::unique_ptr<ozz::animation::Skeleton> skeleton;
	bgfx::VertexBufferHandle bindpose = BGFX_INVALID_HANDLE;

public:
	SkeletonAsset(const std::string& path);
	
	~SkeletonAsset(){
		bgfx::destroy(bindpose);
	}
	
	/**
	 Get the internal skeleton. For internal use only.
	 */
	const decltype(skeleton)& GetSkeleton() const{
		return skeleton;
	}
	
	inline const decltype(bindpose) getBindpose() const{
		return bindpose;
	}
};
}
