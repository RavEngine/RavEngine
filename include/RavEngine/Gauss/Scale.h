/*
 * Scale.h
 * 
 * This file is part of the "GaussianLib" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef GS_SCALE_H
#define GS_SCALE_H


#include "Decl.h"


namespace Gs
{


namespace Details
{


template <class M, typename T>
void Scale3x3(M& m, const Vector3T<T>& v)
{
    GS_ASSERT_MxN_MATRIX("scale with 3D vector", M, 3, 3);

    m.At(0, 0) *= v.x;
    m.At(1, 0) *= v.x;
    m.At(2, 0) *= v.x;

    m.At(0, 1) *= v.y;
    m.At(1, 1) *= v.y;
    m.At(2, 1) *= v.y;

    m.At(0, 2) *= v.z;
    m.At(1, 2) *= v.z;
    m.At(2, 2) *= v.z;
}


} // /namespace Details


//! Translates the specified 4x4 matrix 'm' by the vector 'v'.
template <typename T>
void Scale(Matrix<T, 4, 4>& m, const Vector3T<T>& v)
{
    /* Scale left-upper 3x3 matrix */
    Details::Scale3x3(m, v);

    /* Also scale 4th row */
    m.At(3, 0) *= v.x;
    m.At(3, 1) *= v.y;
    m.At(3, 2) *= v.z;
}

//! Translates the specified 3x3 matrix 'm' by the vector 'v'.
template <typename T>
void Scale(Matrix<T, 3, 3>& m, const Vector3T<T>& v)
{
    Details::Scale3x3(m, v);
}

//! Computes the inverse of the specified affine 4x4 matrix 'm'.
template <typename T>
void Scale(AffineMatrix4T<T>& m, const Vector3T<T>& v)
{
    Details::Scale3x3(m, v);
}


} // /namespace Gs


#endif



// ================================================================================
