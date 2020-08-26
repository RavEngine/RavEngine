/*
 * Equals.h
 * 
 * This file is part of the "GaussianLib" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef GS_EQUALS_H
#define GS_EQUALS_H


#include "Real.h"
#include "Epsilon.h"
#include "Decl.h"

#include <cmath>


namespace Gs
{


/* --- Global Functions --- */

template <typename T>
inline bool Equals(const T& lhs, const T& rhs)
{
    return (lhs == rhs);
}

template <>
inline bool Equals<float>(const float& lhs, const float& rhs)
{
    return (std::abs(lhs - rhs) <= Epsilon<float>());
}

template <>
inline bool Equals<double>(const double& lhs, const double& rhs)
{
    return (std::abs(lhs - rhs) <= Epsilon<double>());
}

template <typename T, std::size_t N>
inline bool Equals(const Vector<T, N>& lhs, const Vector<T, N>& rhs)
{
    for (std::size_t i = 0; i < N; ++i)
    {
        if (!Equals(lhs[i], rhs[i]))
            return false;
    }
    return true;
}


/* --- Global Operators --- */

template <typename T, std::size_t N>
bool operator == (const Vector<T, N>& lhs, const Vector<T, N>& rhs)
{
    return Equals(lhs, rhs);
}

template <typename T, std::size_t N>
bool operator != (const Vector<T, N>& lhs, const Vector<T, N>& rhs)
{
    return !(lhs == rhs);
}


} // /namespace Gs


#endif



// ================================================================================
