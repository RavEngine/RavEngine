/*
 * Algebra.h
 * 
 * This file is part of the "GaussianLib" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef GS_ALGEBRA_H
#define GS_ALGEBRA_H


#include "Macros.h"
#include "Details.h"
#include "Determinant.h"
#include "Inverse.h"
#include "Decl.h"
#include "Real.h"
#include "Tags.h"

#include <cmath>
#include <cstddef>
#include <algorithm>
#include <vector>
#include <type_traits>


namespace Gs
{


/* --- Global Functions --- */

//! Returns the value of 1 + 2 + ... + n = n*(n+1)/2.
template <typename T>
T GaussianSum(T n)
{
    static_assert(std::is_integral<T>::value, "GaussianSum function only allows integral types");
    return n*(n + T(1))/T(2);
}

//! Returns the value of 1^2 + 2^2 + ... + n^2 = n*(n+1)*(2n+1)/6.
template <typename T>
T GaussianSumSq(T n)
{
    static_assert(std::is_integral<T>::value, "GaussianSumSq function only allows integral types");
    return n*(n + T(1))*(n*T(2) + T(1))/T(6);
}

//! Computes a normal (gaussian) distribution value for the specified (1-dimensional) position x with the specified mean and variance.
template <typename T>
T NormalDistribution(const T& x, const T& mean, const T& variance)
{
    return std::exp(-(x - mean)*(x - mean) / (variance + variance)) / std::sqrt(T(2) * T(Gs::pi) * variance);
}

//! Computes a normal (gaussian) distribution value for the specified (1-dimensional) position x with the specified mean and variance.
template <typename T>
T NormalDistribution(const T& x)
{
    return std::exp(-(x*x) / T(2)) / std::sqrt(T(2) * T(Gs::pi));
}

//! Returns the dot or rather scalar product between the two vectors 'lhs' and 'rhs'.
template <typename VectorType, typename ScalarType = typename VectorType::ScalarType>
ScalarType Dot(const VectorType& lhs, const VectorType& rhs)
{
    ScalarType result = ScalarType(0);

    for (std::size_t i = 0; i < VectorType::components; ++i)
        result += lhs[i]*rhs[i];

    return result;
}

//! Returns the cross or rather vector product between the two vectors 'lhs' and 'rhs'.
template <typename VectorType>
VectorType Cross(const VectorType& lhs, const VectorType& rhs)
{
    static_assert(VectorType::components == 3, "Vector type must have exactly three components");
    return VectorType
    {
        lhs.y*rhs.z - rhs.y*lhs.z,
        rhs.x*lhs.z - lhs.x*rhs.z,
        lhs.x*rhs.y - rhs.x*lhs.y
    };
}

//! Returns the squared length of the specified vector.
template <typename VectorType, typename ScalarType = typename VectorType::ScalarType>
ScalarType LengthSq(const VectorType& vec)
{
    return Dot<VectorType, ScalarType>(vec, vec);
}

//! Returns the length (euclidian norm) of the specified vector.
template <typename VectorType, typename ScalarType = typename VectorType::ScalarType>
ScalarType Length(const VectorType& vec)
{
    return std::sqrt(LengthSq<VectorType, ScalarType>(vec));
}

//! Returns the angle (in radians) between the two (normalized or unnormalized) vectors 'lhs' and 'rhs'.
template <typename VectorType, typename ScalarType = typename VectorType::ScalarType>
ScalarType Angle(const VectorType& lhs, const VectorType& rhs)
{
    return std::acos( Dot<VectorType, ScalarType>(lhs, rhs) / (Length<VectorType, ScalarType>(lhs) * Length<VectorType, ScalarType>(rhs)) );
}

//! Returns the angle (in radians) between the two normalized vectors 'lhs' and 'rhs'.
template <typename VectorType, typename ScalarType = typename VectorType::ScalarType>
ScalarType AngleNorm(const VectorType& lhs, const VectorType& rhs)
{
    return std::acos(Dot<VectorType, ScalarType>(lhs, rhs));
}

