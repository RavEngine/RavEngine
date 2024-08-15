
// adapted from: https://learnopengl.com/Guest-Articles/2020/OIT/Weighted-Blended

layout(binding = 0) uniform sampler g_sampler; 

layout(binding = 1) uniform texture2D accumTexture;
layout(binding = 2) uniform texture2D revealageTexture;

layout(location = 0) out vec4 outcolor;

// epsilon number
const float EPSILON = 0.00001f;

// calculate floating point numbers equality accurately
bool isApproximatelyEqual(float a, float b)
{
    return abs(a - b) <= (abs(a) < abs(b) ? abs(b) : abs(a)) * EPSILON;
}

// get the max value between three values
float max3(vec3 v)
{
    return max(max(v.x, v.y), v.z);
}

void main()
{
    // fragment coordination
    ivec2 coords = ivec2(gl_FragCoord.xy);

    // fragment revealage
    float revealage = texelFetch(sampler2D(revealageTexture, g_sampler), coords, 0).r;

    // save the blending and color texture fetch cost if there is not a transparent fragment
    if (isApproximatelyEqual(revealage, 1.0f))
        discard;

    // fragment color
    vec4 accumulation = texelFetch(sampler2D(accumTexture,g_sampler), coords, 0);

    // suppress overflow
    if (isinf(max3(abs(accumulation.rgb))))
        accumulation.rgb = vec3(accumulation.a);

    // prevent floating point precision bug
    vec3 average_color = accumulation.rgb / max(accumulation.a, EPSILON);

    // blend pixels
    outcolor = vec4(average_color, 1.0f - revealage);
}
