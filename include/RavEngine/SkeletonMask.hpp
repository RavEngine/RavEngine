#pragma once
#include "Ref.hpp"
#include "Vector.hpp"
#include <ozz/base/maths/simd_math.h>

namespace RavEngine{

class SkeletonAsset;

struct SkeletonMask{
    friend class AnimatorComponent;
    
    SkeletonMask(Ref<SkeletonAsset> asset, float defaultValue = 1);
    SkeletonMask(uint16_t numBones, float defaultValue = 1);
    
    void SetMaskForJoint(uint16_t joint_idx, float value);
    
    float GetMaskForJoint(uint16_t joint_idx) const;
    
    uint16_t GetNumPackedJoints() const{
        return mask.size();
    }
    
private:
    Vector<ozz::math::SimdFloat4> mask;    
};

}
