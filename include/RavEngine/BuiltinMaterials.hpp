#pragma once
#include "Material.hpp"
#include "Uniform.hpp"
#include <array>

namespace RavEngine {
	class DefaultMaterial : public Material {
	public:
		DefaultMaterial() : Material("default") {}
	};
	class DefaultMaterialInstance : public MaterialInstance<DefaultMaterial> {
	public:
		DefaultMaterialInstance(Ref<DefaultMaterial> m) : MaterialInstance(m) { };
	};

	class DebugMaterial : public Material{
	public:
		DebugMaterial() : Material("debug"){};
	};

	class DebugMaterialInstance : public MaterialInstance<DebugMaterial>{
	public:
		DebugMaterialInstance(Ref<DebugMaterial> m ) : MaterialInstance(m){};
	protected:
	};
}
