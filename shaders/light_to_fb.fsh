
layout(binding = 0) uniform sampler g_sampler;
layout(binding = 1) uniform texture2D t_light;

layout(push_constant) uniform UniformBufferObject{
	ivec4 viewRect;
} ubo;

layout(location = 0) out vec4 outcolor;

void main()
{
	vec2 texcoord = vec2(gl_FragCoord.x / ubo.viewRect.z, gl_FragCoord.y / ubo.viewRect.w);
	
	vec3 light = texture(sampler2D(t_light, g_sampler), texcoord).xyz;
    outcolor = vec4(light, 1);
}
