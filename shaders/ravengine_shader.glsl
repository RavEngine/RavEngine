
#define PI 3.1415926

vec4 ComputeClipSpacePosition(vec2 pos, float depth){
	pos.y = 1.0 - pos.y;
	vec4 positionCS = vec4(pos * 2.0 - 1.0, depth, 1);
	return positionCS;
}

vec3 ComputeWorldSpacePos(vec2 positionNDC, float depth, mat4 invViewProj){
	vec4 positionCS = ComputeClipSpacePosition(positionNDC, depth);
	vec4 hpositionWS = invViewProj * positionCS;
	return (hpositionWS / hpositionWS.w).xyz;
}

vec3 ComputeViewSpacePos(vec2 positionNDC, float depth, mat4 invProj){
	vec4 positionCS = ComputeClipSpacePosition(positionNDC, depth);
	vec4 hpositionVS = invProj * positionCS;
	return (hpositionVS / hpositionVS.w).xyz;
}

float pcfForShadow(vec3 pixelWorldPos, mat4 lightViewProj, sampler shadowSampler, texture2D t_depthshadow){
	vec4 sampledPos = vec4(pixelWorldPos,1);
	sampledPos = lightViewProj * sampledPos;    // where is this on the light
	sampledPos /= sampledPos.w; // perspective divide
	sampledPos.xy = sampledPos.xy * 0.5 + 0.5;    // transform to [0,1] 
	sampledPos.y = 1 - sampledPos.y;

    return texture(sampler2DShadow(t_depthshadow,shadowSampler), sampledPos.xyz, 0).x;
}
