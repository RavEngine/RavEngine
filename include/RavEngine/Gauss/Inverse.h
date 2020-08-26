/*
 * Inverse.h
 * 
 * This file is part of the "GaussianLib" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef GS_INVERSE_H
#define GS_INVERSE_H


#include "Decl.h"
#include "Details.h"
#include "Determinant.h"
#include "Assert.h"


namespace Gs
{


/**
\brief Computes the inverse of the specified matrix 'm'.
\todo Not yet implemented!
*/
template <typename T, std::size_t N>
bool Inverse(Matrix<T, N, N>& inv, const Matrix<T, N, N>& m)
{
    //using Helper = Details::MatrixHelper<M, T, Rows, Cols>;
    //return Helper::OrderedInverse(inv, Helper::MatrixToArray(m), Rows);
    return false;//!!!
}

//! Computes the inverse of the specified 2x2 matrix 'm'.
template <typename T>
bool Inverse(Matrix<T, 2, 2>& inv, const Matrix<T, 2, 2>& m)
{
    /* Compute inverse determinant */
    T d = Determinant(m);

    if (d == T(0))
        return false;

    d = T(1) / d;

    /* Compute inverse matrix */
    inv.At(0, 0) = d * (  m.At(1, 1) );
    inv.At(1, 0) = d * ( -m.At(1, 0) );

    inv.At(0, 1) = d * ( -m.At(0, 1) );
    inv.At(1, 1) = d * (  m.At(0, 0) );

    return true;
}

//! Computes the inverse of the specified 3x3 matrix 'm'.
template <typename T>
bool Inverse(Matrix<T, 3, 3>& inv, const Matrix<T, 3, 3>& m)
{
    /* Compute inverse determinant */
    T d = Determinant(m);

    if (d == T(0))
        return false;

    d = T(1) / d;

    /* Compute inverse matrix */
    inv.At(0, 0) = d * ( m.At(1, 1) * m.At(2, 2) - m.At(2, 1) * m.At(1, 2) );
    inv.At(1, 0) = d * ( m.At(1, 2) * m.At(2, 0) - m.At(1, 0) * m.At(2, 2) );
    inv.At(2, 0) = d * ( m.At(1, 0) * m.At(2, 1) - m.At(2, 0) * m.At(1, 1) );

    inv.At(0, 1) = d * ( m.At(0, 2) * m.At(2, 1) - m.At(0, 1) * m.At(2, 2) );
    inv.At(1, 1) = d * ( m.At(0, 0) * m.At(2, 2) - m.At(0, 2) * m.At(2, 0) );
    inv.At(2, 1) = d * ( m.At(2, 0) * m.At(0, 1) - m.At(0, 0) * m.At(2, 1) );

    inv.At(0, 2) = d * ( m.At(0, 1) * m.At(1, 2) - m.At(0, 2) * m.At(1, 1) );
    inv.At(1, 2) = d * ( m.At(1, 0) * m.At(0, 2) - m.At(0, 0) * m.At(1, 2) );
    inv.At(2, 2) = d * ( m.At(0, 0) * m.At(1, 1) - m.At(1, 0) * m.At(0, 1) );

    return true;
}

