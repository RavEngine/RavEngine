/*
 * AffineMatrix4.h
 * 
 * This file is part of the "GaussianLib" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef GS_AFFINE_MATRIX4_H
#define GS_AFFINE_MATRIX4_H


#include "Real.h"
#include "Assert.h"
#include "Macros.h"
#include "Matrix.h"
#include "Tags.h"
#include "AffineMatrix.h"
#include "MatrixInitializer.h"
#include "Vector3.h"
#include "Vector4.h"

#include <cmath>
#include <cstring>
#include <algorithm>
#include <cstdint>
#include <initializer_list>


namespace Gs
{


#ifdef GS_ROW_MAJOR_STORAGE
#   define GS_FOREACH_ROW_COL(r, c)                                         \
        for (std::size_t r = 0; r < AffineMatrix4T<T>::rowsSparse; ++r)     \
        for (std::size_t c = 0; c < AffineMatrix4T<T>::columnsSparse; ++c)
#else
#   define GS_FOREACH_ROW_COL(r, c)                                         \
        for (std::size_t c = 0; c < AffineMatrix4T<T>::columnsSparse; ++c)  \
        for (std::size_t r = 0; r < AffineMatrix4T<T>::rowsSparse; ++r)
#endif

/**
This is an affine 4x4 matrix for affine transformations,
i.e. it can only contain translations, scaling, rotations and shearing.
It only stores a 3x4 matrix where the 4th row is always implicitly (0, 0, 0, 1).
\tparam T Specifies the data type of the matrix components.
This should be a primitive data type such as float, double, int etc.
\remarks The macro GS_ROW_MAJOR_STORAGE can be defined, to use row-major storage layout.
By default column-major storage layout is used.
The macro GS_ROW_VECTORS can be defined, to use row vectors. By default column vectors are used.
Here is an example, how an affine 4x4 matrix is laid-out with column- and row vectors:
\code
// Affine 4x4 matrix with column vectors:
// / x1 y1 z1 w1 \
// | x2 y2 z2 w2 |
// | x3 y3 z3 w3 |
// \  0  0  0  1 /

// Affine 4x4 matrix with row vectors:
// / x1 x2 x3 0 \
// | y1 y2 y3 0 |
// | z1 z2 z3 0 |
// \ w1 w2 w3 1 /

// In both cases, (w1, w2, w3, 1) stores the position in the affine transformation.
\endcode
*/
template <typename T>
class AffineMatrix4T
{

    public:

        /* ----- Static members ----- */

        static const std::size_t rows           = 4;
        static const std::size_t columns        = 4;
        static const std::size_t elements       = AffineMatrix4T<T>::rows*AffineMatrix4T<T>::columns;

        #ifdef GS_ROW_VECTORS
        static const std::size_t rowsSparse     = 4;
        static const std::size_t columnsSparse  = 3;
        #else
        static const std::size_t rowsSparse     = 3;
        static const std::size_t columnsSparse  = 4;
        #endif

        static const std::size_t elementsSparse = AffineMatrix4T<T>::rowsSparse*AffineMatrix4T<T>::columnsSparse;

        /* ----- Typenames ----- */

        //! Specifies the typename of the scalar components.
        using ScalarType        = T;

        //! Typename of this matrix type.
        using ThisType          = AffineMatrix4T<T>;

        //! Typename of the transposed of this matrix type.
        using TransposedType    = Matrix<T, AffineMatrix4T<T>::rows, AffineMatrix4T<T>::columns>;

        /* ----- Functions ----- */

        AffineMatrix4T()
        {
            #ifndef GS_ENABLE_AUTO_INIT
            LoadIdentity();
            #endif
        }

        AffineMatrix4T(const ThisType& rhs)
        {
            *this = rhs;
        }

        #ifdef GS_ROW_VECTORS

        AffineMatrix4T(
            const T& m11, const T& m12, const T& m13,
            const T& m21, const T& m22, const T& m23,
            const T& m31, const T& m32, const T& m33,
            const T& m41, const T& m42, const T& m43)
        {
            (*this)(0, 0) = m11; (*this)(0, 1) = m12; (*this)(0, 2) = m13;
            (*this)(1, 0) = m21; (*this)(1, 1) = m22; (*this)(1, 2) = m23;
            (*this)(2, 0) = m31; (*this)(2, 1) = m32; (*this)(2, 2) = m33;
            (*this)(3, 0) = m41; (*this)(3, 1) = m42; (*this)(3, 2) = m43;
        }

        #else

