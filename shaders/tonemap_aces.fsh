layout(binding = 0) uniform sampler g_sampler;
layout(binding = 1) uniform texture2D t_light;

layout(push_constant) uniform UniformBufferObject{
	ivec4 viewRect;
} ubo;

layout(location = 0) out vec4 outcolor;

// ACES adapted from: https://graphics-programming.org/tone-mapping#aces-tone-mapping
vec3 rtt_and_odt_fit(vec3 v)
{
    vec3 a = v * (v + 0.0245786f) - 0.000090537f;
    vec3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

vec3 aces_fitted(vec3 v)
{
    const mat3 aces_input_matrix = transpose(mat3(
                                        vec3(0.59719f, 0.35458f, 0.04823f),
                                        vec3(0.07600f, 0.90834f, 0.01566f),
                                        vec3(0.02840f, 0.13383f, 0.83777f)
                                  ));

    const mat3 aces_output_matrix =
    transpose(mat3(
        vec3( 1.60475f, -0.53108f, -0.07367f),
        vec3(-0.10208f,  1.10813f, -0.00605f),
        vec3(-0.00327f, -0.07276f,  1.07602f)
    ));
    
    v = aces_input_matrix * v;
    v = rtt_and_odt_fit(v);
    return aces_output_matrix *v;
}

void main()
{
	vec2 texcoord = vec2(gl_FragCoord.x / ubo.viewRect.z, gl_FragCoord.y / ubo.viewRect.w);
	
	vec3 light = texture(sampler2D(t_light, g_sampler), texcoord).xyz;

	light = aces_fitted(light);
        
    outcolor = vec4(light, 1);
}
