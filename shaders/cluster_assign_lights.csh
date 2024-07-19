// adapted from https://github.com/DaveH355/clustered-shading
#define LOCAL_SIZE 128
layout(local_size_x = LOCAL_SIZE, local_size_y = 1, local_size_z = 1) in;

struct PointLight
{
    vec4 position;
    vec4 color;
    float intensity;
    float radius;
};

struct Cluster
{
    vec4 minPoint;
    vec4 maxPoint;
    uint count;
    uint lightIndices[100];
};

layout(std430, binding = 0) restrict buffer clusterSSBO
{
    Cluster clusters[];
};

layout(std430, binding = 1) restrict readonly buffer lightSSBO
{
    PointLight pointLight[];
};

layout(push_constant, scalar) uniform UniformBufferObject{
    mat4 viewMatrix;
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
    vec3 center = vec3(ubo.viewMatrix * pointLight[i].position);
    float radius = pointLight[i].radius;

    vec3 aabbMin = cluster.minPoint.xyz;
    vec3 aabbMax = cluster.maxPoint.xyz;

    return sphereAABBIntersection(center, radius, aabbMin, aabbMax);
}

// each invocation of main() is a thread processing a cluster
void main()
{
    uint lightCount = pointLight.length();
    uint index = gl_WorkGroupID.x * LOCAL_SIZE + gl_LocalInvocationID.x;
    Cluster cluster = clusters[index];

    // we need to reset count because culling runs every frame.
    // otherwise it would accumulate.
    cluster.count = 0;

    for (uint i = 0; i < lightCount; ++i)
    {
        if (testSphereAABB(i, cluster) && cluster.count < 100)
        {
            cluster.lightIndices[cluster.count] = i;
            cluster.count++;
        }
    }
    clusters[index] = cluster;
}

