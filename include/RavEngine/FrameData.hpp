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
    vector3 cameraFacingVector{1,1,1};
    
	template<typename T>
	struct StoredLight{
        using light_t = T;
		T light;
        matrix4 transform{1};
        StoredLight(){}
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

		bool CastsShadows() const {
			return light.CastsShadows();
		}
	};
	
	struct PackedDL{
		using light_t = DirectionalLight;
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

		bool CastsShadows() const{
			return light.CastsShadows();
		}
		
		PackedDL(const DirectionalLight& l, const tinyvec3& tv) : light(l), rotation(tv){}
        PackedDL(){}    // used by World, do not use
	};
	
    double Time = 0;
	
};
}
