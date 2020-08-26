/*
 * MatrixInitializer.h
 * 
 * This file is part of the "GaussianLib" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef GS_MATRIX_INITIALIZER_H
#define GS_MATRIX_INITIALIZER_H


#include "Decl.h"


namespace Gs
{


namespace Details
{


template <typename T, std::size_t Rows, std::size_t Cols>
struct MatrixDefaultInitializer
{
    static void Initialize(Matrix<T, Rows, Cols>& matrix)
    {
        matrix.Reset();
    }
};

template <typename T, std::size_t N>
struct MatrixDefaultInitializer<T, N, N>
{
    static void Initialize(Matrix<T, N, N>& matrix)
    {
        matrix.LoadIdentity();
    }
};


} // /namespace Details


} // /namespace Gs


#endif



// ================================================================================