//! Returns the squared distance between the two vectors 'lhs' and 'rhs'.
template <typename VectorType, typename ScalarType = typename VectorType::ScalarType>
ScalarType DistanceSq(const VectorType& lhs, const VectorType& rhs)
{
    auto result = rhs;
    result -= lhs;
    return LengthSq<VectorType, ScalarType>(result);
}

//! Returns the distance between the two vectors 'lhs' and 'rhs'.
template <typename VectorType, typename ScalarType = typename VectorType::ScalarType>
ScalarType Distance(const VectorType& lhs, const VectorType& rhs)
{
    auto result = rhs;
    result -= lhs;
    return Length<VectorType, ScalarType>(result);
}

//! Returns the reflected vector of the incident vector for the specified surface normal.
template <typename VectorType, typename ScalarType = typename VectorType::ScalarType>
VectorType Reflect(const VectorType& incident, const VectorType& normal)
{
    /* Compute reflection as: I - N x Dot(N, I) x 2 */
    auto v = normal;
    v *= (Dot<VectorType, ScalarType>(normal, incident) * ScalarType(-2));
    v += incident;
    return v;
}

//! Normalizes the specified vector to the unit length of 1.
template <typename VectorType, typename ScalarType = typename VectorType::ScalarType>
void Normalize(VectorType& vec)
{
    auto len = LengthSq<VectorType, ScalarType>(vec);
    if (len != ScalarType(0) && len != ScalarType(1))
    {
        len = ScalarType(1) / std::sqrt(len);
        vec *= len;
    }
}

//! Resizes the specified vector to the specified length.
template <typename VectorType, typename ScalarType = typename VectorType::ScalarType>
void Resize(VectorType& vec, const ScalarType& length)
{
    auto len = LengthSq<VectorType, ScalarType>(vec);
    if (len != ScalarType(0))
    {
        len = length / std::sqrt(len);
        vec *= len;
    }
}

/**
\brief Computes a linear interpolation between the point 'a' and the point 'b'.
\return Equivalent to: a*(1-t) + b*t
*/
template <typename T, typename I>
void Lerp(T& x, const T& a, const T& b, const I& t)
{
    x = b;
    x -= a;
    x *= t;
    x += a;
}

/**
\brief Computes a linear interpolation between the point 'a' and the point 'b'.
\return Equivalent to: a*(1-t) + b*t
*/
template <typename T, typename I>
T Lerp(const T& a, const T& b, const I& t)
{
    /* Return (b - a) * t + a */
    T x = b;
    x -= a;
    x *= t;
    x += a;
    return x;
}

/**
\brief Mixes the two values by their scalings.
\return Equivalent to: v0*scale0 + v1*scale1
*/
template <typename T, typename I>
T Mix(const T& v0, const T& v1, const I& scale0, const I& scale1)
{
    return v0*scale0 + v1*scale1;
}

/**
\brief Clamps the input value 'x' into the range [0, 1].
\return max{ 0, min{ x, 1 } }
*/
template <typename T>
T Saturate(const T& x)
{
    /* Wrap std::max and std::min into brackets to break macro expansion from Windows.h, so no NOMINMAX macro definition is required */
    return (std::max)(T(0), (std::min)(x, T(1)));
}

/**
\brief Clamps the value 'x' into the range [minima, maxima].
\return max{ minima, min{ x, maxima } }
*/
template <typename T>
T Clamp(const T& x, const T& minima, const T& maxima)
{
    if (x <= minima)
        return minima;
    if (x >= maxima)
        return maxima;
    return x;
}

