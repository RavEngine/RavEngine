#pragma once
#include "CTTI.hpp"
#include "Common3D.hpp"
#include "DebugDrawer.hpp"

namespace RavEngine{
	struct Transform;
struct IDebugRenderable : public AutoCTTI{
	bool debugEnabled = false;
	color_t debug_color = 0xFFFFFFFF;
	/**
	 Draw a wireframe shape representing the boundary of this object
	 @param color the hex color to use to draw, in format 0xRRGGBBAA
	 */
	virtual void DebugDraw(RavEngine::DebugDrawer& dbg, const Transform& transform) const = 0;
};
}
