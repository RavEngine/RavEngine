/*
 * Compare.h
 * 
 * This file is part of the "GaussianLib" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef GS_COMPARE_H
#define GS_COMPARE_H


#include "Real.h"
#include "Epsilon.h"
#include "Vector.h"
#include "Quaternion.h"
#include "Matrix.h"


namespace Gs
{


/* --- Global Functions --- */

/**
\brief Compares the two objects in a strict-weak-ordering relation.
\return True if the 'lhs' object precedes the 'rhs' object.
\remarks This "Compare" function can be used to sort vectors in an ascending order
where the first vector component is the most significant sorting attribute.
\see https://www.sgi.com/tech/stl/StrictWeakOrdering.html
*/
template <typename T, std::size_t N>
bool Compare(const Vector<T, N>& lhs, const Vector<T, N>& rhs)
{
    for (std::size_t i = 0; i < N; ++i)
    {
        if (lhs[i] < rhs[i])
            return true;
        if (lhs[i] > rhs[i])
            return false;
    }
    return false;
}

/**
\brief Compares the two objects in a strict-weak-ordering relation.
\return True if the 'lhs' object precedes the 'rhs' object.
\remarks This "Compare" function can be used to sort quaternions in an ascending order
where the first quaterion component is the most significant sorting attribute.
\see https://www.sgi.com/tech/stl/StrictWeakOrdering.html
*/
template <typename T>
bool Compare(const QuaternionT<T>& lhs, const QuaternionT<T>& rhs)
{
    for (std::size_t i = 0; i < QuaternionT<T>::components; ++i)
    {
        if (lhs[i] < rhs[i])
            return true;
        if (lhs[i] > rhs[i])
            return false;
    }
    return false;
}

/**
\brief Compares the two objects in a strict-weak-ordering relation.
\return True if the 'lhs' object precedes the 'rhs' object.
\remarks This "Compare" function can be used to sort matrices in an ascending order
where the matrix element in the 1st row and the 1st column is the most significant sorting attribute,
the matrix element in the 2nd row and the 1st column is the next most significant sorting attribute, and so on.
\see https://www.sgi.com/tech/stl/StrictWeakOrdering.html
*/
template <typename T, std::size_t Rows, std::size_t Cols>
bool Compare(const Matrix<T, Rows, Cols>& lhs, const Matrix<T, Rows, Cols>& rhs)
{
    /*
    Alwasy compare in the same order (no matter if row-major storage is used or not),
    to ensure always the same sorting results for strict-weak-ordering
    */
    for (std::size_t c = 0; c < Cols; ++c)
    {
        for (std::size_t r = 0; r < Rows; ++r)
        {
            if (lhs(r, c) < rhs(r, c))
                return true;
            if (lhs(r, c) > rhs(r, c))
                return false;
        }
    }
    return false;
}


} // /namespace Gs


#endif



// ================================================================================
