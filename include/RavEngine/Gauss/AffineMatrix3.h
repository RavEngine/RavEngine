/*
 * AffineMatrix3.h
 * 
 * This file is part of the "GaussianLib" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef GS_AFFINE_MATRIX3_H
#define GS_AFFINE_MATRIX3_H


#include "Real.h"
#include "Assert.h"
#include "Macros.h"
#include "Matrix.h"
#include "Tags.h"
#include "AffineMatrix.h"
#include "MatrixInitializer.h"
#include "Vector2.h"
#include "Vector3.h"

#include <cmath>
#include <cstring>
#include <algorithm>
#include <cstdint>
#include <initializer_list>


namespace Gs
{


#ifdef GS_ROW_MAJOR_STORAGE
#   define GS_FOREACH_ROW_COL(r, c)                                         \
        for (std::size_t r = 0; r < AffineMatrix3T<T>::rowsSparse; ++r)     \
        for (std::size_t c = 0; c < AffineMatrix3T<T>::columnsSparse; ++c)
#else
#   define GS_FOREACH_ROW_COL(r, c)                                         \
        for (std::size_t c = 0; c < AffineMatrix3T<T>::columnsSparse; ++c)  \
        for (std::size_t r = 0; r < AffineMatrix3T<T>::rowsSparse; ++r)
#endif

/**
This is an affine 3x3 matrix for affine transformations,
i.e. it can only contain translations, scaling, rotations and shearing.
It only stores a 2x3 matrix where the 3rd row is always implicitly (0, 0, 1).
\tparam T Specifies the data type of the matrix components.
This should be a primitive data type such as float, double, int etc.
\remarks The macro GS_ROW_MAJOR_STORAGE can be defined, to use row-major storage layout.
By default column-major storage layout is used.
The macro GS_ROW_VECTORS can be defined, to use row vectors. By default column vectors are used.
Here is an example, how an affine 3x3 matrix is laid-out with column- and row vectors:
\code
// Affine 3x3 matrix with column vectors:
// / x1 y1 z1 \
// | x2 y2 z2 |
// \  0  0  1 /

// Affine 3x3 matrix with row vectors:
// / x1 x2 0 \
// | y1 y2 0 |
// \ z1 z2 1 /

// In both cases, (z1, z2, 1) stores the position in the affine transformation.
\endcode
*/
template <typename T>
class AffineMatrix3T
{

    public:

        /* ----- Static members ----- */

        static const std::size_t rows           = 3;
        static const std::size_t columns        = 3;
        static const std::size_t elements       = AffineMatrix3T<T>::rows*AffineMatrix3T<T>::columns;

        #ifdef GS_ROW_VECTORS
        static const std::size_t rowsSparse     = 3;
        static const std::size_t columnsSparse  = 2;
        #else
        static const std::size_t rowsSparse     = 2;
        static const std::size_t columnsSparse  = 3;
        #endif

        static const std::size_t elementsSparse = AffineMatrix3T<T>::rowsSparse*AffineMatrix3T<T>::columnsSparse;

        /* ----- Typenames ----- */

        //! Specifies the typename of the scalar components.
        using ScalarType        = T;

        //! Typename of this matrix type.
        using ThisType          = AffineMatrix3T<T>;

        //! Typename of the transposed of this matrix type.
        using TransposedType    = Matrix<T, AffineMatrix3T<T>::rows, AffineMatrix3T<T>::columns>;

        /* ----- Functions ----- */

        AffineMatrix3T()
        {
            #ifndef GS_ENABLE_AUTO_INIT
            LoadIdentity();
            #endif
        }

        AffineMatrix3T(const ThisType& rhs)
        {
            *this = rhs;
        }

        #ifdef GS_ROW_VECTORS

        AffineMatrix3T(
            const T& m11, const T& m12,
            const T& m21, const T& m22,
            const T& m31, const T& m32)
        {
            (*this)(0, 0) = m11; (*this)(0, 1) = m12;
            (*this)(1, 0) = m21; (*this)(1, 1) = m22;
            (*this)(2, 0) = m31; (*this)(2, 1) = m32;
        }

        #else

        AffineMatrix3T(
            const T& m11, const T& m12, const T& m13,
            const T& m21, const T& m22, const T& m23)
        {
            (*this)(0, 0) = m11; (*this)(0, 1) = m12; (*this)(0, 2) = m13;
            (*this)(1, 0) = m21; (*this)(1, 1) = m22; (*this)(1, 2) = m23;
        }

        #endif

        //! Initializes this matrix with the specified values (row by row, and column by column, implicit must NOT be included).
        AffineMatrix3T(const std::initializer_list<T>& values)
        {
            std::size_t i = 0, n = values.size();
            for (auto it = values.begin(); i < n; ++i, ++it)
                (*this)(i / columnsSparse, i % columnsSparse) = *it;
            for (; i < elementsSparse; ++i)
                (*this)(i / columnsSparse, i % columnsSparse) = T(0);
        }

