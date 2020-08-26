/*
 * Typename.h
 * 
 * This file is part of the "GaussianLib" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef GS_TYPENAME_H
#define GS_TYPENAME_H


#include "Vector.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix.h"
#include "AffineMatrix3.h"
#include "AffineMatrix4.h"
#include "ProjectionMatrix4.h"
#include "Quaternion.h"


namespace Gs
{


//! Provides the scalar type of the scalar, vector, or matrix type specified by 'T'.
template <typename T>
struct ScalarType
{
    using Type = T;
};

template <typename T, std::size_t N>
struct ScalarType<Vector<T, N>>
{
    using Type = T;
};

template <typename T, std::size_t Rows, std::size_t Cols>
struct ScalarType<Matrix<T, Rows, Cols>>
{
    using Type = T;
};

template <typename T>
struct ScalarType<AffineMatrix3T<T>>
{
    using Type = T;
};

template <typename T>
struct ScalarType<AffineMatrix4T<T>>
{
    using Type = T;
};

template <typename T>
struct ScalarType<ProjectionMatrix4T<T>>
{
    using Type = T;
};

template <typename T>
struct ScalarType<QuaternionT<T>>
{
    using Type = T;
};


} // /namespace Gs


#endif



// ================================================================================
