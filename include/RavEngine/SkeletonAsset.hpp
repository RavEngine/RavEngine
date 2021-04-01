#pragma once
#include <ozz/animation/runtime/skeleton.h>
#include <string>

namespace RavEngine {
class SkeletonAsset{
	ozz::animation::Skeleton skeleton;
	
public:
	SkeletonAsset(const std::string& path);
	
	class Manager{
		
	};
};
}
