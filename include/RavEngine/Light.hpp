#pragma once
#include "Queryable.hpp"
#include "Common3D.hpp"
#include "BuiltinMaterials.hpp"
#include "IDebugRenderable.hpp"
#include <atomic>

namespace RavEngine{
class MeshAsset;
class CameraComponent;
class Light : public Queryable<Light,IDebugRenderable>, public IDebugRenderable {
    ColorRGBA color{1,1,1,1};
	float Intensity = 1.0;
    bool tickInvalidated = true;    // trigger the world to update its parallel datastructure
protected:
    constexpr inline void invalidate(){tickInvalidated = true;}
public:
    constexpr void SetColorRGBA(const decltype(color)& inColor) {
        invalidate();
        color = inColor;
    }
    constexpr const decltype(color)& GetColorRGBA() const {
        return color;
    }
    constexpr void SetIntensity(decltype(Intensity) inIntensity){
        invalidate();
        Intensity = inIntensity;
    }
    constexpr decltype(Intensity) GetIntensity() const{
        return Intensity;
    }
    constexpr bool isInvalidated() const {
        return tickInvalidated;
    }
    constexpr void clearInvalidate(){
        tickInvalidated = false;
    }
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
	void SetCastsShadows(bool casts) {
        invalidate();
        doesCastShadow = casts;
    }
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
#if 0
		bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_CULL_CW | BGFX_STATE_BLEND_ADD | BGFX_STATE_DEPTH_TEST_GREATER);
#endif
	}
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
#if 0
		bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_CULL_CW | BGFX_STATE_BLEND_ADD | BGFX_STATE_DEPTH_TEST_GREATER);
#endif
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
#if 0
		bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_DEPTH_TEST_GEQUAL | BGFX_STATE_CULL_CCW | BGFX_STATE_BLEND_ADD);
#endif
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
	
private:
	/**
	 Caclulate the radius of the light using its current intensity
	 @return the radius
	 */
    constexpr inline float CalculateRadius() const{
        auto intensity = GetIntensity();
		return intensity*intensity;
	}
};

class SpotLight : public ShadowLight, public QueryableDelta<QueryableDelta<Light,ShadowLight>,SpotLight>{
    //light properties
    float coneAngle = 45.0;    // in degrees
    float penumbraAngle = 10;
public:
	using light_t = SpotLight;
	using QueryableDelta<QueryableDelta<Light,ShadowLight>,SpotLight>::GetQueryTypes;
	
	void DebugDraw(RavEngine::DebugDrawer&, const Transform&) const override;
    
    constexpr void SetConeAngle(decltype(coneAngle) inAngle){
        invalidate();
        coneAngle = inAngle;
    }
    constexpr decltype(coneAngle) GetConeAngle() const {return coneAngle;}
    constexpr void SetPenumbraAngle(decltype(penumbraAngle) inAngle){
        invalidate();
        penumbraAngle = inAngle;
    }
    constexpr decltype(penumbraAngle) GetPenumbraAngle() const {return penumbraAngle;}
    
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
		//light coneAngle (1 float)
		//light intensity (1 float)
		
		return sizeof(float) * (3+1+1+1) + sizeof(float[16-4]);
	}

	static inline constexpr size_t ShadowDataSize() {
		// Spot light needs:
		// the world-space location (3 floats)
		return sizeof(float) * 3;
	}
	
	static inline void SetState(){
#if 0
		bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_DEPTH_TEST_GEQUAL | BGFX_STATE_CULL_CCW | BGFX_STATE_BLEND_ADD);
#endif
	}
	
	/**
	 Calculate the shader's matrix
	 @param mat input transformation matrix
	 @return matrix for shader
	 */
    inline const matrix4& CalculateMatrix(const matrix4& mat) const{
		// no transformations occur, the cone is extended in the vertex shader
		return mat;
	}
};

}

