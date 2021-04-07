#pragma once
#include <ozz/animation/runtime/skeleton.h>
#include <ozz/animation/offline/skeleton_builder.h>
#include <string>

namespace RavEngine {
class SkeletonAsset{
	ozz::unique_ptr<ozz::animation::Skeleton> skeleton;
	
public:
	SkeletonAsset(const std::string& path);
	
	class Manager{
		
	};
	
	/**
	 Get the internal skeleton. For internal use only.
	 */
	const decltype(skeleton)& GetSkeleton() const{
		return skeleton;
	}
};
}