        AffineMatrix4T(
            const T& m11, const T& m12, const T& m13, const T& m14,
            const T& m21, const T& m22, const T& m23, const T& m24,
            const T& m31, const T& m32, const T& m33, const T& m34)
        {
            (*this)(0, 0) = m11; (*this)(0, 1) = m12; (*this)(0, 2) = m13; (*this)(0, 3) = m14;
            (*this)(1, 0) = m21; (*this)(1, 1) = m22; (*this)(1, 2) = m23; (*this)(1, 3) = m24;
            (*this)(2, 0) = m31; (*this)(2, 1) = m32; (*this)(2, 2) = m33; (*this)(2, 3) = m34;
        }

        #endif

        //! Initializes this matrix with the specified values (row by row, and column by column, implicit must NOT be included).
        AffineMatrix4T(const std::initializer_list<T>& values)
        {
            std::size_t i = 0, n = values.size();
            for (auto it = values.begin(); i < n; ++i, ++it)
                (*this)(i / columnsSparse, i % columnsSparse) = *it;
            for (; i < elementsSparse; ++i)
                (*this)(i / columnsSparse, i % columnsSparse) = T(0);
        }

        explicit AffineMatrix4T(UninitializeTag)
        {
            // do nothing
        }

        /**
        \brief Returns a reference to the element at the specified entry.
        \param[in] row Specifies the row index. This must be in the range [0, 2], or [0, 3] if GS_ROW_VECTORS is defined.
        \param[in] col Specifies the column index. This must be in the range [0, 3], or [0, 2] if GS_ROW_VECTORS is defined.
        */
        T& operator () (std::size_t row, std::size_t col)
        {
            GS_ASSERT(row < AffineMatrix4T<T>::rowsSparse);
            GS_ASSERT(col < AffineMatrix4T<T>::columnsSparse);
            #ifdef GS_ROW_MAJOR_STORAGE
            return m_[row*AffineMatrix4T<T>::columnsSparse + col];
            #else
            return m_[col*AffineMatrix4T<T>::rowsSparse + row];
            #endif
        }

        /**
        \brief Returns a constant reference to the element at the specified entry.
        \param[in] row Specifies the row index. This must be in the range [0, 2], or [0, 3] if GS_ROW_VECTORS is defined.
        \param[in] col Specifies the column index. This must be in the range [0, 3], or [0, 2] if GS_ROW_VECTORS is defined.
        */
        const T& operator () (std::size_t row, std::size_t col) const
        {
            GS_ASSERT(row < AffineMatrix4T<T>::rowsSparse);
            GS_ASSERT(col < AffineMatrix4T<T>::columnsSparse);
            #ifdef GS_ROW_MAJOR_STORAGE
            return m_[row*AffineMatrix4T<T>::columnsSparse + col];
            #else
            return m_[col*AffineMatrix4T<T>::rowsSparse + row];
            #endif
        }

        T& operator [] (std::size_t element)
        {
            GS_ASSERT(element < AffineMatrix4T<T>::elementsSparse);
            return m_[element];
        }

        const T& operator [] (std::size_t element) const
        {
            GS_ASSERT(element < AffineMatrix4T<T>::elementsSparse);
            return m_[element];
        }

        ThisType& operator += (const ThisType& rhs)
        {
            for (std::size_t i = 0; i < ThisType::elementsSparse; ++i)
                m_[i] += rhs.m_[i];
            return *this;
        }

        ThisType& operator -= (const ThisType& rhs)
        {
            for (std::size_t i = 0; i < ThisType::elementsSparse; ++i)
                m_[i] -= rhs.m_[i];
            return *this;
        }

        ThisType& operator *= (const ThisType& rhs)
        {
            *this = (*this * rhs);
            return *this;
        }

        ThisType& operator *= (const T& rhs)
        {
            for (std::size_t i = 0; i < ThisType::elementsSparse; ++i)
                m_[i] *= rhs;
            return *this;
        }

        ThisType& operator = (const ThisType& rhs)
        {
            for (std::size_t i = 0; i < ThisType::elementsSparse; ++i)
                m_[i] = rhs.m_[i];
            return *this;
        }

        #ifdef GS_ROW_VECTORS

        T& At(std::size_t col, std::size_t row)
        {
            return (*this)(row, col);
        }

        const T& At(std::size_t col, std::size_t row) const
        {
            return (*this)(row, col);
        }

        #else

        T& At(std::size_t row, std::size_t col)
        {
            return (*this)(row, col);
        }

        const T& At(std::size_t row, std::size_t col) const
        {
            return (*this)(row, col);
        }

        #endif

        void Reset()
        {
            for (std::size_t i = 0; i < ThisType::elementsSparse; ++i)
                m_[i] = T(0);
        }

        void LoadIdentity()
        {
            GS_FOREACH_ROW_COL(r, c)
            {
                (*this)(r, c) = (r == c ? T(1) : T(0));
            }
        }

        static ThisType Identity()
        {
            ThisType result;
            result.LoadIdentity();
            return result;
        }

