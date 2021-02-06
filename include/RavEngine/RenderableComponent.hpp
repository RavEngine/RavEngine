#pragma once
#include "Component.hpp"
#include "Queryable.hpp"
#include "Ref.hpp"

namespace RavEngine{
class Material;
	/**
	Defines a component that can be rendered to the screen.
	*/
	struct RenderableComponent : public Component, public Queryable<RenderableComponent> {

		virtual void Draw(int view = 0){}
		
		/**
		 Draw with a material override
		 @param mat the material override
		 */
		virtual void Draw(Ref<Material> mat, int view = 0){}

		virtual ~RenderableComponent() {}
	};
}
