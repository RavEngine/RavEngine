#pragma once
#include "Component.hpp"
#include "Queryable.hpp"

namespace RavEngine{
	/**
	Defines a component that can be rendered to the screen.
	*/
	struct RenderableComponent : public Component, public Queryable<RenderableComponent> {

		virtual void Draw(){}

		virtual ~RenderableComponent() {}
	};
}