        TransposedType Transposed() const
        {
            TransposedType result;

            GS_FOREACH_ROW_COL(r, c)
            {
                result(c, r) = (*this)(r, c);
            }

            #ifdef GS_ROW_VECTORS
            for (std::size_t i = 0; i < ThisType::columnsSparse; ++i)
                result(i, TransposedType::columns - 1) = (*this)(ThisType::rowsSparse - 1, i);
            #else
            for (std::size_t i = 0; i < ThisType::rowsSparse; ++i)
                result(TransposedType::rows - 1, i) = (*this)(i, ThisType::columnsSparse - 1);
            #endif

            result(TransposedType::rows - 1, TransposedType::columns - 1) = T(1);

            return result;
        }

        T Determinant() const
        {
            return Gs::Determinant(*this);
        }

        //! Returns the trace of this matrix: M(0, 0) + M(1, 1) + M(2, 2) + 1.
        T Trace() const
        {
            return (*this)(0, 0) + (*this)(1, 1) + (*this)(2, 2) + T(1);
        }

        AffineMatrix4T<T> Inverse() const
        {
            AffineMatrix4T<T> inv{ *this };
            inv.MakeInverse();
            return inv;
        }

        bool MakeInverse()
        {
            AffineMatrix4T<T> in{ *this };
            return Gs::Inverse(*this, in);
        }

        //! Returns a pointer to the first element of this matrix.
        T* Ptr()
        {
            return &(m_[0]);
        }

        //! Returns a constant pointer to the first element of this matrix.
        const T* Ptr() const
        {
            return &(m_[0]);
        }

        /* --- Extended functions for affine transformations --- */

        /**
        \brief Returns the specified (implicit) row vector.
        \param[in] row Specifies the row. This must be in the range [0, rows).
        \remarks This function uses the "At" function to access the matrix elements.
        \see At
        */
        Vector4T<T> GetRow(std::size_t row) const
        {
            if (row + 1 == rows)
                return Vector4T<T>(0, 0, 0, 1);
            else
                return Vector4T<T>(At(row, 0), At(row, 1), At(row, 2), At(row, 3));
        }

        /**
        \brief Returns the specified (implicit) column vector.
        \param[in] col Specifies the column. This must be in the range [0, columns).
        \remarks This function uses the "At" function to access the matrix elements.
        \see At
        */
        Vector4T<T> GetColumn(std::size_t col) const
        {
            return Vector4T<T>(At(0, col), At(1, col), At(2, col), (col + 1 == columns ? T(1) : T(0)));
        }

        void SetPosition(const Vector3T<T>& position)
        {
            At(0, 3) = position.x;
            At(1, 3) = position.y;
            At(2, 3) = position.z;
        }

        Vector3T<T> GetPosition() const
        {
            return Vector3T<T>(At(0, 3), At(1, 3), At(2, 3));
        }

        /**
        Sets the scaling of this matrix.
        \param[in] vec Specifies the scaling vector.
        \note This will destroy the rotation. You can set the rotation and scaling at once with 'SetRotationAndScale'.
        \see SetRotationAndScale
        */
        void SetScale(const Vector3T<T>& vec)
        {
            Vector3T<T> col0(At(0, 0), At(1, 0), At(2, 0)),
                        col1(At(0, 1), At(1, 1), At(2, 1)),
                        col2(At(0, 2), At(1, 2), At(2, 2));

            col0.Resize(vec.x);
            col1.Resize(vec.y);
            col2.Resize(vec.z);

            At(0, 0) = col0.x;
            At(1, 0) = col0.y;
            At(2, 0) = col0.z;

            At(0, 1) = col1.x;
            At(1, 1) = col1.y;
            At(2, 1) = col1.z;

            At(0, 2) = col2.x;
            At(1, 2) = col2.y;
            At(2, 2) = col2.z;
        }

        //! Returns the unsigned scaling of this matrix (independent of rotation and shearing).
        Vector3T<T> GetScale() const
        {
            return Vector3T<T>(
                Vector3T<T>(At(0, 0), At(1, 0), At(2, 0)).Length(),
                Vector3T<T>(At(0, 1), At(1, 1), At(2, 1)).Length(),
                Vector3T<T>(At(0, 2), At(1, 2), At(2, 2)).Length()
            );
        }

        //! Rotates the matrix at the X-axis with the specified angle (in radians).
        void RotateX(const T& angle)
        {
            const T c = std::cos(angle);
            const T s = std::sin(angle);

            /* Temporaries */
            const T m01 = At(0, 1);
            const T m11 = At(1, 1);
            const T m21 = At(2, 1);

            /* Rotation */
            At(0, 1) = At(0, 1)*c + At(0, 2)*s;
            At(1, 1) = At(1, 1)*c + At(1, 2)*s;
            At(2, 1) = At(2, 1)*c + At(2, 2)*s;

            At(0, 2) = At(0, 2)*c - m01*s;
            At(1, 2) = At(1, 2)*c - m11*s;
            At(2, 2) = At(2, 2)*c - m21*s;
        }

