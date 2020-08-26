/*
 * Vector.h
 * 
 * This file is part of the "GaussianLib" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef GS_VECTOR_H
#define GS_VECTOR_H


#include "Real.h"
#include "Assert.h"
#include "Tags.h"

#include <algorithm>
#include <iterator>
#include <cstdint>


namespace Gs
{


/**
\brief Base vector class with N components.
\tparam T Specifies the data type of the vector components.
This should be a primitive data type such as float, double, int etc.
\tparam N Specifies the number of components. There are specialized templates for N = 2, 3, and 4.
*/
template <typename T, std::size_t N>
class Vector
{

    public:

        //! Specifies the typename of the scalar components.
        using ScalarType = T;

        //! Specifies the number of vector components.
        static const std::size_t components = N;

        #ifndef GS_DISABLE_AUTO_INIT
        Vector()
        {
            std::fill(std::begin(v_), std::end(v_), T(0));
        }
        #else
        Vector() = default;
        #endif

        Vector(const Vector<T, N>& rhs)
        {
            std::copy(std::begin(rhs.v_), std::end(rhs.v_), v_);
        }

        explicit Vector(const T& scalar)
        {
            std::fill(std::begin(v_), std::end(v_), scalar);
        }

        explicit Vector(UninitializeTag)
        {
            // do nothing
        }

        Vector<T, N>& operator += (const Vector<T, N>& rhs)
        {
            for (std::size_t i = 0; i < N; ++i)
                v_[i] += rhs[i];
            return *this;
        }

        Vector<T, N>& operator -= (const Vector<T, N>& rhs)
        {
            for (std::size_t i = 0; i < N; ++i)
                v_[i] -= rhs[i];
            return *this;
        }

        Vector<T, N>& operator *= (const Vector<T, N>& rhs)
        {
            for (std::size_t i = 0; i < N; ++i)
                v_[i] *= rhs[i];
            return *this;
        }

        Vector<T, N>& operator /= (const Vector<T, N>& rhs)
        {
            for (std::size_t i = 0; i < N; ++i)
                v_[i] /= rhs[i];
            return *this;
        }

        Vector<T, N>& operator *= (const T rhs)
        {
            for (std::size_t i = 0; i < N; ++i)
                v_[i] *= rhs;
            return *this;
        }

        Vector<T, N>& operator /= (const T rhs)
        {
            for (std::size_t i = 0; i < N; ++i)
                v_[i] /= rhs;
            return *this;
        }

        /**
        \brief Returns the specified vector component.
        \param[in] component Specifies the vector component index. This must be in the range [0, N).
        */
        T& operator [] (std::size_t component)
        {
            GS_ASSERT(component < N);
            return v_[component];
        }

        /**
        \brief Returns the specified vector component.
        \param[in] component Specifies the vector component index. This must be in the range [0, N).
        */
        const T& operator [] (std::size_t component) const
        {
            GS_ASSERT(component < N);
            return v_[component];
        }

        Vector<T, N> operator - () const
        {
            auto result = *this;
            for (std::size_t i = 0; i < N; ++i)
                result[i] = -result[i];
            return result;
        }

        /**
        Returns a type casted instance of this vector.
        \tparam C Specifies the static cast type.
        */
        template <typename C>
        Vector<C, N> Cast() const
        {
            Vector<C, N> result { UninitializeTag{} };

            for (std::size_t i = 0; i < N; ++i)
                result[i] = static_cast<C>(v_[i]);

            return result;
        }

        //! Returns a pointer to the first element of this vector.
        T* Ptr()
        {
            return v_;
        }

        //! Returns a constant pointer to the first element of this vector.
        const T* Ptr() const
        {
            return v_;
        }

    private:

        T v_[N];

};


/* --- Global Operators --- */

template <typename T, std::size_t N>
Vector<T, N> operator + (const Vector<T, N>& lhs, const Vector<T, N>& rhs)
{
    auto result = lhs;
    result += rhs;
    return result;
}

template <typename T, std::size_t N>
Vector<T, N> operator - (const Vector<T, N>& lhs, const Vector<T, N>& rhs)
{
    auto result = lhs;
    result -= rhs;
    return result;
}

template <typename T, std::size_t N>
Vector<T, N> operator * (const Vector<T, N>& lhs, const Vector<T, N>& rhs)
{
    auto result = lhs;
    result *= rhs;
    return result;
}

template <typename T, std::size_t N>
Vector<T, N> operator / (const Vector<T, N>& lhs, const Vector<T, N>& rhs)
{
    auto result = lhs;
    result /= rhs;
    return result;
}

template <typename T, std::size_t N>
Vector<T, N> operator * (const Vector<T, N>& lhs, const T& rhs)
{
    auto result = lhs;
    result *= rhs;
    return result;
}

//! \note This implementation is equivavlent to (rhs * lhs) for optimization purposes.
template <typename T, std::size_t N>
Vector<T, N> operator * (const T& lhs, const Vector<T, N>& rhs)
{
    auto result = rhs;
    result *= lhs;
    return result;
}

template <typename T, std::size_t N>
Vector<T, N> operator / (const Vector<T, N>& lhs, const T& rhs)
{
    auto result = lhs;
    result /= rhs;
    return result;
}

template <typename T, std::size_t N>
Vector<T, N> operator / (const T& lhs, const Vector<T, N>& rhs)
{
    auto result = Vector<T, N> { lhs };
    result /= rhs;
    return result;
}


} // /namespace Gs


#endif



// ================================================================================
