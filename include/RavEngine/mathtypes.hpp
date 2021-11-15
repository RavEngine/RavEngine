#pragma once

#include "glm/mat4x4.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"
#include "DataStructures.hpp"
#include "CTTI.hpp"
#include <iostream>

//needed on windows
#ifndef M_PI
#define M_PI RavEngine::PI
#endif

//defines the vector and quaternion types
//can change these to 32 bit float instead of 64 bit

#define DOUBLE_PRECISION
#ifdef DOUBLE_PRECISION


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
typedef glm::dvec2 vector2;

//a float-precision quaternion
typedef glm::quat quaternion;

typedef glm::mat4x4 matrix4;
typedef glm::mat3x3 matrix3;

typedef float decimalType;
#endif

//constant vector directions
constexpr vector3 vector3_right = vector3(1, 0, 0);
constexpr vector3 vector3_up = vector3(0, 1, 0);
constexpr vector3 vector3_forward = vector3(0, 0, -1);

static inline std::ostream& operator<<(std::ostream& os, const vector3& vec){
    os << "vector3(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
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
}

namespace RavEngine {
	// manual specializations for the networking and CTTI systems
	template<>
    inline std::string_view type_name<RawVec3>() {
		return "RawVec3";
	}

	template<>
    inline std::string_view type_name<RawQuat>() {
		return "RawQuat";
	}
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

struct soatransform{
	vector3 translate, scale;
	quaternion rotate;
};
