

layout(location = 0) in vec2 inPosition;
layout(location = 0) out vec3 v_sky_ray;

struct EngineData{
    mat3 invView;
    float fov;
    float aspectRatio;
};

layout(scalar, binding = 1) readonly buffer engineDataSSBO
{
    EngineData constants;
};

vec3 skyray(vec2 uv, float fieldOfView, float aspectRatio)
{
    float d = 0.5 / tan(fieldOfView / 2.0);
    return vec3((uv.x - 0.5) * aspectRatio, uv.y - 0.5, -d);
}


void main()
{
    
    float x = -1.0 + float((gl_VertexID & 1) << 2);
    float y = -1.0 + float((gl_VertexID & 2) << 1);

    // render behind everything
    gl_Position = vec4(x, y, 0, 1);
    vec2 v_uv = vec2((x + 1.0) * 0.5, (y + 1.0) * 0.5);

    v_sky_ray = constants.invView * skyray(v_uv, constants.fov, constants.aspectRatio);
}
