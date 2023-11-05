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

// adapted from: https://gamedev.stackexchange.com/questions/49222/aabb-of-a-spheres-screen-space-projection
/// <summary>
/// returns the screen-space (normalized device coordinates) bounds of a projected sphere
/// </summary>
/// <param name="center">view-space center of the sphere</param>
/// <param name="radius">world or view space radius of the sphere</param>
/// <param name="boxMin">minimum (bottom left) projected bounds</param>
/// <param name="boxMax">maximum (top right) projected bounds</param>
void GetProjectedBounds(vec3 center, float radius, mat4 Projection, inout vec3 boxMin, inout vec3 boxMax)
{

    float d2 = dot(center,center);

    float a = sqrt(d2 - radius * radius);

    /// view-aligned "right" vector (right angle to the view plane from the center of the sphere. Since  "up" is always (0,n,0), replaced cross product with vec3(-c.z, 0, c.x)
    vec3 right = (radius / a) * vec3(-center.z, 0, center.x);
    vec3 up = vec3(0,radius,0);

    vec4 projectedRight  = Projection * vec4(right,0);
    vec4 projectedUp     = Projection * vec4(up,0);

    vec4 projectedCenter = Projection * vec4(center,1);

    vec4 north  = projectedCenter + projectedUp;
    vec4 east   = projectedCenter + projectedRight;
    vec4 south  = projectedCenter - projectedUp;
    vec4 west   = projectedCenter - projectedRight;

    north /= north.w ;
    east  /= east.w  ;
    west  /= west.w  ;
    south /= south.w ;

    boxMin = min(min(min(east,west),north),south).xyz;
    boxMax = max(max(max(east,west),north),south).xyz;
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

		vec3 bbmin, bbmax;
        GetProjectedBounds(center, radius, ubo.viewProj, bbmin, bbmax);

        // adapted from: https://vkguide.dev/docs/gpudriven/compute_culling/
        float width = abs(bbmax.x - bbmin.x), height = abs(bbmax.y - bbmin.y);

        ivec2 pyramidSize = textureSize(depthPyramid,0);
        width *= pyramidSize.x;
        height *= pyramidSize.y;

        //find the mipmap level that will match the screen size of the sphere
	    float level = floor(log2(max(width, height)));

		//sample the depth pyramid at that specific level
		float depth = textureLod(sampler2D(depthPyramid, depthPyramidSampler), vec2((bbmin.xy+bbmax.xy)/2) * 0.5, level).x;
        
        vec4 transformed = ubo.viewProj * vec4(center,1);
        float depthSphereFront = transformed.z / transformed.w; //TODO: this is currently the center being transformed

        isOnCamera = isOnCamera && depthSphereFront >= depth;
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
