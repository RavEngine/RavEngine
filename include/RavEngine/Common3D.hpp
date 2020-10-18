#pragma once
#include "mathtypes.hpp"

namespace RavEngine{
typedef uint32_t color_t;

struct Vertex
{
	float		position[3];
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
	vector3 position;
	quaternion rotation;
	vector3 scale;
};
}
