/*
 * AffineMatrix.h
 * 
 * This file is part of the "GaussianLib" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef GS_AFFINE_MATRIX_H
#define GS_AFFINE_MATRIX_H


#include "Tags.h"

#include <cmath>
#include <cstring>
#include <algorithm>


namespace Gs
{

namespace Details
{


template <template <typename> class M, typename T>
M<T> MulAffineMatrices(const M<T>& lhs, const M<T>& rhs)
{
    M<T> result { UninitializeTag{} };

    #ifdef GS_ROW_VECTORS

    for (std::size_t c = 0; c < M<T>::columnsSparse; ++c)
    {
        for (std::size_t r = 0; r < M<T>::rowsSparse; ++r)
        {
            /* Only accumulate with 'rowsSparse' here! */
            result(r, c) = T(0);
            for (std::size_t i = 0; i < M<T>::columnsSparse; ++i)
                result(r, c) += lhs(r, i)*rhs(i, c);
        }

        /* Accumulate the rest of the current column of 'lhs' and the implicit 1 of 'rhs' */
        result(M<T>::rowsSparse - 1, c) += lhs(M<T>::rowsSparse - 1, c);
    }

    #else

    for (std::size_t r = 0; r < M<T>::rowsSparse; ++r)
    {
        for (std::size_t c = 0; c < M<T>::columnsSparse; ++c)
        {
            /* Only accumulate with 'rowsSparse' here! */
            result(r, c) = T(0);
            for (std::size_t i = 0; i < M<T>::rowsSparse; ++i)
                result(r, c) += lhs(r, i)*rhs(i, c);
        }

        /* Accumulate the rest of the current row of 'lhs' and the implicit 1 of 'rhs' */
        result(r, M<T>::columnsSparse - 1) += lhs(r, M<T>::columnsSparse - 1);
    }

    #endif

    return result;
}


} // /namespace Details

} // /namespace Gs


#endif



// ================================================================================
