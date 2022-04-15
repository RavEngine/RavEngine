#pragma once
#include "mathtypes.hpp"

namespace RavEngine{
typedef uint32_t color_t;

struct Vertex
{
	float position[3];
	inline Vertex operator-(const Vertex& other) {
		return Vertex{ position[0] - other.position[0],  position[1] - other.position[1], position[2] - other.position[2] };
	}
};

struct UV
{
    float uv[2];
};

struct VertexNormalUV : public Vertex, public UV{
	float normal[3];
};

struct VertexUV : public Vertex, public UV{};

struct VertexColor : public Vertex{
	color_t color;
};

struct VertexColorUV : public VertexColor, public UV{};

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
constexpr static inline void copyMat4(const T* input, U* output, int size = 16) {
	for (int i = 0; i < size; i++) {
		output[i] = static_cast<U>(input[i]);
	}
}

/**
 @param x the number to round
 @param B the multiple base
 @return the closest multiple of B to x in the upwards direction. If x is already a multiple of B, returns x.
 */
inline constexpr int closest_multiple_of(int x, int B) {
	return ((x - 1) | (B - 1)) + 1;
}
}

inline constexpr decimalType deg_to_rad(decimalType val){
    return glm::radians(val);
}

inline constexpr auto Slerp(const quaternion& x, const quaternion& y, decimalType a){
    return glm::slerp(x,y,a);
}

template<typename S>
inline constexpr auto Slerp(const quaternion& x, const quaternion& y, decimalType a, S k){
    return glm::slerp(x,y,a,k);
}
