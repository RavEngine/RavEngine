#pragma once
#include "mathtypes.hpp"

namespace RavEngine{
typedef uint32_t color_t;

struct Vertex
{
	float position[3];
};

struct VertexNormalUV : public Vertex{
	float normal[3];
	float uv[2];
};

struct VertexUV : public Vertex{
    float uv[2];
};

struct VertexColor : public Vertex{
	color_t color;
};

enum class ShaderStage{
	Vertex,
	Fragment,
	TessControl,
	TessEval,
	Geometry,
	Compute
};

struct Transformation{
	vector3 position = vector3(0,0,0);
	quaternion rotation = quaternion(1.0,0.0,0.0,0.0);
	vector3 scale = vector3(1,1,1);
	inline operator matrix4() const{
		return glm::translate(matrix4(1), (vector3)position) * glm::toMat4((quaternion)rotation) * glm::scale(matrix4(1), (vector3)scale);
	}
};

struct ColorRGBA{
	float R;
	float G;
	float B;
	float A;
};

/**
 Copy an array of type T to an array of type U
 @param input the source array
 @param output the destination array
 @param size optional size
 */
template<typename T, typename U>
static inline void copyMat4(const T* input, U* output, int size = 16) {
	for (int i = 0; i < size; i++) {
		output[i] = input[i];
	}
}

/**
 @param x the number to round
 @param B the multiple base
 @return the closest multiple of B to x in the upwards direction. If x is already a multiple of B, returns x.
 */
static inline constexpr int closest_multiple_of(int x, int B) {
	return ((x - 1) | (B - 1)) + 1;
}

// The stackarray creates a stack-resident array using a runtime-known size.
// There are no safety checks for overflowing the stack, and overflowing results in undefined behavior. 
// Only use for small sizes. For larger sizes, use the maybestackarray instead
#if defined __APPLE__ || __STDC_VERSION__ >= 199901L	//check for C99
#define stackarray(name, type, size) type name[size]	//prefer C VLA on supported systems
#else
#define stackarray(name, type, size) type* name = (type*)alloca(sizeof(type) * size) //warning: alloca may not be supported in the future
#endif

template<typename T>
struct maybestackarray_freer {
	T* ptr_to_stack = nullptr;
	~maybestackarray_freer() {
		assert(ptr_to_stack != nullptr);
		_freea(ptr_to_stack);
	}
};
//The Maybestackarray creates an array using a runtime-known size. 
// It will use stack if it can fit inside _ALLOCA_S_THRESHOLD, and use heap memory if it does not. 
// Do not call _freea on this, the structure will free itself when its scope ends
#ifdef _WIN32
#define maybestackarray(name, type, size) type* name = (type*)_malloca(sizeof(type) * size); maybestackarray_freer<type> name ## _freer{name}
#else
#define maybestackarray(name, type, size) type* name = (type*)malloca(sizeof(type) * size); maybestackarray_freer<type> name ## _freer{name}
#endif
}
