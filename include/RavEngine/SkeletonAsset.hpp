#pragma once
#include <ozz/animation/runtime/skeleton.h>
#include <ozz/animation/offline/skeleton_builder.h>
#include <string>
#include "Vector.hpp"
#include "mathtypes.hpp"
#if !RVE_SERVER
#include <RGL/Types.hpp>
#endif


namespace RavEngine {
class SkeletonAsset{
	ozz::unique_ptr<ozz::animation::Skeleton> skeleton;
#if !RVE_SERVER
	RGLBufferPtr bindpose, boneHierarchy;
#endif

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
#if !RVE_SERVER
	/**
	 @return bindposes for use in a shader
	 */
    inline const auto GetBindposeHandle() const{
		return bindpose;
	}
#endif
	
	/**
	 @return bindposes for use in software
	 */
    inline const auto GetBindposes() const{
		return bindposes;
	}
	
#if !RVE_SERVER
	/**
	 @return the bone hierarchy information. This is a linear buffer where each bone is represented by one entry in the buffer.
	 The value at each index is the index of the bone's parent. If the value is -1, then the bone has no parent
	 */
    inline const auto GetBoneHierarchy() const{
		return boneHierarchy;
	}
#endif
	
	/**
	 @param boneName name of the bone to find
	 @return True if the skeleton has a bone by the name, false if not
	 */
	bool HasBone(const std::string_view boneName) const;
    
    std::optional<uint16_t> IndexForBone(const std::string_view boneName) const;
};
}
