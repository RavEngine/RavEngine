#pragma once

#include "glm/mat4x4.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"
#include <ostream>

namespace RavEngine {
	constexpr double PI = 3.1415926535897932385;
}
//needed on windows
#ifndef M_PI
#define M_PI 3.14159265358979323846
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

#define print_vec3(vec) "vector3(" << vec.x << ", " << vec.y << ", " << vec.z << ")"
