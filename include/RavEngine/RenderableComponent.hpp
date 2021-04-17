#pragma once
#include "Component.hpp"
#include "Queryable.hpp"
#include "Ref.hpp"

namespace RavEngine{
class Material;
	/**
	Defines a component that can be rendered to the screen.
	*/
	struct RenderableComponent : public Component, public Queryable<RenderableComponent> {};
}
