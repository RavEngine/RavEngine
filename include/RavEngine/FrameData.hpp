#pragma once
#include "Ref.hpp"
#include "mathtypes.hpp"
#include "Material.hpp"
#include "MeshAsset.hpp"

namespace RavEngine {

class MaterialInstanceBase;

struct FrameData{
	//global matrices
	matrix4 viewmatrix, projmatrix;
	
	//opaque pass data
	phmap::flat_hash_map<std::pair<Ref<MeshAsset>, Ref<MaterialInstanceBase>>, plf::list<matrix4>> opaques;
	
	//TODO: write lighting data here
};

}
