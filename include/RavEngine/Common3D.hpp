#pragma once

namespace RavEngine{

struct Vertex
{
	float      position[3];
	LLGL::ColorRGBf   color;
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
