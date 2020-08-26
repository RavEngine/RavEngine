/*
 * TransformVector.h
 * 
 * This file is part of the "GaussianLib" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef GS_TRANSFORM_VECTOR_H
#define GS_TRANSFORM_VECTOR_H


#include "Decl.h"
#include "Macros.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"


namespace Gs
{


template <typename M, typename T>
Vector2T<T> TransformVector(const M& mat, const Vector2T<T>& vec)
{
    GS_ASSERT_MxN_MATRIX("2D vector transformation by matrix", M, 2, 2);
    return Vector2T<T>(
        vec.x*mat.At(0, 0) + vec.y*mat.At(0, 1) + mat.At(0, 2),
        vec.x*mat.At(1, 0) + vec.y*mat.At(1, 1) + mat.At(1, 2)
    );
}

template <typename M, typename T>
Vector3T<T> TransformVector(const M& mat, const Vector3T<T>& vec)
{
    GS_ASSERT_MxN_MATRIX("3D vector transformation by matrix", M, 4, 3);
    return Vector3T<T>(
        vec.x*mat.At(0, 0) + vec.y*mat.At(0, 1) + vec.z*mat.At(0, 2) + mat.At(0, 3),
        vec.x*mat.At(1, 0) + vec.y*mat.At(1, 1) + vec.z*mat.At(1, 2) + mat.At(1, 3),
        vec.x*mat.At(2, 0) + vec.y*mat.At(2, 1) + vec.z*mat.At(2, 2) + mat.At(2, 3)
    );
}

template <typename M, typename T>
Vector4T<T> TransformVector(const M& mat, const Vector4T<T>& vec)
{
    GS_ASSERT_MxN_MATRIX("4D vector transformation by matrix", M, 4, 4);
    return Vector4T<T>(
        vec.x*mat.At(0, 0) + vec.y*mat.At(0, 1) + vec.z*mat.At(0, 2) + vec.w*mat.At(0, 3),
        vec.x*mat.At(1, 0) + vec.y*mat.At(1, 1) + vec.z*mat.At(1, 2) + vec.w*mat.At(1, 3),
        vec.x*mat.At(2, 0) + vec.y*mat.At(2, 1) + vec.z*mat.At(2, 2) + vec.w*mat.At(2, 3),
        vec.x*mat.At(3, 0) + vec.y*mat.At(3, 1) + vec.z*mat.At(3, 2) + vec.w*mat.At(3, 3)
    );
}

template <typename T>
Vector4T<T> TransformVector(const AffineMatrix4T<T>& mat, const Vector4T<T>& vec)
{
    return Vector4T<T>(
        vec.x*mat.At(0, 0) + vec.y*mat.At(0, 1) + vec.z*mat.At(0, 2) + vec.w*mat.At(0, 3),
        vec.x*mat.At(1, 0) + vec.y*mat.At(1, 1) + vec.z*mat.At(1, 2) + vec.w*mat.At(1, 3),
        vec.x*mat.At(2, 0) + vec.y*mat.At(2, 1) + vec.z*mat.At(2, 2) + vec.w*mat.At(2, 3),
        vec.w
    );
}


} // /namespace Gs


#endif



// ================================================================================
