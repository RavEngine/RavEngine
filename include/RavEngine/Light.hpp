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
class Light : public Queryable<Light,IDebugRenderable>, public IDebugRenderable {
    ColorRGBA color{1,1,1,1};
	float Intensity = 1.0;
    renderlayer_t illuminationLayers = ALL_LAYERS;
protected:
    bool tickInvalidated = true;    // trigger the world to update its parallel datastructure
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
    
    constexpr void SetIlluminationLayers(renderlayer_t layers){
        invalidate();
        illuminationLayers = layers;
    }
    constexpr auto GetIlluminationLayers(){
        return illuminationLayers;
    }
};

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
    constexpr void SetShadowLayers(renderlayer_t layers){
        invalidate();
        shadowLayers = layers;
    }
    
    constexpr auto GetShadowLayers() const{
        return shadowLayers;
    }
};

struct AmbientLight : public Light, public QueryableDelta<Light,AmbientLight>{
	using light_t = AmbientLight;
	using QueryableDelta<Light,AmbientLight>::GetQueryTypes;
	
	// does not cast shadows
	constexpr bool CastsShadows() const { return false;  }
	
	void DebugDraw(RavEngine::DebugDrawer&, const Transform&) const override;

};

struct DirectionalLight : public ShadowLightBase, public QueryableDelta<QueryableDelta<Light,ShadowLightBase>,DirectionalLight>{
	using light_t = DirectionalLight;
	using QueryableDelta<QueryableDelta<Light,ShadowLightBase>,DirectionalLight>::GetQueryTypes;
private:
    float shadowDistance = 30;
public:
    
    Array<float, MAX_CASCADES> shadowCascades{0.1, 0.2, 0.3, 1};
    uint8_t numCascades = shadowCascades.size();
    
    DirectionalLight();
    
#if !RVE_SERVER
    struct ShadowMap {
        Array<DepthPyramid,MAX_CASCADES> pyramid;
        Array<RGLTexturePtr,4> shadowMap;
    } shadowData;
#endif
    
    void SetShadowDistance(decltype(shadowDistance) distance){
        tickInvalidated = true;
        shadowDistance = distance;
    }
    
    auto GetShadowDistance() const{
        return shadowDistance;
    }
	
	void DebugDraw(RavEngine::DebugDrawer&, const Transform&) const override;

};

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
	
private:
	/**
	 Caclulate the radius of the light using its current intensity
	 @return the radius
	 */
    float CalculateRadius() const{
        return sqrt( GetIntensity() / LIGHT_MIN_INFLUENCE);
	}
};

class SpotLight : public ShadowLightBase, public QueryableDelta<QueryableDelta<Light,ShadowLightBase>,SpotLight>{
    //light properties
    float coneAngle = 45.0;    // in degrees
    float penumbraAngle = 10;
public:
	using light_t = SpotLight;
	using QueryableDelta<QueryableDelta<Light,ShadowLightBase>,SpotLight>::GetQueryTypes;
	
	void DebugDraw(RavEngine::DebugDrawer&, const Transform&) const override;
    
    SpotLight();
    
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

