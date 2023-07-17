layout(location = 0) in vec4 v_color0;
layout(location = 1) in vec2 v_texcoord0;

layout(binding = 0) uniform sampler g_sampler;
layout(binding = 1) uniform texture2D t_uitex;

layout(location = 0) out vec4 outcolor;

void main()
{
	vec4 color = texture(sampler2D(t_uitex,g_sampler), v_texcoord0) * v_color0;
	outcolor = color;
}
