#pragma once
#include "Component.hpp"
#include "Queryable.hpp"

namespace RavEngine{
class Material;
	/**
	Defines a component that can be rendered to the screen.
	*/
	struct RenderableComponent : public Component, public Queryable<RenderableComponent> {

		virtual void Draw(){}
		
		/**
		 Draw with a material override
		 @param mat the material override
		 */
		virtual void Draw(Ref<Material> mat){}

		virtual ~RenderableComponent() {}
	};
}
