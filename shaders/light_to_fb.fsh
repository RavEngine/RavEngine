
uniform sampler2D s_light;

layout(push_constant) uniform UniformBufferObject{
	mat4 viewProj;
	ivec4 viewRect;
} ubo;

layout(location = 0) out vec4 outcolor;

void main()
{
	vec2 texcoord = vec2(gl_FragCoord.x / ubo.viewRect.z, gl_FragCoord.y / ubo.viewRect.w);
	
	vec3 light = texture(s_light, texcoord).xyz;
    outcolor = vec4(light, 1);
}
