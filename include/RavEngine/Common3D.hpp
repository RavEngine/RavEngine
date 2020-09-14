#pragma once

namespace RavEngine{

struct Vertex
{
	float   position[3];
	float   color[4];
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
