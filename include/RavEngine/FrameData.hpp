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
#include "GUI.hpp"

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
        Vector<T> items;
        //SpinLock mtx;
        
        inline void AddItem(const typename decltype(items)::value_type& item){
            //mtx.lock();
            items.push_back(item);
            //mtx.unlock();
        }
        inline void clear(){
            items.clear();
        }
    };
    
    template<typename T>
    struct skinningEntry : public entry<T>{
        //SpinLock skinningMtx;
        
        //used by skinned mesh
        ozz::vector<ozz::vector<T>> skinningdata;
        
        inline void AddSkinningData(const typename decltype(skinningdata)::value_type& item){
            //skinningMtx.lock();
            skinningdata.push_back(item);
            //skinningMtx.unlock();
        }
        inline void clear(){
            entry<T>::clear();
            skinningdata.clear();
        }
    };
	
	//opaque pass data
	UnorderedMap<std::tuple<Ref<MeshAsset>, Ref<MaterialInstanceBase>>,entry<matrix4>/*,SpinLock*/> opaques;
    UnorderedMap<std::tuple<Ref<MeshAssetSkinned>, Ref<MaterialInstanceBase>,Ref<SkeletonAsset>>, skinningEntry<matrix4>/*,SpinLock*/> skinnedOpaques;
	
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
    Vector<PackedDL> directionals;
    Vector<AmbientLight> ambients;
    Vector<StoredLight<PointLight>> points;
    Vector<StoredLight<SpotLight>> spots;
    
    template<typename T, typename ... A>
    inline void AddLight(T& structure, A... args){
        structure.emplace_back(args...);
    }
    
    template<typename T>
    inline void ResetMap(T& map){
        for(auto& row : map){
            row.second.clear();
        }
    }
	
	inline void Clear(){
        ResetMap(opaques);
        ResetMap(skinnedOpaques);
		directionals.clear();
		ambients.clear();
		points.clear();
		spots.clear();
	}
    
    // call on app quit
    void Reset(){
        Clear();
    }
    double Time = 0;
	
};
}