/**
\brief Returns the spherical linear interpolation between the two quaternions 'from' and 'to'.
\see QuaternionT::Slerp
*/
template <typename VectorType, typename ScalarType = typename VectorType::ScalarType>
VectorType Slerp(const VectorType& from, const VectorType& to, const ScalarType& t)
{
    ScalarType omega, cosom, sinom;
    ScalarType scale0, scale1;

    /* Calculate cosine */
    cosom = Dot<VectorType, ScalarType>(from, to);

    /* Adjust signs (if necessary) */
    if (cosom < ScalarType(0))
    {
        cosom = -cosom;
        scale1 = ScalarType(-1);
    }
    else
        scale1 = ScalarType(1);

    /* Calculate coefficients */
    if ((ScalarType(1) - cosom) > std::numeric_limits<ScalarType>::epsilon())
    {
        /* Standard case (slerp) */
        omega = std::acos(cosom);
        sinom = std::sin(omega);
        scale0 = std::sin((ScalarType(1) - t) * omega) / sinom;
        scale1 *= std::sin(t * omega) / sinom;
    }
    else
    {
        /* 'from' and 'to' quaternions are very close, so we can do a linear interpolation */
        scale0 = ScalarType(1) - t;
        scale1 *= t;
    }

    /* Calculate final values */
    return Mix(from, to, scale0, scale1);
}

/**
\brief Returns a smooth 'hermite interpolation' in the range [0, 1].
\remarks This hermite interpolation is: 3x^2 - 2x^3.
*/
template <typename T>
T SmoothStep(const T& x)
{
    return x*x * (T(3) - x*T(2));
}

/**
\brief Returns a smooth 'hermite interpolation' in the range [0, 1].
\remarks This hermite interpolation is: 6x^5 - 15x^4 + 10x^3.
*/
template <typename T>
T SmootherStep(const T& x)
{
    return x*x*x * (x*(x*T(6) - T(15)) + T(10));
}

//! Returns the reciprocal of the specified scalar value.
template <typename T>
T Rcp(const T& x)
{
	return T(1) / x;
}

//! Returns the per-component reciprocal of the specified N-dimensional vector.
template <typename T, std::size_t N>
Vector<T, N> Rcp(const Vector<T, N>& vec)
{
	Vector<T, N> vecRcp { UninitializeTag{} };

	for (std::size_t i = 0; i < N; ++i)
		vecRcp[i] = T(1) / vec[i];

	return vecRcp;
}

//! Returns the per-component reciprocal of the specified NxM-dimensional matrix.
template <typename T, std::size_t N, std::size_t M>
Matrix<T, N, M> Rcp(const Matrix<T, N, M>& mat)
{
	Matrix<T, N, M> matRcp { UninitializeTag{} };

	for (std::size_t i = 0; i < N*M; ++i)
		matRcp[i] = T(1) / mat[i];

	return matRcp;
}

//! Rescales the specified value 't' from the first range [lower0, upper0] into the second range [lower1, upper1].
template <typename T, typename I>
T Rescale(const T& t, const I& lower0, const I& upper0, const I& lower1, const I& upper1)
{
    /* Return (((t - lower0) / (upper0 - lower0)) * (upper1 - lower1) + lower1) */
    T x = t;
    x -= T(lower0);
    x /= (upper0 - lower0);
    x *= (upper1 - lower1);
    x += T(lower1);
    return x;
}


/* --- Global Operators --- */

/**
\brief Multiplies the N-dimensional row-vector with the NxM matrix.
\remarks This is equivalent to: Transpose(rhs) * lhs.
*/
template <typename T, std::size_t Rows, std::size_t Cols>
Vector<T, Cols> operator * (const Vector<T, Rows>& lhs, const Matrix<T, Rows, Cols>& rhs)
{
    Vector<T, Cols> result;

    for (std::size_t c = 0; c < Cols; ++c)
    {
        result[c] = T(0);
        for (std::size_t r = 0; r < Rows; ++r)
            result[c] += rhs(r, c)*lhs[r];
    }

    return result;
}

/**
\brief Multiplies the NxM matrix with the M-dimensional column-vector.
\remarks This is equivalent to: rhs * Transpose(lhs).
*/
template <typename T, std::size_t Rows, std::size_t Cols>
Vector<T, Rows> operator * (const Matrix<T, Rows, Cols>& lhs, const Vector<T, Cols>& rhs)
{
    Vector<T, Rows> result;

    for (std::size_t r = 0; r < Rows; ++r)
    {
        result[r] = T(0);
        for (std::size_t c = 0; c < Cols; ++c)
            result[r] += lhs(r, c)*rhs[c];
    }

    return result;
}


} // /namespace Gs


#endif



// ================================================================================
