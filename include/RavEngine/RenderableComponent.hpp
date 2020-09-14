#pragma once
#include "Component.hpp"


namespace RavEngine{
	/**
	Defines a component that can be rendered to the screen.
	*/
	struct RenderableComponent : public Component {
		RenderableComponent() : Component(){}

		virtual void Draw(){}

		//ensure all derived classes invoke this method, otherwise they will not render!
		virtual void RegisterAllAlternateTypes() override {
			RegisterAlternateQueryType<RenderableComponent>();
		}

		virtual ~RenderableComponent() {}
	};
}