        explicit AffineMatrix3T(UninitializeTag)
        {
            // do nothing
        }

        /**
        \brief Returns a reference to the element at the specified entry.
        \param[in] row Specifies the row index. This must be in the range [0, 1], or [0, 2] if GS_ROW_VECTORS is defined.
        \param[in] col Specifies the column index. This must be in the range [0, 2], or [0, 1] if GS_ROW_VECTORS is defined.
        */
        T& operator () (std::size_t row, std::size_t col)
        {
            GS_ASSERT(row < AffineMatrix3T<T>::rowsSparse);
            GS_ASSERT(col < AffineMatrix3T<T>::columnsSparse);
            #ifdef GS_ROW_MAJOR_STORAGE
            return m_[row*AffineMatrix3T<T>::columnsSparse + col];
            #else
            return m_[col*AffineMatrix3T<T>::rowsSparse + row];
            #endif
        }

        /**
        \brief Returns a constant reference to the element at the specified entry.
        \param[in] row Specifies the row index. This must be in the range [0, 1], or [0, 2] if GS_ROW_VECTORS is defined.
        \param[in] col Specifies the column index. This must be in the range [0, 2], or [0, 1] if GS_ROW_VECTORS is defined.
        */
        const T& operator () (std::size_t row, std::size_t col) const
        {
            GS_ASSERT(row < AffineMatrix3T<T>::rowsSparse);
            GS_ASSERT(col < AffineMatrix3T<T>::columnsSparse);
            #ifdef GS_ROW_MAJOR_STORAGE
            return m_[row*AffineMatrix3T<T>::columnsSparse + col];
            #else
            return m_[col*AffineMatrix3T<T>::rowsSparse + row];
            #endif
        }

        T& operator [] (std::size_t element)
        {
            GS_ASSERT(element < AffineMatrix3T<T>::elementsSparse);
            return m_[element];
        }

