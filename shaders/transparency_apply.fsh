
// adapted from: https://casual-effects.blogspot.com/2015/03/implemented-weighted-blended-order.html

layout(binding = 0) uniform sampler g_sampler; 
/* sum(rgb * a, a) */
layout(binding = 1) uniform texture2D accumTexture;

/* prod(1 - a) */
layout(binding = 2) uniform texture2D revealageTexture;

layout(location = 0) out vec4 outcolor;

float max4 (vec4 v) {
  return max(max (max (v.x, v.y), v.z), v.w);
}

void main() {
    ivec2 C = ivec2(gl_FragCoord.xy);
    float revealage = texelFetch(sampler2D(revealageTexture,g_sampler), C, 0).r;
    if (revealage == 1.0) {
        // Save the blending and color texture fetch cost
        discard; 
    }

    vec4 accum = texelFetch(sampler2D(accumTexture,g_sampler), C, 0);
    // Suppress overflow
    if (isinf(max4(abs(accum)))) {
        accum.rgb = vec3(accum.a);
    }

    vec3 averageColor = accum.rgb / max(accum.a, 0.00001);


    // dst' =  (accum.rgb / accum.a) * (1 - revealage) + dst * revealage
    outcolor = vec4(averageColor, 1.0 - revealage);
}
