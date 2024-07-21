// adapted from: https://github.com/DaveH355/clustered-shading
#include "cluster_shared.glsl"

layout(scalar, binding = 0) restrict buffer clusterSSBO {
    Cluster clusters[];
};

layout(push_constant, scalar) uniform UniformBufferObject{
    mat4 inverseProjection;
    uvec3 gridSize;
    float zNear;
    uvec2 screenDimensions;
    float zFar;
} ubo;

vec3 screenToView(vec2 screenCoord)
{
    // normalize screenCoord to [-1, 1] and
    // set the NDC depth of the coordinate to be on the near plane. This is -1 by
    // default in OpenGL
    vec4 ndc = vec4(screenCoord / ubo.screenDimensions * 2.0 - 1.0, -1.0, 1.0);

    vec4 viewCoord = ubo.inverseProjection * ndc;
    viewCoord /= viewCoord.w;
    return viewCoord.xyz;
}

// Returns the intersection point of an infinite line and a
// plane perpendicular to the Z-axis
vec3 lineIntersectionWithZPlane(vec3 startPoint, vec3 endPoint, float zDistance)
{
    vec3 direction = endPoint - startPoint;
    vec3 normal = vec3(0.0, 0.0, -1.0); // plane normal

    // skip check if the line is parallel to the plane.

    float t = (zDistance - dot(normal, startPoint)) / dot(normal, direction);
    return startPoint + t * direction; // the parametric form of the line equation
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
    // Eye position is zero in view space
    const vec3 eyePos = vec3(0.0);

    uint tileIndex = gl_WorkGroupID.x + (gl_WorkGroupID.y * ubo.gridSize.x) +
            (gl_WorkGroupID.z * ubo.gridSize.x * ubo.gridSize.y);
    vec2 tileSize = ubo.screenDimensions / ubo.gridSize.xy;

    // calculate the min and max points of a tile in screen space
    vec2 minPoint_screenSpace = gl_WorkGroupID.xy * tileSize;
    vec2 maxPoint_screenSpace = (gl_WorkGroupID.xy + 1) * tileSize;

    // convert them to view space sitting on the near plane
    vec3 minPoint_viewSpace = screenToView(minPoint_screenSpace);
    vec3 maxPoint_viewSpace = screenToView(maxPoint_screenSpace);

    float tileNear =
        ubo.zNear * pow(ubo.zFar / ubo.zNear, gl_WorkGroupID.z / float(ubo.gridSize.z));
    float tileFar =
        ubo.zNear * pow(ubo.zFar / ubo.zNear, (gl_WorkGroupID.z + 1) / float(ubo.gridSize.z));

    // Find the 4 intersection points from a tile's min/max points to this cluster's
    // near and far planes
    vec3 minPointNear =
        lineIntersectionWithZPlane(eyePos, minPoint_viewSpace, tileNear);
    vec3 minPointFar =
        lineIntersectionWithZPlane(eyePos, minPoint_viewSpace, tileFar);
    vec3 maxPointNear =
        lineIntersectionWithZPlane(eyePos, maxPoint_viewSpace, tileNear);
    vec3 maxPointFar =
        lineIntersectionWithZPlane(eyePos, maxPoint_viewSpace, tileFar);

    vec3 minPointAABB = min(minPointNear, minPointFar);
    vec3 maxPointAABB = max(maxPointNear, maxPointFar);

    clusters[tileIndex].minPoint = minPointAABB;
    clusters[tileIndex].maxPoint = maxPointAABB;
}
