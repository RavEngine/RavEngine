/*
 * GLSLTypes.h
 * 
 * This file is part of the "GaussianLib" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef GS_GLSL_TYPES_H
#define GS_GLSL_TYPES_H

// <<< extension header >>>


#include <Gauss/Gauss.h>


/* --- Type Alias --- */

using vec2      = Gs::Vector2f;
using vec3      = Gs::Vector3f;
using vec4      = Gs::Vector4f;

using dvec2     = Gs::Vector2d;
using dvec3     = Gs::Vector3d;
using dvec4     = Gs::Vector4d;

using ivec2     = Gs::Vector2i;
using ivec3     = Gs::Vector3i;
using ivec4     = Gs::Vector4i;

using uvec2     = Gs::Vector2ui;
using uvec3     = Gs::Vector3ui;
using uvec4     = Gs::Vector4ui;

using bvec2     = Gs::Vector2T<bool>;
using bvec3     = Gs::Vector3T<bool>;
using bvec4     = Gs::Vector4T<bool>;

using mat2      = Gs::Matrix2f;
using mat2x3    = Gs::Matrix<float, 2, 3>;
using mat2x4    = Gs::Matrix<float, 2, 4>;

using mat3x2    = Gs::Matrix<float, 3, 2>;
using mat3      = Gs::Matrix3f;
using mat3x4    = Gs::Matrix<float, 3, 4>;

using mat4x2    = Gs::Matrix<float, 4, 2>;
using mat4x3    = Gs::Matrix<float, 4, 3>;
using mat4      = Gs::Matrix4f;

using dmat2     = Gs::Matrix2d;
using dmat2x3   = Gs::Matrix<double, 2, 3>;
using dmat2x4   = Gs::Matrix<double, 2, 4>;

using dmat3x2   = Gs::Matrix<double, 3, 2>;
using dmat3     = Gs::Matrix3d;
using dmat3x4   = Gs::Matrix<double, 3, 4>;

using dmat4x2   = Gs::Matrix<double, 4, 2>;
using dmat4x3   = Gs::Matrix<double, 4, 3>;
using dmat4     = Gs::Matrix4d;


#endif



// ================================================================================
