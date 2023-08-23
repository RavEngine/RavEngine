
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

float remap(float value, float min1, float max1, float min2, float max2) {
  return clamp(min2 + (value - min1) * (max2 - min2) / (max1 - min1),min2,max2);
}
