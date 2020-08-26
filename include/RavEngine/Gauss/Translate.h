/*
 * Translate.h
 * 
 * This file is part of the "GaussianLib" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef GS_TRANSLATE_H
#define GS_TRANSLATE_H


#include "Decl.h"
#include "Macros.h"
#include "Vector3.h"


namespace Gs
{


namespace Details
{


template <class M, typename T>
void Translate(M& m, const Vector3T<T>& v)
{
    GS_ASSERT_MxN_MATRIX("translation with column vectors", M, 3, 4);
    m.At(0, 3) += ( m.At(0, 0)*v.x + m.At(0, 1)*v.y + m.At(0, 2)*v.z );
    m.At(1, 3) += ( m.At(1, 0)*v.x + m.At(1, 1)*v.y + m.At(1, 2)*v.z );
    m.At(2, 3) += ( m.At(2, 0)*v.x + m.At(2, 1)*v.y + m.At(2, 2)*v.z );
}


} // /namespace Details


//! Translates the specified matrix 'm' by the vector 'v'.
template <typename T>
void Translate(Matrix<T, 4, 4>& m, const Vector3T<T>& v)
{
    /* Translate x, y, z */
    Details::Translate(m, v);

    /* Also translate w */
    m.At(3, 3) += ( m.At(3, 0)*v.x + m.At(3, 1)*v.y + m.At(3, 2)*v.z );
}

//! Computes the inverse of the specified affine 4x4 matrix 'm'.
template <typename T>
void Translate(AffineMatrix4T<T>& m, const Vector3T<T>& v)
{
    Details::Translate(m, v);
}


} // /namespace Gs


#endif



// ================================================================================
