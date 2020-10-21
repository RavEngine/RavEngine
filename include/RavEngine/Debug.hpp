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
	 @param color the hexadecimal color for this wireframe, in format 0xRRGGBBAA
	 @param deltas the x, y, and z lengths of the prism
	 */
	static void DrawRectangularPrism(const matrix4& transform, const color_t color, const vector3& deltas);
	
	/**
	 Render a wireframe sphere
	 @param transform the world-space transform for this shape
	 @param color the hexadecimal color for this wireframe, in format 0xRRGGBBAA
	 @param radius the radius of the sphere
	 */
	static void DrawSphere(const matrix4& transform, const color_t color, decimalType radius);
	
	/**
	 Render a wireframe cylinder
	 @param transform the world-space transform for this shape
	 @param color the hexadecimal color for this wireframe, in format 0xRRGGBBAA
	 @param radius the radius of the cylinder
	 @param height the height of the cylinder
	 */
	static void DrawCylinder(const matrix4& transform, const color_t color, decimalType radius, decimalType height);
    
    /**
     Render a wireframe capsule
     @param transform the world-space transform for this shape
     @param color the hexadecimal color for this wireframe, in format 0xRRGGBBAA
     @param radius the radius of the cylinder
     @param height the height of the cylinder
     */
    static void DrawCapsule(const matrix4& transform, const color_t color, decimalType radius, decimalType height);
    
    /**
     Render an N-sided prism capsule
     @param transform the world-space transform for this shape
     @param color the hexadecimal color for this wireframe, in format 0xRRGGBBAA
     @param radius the radius of the prism
     @param height  the height of the prism
     @param sides the number of sides of the prism
     */
    static void DrawPrism(const matrix4& transform, const color_t color, decimalType radius, decimalType height, decimalType sides);
    
    /**
     Render an arrow
     @param start the begin coordinate for the arrow
     @param end the end coordinate for the arrow
     @param color the color of the arrow
     */
    static void DrawArrow(const vector3& start, const vector3& end, const color_t color);
    
private:
	
	/**
	 Helper method, handles boilerplate like thread-safety and setting the matrices
	 @param transform the world space transform for the shape
	 @param impl the callback to invoke, pass a lambda
	 */
	static void DrawHelper(const matrix4& transform, std::function<void()> impl);
};
}
