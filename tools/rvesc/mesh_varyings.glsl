layout(location = 8) VARYINGDIR vec3[3] inTBN;          // even though it's unused in the Unlit pathway, all must be here for D3D
layout(location = 11) VARYINGDIR vec3 worldPosition;    // even though it's unused in the Unlit pathway, all must be here for D3D
layout(location = 12) VARYINGDIR vec3 viewPosition;
layout(location = 13) VARYINGDIR float clipSpaceZ;
layout(location = 14) VARYINGDIR flat uint varyingEntityID;
