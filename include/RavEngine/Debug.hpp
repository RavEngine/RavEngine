#pragma once
#include "Common3D.hpp"
#include <SpinLock.hpp>
#include <vector>
#include <functional>

namespace  RavEngine {

class DebugDraw{
	static SpinLock mtx;
public:
	/**
	 Render a wirefram rectangular prism
	 @param transform the world-space trasformation for this shape
	 @param color the hexadecimal color for this wireframe
	 @param deltas the x, y, and z lengths of the prism
	 */
	static void DrawRectangularPrism(const matrix4& transform, const vector3& color, const vector3& deltas);
	
	/**
	 Render a wireframe sphere
	 @param transform the world-space transform for this shape
	 @param color the color for this shape
	 @param radius the radius of the sphere
	 */
	static void DrawSphere(const matrix4& transform, const vector3& color, decimalType radius);
	
	/**
	 Render a wireframe cylinder
	 @param transform the world-space transform for this shape
	 @param color the color for this shape
	 @param radius the radius of the cylinder
	 @param height the height of the cylinder
	 */
	static void DrawCylinder(const matrix4& transform, const vector3& color, decimalType radius, decimalType height);
private:
	
	/**
	 Helper method, handles boilerplate like thread-safety and setting the matrices
	 @param transform the world space transform for the shape
	 @param impl the callback to invoke, pass a lambda
	 */
	static void DrawHelper(const matrix4& transform, std::function<void(void)> impl);
};
}
