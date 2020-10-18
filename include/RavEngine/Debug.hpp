#pragma once
#include "Common3D.hpp"

namespace  RavEngine {

class DebugDraw{
	/**
	 Render a wirefram rectangular prism
	 @param transform the trasformation for this prism
	 @param color the hexadecimal color for this wireframe
	 */
	static void DrawRectangularPrism(const Transformation& transform, color_t color);
};
}
