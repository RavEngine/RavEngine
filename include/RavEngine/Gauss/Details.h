/*
 * Details.h
 *
 * This file is part of the "GaussianLib" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef GS_DETAILS_H
#define GS_DETAILS_H


#include "Decl.h"

#include <vector>


namespace Gs
{


// Determinant
template <typename T, std::size_t N>
T Determinant(const Matrix<T, N, N>&);

// Inverse
//template <typename T, std::size_t N>
//bool Inverse(Matrix<T, N, N>&, const Matrix<T, N, N>&);


namespace Details
{


//! Internal class for implementation details.
template <template <typename, std::size_t, std::size_t> class M, typename T, std::size_t Rows, std::size_t Cols>
class MatrixHelper
{

        MatrixHelper() = delete;

    protected:

        friend T Gs::Determinant<T, Rows>(const Matrix<T, Rows, Cols>&);
        //friend bool Gs::Inverse<T, Rows>(Matrix<T, Rows, Cols>&, const Matrix<T, Rows, Cols>&);

        static std::vector<T> MatrixToArray(const M<T, Rows, Cols>& mat)
        {
            std::vector<T> vec(Rows*Cols);

            for (std::size_t r = 0, i = 0; r < Rows; ++r)
            {
                for (std::size_t c = 0; c < Cols; ++c)
                    vec[i++] = mat(r, c);
            }

            return vec;
        }

        static T OrderedDeterminant(const std::vector<T>& mat, std::size_t order)
        {
            if (order == 1)
                return mat[0];

            std::vector<T> minorMat((order - 1)*(order - 1), T());

            T det = T(0);

            for (std::size_t i = 0; i < order; ++i)
            {
                GetMinorMatrix(mat, minorMat, 0, i, order);
                if (i % 2 == 1)
                    det -= mat[i] * OrderedDeterminant(minorMat, order - 1);
                else
                    det += mat[i] * OrderedDeterminant(minorMat, order - 1);
            }

            return det;
        }

        static void OrderedInverse(const std::vector<T>& mat, std::size_t order)
        {
            T det = OrderedDeterminant(mat, order);

            //todo...
        }

    private:

        static void GetMinorMatrix(
            const std::vector<T>& mat, std::vector<T>& minorMat, std::size_t row, std::size_t column, std::size_t order)
        {
            for (std::size_t r = 1, i = 0; r < order; ++r)
            {
                if (r != row)
                {
                    for (std::size_t c = 0, j = 0; c < order; ++c)
                    {
                        if (c != column)
                        {
                            minorMat[i*(order - 1) + j] = mat[r*order + c];
                            ++j;
                        }
                    }
                    ++i;
                }
            }
        }

};


} // /namespace Details


} // /namespace Gs


#endif



// ================================================================================
