#pragma once
#include "Queryable.hpp"
#include "Common3D.hpp"
#include "IDebugRenderable.hpp"
#include "Layer.hpp"
#include "Types.hpp"
#if !RVE_SERVER
#include "DepthPyramid.hpp"
#endif

namespace RavEngine{
class MeshAsset;
class CameraComponent;
struct Skybox;
/**
Represents a light-emitting object. Lights can be constrained to specific objects with layers. 
By default, lights will illuminate objects on any layer. 
*/
class Light : public Queryable<Light,IDebugRenderable>, public IDebugRenderable {
    ColorRGBA color{1,1,1,1};
	float Intensity = 1.0;
    renderlayer_t illuminationLayers = ALL_LAYERS;
protected:
    bool tickInvalidated = true;    // trigger the world to update its parallel datastructure
    constexpr inline void invalidate(){tickInvalidated = true;}
public:
    //MOVE_NO_COPY(Light);
    
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
    
    /**
    Set the layers that this light will illuminate. If a bit is unset, that layer is not illuminated.
    @param layers illumination bitmask
    */
    constexpr void SetIlluminationLayers(renderlayer_t layers){
        invalidate();
        illuminationLayers = layers;
    }
    constexpr auto GetIlluminationLayers(){
        return illuminationLayers;
    }
};

/**
Represents a light that can cast shadows. Lights can be constrained to shadow specific objects with layers. 
By default, lights will cast shadows from objects on any layer.
*/
struct ShadowLightBase : public Light {
private:
	bool doesCastShadow = false;
    renderlayer_t shadowLayers = ALL_LAYERS;

public:
	bool CastsShadows() const { return doesCastShadow; }
	void SetCastsShadows(bool casts) {
		invalidate();
		doesCastShadow = casts;
	}
    /**
    Set the layers that this light will shadow. If a bit is unset, that layer is not shadowed.
    @param layers shadow bitmask
    */
    constexpr void SetShadowLayers(renderlayer_t layers){
        invalidate();
        shadowLayers = layers;
    }
    
    constexpr auto GetShadowLayers() const{
        return shadowLayers;
    }
};

/**
A light that additively affects the whole scene. This is the only light type that is affected by SSAO. Useful for faking indirect light.
If an environment provided, then the color and intensity are multipliers for the environment data. Otherwise, the environment is assumed to be {1,1,1,1}.
*/
struct AmbientLight : public Light, public QueryableDelta<Light,AmbientLight>{
	using light_t = AmbientLight;
	using QueryableDelta<Light,AmbientLight>::GetQueryTypes;
	
	// does not cast shadows
	constexpr bool CastsShadows() const { return false;  }
	
	void DebugDraw(RavEngine::DebugDrawer&, const Transform&) const override;

    Ref<Skybox> environment; // optional
};

/**
A light that affects the entire scene by sending parallel rays. This is useful for modelling the sun.
*/
struct DirectionalLight : public ShadowLightBase, public QueryableDelta<QueryableDelta<Light,ShadowLightBase>,DirectionalLight>{
	using light_t = DirectionalLight;
	using QueryableDelta<QueryableDelta<Light,ShadowLightBase>,DirectionalLight>::GetQueryTypes;

    DirectionalLight();
    
#if !RVE_SERVER
    struct ShadowMap {
        Array<DepthPyramid,MAX_CASCADES> pyramid;
        Array<RGLTexturePtr,4> shadowMap;
    } shadowData;
    Array<float, MAX_CASCADES> shadowCascades{0.1, 0.2, 0.3, 1};
    uint8_t numCascades = shadowCascades.size();
#endif
	
	void DebugDraw(RavEngine::DebugDrawer&, const Transform&) const override;

};

/**
A light that emits rays omnidirectionally from a single point. This is useful for modelling lightbulbs.
*/
struct PointLight : public ShadowLightBase, public QueryableDelta<QueryableDelta<Light, ShadowLightBase>,PointLight>{
	using light_t = PointLight;
	using QueryableDelta<QueryableDelta<Light, ShadowLightBase>,PointLight>::GetQueryTypes;
	
	void DebugDraw(RavEngine::DebugDrawer&, const Transform&) const override;

	PointLight();

#if !RVE_SERVER
	struct ShadowData {
		Array<DepthPyramid, 6> cubePyramids;
		Array<RGLTexturePtr, 6> cubeShadowmaps;
		RGLTexturePtr mapCube;
	} shadowData;
#endif

    matrix4 CalcProjectionMatrix() const;
    static matrix4 CalcViewMatrix(const vector3& lightPos, uint8_t index);
	
private:
	/**
	 Caclulate the radius of the light using its current intensity
	 @return the radius
	 */
    float CalculateRadius() const{
        return sqrt( GetIntensity() / LIGHT_MIN_INFLUENCE);
	}
};

/**
A light that emits rays from a sigle point, constrained to a cone. This is useful for modeling flashlights.
*/
class SpotLight : public ShadowLightBase, public QueryableDelta<QueryableDelta<Light,ShadowLightBase>,SpotLight>{
    //light properties
    float coneAngle = 45.0;    // in degrees
    float penumbraAngle = 10;
public:
	using light_t = SpotLight;
	using QueryableDelta<QueryableDelta<Light,ShadowLightBase>,SpotLight>::GetQueryTypes;
	
	void DebugDraw(RavEngine::DebugDrawer&, const Transform&) const override;
    
    SpotLight();
    
    matrix4 CalcProjectionMatrix() const;
    matrix4 CalcViewMatrix(const matrix4& worldTransform) const;
    
#if !RVE_SERVER
    struct ShadowMap {
        DepthPyramid pyramid;
        RGLTexturePtr shadowMap;
    } shadowData;
#endif
    
#if !RVE_SERVER
    const ShadowMap& GetShadowMap() const {
        return shadowData;
    }
#endif
    
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
    
};

}

