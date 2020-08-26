/*
 * Matrix.h
 * 
 * This file is part of the "GaussianLib" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef GS_MATRIX_H
#define GS_MATRIX_H


#include "Real.h"
#include "Assert.h"
#include "Macros.h"
#include "Tags.h"
#include "Rotate.h"
#include "MatrixInitializer.h"

#include <cmath>
#include <cstring>
#include <algorithm>
#include <cstdint>
#include <initializer_list>


namespace Gs
{


#define GS_ASSERT_NxN_MATRIX \
    static_assert(Rows == Cols, GS_FILE_LINE "function can only be used with NxN matrices")

#ifdef GS_ROW_MAJOR_STORAGE
#   define GS_FOREACH_ROW_COL(r, c)             \
        for (std::size_t r = 0; r < Rows; ++r)  \
        for (std::size_t c = 0; c < Cols; ++c)
#else
#   define GS_FOREACH_ROW_COL(r, c)             \
        for (std::size_t c = 0; c < Cols; ++c)  \
        for (std::size_t r = 0; r < Rows; ++r)
#endif

/**
\brief Base matrix class.
\tparam T Specifies the data type of the matrix components.
This should be a primitive data type such as float, double, int etc.
\remarks The macro GS_ROW_MAJOR_STORAGE can be defined, to use row-major storage layout.
By default column-major storage layout is used.
The macro GS_ROW_VECTORS can be defined, to use row vectors. By default column vectors are used.
Here is an example, how a 4x4 matrix is laid-out with column- and row vectors:
\code
// 4x4 matrix with column vectors:
// / x1 y1 z1 w1 \
// | x2 y2 z2 w2 |
// | x3 y3 z3 w3 |
// \ x4 y4 z4 w4 /

// 4x4 matrix with row vectors:
// / x1 x2 x3 x4 \
// | y1 y2 y3 y4 |
// | z1 z2 z3 z4 |
// \ w1 w2 w3 w4 /

// In both cases, (w1, w2, w3, w4) stores the position in an affine transformation.
\endcode
Matrix elements can be accessed by the bracket operator:
\code
Gs::Matrix4 A;
A(0, 0) = row0column0;
A(2, 1) = row2column1;
\endcode
This is independent of the matrix storage layout and independent of the usage of row- or column vectors.
But the following function is dependent of the usage of row- or column vectors:
\code
// For column vectors:
A.At(2, 1) = row2column1;

// For row vectors:
A.At(2, 1) = row1column2;
\endcode
This function is used for easier support between row- and column vectors.
*/
template <typename T, std::size_t Rows, std::size_t Cols>
class Matrix
{

    public:

        static_assert(Rows*Cols > 0, "matrices must consist of at least 1x1 elements");

        /* ----- Static members ----- */

        //! Number of rows of this matrix type.
        static const std::size_t rows       = Rows;

        //! Number of columns of this matrix type.
        static const std::size_t columns    = Cols;

        //! Number of scalar elements of this matrix type.
        static const std::size_t elements   = Rows*Cols;

        /* ----- Typenames ----- */

        //! Specifies the typename of the scalar components.
        using ScalarType        = T;

        //! Typename of this matrix type.
        using ThisType          = Matrix<T, Rows, Cols>;

        //! Typename of the transposed of this matrix type.
        using TransposedType    = Matrix<T, Cols, Rows>;

        /* ----- Functions ----- */

        /**
        \brief Default constructor.
        \remarks If the 'GS_DISABLE_AUTO_INIT' is NOT defined, the matrix elements will be initialized. Otherwise, the matrix is in an uninitialized state.
        */
        Matrix()
        {
            #ifndef GS_DISABLE_AUTO_INIT
            Details::MatrixDefaultInitializer<T, Rows, Cols>::Initialize(*this);
            #endif
        }

        //! Copy constructor.
        Matrix(const ThisType& rhs)
        {
            *this = rhs;
        }

        //! Initializes this matrix with the specified values (row by row, and column by column).
        Matrix(const std::initializer_list<T>& values)
        {
            std::size_t i = 0, n = values.size();
            for (auto it = values.begin(); i < n; ++i, ++it)
                (*this)(i / columns, i % columns) = *it;
            for (; i < elements; ++i)
                (*this)(i / columns, i % columns) = T(0);
        }

        /**
        \brief Explicitly uninitialization constructor.
        \remarks With this constructor, the matrix is always in an uninitialized state.
        */
        explicit Matrix(UninitializeTag)
        {
            // do nothing
        }

