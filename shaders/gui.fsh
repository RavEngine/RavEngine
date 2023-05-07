layout(location = 0) in vec4 v_color0;
layout(location = 1) in vec2 v_texcoord0;

uniform sampler2D s_uitex;

layout(location = 0) out vec4 outcolor;

void main()
{
	vec4 color = texture(s_uitex, v_texcoord0) * v_color0;
	outcolor = color;
}
