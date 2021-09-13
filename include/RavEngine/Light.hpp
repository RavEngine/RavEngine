#pragma once
#include "Queryable.hpp"
#include "CameraComponent.hpp"
#include "Common3D.hpp"
#include "Component.hpp"
#include "BuiltinMaterials.hpp"
#include "Uniform.hpp"
#include "DebugDraw.hpp"
#include <atomic>

namespace RavEngine{
class MeshAsset;

struct Light : public Queryable<Light>, public Component {
	float Intensity = 1.0;
	ColorRGBA color{1,1,1,1};
	virtual void DebugDraw(RavEngine::DebugDraw&) const = 0;
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
	
	void DebugDraw(RavEngine::DebugDraw&) const override;
	
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
		bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_CULL_CW | BGFX_STATE_BLEND_ADD | BGFX_STATE_DEPTH_TEST_GREATER);
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
	
	void DebugDraw(RavEngine::DebugDraw&) const override;
	
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
		bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_CULL_CW | BGFX_STATE_BLEND_ADD | BGFX_STATE_DEPTH_TEST_GREATER);
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
	
	void DebugDraw(RavEngine::DebugDraw&) const override;
	
	/**
	 Structure
	 @code
	 [0:15] = transform matrix
	 [16] = color R
	 [17] = color G
	 [18] = color B
	 [19] = intensity
	 [20] = radius
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
		
		return closest_multiple_of(sizeof(float) * (3+1) + sizeof(float[16]), 16);
	}
	
	/**
	 Calculate the shader's matrix
	 @param mat input transformation matrix
	 @return matrix for shader
	 */
	inline matrix4 CalculateMatrix(const matrix4& mat) const{
		auto radius = CalculateRadius();
		//scale = radius
		return glm::scale(mat, vector3(radius,radius,radius));
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
    constexpr inline float CalculateRadius() const{
		return Intensity*Intensity;
	}
};

struct SpotLight : public ShadowLight, public QueryableDelta<QueryableDelta<Light,ShadowLight>,SpotLight>{
	using QueryableDelta<QueryableDelta<Light,ShadowLight>,SpotLight>::GetQueryTypes;
	
	inline bool IsInFrustum(Ref<CameraComponent> cam) const{
		//TODO: perform AABB intersection on camera bounds
		return true;
	}
	void DebugDraw(RavEngine::DebugDraw&) const override;
	
	/**
	Structure
	@code
	[0:15] = transform matrix
	[16] = color R
	[17] = color G
	[18] = color B
	[19] = penumbra
	@endcode
	*/
	void AddInstanceData(float* offset) const;
	
	/**
	 Calculate the Stride, or the number of bytes needed for each instance
	 */
	static inline constexpr size_t InstancingStride(){
		//point light needs:
		//mvp matrix (1 float[16])
		//light color (3 floats)
		//light penumbra (1 float)
		
		return closest_multiple_of(sizeof(float) * (3+1) + sizeof(float[16]), 16);
	}
	
	/**
	 Execute instanced draw call for this light type
	 */
	static void Draw(int view);
	
	static inline void SetState(){
		bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_DEPTH_TEST_GEQUAL | BGFX_STATE_CULL_CCW | BGFX_STATE_BLEND_ADD);
	}
	
	//light properties
	float radius = 1;
	float penumbra = 0;
	
	/**
	 Calculate the shader's matrix
	 @param mat input transformation matrix
	 @return matrix for shader
	 */
    inline matrix4 CalculateMatrix(const matrix4& mat) const{
		auto intensity = Intensity;
		auto r = radius;
		intensity = intensity * intensity;
		//scale = radius
		return glm::scale(mat, vector3(r,intensity,r));
	}
};

class LightManager{
public:
	LightManager() = delete;
	static void Init();
	static void Teardown();
	
	friend class PointLight;
	friend class DirectionalLight;
	friend class AmbientLight;
	friend class SpotLight;
	
private:
	static Ref<MeshAsset> pointLightMesh;
	static Ref<MeshAsset> spotLightMesh;
	
	class LightShader : public Material{
	protected:
		LightShader(const std::string& name) : Material(name){}
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
	
	struct SpotLightShader : public LightShader{
		SpotLightShader() : LightShader("spotlightvolume"){}
	};
	
	struct SpotLightShaderInstance : public MaterialInstance<SpotLightShader>{
		SpotLightShaderInstance(Ref<SpotLightShader> m ) : MaterialInstance(m){}
	};
	
	static Ref<PointLightShaderInstance> pointLightShader;
	static Ref<AmbientLightShaderInstance> ambientLightShader;
	static Ref<DirectionalLightShaderInstance> directionalLightShader;
	static Ref<SpotLightShaderInstance> spotLightShader;
	
	static bgfx::VertexBufferHandle screenSpaceQuadVert;
	static bgfx::IndexBufferHandle screenSpaceQuadInd;
};
}

