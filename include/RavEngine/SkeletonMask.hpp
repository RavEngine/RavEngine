#pragma once
#include "Ref.hpp"
#include "Vector.hpp"

namespace RavEngine{

struct SkeletonAsset;

struct SkeletonMask{
    SkeletonMask(Ref<SkeletonAsset> asset);
    SkeletonMask(uint16_t numBones);
    
    Vector<bool> mask;
    
};

}
