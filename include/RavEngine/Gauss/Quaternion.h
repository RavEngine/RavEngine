/*
 * Quaternion.h
 * 
 * This file is part of the "GaussianLib" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef GS_QUATERNION_H
#define GS_QUATERNION_H


#include "Decl.h"
#include "Real.h"
#include "Assert.h"
#include "Algebra.h"
#include "Tags.h"
#include "Matrix.h"
#include "Conversions.h"

#include <cmath>
#include <limits>
#include <type_traits>


namespace Gs
{


/**
Base quaternion class with components: x, y, z, and w.
\tparam T Specifies the data type of the quaternion components.
This should be a primitive data type such as float, double.
*/
template <typename T>
class QuaternionT
{

    public:

        static_assert(std::is_floating_point<T>::value, "quaternions can only be used with floating point types");

        //! Specifies the typename of the scalar components.
        using ScalarType = T;

        //! Specifies the number of quaternion components. This is just for the internal template interface.
        static const std::size_t components = 4;

        #ifndef GS_DISABLE_AUTO_INIT
        QuaternionT() :
            x { T(0) },
            y { T(0) },
            z { T(0) },
            w { T(1) }
        {
        }
        #else
        QuaternionT() = default;
        #endif

        QuaternionT(const QuaternionT<T>& rhs) :
            x { rhs.x },
            y { rhs.y },
            z { rhs.z },
            w { rhs.w }
        {
        }

        QuaternionT(const T& x, const T& y, const T& z, const T& w) :
            x { x },
            y { y },
            z { z },
            w { w }
        {
        }

        template <template <typename, std::size_t, std::size_t> class M, std::size_t Rows, std::size_t Cols>
        explicit QuaternionT(const M<T, Rows, Cols>& matrix)
        {
            Gs::MatrixToQuaternion(*this, matrix);
        }

        explicit QuaternionT(UninitializeTag)
        {
            // do nothing
        }

        QuaternionT<T>& operator += (const QuaternionT<T>& rhs)
        {
            x += rhs.x;
            y += rhs.y;
            z += rhs.z;
            w += rhs.w;
            return *this;
        }

        QuaternionT<T>& operator -= (const QuaternionT<T>& rhs)
        {
            x -= rhs.x;
            y -= rhs.y;
            z -= rhs.z;
            w -= rhs.w;
            return *this;
        }

        QuaternionT<T>& operator *= (const QuaternionT<T>& rhs)
        {
            *this = (*this * rhs);
            return *this;
        }

        QuaternionT<T>& operator *= (const T& rhs)
        {
            x *= rhs;
            y *= rhs;
            z *= rhs;
            w *= rhs;
            return *this;
        }

        /**
        \brief Returns the specified quaternion component.
        \param[in] component Specifies the quaternion component index. This must be 0, 1, 2, or 3.
        */
        T& operator [] (std::size_t component)
        {
            GS_ASSERT(component < QuaternionT<T>::components);
            return *((&x) + component);
        }

        /**
        \brief Returns the specified quaternion component.
        \param[in] component Specifies the quaternion component index. This must be 0, 1, 2, or 3.
        */
        const T& operator [] (std::size_t component) const
        {
            GS_ASSERT(component < QuaternionT<T>::components);
            return *((&x) + component);
        }

        /**
        \brief Normalizes the quaternion to the unit length of 1.
        \see Normalized
        \see Length
        */
        void Normalize()
        {
            Gs::Normalize(*this);
        }

        /**
        \brief Returns a normalized instance of this quaternion.
        \see Normalize
        */
        QuaternionT<T> Normalized() const
        {
            auto quat = *this;
            quat.Normalize();
            return quat;
        }

        //! Sets this quaternion to the identity quaternion (x = y = z = 0, w = 1).
        void LoadIdentity()
        {
            x = y = z = T(0);
            w = T(1);
        }

        //! Makes this quaternion to its inverse.
        void MakeInverse()
        {
            x = -x;
            y = -y;
            z = -z;
        }

        //! Returns the inverse of this quaternion.
        QuaternionT<T> Inverse() const
        {
            return QuaternionT<T> { -x, -y, -z, w };
        }

        /**
        \brief Computes a spherical linear interpolation between the two quaternions and stores the result into this quaternion.
        \param[in] t Specifies the interpolation factor. This should be in the range [0.0, 1.0].
        */
        void Slerp(const QuaternionT<T>& from, QuaternionT<T> to, const T& t)
        {
            *this = Gs::Slerp(from, to, t);
        }

        //! Sets the quaternion to an euler rotation with the specified angles (in radian).
        void SetEulerAngles(const Vector<T, 3>& angles)
        {
            const T cr = std::cos(angles.x/T(2));
            const T cp = std::cos(angles.y/T(2));
            const T cy = std::cos(angles.z/T(2));

            const T sr = std::sin(angles.x/T(2));
            const T sp = std::sin(angles.y/T(2));
            const T sy = std::sin(angles.z/T(2));

            const T cpcy = cp * cy;
            const T spsy = sp * sy;
            const T cpsy = cp * sy;
            const T spcy = sp * cy;

            x = sr * cpcy - cr * spsy;
            y = cr * spcy + sr * cpsy;
            z = cr * cpsy - sr * spcy;
            w = cr * cpcy + sr * spsy;

            Normalize();
        }

