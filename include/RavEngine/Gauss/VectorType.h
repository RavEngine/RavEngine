/*
 * VectorType.h
 * 
 * This file is part of the "GaussianLib" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef GS_VECTOR_TYPE_H
#define GS_VECTOR_TYPE_H


#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

#include <vector>


namespace Gs
{


template <typename T, std::size_t N>
struct VectorType
{
    using Vector = std::vector<T>;
};

template <typename T>
struct VectorType<T, 2u>
{
    using Vector = Vector2T<T>;
};

template <typename T>
struct VectorType<T, 3u>
{
    using Vector = Vector3T<T>;
};

template <typename T>
struct VectorType<T, 4u>
{
    using Vector = Vector4T<T>;
};


} // /namespace Gs


#endif



// ================================================================================
