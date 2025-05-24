#pragma once

#include "glm/mat4x4.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"
#include "Array.hpp"
#include "CTTI.hpp"
#include <iostream>

//defines the vector and quaternion types
//can change these to 32 bit float instead of 64 bit

typedef glm::ivec2 vector2i;
typedef glm::ivec3 vector3i;
typedef glm::ivec4 vector4i;

#define DOUBLE_PRECISION 0
#if DOUBLE_PRECISION


//a double-precision 3-component vector
typedef glm::dvec3 vector3;
typedef glm::dvec4 vector4;
typedef glm::dvec2 vector2;

//a double-precision quaternion
typedef glm::dquat quaternion;

typedef glm::dmat4x4 matrix4;
typedef glm::dmat3x3 matrix3;

typedef double decimalType;

#else
//a float 3-component vector
typedef glm::vec3 vector3;
typedef glm::vec4 vector4;
typedef glm::vec2 vector2;

//a float-precision quaternion
typedef glm::quat quaternion;

typedef glm::mat4x4 matrix4;
typedef glm::mat3x3 matrix3;

typedef float decimalType;
#endif

inline quaternion quat_identity() {
    return glm::quat_identity<decimalType, glm::packed_highp>();
}

//constant vector directions
constexpr vector3 vector3_right = vector3(1, 0, 0);
constexpr vector3 vector3_up = vector3(0, 1, 0);
constexpr vector3 vector3_forward = vector3(0, 0, -1);

static inline std::ostream& operator<<(std::ostream& os, const vector3& vec){
    os << "vector3(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
    return os;
}

static inline std::ostream& operator<<(std::ostream& os, const glm::vec4& vec) {
    os << "fvec4(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")";
    return os;
}


static inline std::ostream& operator<<(std::ostream& os, const quaternion& quat){
    os << "quat(" << quat.x << ", " << quat.y << ", " << quat.z << ", " << quat.w << ")";
    return os;
}

static inline std::ostream& operator<<(std::ostream& os, const matrix4& mat){
    os << "mat4(";
    for(uint8_t i = 0; i < 4; i++){
        for(uint8_t j = 0; j < 4; j++){
            os << mat[i][j] << " ";
        }
        os << "\n";
    }
    os << ")";
    return os;
}

#define print_vec3(vec) "vector3(" << vec.x << ", " << vec.y << ", " << vec.z << ")"
#define print_quat(quat) "quat(" << quat.x << ", " << quat.y << ", " << quat.z << ", " << quat.w << ")"

namespace RavEngine {
    constexpr decimalType PI = 3.1415926535897932385;
    
    typedef Array<decimalType, 3> RawVec3;
    typedef Array<decimalType, 4> RawQuat;
    
    static_assert(std::is_trivially_copyable_v<RawVec3>, "RawVec3 is not trivially copyable!");
    static_assert(!std::is_fundamental_v<RawVec3>, "RawVec3 is fundamental!");
    static_assert(!std::is_same_v<RawVec3,void>,"RawVec3 is void!");
    static_assert(is_eligible<RawVec3>,"RawVec3 is not eligible for CTTI!");
}

static inline RavEngine::RawVec3 Vec3toRaw(const vector3& vec) {
	return { vec.x, vec.y, vec.z };
}

static inline vector3 RawToVec3(const RavEngine::RawVec3& raw) {
	return vector3(raw[0], raw[1], raw[2]);
}

static inline RavEngine::RawQuat QuatToRaw(const quaternion& quat) {
	return { quat.w,quat.x,quat.y,quat.z };
}

static inline quaternion RawToQuat(const RavEngine::RawQuat& raw) {
	return quaternion(raw[0], raw[1], raw[2], raw[3]);
}

template<typename T>
static inline T remap_range(T value, T low1, T high1, T low2, T high2) {
    return low2 + (value - low1) * (high2 - low2) / (high1 - low1);
}

struct soatransform{
	vector3 translate, scale;
	quaternion rotate;
};

struct Bounds {
    float min[3]{ 0,0,0 };
    float max[3]{ 0,0,0 };
};

template<typename T>
struct dim_t {
    T width = 0, height = 0;
};

namespace RMath {
    // use these instead of glm::perspective or glm::ortho
    // these also implement reverse Z.

    template<typename T>
    auto orthoProjection(T left, T right, T bottom, T top, T zNear, T zFar) {
        return glm::orthoRH_ZO(left, right, bottom, top, zFar, zNear);
    }

    template<typename T>
    auto perspectiveProjection(T fovy, T aspect, T zNear, T zFar) {
        return glm::perspectiveRH_ZO(fovy, aspect, zFar, zNear);
    }

    /**
    * Point-in-AABB, where the point is in the AABB's local space
    */
    static inline bool pointInAABB(const vector3& point, const vector3& boxHalfExts) {
        return (point.x <= boxHalfExts.x && point.x >= -boxHalfExts.x) &&
            (point.y <= boxHalfExts.y && point.y >= -boxHalfExts.y) &&
            (point.z <= boxHalfExts.z && point.z >= -boxHalfExts.z);
    }
}

//ssize_t on MSVC
#if defined(_MSC_VER)
#include <BaseTsd.h>
#include <intsafe.h>
typedef SSIZE_T ssize_t;
#endif
