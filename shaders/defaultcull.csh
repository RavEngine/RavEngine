#extension GL_EXT_samplerless_texture_functions : enable
layout(push_constant, std430) uniform UniformBufferObject{
	mat4 viewProj;
    vec3 camPos;
	uint indirectBufferOffset;			// needs to be like this because of padding / alignment
	uint numObjects;
	uint cullingBufferOffset;
    float radius;
    uint isSingleInstanceMode;
    uint numLODs;
} ubo;

layout(std430, binding = 0) readonly buffer idBuffer
{
	uint entityIDs[];
};

layout(std430, binding = 1) readonly buffer modelMatrixBuffer
{
	mat4 modelBuffer[];
};

layout(std430, binding = 2) buffer idOutputBuffer
{
	uint entityIDsToRender[];
};


struct IndirectCommand {
	uint indexCount;
	uint instanceCount;
	uint indexStart;
	uint baseVertex;
	uint baseInstance;
};

layout(std430, binding = 3) buffer drawcallBuffer
{
	IndirectCommand indirectBuffer[];
};

layout(std430, binding = 4) readonly buffer lodDistanceSSBO{
    float lodDistanceBuffer[];
};

layout(binding = 5) uniform texture2D depthPyramid;
layout(binding = 6) uniform sampler depthPyramidSampler;

// adapted from: https://gist.github.com/XProger/6d1fd465c823bba7138b638691831288
// Computes signed distance between a point and a plane
// vPlane: Contains plane coefficients (a,b,c,d) where: ax + by + cz = d
// vPoint: Point to be tested against the plane.
float DistanceToPlane( vec4 vPlane, vec3 vPoint )
{
    return dot(vec4(vPoint, 1.0), vPlane);
}
 
// Frustum cullling on a sphere. Returns > 0 if visible, <= 0 otherwise
float CullSphere( vec4 vPlanes[6], vec3 vCenter, float fRadius )
{
   float dist01 = min(DistanceToPlane(vPlanes[0], vCenter), DistanceToPlane(vPlanes[1], vCenter));
   float dist23 = min(DistanceToPlane(vPlanes[2], vCenter), DistanceToPlane(vPlanes[3], vCenter));
   float dist45 = min(DistanceToPlane(vPlanes[4], vCenter), DistanceToPlane(vPlanes[5], vCenter));
 
   return min(min(dist01, dist23), dist45) + fRadius;
}

// adapted from https://github.com/JuanDiegoMontoya/Frogfood/blob/main/src/FrogRenderer.cpp#L52
void MakeFrustumPlanes(mat4 viewProj, inout vec4 planes[6]){
    for (uint i = 0; i < 4; ++i) { planes[0][i] = viewProj[i][3] + viewProj[i][0]; }
    for (uint i = 0; i < 4; ++i) { planes[1][i] = viewProj[i][3] - viewProj[i][0]; }
    for (uint i = 0; i < 4; ++i) { planes[2][i] = viewProj[i][3] + viewProj[i][1]; }
    for (uint i = 0; i < 4; ++i) { planes[3][i] = viewProj[i][3] - viewProj[i][1]; }
    for (uint i = 0; i < 4; ++i) { planes[4][i] = viewProj[i][3] + viewProj[i][2]; }
    for (uint i = 0; i < 4; ++i) { planes[5][i] = viewProj[i][3] - viewProj[i][2]; }

    for (uint i = 0; i < planes.length(); i++) {
      planes[i] /= length(planes[i].xyz);
    }
}

// https://stackoverflow.com/questions/38112526/why-do-people-use-sqrtdotdistancevector-distancevector-over-opengls-distan
float distSquared( vec3 A, vec3 B )
{
    vec3 C = A - B;
    return dot( C, C );

}

struct ClipBoundingBoxResult{
    float minX, maxX, minY, maxY;
    float referenceZ;
};


