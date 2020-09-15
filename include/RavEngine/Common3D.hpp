#pragma once

namespace RavEngine{

struct Vertex
{
	float		position[3];
	uint32_t	abgr;	//color in alpha-blue-green-red
};

enum class ShaderStage{
	Vertex,
	Fragment,
	TessControl,
	TessEval,
	Geometry,
	Compute
};

}
