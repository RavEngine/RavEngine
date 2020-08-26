/*
 * Vector4.h
 * 
 * This file is part of the "GaussianLib" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef GS_VECTOR4_H
#define GS_VECTOR4_H


#include "Vector.h"
#include "Algebra.h"
#include "Swizzle.h"

#include <cmath>


namespace Gs
{


/**
\brief Base 4D vector class with components: x, y, z, and w.
\tparam T Specifies the data type of the vector components.
This should be a primitive data type such as float, double, int etc.
*/
template <typename T>
class Vector<T, 4>
{

    public:

        //! Specifies the typename of the scalar components.
        using ScalarType = T;

        //! Specifies the number of vector components.
        static const std::size_t components = 4;

        #ifndef GS_DISABLE_AUTO_INIT
        Vector() :
            x { T(0) },
            y { T(0) },
            z { T(0) },
            w { T(1) }
        {
        }
        #else
        Vector() = default;
        #endif

        Vector(const Vector<T, 4>& rhs) :
            x { rhs.x },
            y { rhs.y },
            z { rhs.z },
            w { rhs.w }
        {
        }

        explicit Vector(const Vector<T, 2>& xy, const Vector<T, 2>& zw) :
            x { xy.x },
            y { xy.y },
            z { zw.x },
            w { zw.y }
        {
        }

        explicit Vector(const Vector<T, 2>& xy, const T& z, const T& w) :
            x { xy.x },
            y { xy.y },
            z { z    },
            w { w    }
        {
        }

        explicit Vector(const Vector<T, 3>& xyz, const T& w) :
            x { xyz.x },
            y { xyz.y },
            z { xyz.z },
            w { w     }
        {
        }

        explicit Vector(const T& scalar) :
            x { scalar },
            y { scalar },
            z { scalar },
            w { scalar }
        {
        }

        Vector(const T& x, const T& y, const T& z, const T& w) :
            x { x },
            y { y },
            z { z },
            w { w }
        {
        }

        explicit Vector(UninitializeTag)
        {
            // do nothing
        }

        Vector<T, 4>& operator += (const Vector<T, 4>& rhs)
        {
            x += rhs.x;
            y += rhs.y;
            z += rhs.z;
            w += rhs.w;
            return *this;
        }

        Vector<T, 4>& operator -= (const Vector<T, 4>& rhs)
        {
            x -= rhs.x;
            y -= rhs.y;
            z -= rhs.z;
            w -= rhs.w;
            return *this;
        }

        Vector<T, 4>& operator *= (const Vector<T, 4>& rhs)
        {
            x *= rhs.x;
            y *= rhs.y;
            z *= rhs.z;
            w *= rhs.w;
            return *this;
        }

        Vector<T, 4>& operator /= (const Vector<T, 4>& rhs)
        {
            x /= rhs.x;
            y /= rhs.y;
            z /= rhs.z;
            w /= rhs.w;
            return *this;
        }

        Vector<T, 4>& operator *= (const T rhs)
        {
            x *= rhs;
            y *= rhs;
            z *= rhs;
            w *= rhs;
            return *this;
        }

        Vector<T, 4>& operator /= (const T rhs)
        {
            x /= rhs;
            y /= rhs;
            z /= rhs;
            w /= rhs;
            return *this;
        }

        Vector<T, 4> operator - () const
        {
            return Vector<T, 4> { -x, -y, -z, -w };
        }

        /**
        \brief Returns the specified vector component.
        \param[in] component Specifies the vector component index. This must be 0, 1, 2, or 3.
        */
        T& operator [] (std::size_t component)
        {
            GS_ASSERT(component < (Vector<T, 4>::components));
            return *((&x) + component);
        }

        /**
        \brief Returns the specified vector component.
        \param[in] component Specifies the vector component index. This must be 0, 1, 2, or 3.
        */
        const T& operator [] (std::size_t component) const
        {
            GS_ASSERT(component < (Vector<T, 4>::components));
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
        */
        void Normalize()
        {
            Gs::Normalize(*this);
        }

        /**
        Returns a normalized instance of this vector.
        \see Normalize
        */
        Vector<T, 4> Normalized() const
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
        Vector<C, 4> Cast() const
        {
            return Vector<C, 4>(
                static_cast<C>(x),
                static_cast<C>(y),
                static_cast<C>(z),
                static_cast<C>(w)
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
        #   include "SwizzleVec3Op2.h"
        #   include "SwizzleVec3Op3.h"
        #   include "SwizzleVec3Op4.h"
        #   include "SwizzleVec4Op2.h"
        #   include "SwizzleVec4Op3.h"
        #   include "SwizzleVec4Op4.h"
        #endif

        T x, y, z, w;

};


/* --- Type Alias --- */

template <typename T>
using Vector4T = Vector<T, 4>;

using Vector4   = Vector4T<Real>;
using Vector4f  = Vector4T<float>;
using Vector4d  = Vector4T<double>;
using Vector4i  = Vector4T<std::int32_t>;
using Vector4ui = Vector4T<std::uint32_t>;
using Vector4b  = Vector4T<std::int8_t>;
using Vector4ub = Vector4T<std::uint8_t>;


} // /namespace Gs


#endif



// ================================================================================