//! Computes the inverse of the specified 4x4 matrix 'm'.
template <typename T>
bool Inverse(Matrix<T, 4, 4>& inv, const Matrix<T, 4, 4>& m)
{
    /* Compute inverse determinant */
    T d = Determinant(m);

    if (d == T(0))
        return false;

    d = T(1) / d;

    /* Compute inverse matrix */
    inv.At(0, 0) = d * ( m.At(1, 1) * (m.At(2, 2) * m.At(3, 3) - m.At(3, 2) * m.At(2, 3)) + m.At(2, 1) * (m.At(3, 2) * m.At(1, 3) - m.At(1, 2) * m.At(3, 3)) + m.At(3, 1) * (m.At(1, 2) * m.At(2, 3) - m.At(2, 2) * m.At(1, 3)) );
    inv.At(1, 0) = d * ( m.At(1, 2) * (m.At(2, 0) * m.At(3, 3) - m.At(3, 0) * m.At(2, 3)) + m.At(2, 2) * (m.At(3, 0) * m.At(1, 3) - m.At(1, 0) * m.At(3, 3)) + m.At(3, 2) * (m.At(1, 0) * m.At(2, 3) - m.At(2, 0) * m.At(1, 3)) );
    inv.At(2, 0) = d * ( m.At(1, 3) * (m.At(2, 0) * m.At(3, 1) - m.At(3, 0) * m.At(2, 1)) + m.At(2, 3) * (m.At(3, 0) * m.At(1, 1) - m.At(1, 0) * m.At(3, 1)) + m.At(3, 3) * (m.At(1, 0) * m.At(2, 1) - m.At(2, 0) * m.At(1, 1)) );
    inv.At(3, 0) = d * ( m.At(1, 0) * (m.At(3, 1) * m.At(2, 2) - m.At(2, 1) * m.At(3, 2)) + m.At(2, 0) * (m.At(1, 1) * m.At(3, 2) - m.At(3, 1) * m.At(1, 2)) + m.At(3, 0) * (m.At(2, 1) * m.At(1, 2) - m.At(1, 1) * m.At(2, 2)) );

    inv.At(0, 1) = d * ( m.At(2, 1) * (m.At(0, 2) * m.At(3, 3) - m.At(3, 2) * m.At(0, 3)) + m.At(3, 1) * (m.At(2, 2) * m.At(0, 3) - m.At(0, 2) * m.At(2, 3)) + m.At(0, 1) * (m.At(3, 2) * m.At(2, 3) - m.At(2, 2) * m.At(3, 3)) );
    inv.At(1, 1) = d * ( m.At(2, 2) * (m.At(0, 0) * m.At(3, 3) - m.At(3, 0) * m.At(0, 3)) + m.At(3, 2) * (m.At(2, 0) * m.At(0, 3) - m.At(0, 0) * m.At(2, 3)) + m.At(0, 2) * (m.At(3, 0) * m.At(2, 3) - m.At(2, 0) * m.At(3, 3)) );
    inv.At(2, 1) = d * ( m.At(2, 3) * (m.At(0, 0) * m.At(3, 1) - m.At(3, 0) * m.At(0, 1)) + m.At(3, 3) * (m.At(2, 0) * m.At(0, 1) - m.At(0, 0) * m.At(2, 1)) + m.At(0, 3) * (m.At(3, 0) * m.At(2, 1) - m.At(2, 0) * m.At(3, 1)) );
    inv.At(3, 1) = d * ( m.At(2, 0) * (m.At(3, 1) * m.At(0, 2) - m.At(0, 1) * m.At(3, 2)) + m.At(3, 0) * (m.At(0, 1) * m.At(2, 2) - m.At(2, 1) * m.At(0, 2)) + m.At(0, 0) * (m.At(2, 1) * m.At(3, 2) - m.At(3, 1) * m.At(2, 2)) );

    inv.At(0, 2) = d * ( m.At(3, 1) * (m.At(0, 2) * m.At(1, 3) - m.At(1, 2) * m.At(0, 3)) + m.At(0, 1) * (m.At(1, 2) * m.At(3, 3) - m.At(3, 2) * m.At(1, 3)) + m.At(1, 1) * (m.At(3, 2) * m.At(0, 3) - m.At(0, 2) * m.At(3, 3)) );
    inv.At(1, 2) = d * ( m.At(3, 2) * (m.At(0, 0) * m.At(1, 3) - m.At(1, 0) * m.At(0, 3)) + m.At(0, 2) * (m.At(1, 0) * m.At(3, 3) - m.At(3, 0) * m.At(1, 3)) + m.At(1, 2) * (m.At(3, 0) * m.At(0, 3) - m.At(0, 0) * m.At(3, 3)) );
    inv.At(2, 2) = d * ( m.At(3, 3) * (m.At(0, 0) * m.At(1, 1) - m.At(1, 0) * m.At(0, 1)) + m.At(0, 3) * (m.At(1, 0) * m.At(3, 1) - m.At(3, 0) * m.At(1, 1)) + m.At(1, 3) * (m.At(3, 0) * m.At(0, 1) - m.At(0, 0) * m.At(3, 1)) );
    inv.At(3, 2) = d * ( m.At(3, 0) * (m.At(1, 1) * m.At(0, 2) - m.At(0, 1) * m.At(1, 2)) + m.At(0, 0) * (m.At(3, 1) * m.At(1, 2) - m.At(1, 1) * m.At(3, 2)) + m.At(1, 0) * (m.At(0, 1) * m.At(3, 2) - m.At(3, 1) * m.At(0, 2)) );

    inv.At(0, 3) = d * ( m.At(0, 1) * (m.At(2, 2) * m.At(1, 3) - m.At(1, 2) * m.At(2, 3)) + m.At(1, 1) * (m.At(0, 2) * m.At(2, 3) - m.At(2, 2) * m.At(0, 3)) + m.At(2, 1) * (m.At(1, 2) * m.At(0, 3) - m.At(0, 2) * m.At(1, 3)) );
    inv.At(1, 3) = d * ( m.At(0, 2) * (m.At(2, 0) * m.At(1, 3) - m.At(1, 0) * m.At(2, 3)) + m.At(1, 2) * (m.At(0, 0) * m.At(2, 3) - m.At(2, 0) * m.At(0, 3)) + m.At(2, 2) * (m.At(1, 0) * m.At(0, 3) - m.At(0, 0) * m.At(1, 3)) );
    inv.At(2, 3) = d * ( m.At(0, 3) * (m.At(2, 0) * m.At(1, 1) - m.At(1, 0) * m.At(2, 1)) + m.At(1, 3) * (m.At(0, 0) * m.At(2, 1) - m.At(2, 0) * m.At(0, 1)) + m.At(2, 3) * (m.At(1, 0) * m.At(0, 1) - m.At(0, 0) * m.At(1, 1)) );
    inv.At(3, 3) = d * ( m.At(0, 0) * (m.At(1, 1) * m.At(2, 2) - m.At(2, 1) * m.At(1, 2)) + m.At(1, 0) * (m.At(2, 1) * m.At(0, 2) - m.At(0, 1) * m.At(2, 2)) + m.At(2, 0) * (m.At(0, 1) * m.At(1, 2) - m.At(1, 1) * m.At(0, 2)) );

    return true;
}

