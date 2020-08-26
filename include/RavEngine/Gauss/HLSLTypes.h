/*
 * HLSLTypes.h
 * 
 * This file is part of the "GaussianLib" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef GS_HLSL_TYPES_H
#define GS_HLSL_TYPES_H

// <<< extension header >>>


#include <Gauss/Gauss.h>


/* --- Type Alias --- */

using float2    = Gs::Vector2f;
using float3    = Gs::Vector3f;
using float4    = Gs::Vector4f;

using double2   = Gs::Vector2d;
using double3   = Gs::Vector3d;
using double4   = Gs::Vector4d;

using int2      = Gs::Vector2i;
using int3      = Gs::Vector3i;
using int4      = Gs::Vector4i;

using uint2     = Gs::Vector2ui;
using uint3     = Gs::Vector3ui;
using uint4     = Gs::Vector4ui;

using bool2     = Gs::Vector2T<bool>;
using bool3     = Gs::Vector3T<bool>;
using bool4     = Gs::Vector4T<bool>;

using float2x2  = Gs::Matrix2f;
using float2x3  = Gs::Matrix<float, 2, 3>;
using float2x4  = Gs::Matrix<float, 2, 4>;

using float3x2  = Gs::Matrix<float, 3, 2>;
using float3x3  = Gs::Matrix3f;
using float3x4  = Gs::Matrix<float, 3, 4>;

using float4x2  = Gs::Matrix<float, 4, 2>;
using float4x3  = Gs::Matrix<float, 4, 3>;
using float4x4  = Gs::Matrix4f;

using double2x2 = Gs::Matrix2d;
using double2x3 = Gs::Matrix<double, 2, 3>;
using double2x4 = Gs::Matrix<double, 2, 4>;

using double3x2 = Gs::Matrix<double, 3, 2>;
using double3x3 = Gs::Matrix3d;
using double3x4 = Gs::Matrix<double, 3, 4>;

using double4x2 = Gs::Matrix<double, 4, 2>;
using double4x3 = Gs::Matrix<double, 4, 3>;
using double4x4 = Gs::Matrix4d;

using int2x2    = Gs::Matrix2i;
using int2x3    = Gs::Matrix<std::int32_t, 2, 3>;
using int2x4    = Gs::Matrix<std::int32_t, 2, 4>;

using int3x2    = Gs::Matrix<std::int32_t, 3, 2>;
using int3x3    = Gs::Matrix3i;
using int3x4    = Gs::Matrix<std::int32_t, 3, 4>;

using int4x2    = Gs::Matrix<std::int32_t, 3, 2>;
using int4x3    = Gs::Matrix<std::int32_t, 3, 3>;
using int4x4    = Gs::Matrix4i;

using uint2x2   = Gs::Matrix2ui;
using uint2x3   = Gs::Matrix<std::uint32_t, 2, 3>;
using uint2x4   = Gs::Matrix<std::uint32_t, 2, 4>;

using uint3x2   = Gs::Matrix<std::uint32_t, 3, 2>;
using uint3x3   = Gs::Matrix3ui;
using uint3x4   = Gs::Matrix<std::uint32_t, 3, 4>;

using uint4x2   = Gs::Matrix<std::uint32_t, 4, 2>;
using uint4x3   = Gs::Matrix<std::uint32_t, 4, 3>;
using uint4x4   = Gs::Matrix4ui;

using bool2x2   = Gs::Matrix<bool, 2, 2>;
using bool2x3   = Gs::Matrix<bool, 2, 3>;
using bool2x4   = Gs::Matrix<bool, 2, 4>;

using bool3x2   = Gs::Matrix<bool, 3, 2>;
using bool3x3   = Gs::Matrix<bool, 3, 3>;
using bool3x4   = Gs::Matrix<bool, 3, 4>;

using bool4x2   = Gs::Matrix<bool, 4, 2>;
using bool4x3   = Gs::Matrix<bool, 4, 3>;
using bool4x4   = Gs::Matrix<bool, 4, 4>;


#endif



// ================================================================================
