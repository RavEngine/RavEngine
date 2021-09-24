#pragma once
#include "Ref.hpp"
#include "mathtypes.hpp"
#include "Material.hpp"
#include "MeshAsset.hpp"
#include "MeshAssetSkinned.hpp"
#include "DataStructures.hpp"
#include <plf_colony.h>
#include "Light.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "mathtypes.hpp"
#include <ozz/base/containers/vector.h>

namespace RavEngine {

class MaterialInstanceBase;
struct DirectionalLight;

struct FrameData{
	//global matrices
	matrix4 viewmatrix, projmatrix;
	vector3 cameraWorldpos;
    
	// these need to be ordered to
	// ensure skinning data gets correct matrices
	template<typename T>
    struct entry{
        ozz::vector<T> items;
        entry(const entry<T>& other){
            items = other.items;
			skinningdata = other.skinningdata;
        }
        entry(){}
        SpinLock mtx, skinningMtx;
        
        inline void AddItem(const typename decltype(items)::value_type& item){
            mtx.lock();
            items.push_back(item);
            mtx.unlock();
        }
        
        //used by skinned mesh
        ozz::vector<ozz::vector<T>> skinningdata;
        
        inline void AddSkinningData(const typename decltype(skinningdata)::value_type& item){
            skinningMtx.lock();
            skinningdata.push_back(item);
            skinningMtx.unlock();
        }
    };
	
	//opaque pass data
	locked_hashmap<std::tuple<Ref<MeshAsset>, Ref<MaterialInstanceBase>>,entry<matrix4>,SpinLock> opaques;
    locked_hashmap<std::tuple<Ref<MeshAssetSkinned>, Ref<MaterialInstanceBase>,Ref<SkeletonAsset>>, entry<matrix4>,SpinLock> skinnedOpaques;
	
	template<typename T>
	struct StoredLight{
		T light;
		matrix4 transform;
		StoredLight(const T& l, const matrix4& mtx) : light(l), transform(mtx){}
		StoredLight(const StoredLight& other) : light(other.light), transform(other.transform){}
		
		constexpr inline void AddInstanceData(float* offset) const{
			auto ptr1 = glm::value_ptr(transform);
			
			//don't need to send the last value of each row, because it is always [0,0,0,1] and can be reconstructed in shader
			offset[0] = ptr1[0];
			offset[1] = ptr1[1];
			offset[2] = ptr1[2];
			
			offset[3] = ptr1[4];
			offset[4] = ptr1[5];
			offset[5] = ptr1[6];
			
			offset[6] = ptr1[8];
			offset[7] = ptr1[9];
			offset[8] = ptr1[10];
			
			offset[9] = ptr1[12];
			offset[10] = ptr1[13];
			offset[11] = ptr1[14];
			light.AddInstanceData(offset); //don't increment, the light will add offset
		}
	};
	
	struct PackedDL{
		DirectionalLight light;
		struct tinyvec3{
			float x, y, z;
		} rotation;
		
		inline void AddInstanceData(float* offset) const{
			offset[4] = rotation.x;
			offset[5] = rotation.y;
			offset[6] = rotation.z;
			light.AddInstanceData(offset);
		}
		
		PackedDL(const DirectionalLight& l, const tinyvec3& tv) : light(l), rotation(tv){}
	};
	
	//lighting data
    Colony<PackedDL> directionals;
    Colony<AmbientLight> ambients;
    Colony<StoredLight<PointLight>> points;
    Colony<StoredLight<SpotLight>> spots;
	
#ifdef _DEBUG
	unordered_deduplicating_vector<Ref<Component>> debugShapesToDraw;
#endif
	unordered_deduplicating_vector<Ref<Component>> guisToCalculate;
	
	inline void Clear(){
		opaques.clear();
		skinnedOpaques.clear();
		directionals.clear();
		ambients.clear();
		points.clear();
		spots.clear();
	}
    
    // call on app quit
    void Reset(){
        Clear();
#ifdef _DEBUG
        debugShapesToDraw.clear();
#endif
        guisToCalculate.clear();
    }
	
	//default constructor
	FrameData(){}
    
    double Time = 0;
	
};
}