//! Computes the inverse of the specified affine 3x3 matrix 'm'.
template <typename T> bool Inverse(AffineMatrix3T<T>& inv, const AffineMatrix3T<T>& m)
{
    /* Compute inverse determinant */
    T d = Determinant(m);

    if (d == T(0))
        return false;

    d = T(1) / d;

    /* Compute inverse matrix */
    inv.At(0, 0) = d * (  m.At(1, 1) );
    inv.At(1, 0) = d * ( -m.At(1, 0) );
  /*inv.At(2, 0) = 0*/;

    inv.At(0, 1) = d * ( -m.At(0, 1) );
    inv.At(1, 1) = d * (  m.At(0, 0) );
  /*inv.At(2, 1) = 0*/;

    inv.At(0, 2) = d * ( m.At(0, 1) * m.At(1, 2) - m.At(0, 2) * m.At(1, 1) );
    inv.At(1, 2) = d * ( m.At(1, 0) * m.At(0, 2) - m.At(0, 0) * m.At(1, 2) );
  /*inv.At(2, 2) = 1*/;

    return true;
}

//! Computes the inverse of the specified affine 4x4 matrix 'm'.
template <typename T> bool Inverse(AffineMatrix4T<T>& inv, const AffineMatrix4T<T>& m)
{
    /* Compute inverse determinant */
    T d = Determinant(m);

    if (d == T(0))
        return false;

    d = T(1) / d;

    /* Compute inverse matrix */
    inv.At(0, 0) = d * ( m.At(1, 1) * m.At(2, 2) + m.At(2, 1) * ( -m.At(1, 2) ) );
    inv.At(1, 0) = d * ( m.At(1, 2) * m.At(2, 0) + m.At(2, 2) * ( -m.At(1, 0) ) );
    inv.At(2, 0) = d * ( m.At(1, 0) * m.At(2, 1) - m.At(2, 0) * m.At(1, 1) );
  /*inv.At(3, 0) = 0;*/

    inv.At(0, 1) = d * ( m.At(2, 1) * m.At(0, 2) + m.At(0, 1) * ( -m.At(2, 2) ) );
    inv.At(1, 1) = d * ( m.At(2, 2) * m.At(0, 0) + m.At(0, 2) * ( -m.At(2, 0) ) );
    inv.At(2, 1) = d * ( m.At(2, 0) * m.At(0, 1) - m.At(0, 0) * m.At(2, 1) );
  /*inv.At(3, 1) = 0;*/

    inv.At(0, 2) = d * ( m.At(0, 1) * m.At(1, 2) + m.At(1, 1) * ( -m.At(0, 2) ) );
    inv.At(1, 2) = d * ( m.At(0, 2) * m.At(1, 0) + m.At(1, 2) * ( -m.At(0, 0) ) );
    inv.At(2, 2) = d * ( m.At(0, 0) * m.At(1, 1) - m.At(1, 0) * m.At(0, 1) );
  /*inv.At(3, 2) = 0;*/

    inv.At(0, 3) = d * ( m.At(0, 1) * (m.At(2, 2) * m.At(1, 3) - m.At(1, 2) * m.At(2, 3)) + m.At(1, 1) * (m.At(0, 2) * m.At(2, 3) - m.At(2, 2) * m.At(0, 3)) + m.At(2, 1) * (m.At(1, 2) * m.At(0, 3) - m.At(0, 2) * m.At(1, 3)) );
    inv.At(1, 3) = d * ( m.At(0, 2) * (m.At(2, 0) * m.At(1, 3) - m.At(1, 0) * m.At(2, 3)) + m.At(1, 2) * (m.At(0, 0) * m.At(2, 3) - m.At(2, 0) * m.At(0, 3)) + m.At(2, 2) * (m.At(1, 0) * m.At(0, 3) - m.At(0, 0) * m.At(1, 3)) );
    inv.At(2, 3) = d * ( m.At(0, 3) * (m.At(2, 0) * m.At(1, 1) - m.At(1, 0) * m.At(2, 1)) + m.At(1, 3) * (m.At(0, 0) * m.At(2, 1) - m.At(2, 0) * m.At(0, 1)) + m.At(2, 3) * (m.At(1, 0) * m.At(0, 1) - m.At(0, 0) * m.At(1, 1)) );
  /*inv.At(3, 3) = 1;*/

    return true;
}