        /**
        \brief Returns a reference to a single matrix element at the specified location.
        \param[in] row Specifies the zero-based row index. Must be in the range [0, Rows).
        \param[in] col Specifies the zero-based column index. Must be in the range [0, Cols).
        \throws std::runtime_error If the macro 'GS_ENABLE_ASSERT' and the macro 'GS_ASSERT_EXCEPTION' are defined,
        and either the row or the column is out of range.
        */
        T& operator () (std::size_t row, std::size_t col)
        {
            GS_ASSERT(row < Rows);
            GS_ASSERT(col < Cols);
            #ifdef GS_ROW_MAJOR_STORAGE
            return m_[row*Cols + col];
            #else
            return m_[col*Rows + row];
            #endif
        }

        /**
        \brief Returns a constant reference to a single matrix element at the specified location.
        \param[in] row Specifies the zero-based row index. Must be in the range [0, Rows).
        \param[in] col Specifies the zero-based column index. Must be in the range [0, Cols).
        \throws std::runtime_error If the macro 'GS_ENABLE_ASSERT' and the macro 'GS_ASSERT_EXCEPTION' are defined,
        and either the row or the column is out of range.
        */
        const T& operator () (std::size_t row, std::size_t col) const
        {
            GS_ASSERT(row < Rows);
            GS_ASSERT(col < Cols);
            #ifdef GS_ROW_MAJOR_STORAGE
            return m_[row*Cols + col];
            #else
            return m_[col*Rows + row];
            #endif
        }

        T& operator [] (std::size_t element)
        {
            GS_ASSERT(element < ThisType::elements);
            return m_[element];
        }

        const T& operator [] (std::size_t element) const
        {
            GS_ASSERT(element < ThisType::elements);
            return m_[element];
        }

        ThisType& operator += (const ThisType& rhs)
        {
            for (std::size_t i = 0; i < ThisType::elements; ++i)
                m_[i] += rhs.m_[i];
            return *this;
        }

        ThisType& operator -= (const ThisType& rhs)
        {
            for (std::size_t i = 0; i < ThisType::elements; ++i)
                m_[i] -= rhs.m_[i];
            return *this;
        }

        ThisType& operator *= (const ThisType& rhs)
        {
            GS_ASSERT_NxN_MATRIX;
            *this = (*this * rhs);
            return *this;
        }

        ThisType& operator *= (const T& rhs)
        {
            for (std::size_t i = 0; i < ThisType::elements; ++i)
                m_[i] *= rhs;
            return *this;
        }

