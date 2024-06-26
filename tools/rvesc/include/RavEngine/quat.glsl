// note: the quaternion must be normalized for this to produce a valid result
mat3 quatToMat3(vec4 q) {
    float xx = q.x * q.x;
    float xy = q.x * q.y;
    float xz = q.x * q.z;
    float xw = q.x * q.w;

    float yy = q.y * q.y;
    float yz = q.y * q.z;
    float yw = q.y * q.w;

    float zz = q.z * q.z;
    float zw = q.z * q.w;

    return mat3(
        1.0 - 2.0 * (yy + zz), 2.0 * (xy - zw),       2.0 * (xz + yw),
        2.0 * (xy + zw),       1.0 - 2.0 * (xx + zz), 2.0 * (yz - xw),
        2.0 * (xz - yw),       2.0 * (yz + xw),       1.0 - 2.0 * (xx + yy)
    );
}

// Sum the rotations of two quaternions
vec4 quatAdd(vec4 q1, vec4 q2){
    vec3 u = q1.xyz;
    float s = q1.w;
    
    vec3 v = q2.xyz;
    float t = q2.w;
    
    float w = s * t - dot(u, v);
    vec3 xyz = cross(u, v) + s * v + t * u;

    return vec4(xyz, w);
}

// the vec must contain angles in radians
vec4 eulerToQuat(vec3 eulerAngles) {
    // Half angles
    float cx = cos(eulerAngles.x * 0.5);
    float sx = sin(eulerAngles.x * 0.5);
    float cy = cos(eulerAngles.y * 0.5);
    float sy = sin(eulerAngles.y * 0.5);
    float cz = cos(eulerAngles.z * 0.5);
    float sz = sin(eulerAngles.z * 0.5);

    // Compute quaternion
    vec4 q;
    q.w = cx * cy * cz + sx * sy * sz;
    q.x = sx * cy * cz - cx * sy * sz;
    q.y = cx * sy * cz + sx * cy * sz;
    q.z = cx * cy * sz - sx * sy * cz;

    return q;
}

vec3 degToRad(vec3 deg){
    return deg * 3.14159265 / 180.0;
}

mat4 mat3toMat4(mat3 m){

    return mat4(
        vec4(m[0],0),
        vec4(m[1],0),
        vec4(m[2],0),
        vec4(0,0,0,1)
    );
}