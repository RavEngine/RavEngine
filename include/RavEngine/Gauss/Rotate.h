/*
 * Rotate.h
 * 
 * This file is part of the "GaussianLib" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef GS_ROTATE_H
#define GS_ROTATE_H


#include "Decl.h"
#include "Macros.h"
#include "Vector3.h"


namespace Gs
{


namespace Details
{


template <class M, typename T>
void FreeRotation(M& mat, const Vector3T<T>& axis, const T& angle)
{
    GS_ASSERT_MxN_MATRIX("free rotation", M, 3, 3);

    /* Setup rotation values */
    const T  c  = std::cos(angle);
    const T  s  = std::sin(angle);
    const T  cc = T(1) - c;

    const T& x  = axis.x;
    const T& y  = axis.y;
    const T& z  = axis.z;

    /* Perform matrix rotation */
    mat.At(0, 0) = x*x*cc + c;
    mat.At(1, 0) = x*y*cc - z*s;
    mat.At(2, 0) = x*z*cc + y*s;

    mat.At(0, 1) = y*x*cc + z*s;
    mat.At(1, 1) = y*y*cc + c;
    mat.At(2, 1) = y*z*cc - x*s;

    mat.At(0, 2) = x*z*cc - y*s;
    mat.At(1, 2) = y*z*cc + x*s;
    mat.At(2, 2) = z*z*cc + c;
}


} // /namespace Details


/**
\brief Computes a free rotation around an axis and stores the result into the matrix 'm'.
\tparam M Specifies the matrix type. This should be Matrix3, Matrix4, or AffineMatrix4.
This type must implement the following interface:
\code
static const std::size_t rows;    // >= 3
static const std::size_t columns; // >= 3
\endcode
\tparam T Specifies the data type. This should should be float or double.
\param[in,out] mat Specifies the matrix which is to be rotated.
\param[in] axis Specifies the rotation axis. This must be normalized!
\param[in] angle Specifies the rotation angle (in radians).
*/
template <typename T>
void FreeRotation(Matrix<T, 4, 4>& mat, const Vector3T<T>& axis, const T& angle)
{
    Details::FreeRotation(mat, axis, angle);
}

template <typename T>
void FreeRotation(Matrix<T, 3, 3>& mat, const Vector3T<T>& axis, const T& angle)
{
    Details::FreeRotation(mat, axis, angle);
}

template <typename T>
void FreeRotation(AffineMatrix4T<T>& mat, const Vector3T<T>& axis, const T& angle)
{
    Details::FreeRotation(mat, axis, angle);
}

/**
\brief Rotates the matrix 'mat' around the specified axis.
\param[out] mat Specifies the resulting matrix.
\param[in] axis Specifies the rotation axis. This must be normalized!
\param[in] angle Specifies the rotation angle (in radians).
\see FreeRotation
*/
template <class M, typename T>
void RotateFree(M& mat, const Vector3T<T>& axis, const T& angle)
{
    auto rotation = M::Identity();
    FreeRotation(rotation, axis, angle);
    mat *= rotation;
}


} // /namespace Gs


#endif



// ================================================================================