        ThisType& operator = (const ThisType& rhs)
        {
            for (std::size_t i = 0; i < ThisType::elements; ++i)
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

        //! Restes all matrix elements to zero.
        void Reset()
        {
            for (std::size_t i = 0; i < ThisType::elements; ++i)
                m_[i] = T(0);
        }

        //! Loads the identity for this matrix.
        void LoadIdentity()
        {
            GS_ASSERT_NxN_MATRIX;
            GS_FOREACH_ROW_COL(r, c)
            {
                (*this)(r, c) = (r == c ? T(1) : T(0));
            }
        }

        //! Returns an identity matrix.
        static ThisType Identity()
        {
            ThisType result;
            result.LoadIdentity();
            return result;
        }

        //! Returns a transposed copy of this matrix.
        TransposedType Transposed() const
        {
            TransposedType result;

            GS_FOREACH_ROW_COL(r, c)
            {
                result(c, r) = (*this)(r, c);
            }

            return result;
        }

        //! Transposes this matrix.
        void Transpose()
        {
            GS_ASSERT_NxN_MATRIX;

            for (std::size_t i = 0; i + 1 < Cols; ++i)
            {
                for (std::size_t j = 1; j + i < Cols; ++j)
                {
                    std::swap(
                        m_[i*(Cols + 1) + j],
                        m_[(j + i)*Cols + i]
                    );
                }
            }
        }

        /**
        \brief Returns the determinant of this matrix.
        \see Gs::Determinant
        */
        T Determinant() const
        {
            return Gs::Determinant(*this);
        }

        /**
        Returns the trace of this matrix: M(0, 0) + M(1, 1) + ... + M(N - 1, N - 1).
        \note This can only be used for squared matrices!
        */
        T Trace() const
        {
            static_assert(Rows == Cols, "traces can only be computed for squared matrices");

            T trace = T(0);

            for (std::size_t i = 0; i < Rows; ++i)
                trace += (*this)(i, i);

            return trace;
        }

        Matrix<T, Rows, Cols> Inverse() const
        {
            Matrix<T, Rows, Cols> inv { *this };
            inv.MakeInverse();
            return inv;
        }

        bool MakeInverse()
        {
            Matrix<T, Rows, Cols> in { *this };
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

        /**
        Returns a type casted instance of this matrix.
        \tparam C Specifies the static cast type.
        */
        template <typename C> Matrix<C, Rows, Cols> Cast() const
        {
            Matrix<C, Rows, Cols> result { UninitializeTag{} };

            for (std::size_t i = 0; i < ThisType::elements; ++i)
                result[i] = static_cast<C>(m_[i]);

            return result;
        }

    private:

        T m_[ThisType::elements];

};


/* --- Global Operators --- */

template <typename T, std::size_t Rows, std::size_t Cols>
Matrix<T, Rows, Cols> operator + (const Matrix<T, Rows, Cols>& lhs, const Matrix<T, Rows, Cols>& rhs)
{
    auto result = lhs;
    result += rhs;
    return result;
}

template <typename T, std::size_t Rows, std::size_t Cols>
Matrix<T, Rows, Cols> operator - (const Matrix<T, Rows, Cols>& lhs, const Matrix<T, Rows, Cols>& rhs)
{
    auto result = lhs;
    result -= rhs;
    return result;
}

template <typename T, std::size_t Rows, std::size_t Cols>
Matrix<T, Rows, Cols> operator * (const Matrix<T, Rows, Cols>& lhs, const T& rhs)
{
    auto result = lhs;
    result *= rhs;
    return result;
}

template <typename T, std::size_t Rows, std::size_t Cols>
Matrix<T, Rows, Cols> operator * (const T& lhs, const Matrix<T, Rows, Cols>& rhs)
{
    auto result = rhs;
    result *= lhs;
    return result;
}

template <typename T, std::size_t Rows, std::size_t ColsRows, std::size_t Cols>
Matrix<T, Rows, Cols> operator * (const Matrix<T, Rows, ColsRows>& lhs, const Matrix<T, ColsRows, Cols>& rhs)
{
    Matrix<T, Rows, Cols> result { UninitializeTag{} };

    GS_FOREACH_ROW_COL(r, c)
    {
        result(r, c) = T(0);
        for (std::size_t i = 0; i < ColsRows; ++i)
            result(r, c) += lhs(r, i)*rhs(i, c);
    }

    return result;
}


/* --- Type Alias --- */

#define GS_DEF_MATRIX_TYPES_MxN(m, n)                               \
    template <typename T> using Matrix##m##n##T = Matrix<T, m, n>;  \
    using Matrix##m##n      = Matrix##m##n##T<Real>;                \
    using Matrix##m##n##f   = Matrix##m##n##T<float>;               \
    using Matrix##m##n##d   = Matrix##m##n##T<double>;              \
    using Matrix##m##n##i   = Matrix##m##n##T<std::int32_t>;        \
    using Matrix##m##n##ui  = Matrix##m##n##T<std::uint32_t>;       \
    using Matrix##m##n##b   = Matrix##m##n##T<std::int8_t>;         \
    using Matrix##m##n##ub  = Matrix##m##n##T<std::uint8_t>

GS_DEF_MATRIX_TYPES_MxN(3, 4);
GS_DEF_MATRIX_TYPES_MxN(4, 3);

#define GS_DEF_MATRIX_TYPES_NxN(n)                              \
    template <typename T> using Matrix##n##T = Matrix<T, n, n>; \
    using Matrix##n     = Matrix##n##T<Real>;                   \
    using Matrix##n##f  = Matrix##n##T<float>;                  \
    using Matrix##n##d  = Matrix##n##T<double>;                 \
    using Matrix##n##i  = Matrix##n##T<std::int32_t>;           \
    using Matrix##n##ui = Matrix##n##T<std::uint32_t>;          \
    using Matrix##n##b  = Matrix##n##T<std::int8_t>;            \
    using Matrix##n##ub = Matrix##n##T<std::uint8_t>

GS_DEF_MATRIX_TYPES_NxN(2);
GS_DEF_MATRIX_TYPES_NxN(3);
GS_DEF_MATRIX_TYPES_NxN(4);

#undef GS_DEF_MATRIX_TYPES_MxN
#undef GS_DEF_MATRIX_TYPES_NxN

#undef GS_ASSERT_NxN_MATRIX
#undef GS_FOREACH_ROW_COL


} // /namespace Gs


#endif



// ================================================================================
