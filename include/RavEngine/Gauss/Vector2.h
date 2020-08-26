/*
 * Vector2.h
 * 
 * This file is part of the "GaussianLib" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef GS_VECTOR2_H
#define GS_VECTOR2_H


#include "Vector.h"
#include "Algebra.h"
#include "Swizzle.h"

#include <cmath>


namespace Gs
{


/**
\brief Base 2D vector class with components: x, and y.
\tparam T Specifies the data type of the vector components.
This should be a primitive data type such as float, double, int etc.
*/
template <typename T>
class Vector<T, 2>
{

    public:

        //! Specifies the typename of the scalar components.
        using ScalarType = T;

        //! Specifies the number of vector components.
        static const std::size_t components = 2;

        #ifndef GS_DISABLE_AUTO_INIT
        Vector() :
            x { T(0) },
            y { T(0) }
        {
        }
        #else
        Vector() = default;
        #endif

        Vector(const Vector<T, 2>& rhs) :
            x { rhs.x },
            y { rhs.y }
        {
        }

        explicit Vector(const Vector<T, 3>& rhs) :
            x { rhs.x },
            y { rhs.y }
        {
        }

        explicit Vector(const Vector<T, 4>& rhs) :
            x { rhs.x },
            y { rhs.y }
        {
        }

        explicit Vector(const T& scalar) :
            x { scalar },
            y { scalar }
        {
        }

        Vector(const T& x, const T& y) :
            x { x },
            y { y }
        {
        }

        explicit Vector(UninitializeTag)
        {
            // do nothing
        }

        Vector<T, 2>& operator += (const Vector<T, 2>& rhs)
        {
            x += rhs.x;
            y += rhs.y;
            return *this;
        }

        Vector<T, 2>& operator -= (const Vector<T, 2>& rhs)
        {
            x -= rhs.x;
            y -= rhs.y;
            return *this;
        }

        Vector<T, 2>& operator *= (const Vector<T, 2>& rhs)
        {
            x *= rhs.x;
            y *= rhs.y;
            return *this;
        }

        Vector<T, 2>& operator /= (const Vector<T, 2>& rhs)
        {
            x /= rhs.x;
            y /= rhs.y;
            return *this;
        }

        Vector<T, 2>& operator *= (const T rhs)
        {
            x *= rhs;
            y *= rhs;
            return *this;
        }

        Vector<T, 2>& operator /= (const T rhs)
        {
            x /= rhs;
            y /= rhs;
            return *this;
        }

        Vector<T, 2> operator - () const
        {
            return Vector<T, 2> { -x, -y };
        }

        /**
        \brief Returns the specified vector component.
        \param[in] component Specifies the vector component index. This must be 0, or 1.
        */
        T& operator [] (std::size_t component)
        {
            GS_ASSERT(component < (Vector<T, 2>::components));
            return *((&x) + component);
        }

        /**
        \brief Returns the specified vector component.
        \param[in] component Specifies the vector component index. This must be 0, or 1.
        */
        const T& operator [] (std::size_t component) const
        {
            GS_ASSERT(component < (Vector<T, 2>::components));
            return *((&x) + component);
        }

        //! Returns the squared length of this vector.
        T LengthSq() const
        {
            return Gs::LengthSq(*this);
        }

        //! Returns the length of this vector.
        T Length() const
        {
            return Gs::Length(*this);
        }

        /**
        Normalizes this vector to the unit length of 1.
        \see Normalized
        \see Length
        \see Resize
        */
        void Normalize()
        {
            Gs::Normalize(*this);
        }

        /**
        Returns a normalized instance of this vector.
        \see Normalize
        */
        Vector<T, 2> Normalized() const
        {
            auto vec = *this;
            vec.Normalize();
            return vec;
        }

        /**
        Resizes this vector to the specified length.
        \see Normalize
        \see Length
        */
        void Resize(const T& length)
        {
            Gs::Resize(*this, length);
        }

        /**
        Returns a type casted instance of this vector.
        \tparam C Specifies the static cast type.
        */
        template <typename C>
        Vector<C, 2> Cast() const
        {
            return Vector<C, 2>(
                static_cast<C>(x),
                static_cast<C>(y)
            );
        }

        //! Returns a pointer to the first element of this vector.
        T* Ptr()
        {
            return &x;
        }

        //! Returns a constant pointer to the first element of this vector.
        const T* Ptr() const
        {
            return &x;
        }

        #ifdef GS_ENABLE_SWIZZLE_OPERATOR
        #   include "SwizzleVec2Op2.h"
        #   include "SwizzleVec2Op3.h"
        #   include "SwizzleVec2Op4.h"
        #endif

        T x, y;

};


/* --- Type Alias --- */

template <typename T>
using Vector2T = Vector<T, 2>;

using Vector2   = Vector2T<Real>;
using Vector2f  = Vector2T<float>;
using Vector2d  = Vector2T<double>;
using Vector2i  = Vector2T<std::int32_t>;
using Vector2ui = Vector2T<std::uint32_t>;
using Vector2b  = Vector2T<std::int8_t>;
using Vector2ub = Vector2T<std::uint8_t>;


} // /namespace Gs


#endif



// ================================================================================
