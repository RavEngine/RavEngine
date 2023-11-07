#extension GL_EXT_samplerless_texture_functions : enable
layout(push_constant, std430) uniform UniformBufferObject{
	mat4 viewProj;
    vec3 camPos;
	uint indirectBufferOffset;			// needs to be like this because of padding / alignment
	uint numObjects;
	uint cullingBufferOffset;
    float radius;
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

layout(binding = 4) uniform texture2D depthPyramid;
layout(binding = 5) uniform sampler depthPyramidSampler;

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

/**
@param centerWorldSpace - the center of the sphere in world space
@param radius - the sphere radius in world space
@param viewProj - the viewProjection matrix of the camera
@return the max radius in NDC
*/
float findMaxRadiusInNDC(float radius, mat4 viewProj){

    // project 6 radius vectors and find the longest one in NDC
    vec3 radii[] = {
        vec3(radius,0,0),
        vec3(0,radius,0),
        vec3(0,0,radius)
    };
    float maxRadiusNDC = 0;
    for(uint i = 0; i < radii.length(); i++){
        vec4 projected = (viewProj * vec4(radii[i],1));
        radii[i] = projected.xyz / projected.w;
        maxRadiusNDC = max(maxRadiusNDC, length(radii[i])); 
    }

    return maxRadiusNDC;
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
		float maxRadiusNDC = findMaxRadiusInNDC(radius,ubo.viewProj);
        vec4 projectedCenter = (ubo.viewProj * vec4(center,1));

        float maxRadiusPixels = maxRadiusNDC * textureSize(depthPyramid,0).x;      // square texture

        //find the mipmap level that will match the screen size of the sphere
	    float level = floor(log2(maxRadiusPixels));

        // create the corners of the AABB
        vec3 projectedCenterNDC = projectedCenter.xyz / projectedCenter.w;
        vec2 ndcCorners[] = {
             projectedCenterNDC.xy + vec2(-maxRadiusNDC,maxRadiusNDC),      // top left
             projectedCenterNDC.xy + vec2(maxRadiusNDC,maxRadiusNDC),      // top right
             projectedCenterNDC.xy + vec2(maxRadiusNDC,-maxRadiusNDC),      // bottom right
             projectedCenterNDC.xy + vec2(-maxRadiusNDC,-maxRadiusNDC),      // bottom left
        };

        float minDepth = 1;
        for(uint i = 0; i < ndcCorners.length(); i++){
            // transform from [-1,1] to [0,1]
            ndcCorners[i] = (ndcCorners[i] + 1) * 0.5;
            ndcCorners[i] = 1 - ndcCorners[i];          // flip Y because we access textures that way

        	//sample the depth pyramid at that specific level
        	float depth = textureLod(sampler2D(depthPyramid, depthPyramidSampler), ndcCorners[i], level).x;
            minDepth = min(minDepth, depth);
        }
        
        float depthSphereFront = (projectedCenterNDC.z); // the front face of the AABB in NDC

        isOnCamera = isOnCamera && depthSphereFront >= minDepth;
    }

	// check 2: what LOD am I in
	uint lodID = 0;	//TODO: when multi-LOD support is added

	// if both checks are true, atomic-increment the instance count and write the entity ID into the output ID buffer based on the previous value of the instance count
	if (isOnCamera) {
		uint idx = atomicAdd(indirectBuffer[ubo.indirectBufferOffset + lodID].instanceCount,1);
		uint idxLODOffset = ubo.numObjects * lodID + ubo.cullingBufferOffset;
		entityIDsToRender[idx + idxLODOffset] = entityID;
	}

}
