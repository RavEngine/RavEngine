#pragma once
#include "Ref.hpp"
#include "mathtypes.hpp"
#include "Material.hpp"
#include "MeshAsset.hpp"
#include "DataStructures.hpp"
#include <plf_colony.h>

namespace RavEngine {

class MaterialInstanceBase;

struct FrameData{
	//global matrices
	matrix4 viewmatrix, projmatrix;
    
    struct entry{
        SpinLock mtx;
        plf::colony<matrix4> items;
        entry(const entry& other){
            items = other.items;
        }
        entry(){}
    };
	
	//opaque pass data
	locked_node_hashmap<std::pair<Ref<MeshAsset>, Ref<MaterialInstanceBase>>,entry,SpinLock> opaques;
	
	//TODO: write lighting data here
};

}