        const T& operator [] (std::size_t element) const
        {
            GS_ASSERT(element < AffineMatrix3T<T>::elementsSparse);
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

        //! Returns the trace of this matrix: M(0, 0) + M(1, 1) + 1.
        T Trace() const
        {
            return (*this)(0, 0) + (*this)(1, 1) + T(1);
        }

        AffineMatrix3T<T> Inverse() const
        {
            AffineMatrix3T<T> inv{ *this };
            inv.MakeInverse();
            return inv;
        }

        bool MakeInverse()
        {
            AffineMatrix3T<T> in{ *this };
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
        Vector3T<T> GetRow(std::size_t row) const
        {
            if (row + 1 == rows)
                return Vector3T<T>(0, 0, 1);
            else
                return Vector3T<T>(At(row, 0), At(row, 1), At(row, 2));
        }

        /**
        \brief Returns the specified (implicit) column vector.
        \param[in] col Specifies the column. This must be in the range [0, columns).
        \remarks This function uses the "At" function to access the matrix elements.
        \see At
        */
        Vector3T<T> GetColumn(std::size_t col) const
        {
            return Vector3T<T>(At(0, col), At(1, col), (col + 1 == columns ? T(1) : T(0)));
        }

        //! Sets the position of this affine transformation.
        void SetPosition(const Vector2T<T>& position)
        {
            At(0, 2) = position.x;
            At(1, 2) = position.y;
        }

        //! Returns the position of this affine transformation.
        Vector2T<T> GetPosition() const
        {
            return Vector2T<T>(At(0, 2), At(1, 2));
        }

        //! Translates this affine transformation in X and Y direction with the specified vector.
        void Translate(const Vector2T<T>& vec)
        {
            At(0, 2) += ( At(0, 0)*vec.x + At(0, 1)*vec.y );
            At(1, 2) += ( At(1, 0)*vec.x + At(1, 1)*vec.y );
        }

        /**
        Sets the scaling of this matrix.
        \param[in] vec Specifies the scaling vector.
        \note This will destroy the rotation. You can set the rotation and scaling at once with 'SetRotationAndScale'.
        \see SetRotationAndScale
        */
        void SetScale(const Vector2T<T>& vec)
        {
            Vector2T<T> col0(At(0, 0), At(1, 0)),
                        col1(At(0, 1), At(1, 1));

            col0.Resize(vec.x);
            col1.Resize(vec.y);

            At(0, 0) = col0.x;
            At(1, 0) = col0.y;

            At(0, 1) = col1.x;
            At(1, 1) = col1.y;
        }

        //! Returns the unsigned scaling of this matrix (independent of rotation and shearing).
        Vector2T<T> GetScale() const
        {
            return Vector2T<T>(
                Vector2T<T>(At(0, 0), At(1, 0)).Length(),
                Vector2T<T>(At(0, 1), At(1, 1)).Length()
            );
        }

        //! Scales this affine transformation in X and Y direction with the specified vector.
        void Scale(const Vector2T<T>& vec)
        {
            At(0, 0) *= vec.x;
            At(1, 0) *= vec.x;
            At(0, 1) *= vec.y;
            At(1, 1) *= vec.y;
        }

        /**
        \brief Sets the rotation around the Z axis. This is the only allowed rotation for this 3x3 affine matrix.
        \param[in] angle Specifies the rotation angle (in radians).
        \note This will destroy the scaling. You can set the rotation and scaling at once with 'SetRotationAndScale'.
        \see SetRotationAndScale
        */
        void SetRotation(const T& angle)
        {
            const T c = std::cos(angle);
            const T s = std::sin(angle);

            At(0, 0) = c;
            At(1, 0) = s;
            At(0, 1) = -s;
            At(1, 1) = c;
        }

        /**
        \brief Returns the rotation (in radians) of this matrix.
        \remarks This requires that the rotation was set with 'SetRotation' or 'SetRotationAndScale' and no shearing was used.
        \see SetRotation
        \see SetRotationAndScale
        */
        T GetRotation() const
        {
            auto len = Vector2T<T>(At(0, 0), At(1, 0)).Length();
            return std::acos(At(0, 0) / len);
        }

        /**
        \brief Rotates this matrix around the Z axis. This is the only allowed rotation for this 3x3 affine matrix.
        \param[in] angle Specifies the rotation angle (in radians).
        \remarks This will keep the current scaling.
        */
        void Rotate(const T& angle)
        {
            const T c = std::cos(angle);
            const T s = std::sin(angle);

            const T m00 = At(0, 0);
            const T m10 = At(1, 0);

            At(0, 0) = m00*c + At(0, 1)*s;
            At(1, 0) = m10*c + At(1, 1)*s;

            At(0, 1) = At(0, 1)*c - m00*s;
            At(1, 1) = At(1, 1)*c - m10*s;
        }

        /**
        \brief Sets the rotation around the Z axis and the scaling.
        \param[in] angle Specifies the rotation angle (in radians).
        \param[in] scale Specifies the scaling vector.
        \see SetRotation
        \see SetScale
        */
        void SetRotationAndScale(const T& angle, const Vector2T<T>& scale)
        {
            const T c = std::cos(angle);
            const T s = std::sin(angle);

            At(0, 0) = c*scale.x;
            At(1, 0) = s*scale.x;
            At(0, 1) = -s*scale.y;
            At(1, 1) = c*scale.y;
        }

        void ToMatrix3(Matrix<T, 3, 3>& m) const
        {
            m.At(0, 0) = At(0, 0);
            m.At(1, 0) = At(1, 0);
            m.At(2, 0) = T(0);

            m.At(0, 1) = At(0, 1);
            m.At(1, 1) = At(1, 1);
            m.At(2, 1) = T(0);

            m.At(0, 2) = At(0, 2);
            m.At(1, 2) = At(1, 2);
            m.At(2, 2) = T(1);
        }

        Matrix<T, 3, 3> ToMatrix3() const
        {
            Matrix<T, 3, 3> result;
            ToMatrix3(result);
            return result;
        }

        /**
        Returns a type casted instance of this affine matrix.
        \tparam C Specifies the static cast type.
        */
        template <typename C> AffineMatrix3T<C> Cast() const
        {
            AffineMatrix3T<C> result { UninitializeTag{} };

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
AffineMatrix3T<T> operator + (const AffineMatrix3T<T>& lhs, const AffineMatrix3T<T>& rhs)
{
    auto result = lhs;
    result += rhs;
    return result;
}

template <typename T>
AffineMatrix3T<T> operator - (const AffineMatrix3T<T>& lhs, const AffineMatrix3T<T>& rhs)
{
    auto result = lhs;
    result -= rhs;
    return result;
}

template <typename T>
AffineMatrix3T<T> operator * (const AffineMatrix3T<T>& lhs, const T& rhs)
{
    auto result = lhs;
    result *= rhs;
    return result;
}

template <typename T>
AffineMatrix3T<T> operator * (const T& lhs, const AffineMatrix3T<T>& rhs)
{
    auto result = rhs;
    result *= lhs;
    return result;
}

template <typename T>
AffineMatrix3T<T> operator * (const AffineMatrix3T<T>& lhs, const AffineMatrix3T<T>& rhs)
{
    return Details::MulAffineMatrices(lhs, rhs);
}


/* --- Type Alias --- */

using AffineMatrix3     = AffineMatrix3T<Real>;
using AffineMatrix3f    = AffineMatrix3T<float>;
using AffineMatrix3d    = AffineMatrix3T<double>;
using AffineMatrix3i    = AffineMatrix3T<std::int32_t>;


} // /namespace Gs


#endif



// ================================================================================
