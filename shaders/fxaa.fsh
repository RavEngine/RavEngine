
// adapted from: http://www.geeks3d.com/20110405/fxaa-fast-approximate-anti-aliasing-demo-glsl-opengl-test-radeon-geforce/3/
// https://www.shadertoy.com/view/ls3GWS

#define FXAA_SPAN_MAX 8.0
#define FXAA_REDUCE_MUL   (1.0/FXAA_SPAN_MAX)
#define FXAA_REDUCE_MIN   (1.0/128.0)
#define FXAA_SUBPIX_SHIFT (1.0/4.0)

layout(push_constant, std430) uniform UniformBufferObject{
    ivec4 targetDim;
} ubo;

layout(binding = 0) uniform texture2D srcTexture;
layout(binding = 1) uniform sampler srcSampler;

layout(location = 0) out vec4 outcolor;


void main(){
    ivec2 srcResolution = ubo.targetDim.zw;
    vec2 uv2 = gl_FragCoord.xy / srcResolution;
    vec2 rcpFrame = 1./srcResolution;

    vec4 uv = vec4( uv2, uv2 - (rcpFrame * (0.5 + FXAA_SUBPIX_SHIFT)));

   // FXAA part
   vec3 color = vec3(0,0,0);

    vec3 rgbNW = texture(sampler2D(srcTexture, srcSampler), uv.zw).xyz;
    vec3 rgbNE = texture(sampler2D(srcTexture, srcSampler), uv.zw + vec2(1,0)*rcpFrame.xy).xyz;
    vec3 rgbSW = texture(sampler2D(srcTexture, srcSampler), uv.zw + vec2(0,1)*rcpFrame.xy).xyz;
    vec3 rgbSE = texture(sampler2D(srcTexture, srcSampler), uv.zw + vec2(1,1)*rcpFrame.xy).xyz;
    vec3 rgbM  = texture(sampler2D(srcTexture, srcSampler), uv.xy).xyz;

    vec3 luma = vec3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM  = dot(rgbM,  luma);

    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max(
        (lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL),
        FXAA_REDUCE_MIN);
    float rcpDirMin = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);
    
    dir = min(vec2( FXAA_SPAN_MAX,  FXAA_SPAN_MAX),
          max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
          dir * rcpDirMin)) * rcpFrame.xy;

    vec3 rgbA = (1.0/2.0) * (
        texture(sampler2D(srcTexture, srcSampler), uv.xy + dir * (1.0/3.0 - 0.5)).xyz +
        texture(sampler2D(srcTexture, srcSampler), uv.xy + dir * (2.0/3.0 - 0.5)).xyz);
    vec3 rgbB = rgbA * (1.0/2.0) + (1.0/4.0) * (
        texture(sampler2D(srcTexture, srcSampler), uv.xy + dir * (0.0/3.0 - 0.5)).xyz +
        texture(sampler2D(srcTexture, srcSampler), uv.xy + dir * (3.0/3.0 - 0.5)).xyz);
    
    float lumaB = dot(rgbB, luma);

    if((lumaB < lumaMin) || (lumaB > lumaMax)) {
        color = rgbA;
    }
    else{
        color = rgbB;
    }
    
   // end FXAA part

    outcolor = vec4(color, 1);
}