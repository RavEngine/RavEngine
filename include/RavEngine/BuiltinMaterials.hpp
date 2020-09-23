#pragma once
#include "Material.hpp"

namespace RavEngine {
	class DefaultMaterial : public Material {
	public:
		DefaultMaterial() : Material("cubes") {}
	};
	class DefaultMaterialInstance : public MaterialInstance<DefaultMaterial> {
	public:
		DefaultMaterialInstance(Ref<DefaultMaterial> m) : MaterialInstance(m) { };
	};
}