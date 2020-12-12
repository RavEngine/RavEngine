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
	void DrawVolume(int view) const;
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
	void DrawVolume(int view) const;
};

struct PointLight : public ShadowLight, public QueryableDelta<QueryableDelta<Light,ShadowLight>,PointLight>{
	using QueryableDelta<QueryableDelta<Light,ShadowLight>,PointLight>::GetQueryTypes;
		
	inline bool IsInFrustum(Ref<CameraComponent> cam) const{
		//TODO: perform sphere intersection on camera bounds
		return true;
	}
	
	void DebugDraw() const override;
	void DrawVolume(int view) const;
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
		Vector4Uniform lightColor = Vector4Uniform("u_lightColor");
	};
	
	/**
	 Material to draw point lights
	 */
	struct PointLightShader : public LightShader{
		PointLightShader() : LightShader("pointlightvolume"){}
		Vector4Uniform lightPosition = Vector4Uniform("u_lightPos");
	};
	/**
	 Holds uniforms for point lights
	 */
	struct PointLightShaderInstance : public MaterialInstance<PointLightShader>{
		PointLightShaderInstance(Ref<PointLightShader> m ) : MaterialInstance(m){}
		inline void SetPosColor(const ColorRGBA& pos, const ColorRGBA& color){
			mat->lightColor.SetValues(&color, 1);
			mat->lightPosition.SetValues(&pos, 1);
		}
	};
	
	struct AmbientLightShader : public LightShader{
	public:
		AmbientLightShader() : LightShader("ambientlightvolume"){}
	};
	struct AmbientLightShaderInstance : public MaterialInstance<AmbientLightShader>{
		AmbientLightShaderInstance(Ref<AmbientLightShader> m ) : MaterialInstance(m){}
		inline void SetColor(const ColorRGBA& color){
			mat->lightColor.SetValues(&color,1);
		}
	};
	
	struct DirectionalLightShader : public LightShader{
		DirectionalLightShader() : LightShader("directionallightvolume"){}
		Vector4Uniform lightDirection = Vector4Uniform("u_lightDir");
	};
	
	struct DirectionalLightShaderInstance : public MaterialInstance<DirectionalLightShader>{
		DirectionalLightShaderInstance(Ref<DirectionalLightShader> m ) : MaterialInstance(m){}
		inline void SetColorDirection(const ColorRGBA& color, const ColorRGBA dir){
			mat->lightColor.SetValues(&color,1);
			mat->lightDirection.SetValues(&dir, 1);
		}
	};
	
	static Ref<PointLightShaderInstance> pointLightShader;
	static Ref<AmbientLightShaderInstance> ambientLightShader;
	static Ref<DirectionalLightShaderInstance> directionalLightShader;
	
	static bgfx::VertexBufferHandle screenSpaceQuadVert;
	static bgfx::IndexBufferHandle screenSpaceQuadInd;
};
}

