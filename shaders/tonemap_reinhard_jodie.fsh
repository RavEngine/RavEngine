layout(binding = 0) uniform sampler g_sampler;
layout(binding = 1) uniform texture2D t_light;

layout(push_constant) uniform UniformBufferObject{
	ivec4 viewRect;
} ubo;

layout(location = 0) out vec4 outcolor;

// reinhard_jodie adapted from: https://graphics-programming.org/tone-mapping#aces-tone-mapping
float luminance(vec3 v)
{
    return dot(v, vec3(0.2126f, 0.7152f, 0.0722f));
}

vec3 change_luminance(vec3 c_in, float l_out)
{
    float l_in = luminance(c_in);
    return c_in * (l_out / l_in);
}


vec3 reinhard_jodie(vec3 v)
{
    float l = luminance(v);
    vec3 tv = v / (1.0f + v);
    return mix(v / (1.0f + l), tv, tv);
}

void main()
{
	vec2 texcoord = vec2(gl_FragCoord.x / ubo.viewRect.z, gl_FragCoord.y / ubo.viewRect.w);
	
	vec3 light = texture(sampler2D(t_light, g_sampler), texcoord).xyz;

	light = reinhard_jodie(light);
        
    outcolor = vec4(light, 1);
}
