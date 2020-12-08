#pragma once
#include "Queryable.hpp"
#include "CameraComponent.hpp"
#include "Common3D.hpp"
#include "Component.hpp"

namespace RavEngine{
struct Light : public Queryable<Light>, public Component {
	virtual bool IsInFrustum(Ref<CameraComponent> cam) const = 0;
	float Intensity = 1.0;
	ColorRGBA color{0,0,0,1};
	virtual void DebugDraw() const = 0;
	virtual void DrawVolume(int view) const = 0;
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
	bool IsInFrustum(Ref<CameraComponent> cam) const override{
		return true;
	}
	
	void DebugDraw() const override;
	virtual void DrawVolume(int view) const override;
};

struct DirectionalLight : public ShadowLight, public QueryableDelta<QueryableDelta<Light,ShadowLight>,DirectionalLight>{
	using QueryableDelta<QueryableDelta<Light,ShadowLight>,DirectionalLight>::GetQueryTypes;
	
	/**
	 Directional lights are always in the frustum
	 */
	bool IsInFrustum(Ref<CameraComponent> cam) const override{
		return true;
	}
	
	void DebugDraw() const override;
	virtual void DrawVolume(int view) const override;
};

struct PointLight : public ShadowLight, public QueryableDelta<QueryableDelta<Light,ShadowLight>,PointLight>{
	using QueryableDelta<QueryableDelta<Light,ShadowLight>,PointLight>::GetQueryTypes;
	
	float radius = 1;
	
	bool IsInFrustum(Ref<CameraComponent> cam) const override{
		//TODO: perform sphere intersection on camera bounds
		return true;
	}
	
	void DebugDraw() const override;
	virtual void DrawVolume(int view) const override;
};
}