        void GetEulerAngles(Vector<T, 3>& angles) const
        {
            const T xx = x*x;
            const T yy = y*y;
            const T zz = z*z;
            const T ww = w*w;

            angles.x = std::atan2(T(2) * (y*z + x*w), -xx - yy + zz + ww);
            angles.y = std::asin(Clamp(T(2) * (y*w - x*z), T(-1), T(1)));
            angles.z = std::atan2(T(2) * (x*y + z*w), xx - yy - zz + ww);
        }

        /**
        \brief Sets the rotation of this quaternion by the specified euler axis.
        \param[in] aixs Specifies the aixs. This must be normalized!
        \param[in] angle Specifies the rotation angle (in radians).
        */
        void SetAngleAxis(const Vector<T, 3>& axis, const T& angle)
        {
            const T halfAngle   = angle / T(2);
            const T sine        = std::sin(halfAngle);

            x = sine * axis.x;
            y = sine * axis.y;
            z = sine * axis.z;
            w = std::cos(halfAngle);
        }

        void GetAngleAxis(Vector<T, 3>& axis, T& angle) const
        {
            const T scale = std::sqrt(x*x + y*y + z*z);

            if ( ( std::abs(scale) <= std::numeric_limits<T>::epsilon() ) || w > T(1) || w < T(-1) )
            {
                axis.x  = T(0);
                axis.y  = T(1);
                axis.z  = T(0);
                angle   = T(0);
            }
            else
            {
                const T invScale = T(1) / scale;
                axis.x  = x * invScale;
                axis.y  = y * invScale;
                axis.z  = z * invScale;
                angle   = T(2) * std::acos(w);
            }
        }

        Matrix3T<T> ToMatrix3() const
        {
            Matrix3T<T> result { UninitializeTag{} };
            Gs::QuaternionToMatrix(result, *this);
            return result;
        }

        Matrix3T<T> ToMatrix3Transposed() const
        {
            Matrix3T<T> result { UninitializeTag{} };
            Gs::QuaternionToMatrixTransposed(result, *this);
            return result;
        }

        /**
        Returns a type casted instance of this quaternion.
        \tparam C Specifies the static cast type.
        */
        template <typename C>
        QuaternionT<C> Cast() const
        {
            return QuaternionT<C>(
                static_cast<C>(x),
                static_cast<C>(y),
                static_cast<C>(z),
                static_cast<C>(w)
            );
        }

        //! Returns a pointer to the first element of this quaternion.
        T* Ptr()
        {
            return &x;
        }

        //! Returns a constant pointer to the first element of this quaternion.
        const T* Ptr() const
        {
            return &x;
        }

        //! Returns a new quaternion, rotated with the specified euler angles.
        static QuaternionT<T> EulerAngles(const Vector<T, 3>& angles)
        {
            QuaternionT<T> result;
            result.SetEulerAngles(angles);
            return result;
        }

        //! Returns a new quaternion, rotated with the specified angle axis.
        static QuaternionT<T> AngleAxis(const Vector<T, 3>& axis, const T& angle)
        {
            QuaternionT<T> result;
            result.SetAngleAxis(axis, angle);
            return result;
        }

        T x, y, z, w;

};


/* --- Global Operators --- */

template <typename T>
QuaternionT<T> operator + (const QuaternionT<T>& lhs, const QuaternionT<T>& rhs)
{
    auto result = lhs;
    result += rhs;
    return result;
}

template <typename T>
QuaternionT<T> operator - (const QuaternionT<T>& lhs, const QuaternionT<T>& rhs)
{
    auto result = lhs;
    result -= rhs;
    return result;
}

template <typename T>
QuaternionT<T> operator * (const QuaternionT<T>& lhs, const QuaternionT<T>& rhs)
{
    return QuaternionT<T>
    {
        ( (lhs.x * rhs.w) + (lhs.w * rhs.x) + (lhs.z * rhs.y) - (lhs.y * rhs.z) ),
        ( (lhs.y * rhs.w) - (lhs.z * rhs.x) + (lhs.w * rhs.y) + (lhs.x * rhs.z) ),
        ( (lhs.z * rhs.w) + (lhs.y * rhs.x) - (lhs.x * rhs.y) + (lhs.w * rhs.z) ),
        ( (lhs.w * rhs.w) - (lhs.x * rhs.x) - (lhs.y * rhs.y) - (lhs.z * rhs.z) )
    };
}

template <typename T>
QuaternionT<T> operator * (const QuaternionT<T>& lhs, const T& rhs)
{
    auto result = lhs;
    result *= rhs;
    return result;
}

template <typename T>
QuaternionT<T> operator * (const T& lhs, const QuaternionT<T>& rhs)
{
    auto result = rhs;
    result *= lhs;
    return result;
}

//! Rotates the specified vector 'rhs' by the quaternion 'lhs' and returns the new rotated vector.
template <typename T>
Vector<T, 3> operator * (const QuaternionT<T>& lhs, const Vector<T, 3>& rhs)
{
    Vector<T, 3> qvec { lhs.x, lhs.y, lhs.z };

    auto uv = Cross(qvec, rhs);
    auto uuv = Cross(qvec, uv);

    uv *= (T(2) * lhs.w);
    uuv *= T(2);

    /* Result := vec + uv + uuv */
    uv += uuv;
    uv += rhs;

    return uv;
}


/* --- Type Alias --- */

using Quaternion = QuaternionT<Real>;
using Quaternionf = QuaternionT<float>;
using Quaterniond = QuaternionT<double>;


} // /namespace Gs


#endif



// ================================================================================
