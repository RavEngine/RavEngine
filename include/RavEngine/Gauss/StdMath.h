/*
 * StdMath.h
 * 
 * This file is part of the "GaussianLib" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef GS_STDMATH_H
#define GS_STDMATH_H


#include "Vector.h"
#include "Matrix.h"
#include <cmath>
#include <functional>


namespace Gs
{


/* --- Global Functions --- */

#define GS_DECL_STDMATH_FUNC1(NAME)                             \
    template <typename T, std::size_t N>                        \
    Vector<T, N> NAME(const Vector<T, N>& x)                    \
    {                                                           \
        Vector<T, N> y { UninitializeTag{} };                   \
        for (std::size_t i = 0; i < N; ++i)                     \
            y[i] = std::NAME(x[i]);                             \
        return y;                                               \
    }                                                           \
    template <typename T, std::size_t Rows, std::size_t Cols>   \
    Matrix<T, Rows, Cols> NAME(const Matrix<T, Rows, Cols>& x)  \
    {                                                           \
        Matrix<T, Rows, Cols> y { UninitializeTag{} };          \
        for (std::size_t i = 0; i < Rows*Cols; ++i)             \
            y[i] = std::NAME(x[i]);                             \
        return y;                                               \
    }

#define GS_DECL_STDMATH_FUNC2(NAME)                                                                 \
    template <typename T, std::size_t N>                                                            \
    Vector<T, N> NAME(const Vector<T, N>& x1, const Vector<T, N>& x2)                               \
    {                                                                                               \
        Vector<T, N> y { UninitializeTag{} };                                                       \
        for (std::size_t i = 0; i < N; ++i)                                                         \
            y[i] = std::NAME(x1[i], x2[i]);                                                         \
        return y;                                                                                   \
    }                                                                                               \
    template <typename T, std::size_t Rows, std::size_t Cols>                                       \
    Matrix<T, Rows, Cols> NAME(const Matrix<T, Rows, Cols>& x1, const Matrix<T, Rows, Cols>& x2)    \
    {                                                                                               \
        Matrix<T, Rows, Cols> y { UninitializeTag{} };                                              \
        for (std::size_t i = 0; i < Rows*Cols; ++i)                                                 \
            y[i] = std::NAME(x1[i], x2[i]);                                                         \
        return y;                                                                                   \
    }


GS_DECL_STDMATH_FUNC1( exp   )
GS_DECL_STDMATH_FUNC1( exp2  )
GS_DECL_STDMATH_FUNC1( log   )
GS_DECL_STDMATH_FUNC1( log10 )
GS_DECL_STDMATH_FUNC1( log2  )

GS_DECL_STDMATH_FUNC2( pow   )
GS_DECL_STDMATH_FUNC1( sqrt  )

GS_DECL_STDMATH_FUNC1( sin   )
GS_DECL_STDMATH_FUNC1( cos   )
GS_DECL_STDMATH_FUNC1( tan   )
GS_DECL_STDMATH_FUNC1( asin  )
GS_DECL_STDMATH_FUNC1( acos  )
GS_DECL_STDMATH_FUNC1( atan  )
GS_DECL_STDMATH_FUNC2( atan2 )

GS_DECL_STDMATH_FUNC1( sinh  )
GS_DECL_STDMATH_FUNC1( cosh  )
GS_DECL_STDMATH_FUNC1( tanh  )
GS_DECL_STDMATH_FUNC1( asinh )
GS_DECL_STDMATH_FUNC1( acosh )
GS_DECL_STDMATH_FUNC1( atanh )

GS_DECL_STDMATH_FUNC1( ceil  )
GS_DECL_STDMATH_FUNC1( floor )


#undef GS_DECL_STDMATH_FUNC1
#undef GS_DECL_STDMATH_FUNC2


} // /namespace Gs


#endif



// ================================================================================
