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
}

/**
 Copy an array of type T to an array of type U
 @param input the source array
 @param output the destination array
 @param size optional size
 */
template<typename T, typename U>
static inline void copyMat4(const T* input, U* output, int size = 16){
    for(int i = 0; i < size; i++){
        output[i] = input[i];
    }
}
