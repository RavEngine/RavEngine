
layout(location = 0) in vec3 inPosition;
#if !RVE_DEPTHONLY || RVE_PREPASS
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBitangent;
#endif
layout(location = 4) in vec2 inUV;

#if !RVE_DEPTHONLY
	#if RVE_LIGHTMAP_UV
	layout(location = 5) in vec2 inLightmapUV;
	#define RVE_HAS_LIGHTMAP_UV 1
	#endif
#endif