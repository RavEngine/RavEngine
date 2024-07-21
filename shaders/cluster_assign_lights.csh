// adapted from https://github.com/DaveH355/clustered-shading
#define LOCAL_SIZE 128
layout(local_size_x = LOCAL_SIZE, local_size_y = 1, local_size_z = 1) in;

#include "cluster_shared.glsl"

layout(scalar, binding = 0) restrict buffer clusterSSBO
{
    Cluster clusters[];
};

layout(scalar, binding = 1) restrict readonly buffer lightSSBO
{
    PointLight pointLight[];
};

layout(scalar, binding = 2) restrict readonly buffer spotLightSSBO
{
    SpotLight spotLight[];
};

layout(push_constant, scalar) uniform UniformBufferObject{
    mat4 viewMatrix;
    uint pointLightCount;
    uint spotLightCount;
} ubo;

bool sphereAABBIntersection(vec3 center, float radius, vec3 aabbMin, vec3 aabbMax)
{
    // closest point on the AABB to the sphere center
    vec3 closestPoint = clamp(center, aabbMin, aabbMax);
    // squared distance between the sphere center and closest point
    float distanceSquared = dot(closestPoint - center, closestPoint - center);
    return distanceSquared <= radius * radius;
}

// this just unpacks data for sphereAABBIntersection
bool testSphereAABB(uint i, Cluster cluster)
{
    vec3 center = vec3(ubo.viewMatrix * vec4(pointLight[i].position,1));
    float radius = pointLight[i].radius;

    vec3 aabbMin = cluster.minPoint.xyz;
    vec3 aabbMax = cluster.maxPoint.xyz;

    return sphereAABBIntersection(center, radius, aabbMin, aabbMax);
}

// each invocation of main() is a thread processing a cluster
void main()
{
    uint index = gl_WorkGroupID.x * LOCAL_SIZE + gl_LocalInvocationID.x;
    Cluster cluster = clusters[index];

    // we need to reset count because culling runs every frame.
    // otherwise it would accumulate.
    cluster.pointLightCount = 0;
    cluster.spotLightCount = 0;

    for (uint i = 0; i < ubo.pointLightCount; ++i)
    {
        if (testSphereAABB(i, cluster) && cluster.pointLightCount < CLUSTER_MAX_POINTS)
        {
            cluster.pointLightIndices[cluster.pointLightCount] = i;
            cluster.pointLightCount++;
        }
    }

    for(uint i = 0; i < ubo.spotLightCount; i++){
        // create a sphere to approximate the cone
        SpotLight light = spotLight[i];
        vec3 center = (light.worldTransform * vec4(0,0,0,1)).xyz;   // get world pos
        center = (ubo.viewMatrix * vec4(center,1)).xyz;             // transform to view space

        float radius = light.intensity * light.intensity;

        vec3 aabbMin = cluster.minPoint.xyz;
        vec3 aabbMax = cluster.maxPoint.xyz;

        if (sphereAABBIntersection(center, radius, aabbMin, aabbMax)){
            cluster.spotLightIndices[cluster.spotLightCount] = i;
            cluster.spotLightCount++;
        }
    }

    clusters[index] = cluster;
}

