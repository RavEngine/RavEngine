#pragma once
#include "Queryable.hpp"
#include "Common3D.hpp"
#include "BuiltinMaterials.hpp"
#include "Uniform.hpp"
#include "IDebugRenderable.hpp"
#include <atomic>

namespace RavEngine{
class MeshAsset;
class CameraComponent;
struct Light : public Queryable<Light,IDebugRenderable>, public IDebugRenderable {
	float Intensity = 1.0;
	ColorRGBA color{1,1,1,1};
};

/**
 A light that casts shadows
 */
struct ShadowLight : public Light, public QueryableDelta<Light,ShadowLight>{
	using QueryableDelta<Light,ShadowLight>::GetQueryTypes;
private:
	bool doesCastShadow = false;
public:
	bool CastsShadows() const { return doesCastShadow; }
	void SetCastsShadows(bool casts) { doesCastShadow = casts; }
};

struct AmbientLight : public Light, public QueryableDelta<Light,AmbientLight>{
	using light_t = AmbientLight;
	using QueryableDelta<Light,AmbientLight>::GetQueryTypes;
	
	/**
	 Ambient lights are always in the frustum
	 */
	inline bool IsInFrustum(Ref<CameraComponent> cam) const{
		return true;
	}

	// does not cast shadows
	constexpr bool CastsShadows() const { return false;  }
	
	void DebugDraw(RavEngine::DebugDrawer&, const Transform&) const override;
	
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
		
		return sizeof(float) * 4;
	}

	static inline constexpr size_t ShadowDataSize() {
		// ambient light needs:
		// nothing (does not cast shadows)
		return 0;
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
	using light_t = DirectionalLight;
	using QueryableDelta<QueryableDelta<Light,ShadowLight>,DirectionalLight>::GetQueryTypes;
	
	/**
	 Directional lights are always in the frustum
	 */
	inline bool IsInFrustum(Ref<CameraComponent> cam) const{
		return true;
	}
	
	void DebugDraw(RavEngine::DebugDrawer&, const Transform&) const override;
	
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
		
		return sizeof(float) * (3+3+1);
	}

	static inline constexpr size_t ShadowDataSize() {
		// directional light needs:
		// the world-space direction (3 floats)
		return sizeof(float) * 3;
	}
	
	/**
	 Execute instanced draw call for this light type
	 */
	static void Draw(int view);
};

struct PointLight : public ShadowLight, public QueryableDelta<QueryableDelta<Light,ShadowLight>,PointLight>{
	using light_t = PointLight;
	using QueryableDelta<QueryableDelta<Light,ShadowLight>,PointLight>::GetQueryTypes;
	
	void DebugDraw(RavEngine::DebugDrawer&, const Transform&) const override;
	
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
		//mvp matrix (1 float[16], but we don't encode every value because the last row is always [0,0,0,1])
		//light color (3 floats)
		//light intensity (1 float)
		
		return sizeof(float) * (3+1) + sizeof(float[16-4]);
	}

	static inline constexpr size_t ShadowDataSize() {
		// Point light needs:
		// the world-space location (3 floats)
		return sizeof(float) * 3;
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
	using light_t = SpotLight;
	using QueryableDelta<QueryableDelta<Light,ShadowLight>,SpotLight>::GetQueryTypes;
	
	void DebugDraw(RavEngine::DebugDrawer&, const Transform&) const override;
	
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
		//mvp matrix (1 float[16]) (but 4 floats are not sent)
		//light color (3 floats)
		//light penumbra (1 float)
		//light radius (1 float)
		//light intensity (1 float) 
		
		return sizeof(float) * (3+1+1+1) + sizeof(float[16-4]);
	}

	static inline constexpr size_t ShadowDataSize() {
		// Spot light needs:
		// the world-space location (3 floats)
		return sizeof(float) * 3;
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
	
	friend struct PointLight;
	friend struct DirectionalLight;
	friend struct AmbientLight;
	friend struct SpotLight;
	
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

