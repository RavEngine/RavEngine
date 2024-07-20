struct Cluster
{
    vec3 minPoint;
    vec3 maxPoint;
    uint count;
    uint lightIndices[100];
};


struct PointLight
{
    vec3 position;
    vec3 color;
    float intensity;
    float radius;
    int castsShadows;
};
