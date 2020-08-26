/*
 * RotateVector.h
 * 
 * This file is part of the "GaussianLib" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef GS_ROTATE_VECTOR_H
#define GS_ROTATE_VECTOR_H


#include "Decl.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Algebra.h"


namespace Gs
{


template <class M, typename T>
Vector2T<T> RotateVector(const M& mat, const Vector2T<T>& vec)
{
    GS_ASSERT_MxN_MATRIX("2D vector rotation by matrix", M, 2, 2);
    return Vector2T<T>(
        vec.x*mat.At(0, 0) + vec.y*mat.At(0, 1),
        vec.x*mat.At(1, 0) + vec.y*mat.At(1, 1)
    );
}

template <class M, typename T>
Vector3T<T> RotateVector(const M& mat, const Vector3T<T>& vec)
{
    GS_ASSERT_MxN_MATRIX("2D vector rotation by matrix", M, 3, 3);
    return Vector3T<T>(
        vec.x*mat.At(0, 0) + vec.y*mat.At(0, 1) + vec.z*mat.At(0, 2),
        vec.x*mat.At(1, 0) + vec.y*mat.At(1, 1) + vec.z*mat.At(1, 2),
        vec.x*mat.At(2, 0) + vec.y*mat.At(2, 1) + vec.z*mat.At(2, 2)
    );
}

template <class M, typename T>
Vector2T<T> RotateVectorInverse(const M& mat, const Vector2T<T>& vec)
{
    GS_ASSERT_MxN_MATRIX("2D vector inverse rotation by matrix", M, 2, 2);
    return Vector2T<T>(
        vec.x*mat.At(0, 0) + vec.y*mat.At(1, 0),
        vec.x*mat.At(0, 1) + vec.y*mat.At(1, 1)
    );
}

template <class M, typename T>
Vector3T<T> RotateVectorInverse(const M& mat, const Vector3T<T>& vec)
{
    GS_ASSERT_MxN_MATRIX("2D vector inverse rotation by matrix", M, 3, 3);
    return Vector3T<T>(
        vec.x*mat.At(0, 0) + vec.y*mat.At(1, 0) + vec.z*mat.At(2, 0),
        vec.x*mat.At(0, 1) + vec.y*mat.At(1, 1) + vec.z*mat.At(2, 1),
        vec.x*mat.At(0, 2) + vec.y*mat.At(1, 2) + vec.z*mat.At(2, 2)
    );
}

/**
\brief Rotates the specified vector 'vec' around the vector 'axis'.
\param[in] vec Specifies the vector which is to be rotated.
\param[in] axis Specifies the axis vector to rotate around.
\param[in] angle Specifies the rotation angle (in radians).
\return The new rotated vector.
*/
template <typename T>
Vector3T<T> RotateVectorAroundAxis(const Vector3T<T>& vec, Vector3T<T> axis, Real angle)
{
    axis.Normalize();

    auto s       = std::sin(angle);
    auto c       = std::cos(angle);
    auto cInv    = Real(1) - c;

    Vector3T<T> row0, row1, row2;

    row0.x = axis.x*axis.x + c*(Real(1) - axis.x*axis.x);
    row0.y = axis.x*axis.y*cInv - s*axis.z;
    row0.z = axis.x*axis.z*cInv + s*axis.y;

    row1.x = axis.x*axis.y*cInv + s*axis.z;
    row1.y = axis.y*axis.y + c*(Real(1) - axis.y*axis.y);
    row1.z = axis.y*axis.z*cInv - s*axis.x;

    row2.x = axis.x*axis.z*cInv - s*axis.y;
    row2.y = axis.y*axis.z*cInv + s*axis.x;
    row2.z = axis.z*axis.z + c*(Real(1) - axis.z*axis.z);

    return Vector3T<T>(
        Dot(vec, row0),
        Dot(vec, row1),
        Dot(vec, row2)
    );
}


} // /namespace Gs


#endif



// ================================================================================
