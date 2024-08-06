#include "cluster_defs.h"

struct Cluster
{
    vec3 minPoint;
    vec3 maxPoint;
    uint pointLightCount;
    uint spotLightCount;
    uint pointLightIndices[CLUSTER_MAX_POINTS];
    uint spotLightIndices[CLUSTER_MAX_SPOTS];
};


struct PointLight
{
    vec3 position;
    vec3 color;
    float intensity;
    int castsShadows;
};

struct SpotLight{
    mat4 lightViewProj;
    mat4 worldTransform;
    vec3 color;
    float intensity;
    float coneAngle;
    float penumbraAngle;
    int castsShadows;
    uint shadowmapBindlessIndex;
};


float getPointLightRadius(float intensity){
    // for quadratic falloff (influence = intensity / dist^2 ), the radius is equal to sqrt(intensity / min_influence)
    return sqrt(intensity / LIGHT_MIN_INFLUENCE);
}

float getLightAttenuation(float dist){
    return 1 / (dist * dist);
}
