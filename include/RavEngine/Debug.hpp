#pragma once
#include "Common3D.hpp"
#include <SpinLock.hpp>
#include <vector>

namespace  RavEngine {

class DebugDraw{
	static SpinLock mtx;
	struct drawinst{
		matrix4 transform;
		vector3 color;
		drawinst(const matrix4& t, const vector3& c) : transform(t), color(c){};
	};
	static std::vector<drawinst> instances;
public:
	/**
	 Render a wirefram rectangular prism
	 @param transform the trasformation for this prism
	 @param color the hexadecimal color for this wireframe
	 */
	static void DrawRectangularPrism(const matrix4& transform, const vector3& color);
	
	static void DrawSphere(const matrix4& transform, const vector3& color, decimalType radius);
	
	static void DrawCylinder(const matrix4& transform, const vector3& color, decimalType radius, decimalType height);
	
	static void Reset();
	
	static const drawinst InstanceAt(size_t index);
};
}
