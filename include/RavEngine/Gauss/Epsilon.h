/*
 * Epsilon.h
 * 
 * This file is part of the "GaussianLib" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef GS_EPSILON_H
#define GS_EPSILON_H


#include "Real.h"

#include <type_traits>


namespace Gs
{


#define GS_EPSILON_F32 ( 1.0e-6f ) // 0.000001f
#define GS_EPSILON_F64 ( 1.0e-8  ) // 0.00000001

/**
Function with a return value which is very small (~0.000001)
which can be used for zero comparision with floating-point values.
\code
bool IsNearlyZero(float x)
{
    return std::abs(x) <= Gs::Epsilon<float>();
}
\endcode
\tparam T Specifies the data type. This function is only defined for float and double!
*/
template <typename T>
inline T Epsilon()
{
    static_assert(std::is_integral<T>::value, "'Gs::Epsilon' function only allows floating-point types");
    return T(GS_EPSILON_F32);
}

template <>
inline float Epsilon()
{
    return GS_EPSILON_F32;
}

template <>
inline double Epsilon()
{
    return GS_EPSILON_F64;
}


} // /namespace Gs


#endif



// ================================================================================
