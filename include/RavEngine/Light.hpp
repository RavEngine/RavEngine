#pragma once
#include "Queryable.hpp"
#include "CameraComponent.hpp"
#include "Common3D.hpp"
#include "Component.hpp"
#include "BuiltinMaterials.hpp"
#include "Uniform.hpp"

namespace RavEngine{
class MeshAsset;

struct Light : public Queryable<Light>, public Component {
	float Intensity = 1.0;
	ColorRGBA color{1,1,1,1};
	virtual void DebugDraw() const = 0;
};

/**
 A light that casts shadows
 */
struct ShadowLight : public Light, public QueryableDelta<Light,ShadowLight>{
	using QueryableDelta<Light,ShadowLight>::GetQueryTypes;
	public:
		bool CastsShadows = true;
};

struct AmbientLight : public Light, public QueryableDelta<Light,AmbientLight>{
	using QueryableDelta<Light,AmbientLight>::GetQueryTypes;
	
	/**
	 Ambient lights are always in the frustum
	 */
	inline bool IsInFrustum(Ref<CameraComponent> cam) const{
		return true;
	}
	
	void DebugDraw() const override;
	
	/**
	 Structure:
	 @code
 [0] = color red
 [1] = color green
 [2] = color blue
 [3] = intensity
	 @endcode
	 */
	void AddInstanceData(float* offset) const;
	
	/**
	 Calculate the Stride, or the number of bytes needed for each instance
	 */
	static inline constexpr size_t InstancingStride(){
		//ambient light needs:
			//light color (3 floats)
			//light intensity (1 float)
		
		return closest_multiple_of(sizeof(float) * 4, 16);
	}
	
	/**
	 Set BGFX state needed to draw this light
	 */
	static inline void SetState(){
		bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_CULL_CW | BGFX_STATE_BLEND_ADD);
	}
	
	/**
	 Execute instanced draw call for this light type
	 */
	static void Draw(int view);
};

struct DirectionalLight : public ShadowLight, public QueryableDelta<QueryableDelta<Light,ShadowLight>,DirectionalLight>{
	using QueryableDelta<QueryableDelta<Light,ShadowLight>,DirectionalLight>::GetQueryTypes;
	
	/**
	 Directional lights are always in the frustum
	 */
	inline bool IsInFrustum(Ref<CameraComponent> cam) const{
		return true;
	}
	
	void DebugDraw() const override;
	
	/**
	 Structure
	 @code
 [0] = color red
 [1] = color green
 [2] = color blue
 [3] = intensity
 [4] = rotation x
 [5] = rotation y
 [6] = rotation z
	 @endcode
	 */
	void AddInstanceData(float* view) const;
	
	/**
	 Set BGFX state needed to draw this light
	 */
	static inline void SetState(){
		bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_CULL_CW | BGFX_STATE_BLEND_ADD);
	}
	
	/**
	 Calculate the Stride, or the number of bytes needed for each instance
	 */
	static inline constexpr size_t InstancingStride(){
		//directional light needs:
		//light color (3 floats)
		//light direction (3 floats)
		//light intensity (1 float)
		
		return closest_multiple_of(sizeof(float) * (3+3+1), 16);
	}
	
	/**
	 Execute instanced draw call for this light type
	 */
	static void Draw(int view);
};

struct PointLight : public ShadowLight, public QueryableDelta<QueryableDelta<Light,ShadowLight>,PointLight>{
	using QueryableDelta<QueryableDelta<Light,ShadowLight>,PointLight>::GetQueryTypes;
		
	inline bool IsInFrustum(Ref<CameraComponent> cam) const{
		//TODO: perform sphere intersection on camera bounds
		return true;
	}
	
	void DebugDraw() const override;
	
	/**
	 Structure
	 @code
	 [0:15] = transform matrix
	 [16] = color R
	 [17] = color G
	 [18] = color B
	 [19] = intensity
	 [20] = pos x
	 [21] = pos y
	 [22] = pos z
	 [23] = radius
	 @endcode
	 */
	void AddInstanceData(float* offset) const;
	
	static inline void SetState(){
		bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_DEPTH_TEST_GEQUAL | BGFX_STATE_CULL_CCW | BGFX_STATE_BLEND_ADD);
	}
	
	/**
	 Calculate the Stride, or the number of bytes needed for each instance
	 */
	static inline constexpr size_t InstancingStride(){
		//point light needs:
		//mvp matrix (1 float[16])
		//light color (3 floats)
		//light intensity (1 float)
		//light position (3 floats)
		//light radius (1 float)
		
		return closest_multiple_of(sizeof(float) * (3+3+1+1) + sizeof(float[16]), 16);
	}
	
	/**
	 Execute instanced draw call for this light type
	 */
	static void Draw(int view);
private:
	/**
	 Caclulate the radius of the light using its current intensity
	 @return the radius
	 */
	inline float CalculateRadius() const{
		return Intensity*Intensity;
	}
};


class LightManager{
public:
	LightManager() = delete;
	static void Init();
	
	friend class PointLight;
	friend class DirectionalLight;
	friend class AmbientLight;
	
private:
	static Ref<MeshAsset> pointLightMesh;
	
	class LightShader : public Material{
	protected:
		LightShader(const std::string& name) : Material(name){}
	public:
	};
	
	/**
	 Material to draw point lights
	 */
	struct PointLightShader : public LightShader{
		PointLightShader() : LightShader("pointlightvolume"){}
	};
	/**
	 Holds uniforms for point lights
	 */
	struct PointLightShaderInstance : public MaterialInstance<PointLightShader>{
		PointLightShaderInstance(Ref<PointLightShader> m ) : MaterialInstance(m){}
	};
	
	struct AmbientLightShader : public LightShader{
	public:
		AmbientLightShader() : LightShader("ambientlightvolume"){}
	};
	struct AmbientLightShaderInstance : public MaterialInstance<AmbientLightShader>{
		AmbientLightShaderInstance(Ref<AmbientLightShader> m ) : MaterialInstance(m){}
	};
	
	struct DirectionalLightShader : public LightShader{
		DirectionalLightShader() : LightShader("directionallightvolume"){}
	};
	
	struct DirectionalLightShaderInstance : public MaterialInstance<DirectionalLightShader>{
		DirectionalLightShaderInstance(Ref<DirectionalLightShader> m ) : MaterialInstance(m){}
	};
	
	static Ref<PointLightShaderInstance> pointLightShader;
	static Ref<AmbientLightShaderInstance> ambientLightShader;
	static Ref<DirectionalLightShaderInstance> directionalLightShader;
	
	static bgfx::VertexBufferHandle screenSpaceQuadVert;
	static bgfx::IndexBufferHandle screenSpaceQuadInd;
};
}