//! Computes the inverse of the specified projection 4x4 matrix 'm'.
template <typename T>
bool Inverse(ProjectionMatrix4T<T>& inv, const ProjectionMatrix4T<T>& m)
{
    /* Compute inverse determinant */
    T d = Determinant(m);

    if (d == T(0))
        return false;

    d = T(1) / d;

    #ifdef GS_ROW_VECTORS

    /* Compute inverse matrix */
    inv.m00 = d * ( m.m11 * ( m.m22 * m.m33 - m.m23 * m.m32 ) );
  /*inv.m01 = 0;*/
  /*inv.m02 = 0;*/
  /*inv.m03 = 0;*/

  /*inv.m10 = 0;*/
    inv.m11 = d * ( m.m22 * m.m00 * m.m33 + m.m23 * ( -m.m00 * m.m32 ) );
  /*inv.m12 = 0;*/
  /*inv.m13 = 0;*/

  /*inv.m20 = 0;*/
  /*inv.m21 = 0;*/
    inv.m22 = d * ( m.m33 * (  m.m00 * m.m11 ) );
    inv.m23 = d * ( m.m00 * ( -m.m11 * m.m23 ) );

  /*inv.m30 = 0;*/
  /*inv.m31 = 0;*/
    inv.m32 = d * ( m.m32 * ( -m.m00 * m.m11 ) );
    inv.m33 = d * ( m.m00 * (  m.m11 * m.m22 ) );

    #else

    /* Compute inverse matrix */
    inv.m00 = d * ( m.m11 * ( m.m22 * m.m33 - m.m32 * m.m23 ) );
  /*inv.m10 = 0;*/
  /*inv.m20 = 0;*/
  /*inv.m30 = 0;*/

  /*inv.m01 = 0;*/
    inv.m11 = d * ( m.m22 * m.m00 * m.m33 + m.m32 * ( -m.m00 * m.m23 ) );
  /*inv.m21 = 0;*/
  /*inv.m31 = 0;*/

  /*inv.m02 = 0;*/
  /*inv.m12 = 0;*/
    inv.m22 = d * ( m.m33 * (  m.m00 * m.m11 ) );
    inv.m32 = d * ( m.m00 * ( -m.m11 * m.m32 ) );

  /*inv.m03 = 0;*/
  /*inv.m13 = 0;*/
    inv.m23 = d * ( m.m23 * ( -m.m00 * m.m11 ) );
    inv.m33 = d * ( m.m00 * (  m.m11 * m.m22 ) );

    #endif

    return true;
}

#ifdef GS_ENABLE_INVERSE_OPERATOR

namespace Details
{

template <class M>
M InverseOp(const M& m, int e)
{
    GS_ASSERT(e == -1);
    return m.Inverse();
}

} // /namespace Details

//! Inverse matrix operator. Input parameter 'e' must always be -1.
template <typename T, std::size_t N>
Matrix<T, N, N> operator ^ (const Matrix<T, N, N>& m, int e)
{
    return Details::InverseOp(m, e);
}

//! Inverse matrix operator. Input parameter 'e' must always be -1.
template <typename T>
AffineMatrix3T<T> operator ^ (const AffineMatrix3T<T>& m, int e)
{
    return Details::InverseOp(m, e);
}

//! Inverse matrix operator. Input parameter 'e' must always be -1.
template <typename T>
AffineMatrix4T<T> operator ^ (const AffineMatrix4T<T>& m, int e)
{
    return Details::InverseOp(m, e);
}

//! Inverse matrix operator. Input parameter 'e' must always be -1.
template <typename T>
ProjectionMatrix4T<T> operator ^ (const ProjectionMatrix4T<T>& m, int e)
{
    return Details::InverseOp(m, e);
}

#endif


} // /namespace Gs


#endif



// ================================================================================