        //! Rotates the matrix at the Y-axis with the specified angle (in radians).
        void RotateY(const T& angle)
        {
            const T c = std::cos(angle);
            const T s = std::sin(angle);

            /* Temporaries */
            const T m00 = At(0, 0);
            const T m10 = At(1, 0);
            const T m20 = At(2, 0);

            /* Rotation */
            At(0, 0) = At(0, 0)*c - At(0, 2)*s;
            At(1, 0) = At(1, 0)*c - At(1, 2)*s;
            At(2, 0) = At(2, 0)*c - At(2, 2)*s;

            At(0, 2) = m00*s - At(0, 2)*c;
            At(1, 2) = m10*s - At(1, 2)*c;
            At(2, 2) = m20*s - At(2, 2)*c;
        }

        //! Rotates the matrix at the Z-axis with the specified angle (in radians).
        void RotateZ(const T& angle)
        {
            const T c = std::cos(angle);
            const T s = std::sin(angle);

            /* Temporaries */
            const T m00 = At(0, 0);
            const T m10 = At(1, 0);
            const T m20 = At(2, 0);

            /* Rotation */
            At(0, 0) = At(0, 0)*c + At(0, 1)*s;
            At(1, 0) = At(1, 0)*c + At(1, 1)*s;
            At(2, 0) = At(2, 0)*c + At(2, 1)*s;

            At(0, 1) = At(0, 1)*c - m00*s;
            At(1, 1) = At(1, 1)*c - m10*s;
            At(2, 1) = At(2, 1)*c - m20*s;
        }

        void ToMatrix4(Matrix<T, 4, 4>& m) const
        {
            m.At(0, 0) = At(0, 0);
            m.At(1, 0) = At(1, 0);
            m.At(2, 0) = At(2, 0);
            m.At(3, 0) = T(0);

            m.At(0, 1) = At(0, 1);
            m.At(1, 1) = At(1, 1);
            m.At(2, 1) = At(2, 1);
            m.At(3, 1) = T(0);

            m.At(0, 2) = At(0, 2);
            m.At(1, 2) = At(1, 2);
            m.At(2, 2) = At(2, 2);
            m.At(3, 2) = T(0);

            m.At(0, 3) = At(0, 3);
            m.At(1, 3) = At(1, 3);
            m.At(2, 3) = At(2, 3);
            m.At(3, 3) = T(1);
        }

        Matrix<T, 4, 4> ToMatrix4() const
        {
            Matrix<T, 4, 4> result;
            ToMatrix4(result);
            return result;
        }

        /**
        Returns a type casted instance of this affine matrix.
        \tparam C Specifies the static cast type.
        */
        template <typename C> AffineMatrix4T<C> Cast() const
        {
            AffineMatrix4T<C> result { UninitializeTag{} };

            for (std::size_t i = 0; i < ThisType::elementsSparse; ++i)
                result[i] = static_cast<C>(m_[i]);

            return result;
        }

    private:

        T m_[ThisType::elementsSparse];

};

#undef GS_FOREACH_ROW_COL


/* --- Global Operators --- */

template <typename T>
AffineMatrix4T<T> operator + (const AffineMatrix4T<T>& lhs, const AffineMatrix4T<T>& rhs)
{
    auto result = lhs;
    result += rhs;
    return result;
}

template <typename T>
AffineMatrix4T<T> operator - (const AffineMatrix4T<T>& lhs, const AffineMatrix4T<T>& rhs)
{
    auto result = lhs;
    result -= rhs;
    return result;
}

template <typename T>
AffineMatrix4T<T> operator * (const AffineMatrix4T<T>& lhs, const T& rhs)
{
    auto result = lhs;
    result *= rhs;
    return result;
}

template <typename T>
AffineMatrix4T<T> operator * (const T& lhs, const AffineMatrix4T<T>& rhs)
{
    auto result = rhs;
    result *= lhs;
    return result;
}

template <typename T>
AffineMatrix4T<T> operator * (const AffineMatrix4T<T>& lhs, const AffineMatrix4T<T>& rhs)
{
    return Details::MulAffineMatrices(lhs, rhs);
}


/* --- Type Alias --- */

using AffineMatrix4     = AffineMatrix4T<Real>;
using AffineMatrix4f    = AffineMatrix4T<float>;
using AffineMatrix4d    = AffineMatrix4T<double>;
using AffineMatrix4i    = AffineMatrix4T<std::int32_t>;


} // /namespace Gs


#endif



// ================================================================================