ClipBoundingBoxResult projectWorldSpaceSphere(vec3 center, float r, mat4 viewProj){
    vec3 boxCorners[] = {
        // front
        center + vec3(r, -r, -r),
        center + vec3(-r, -r, -r),
        center + vec3(-r, r, -r),
        center + vec3(r, r, -r),

        // back
        center + vec3(r, -r, r),
        center + vec3(-r, -r, r),
        center + vec3(-r, r, r),
        center + vec3(r, r, r),
    };

    ClipBoundingBoxResult result;
    result.minX = 1;
    result.maxX = 0;
    result.minY = 1;
    result.maxY = 0;
    result.referenceZ = 0;

    for(uint i = 0; i < boxCorners.length(); i++){
        vec4 projected = viewProj * vec4(boxCorners[i],1);
        projected /= projected.w;

        result.minX = min(projected.x, result.minX);
        result.maxX = max(projected.x, result.maxX);

        result.minY = min(projected.y, result.minY);
        result.maxY = max(projected.y, result.maxY);

        result.referenceZ = max(projected.z, result.referenceZ);
    }


    return result;
}

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main() {
	// bail
	const uint currentEntity = gl_GlobalInvocationID.x;
	if (currentEntity >= ubo.numObjects) {
		return;
	}

	const uint entityID = entityIDs[currentEntity];
	mat4 model = modelBuffer[entityID];
    mat3 modelNoTranslate = mat3(model);

    vec4 planes[6];
    MakeFrustumPlanes(ubo.viewProj, planes);
    
    // Determine the new sphere using the input transformation.
    // we use the largest of the resulting unit vectors.
    vec3 radvecs[] = {
        vec3(0,0,ubo.radius),
        vec3(0,ubo.radius,0),
        vec3(ubo.radius,0,0)
    };
    
    float radius = 0;
    for(uint i = 0; i < radvecs.length(); i++){
        radvecs[i] = modelNoTranslate * radvecs[i];
        radius = max(radius, length(radvecs[i]));
    }
    vec3 center = (model * vec4(0,0,0,1)).xyz;
    
    // test all the transformed NDC planes
    // is considered on camera if the bounding sphere intersects the camera frustum
    bool isOnCamera = CullSphere(planes, center, radius) > 0;

    // check occlusion
    if (isOnCamera){
        // occlusion culling is not available on Metal or WebGPU
#if !defined(RGL_SL_MTL) && !defined(RGL_SL_WGSL)
		ClipBoundingBoxResult projected = projectWorldSpaceSphere(center, radius, ubo.viewProj);

        float mipDim = textureSize(depthPyramid,0).x;

        float bbwidth = (projected.maxX - projected.minX) * mipDim;
        float bbheight = (projected.maxY - projected.minY) * mipDim;

        if(projected.referenceZ <= 1){        // otherwise it intersects the near plane so we consider it to be visible
            //find the mipmap level that will match the screen size of the sphere
            float miplevel = ceil(log2(max(bbwidth, bbheight))) - 1;
            miplevel = max(0, miplevel);

            // create the corners of the bounding box for sampling
            vec2 ndcCorners[] = {
                vec2(projected.minX, projected.maxY),      // top left
                vec2(projected.maxX, projected.maxY),      // top right
                vec2(projected.maxX, projected.minY),      // bottom right,
                vec2(projected.minX, projected.minY),      // bottom left
            };

            float minDepth = 1;
            for(uint i = 0; i < ndcCorners.length(); i++){

                ndcCorners[i] = (ndcCorners[i] + 1) * 0.5;      // transform from [-1,1] to [0,1]
                ndcCorners[i].y = 1 - ndcCorners[i].y;          // flip Y because we access textures that way

                //sample the depth pyramid at that specific level
                float depth = textureLod(sampler2D(depthPyramid, depthPyramidSampler), ndcCorners[i], miplevel).x;
                minDepth = min(minDepth, depth);
            }
            
            isOnCamera = isOnCamera && projected.referenceZ >= minDepth;
        }
#endif
    }

	if (isOnCamera) {

        // check 2: what LOD am I in
        uint lodID = 0;	

        if (ubo.numLODs > 1){       // if there's only one answer, skip doing any of this

            const vec3 worldSpacePos = (model * vec4(0,0,0,1)).xyz;
            float distFromCamera = distSquared(worldSpacePos, ubo.camPos);

            float currentBest = lodDistanceBuffer[0];

            for(uint i = 0; i < ubo.numLODs; i++){
                // the LOD distances may not be in sorted order.
                // to select a LOD, we use "price is right" rules.
                // we use squared distance here.
                float minDisplayableDistForThisLOD = lodDistanceBuffer[i];
                minDisplayableDistForThisLOD *= minDisplayableDistForThisLOD;   // on host side, these are not expressed in squared distanecs

                // to use a LOD, the distance must be > the LOD's min distance

                if (distFromCamera > minDisplayableDistForThisLOD && minDisplayableDistForThisLOD > currentBest){
                    lodID = i;
                    currentBest = minDisplayableDistForThisLOD;
                }

            }
        }

        // atomic-increment the instance count and write the entity ID into the output ID buffer based on the previous value of the instance count
		uint idx = atomicAdd(indirectBuffer[ubo.indirectBufferOffset + lodID + ubo.isSingleInstanceMode * gl_GlobalInvocationID.x].instanceCount,1);
		uint idxLODOffset = ubo.numObjects * lodID + ubo.cullingBufferOffset;

        uint cullingSingleObjectModeOffset = gl_GlobalInvocationID.x * ubo.isSingleInstanceMode; 
		entityIDsToRender[idx + idxLODOffset + cullingSingleObjectModeOffset] = entityID;
	}

}
