/*
 * Decl.h
 * 
 * This file is part of the "GaussianLib" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef GS_DECL_H
#define GS_DECL_H


#include <cstddef>


namespace Gs
{


/* --- Forward Declarations --- */

// Matrices
template <typename T>
class AffineMatrix3T;

template <typename T>
class AffineMatrix4T;

template <typename T>
class ProjectionMatrix4T;

template <typename T, std::size_t Rows, std::size_t Cols>
class Matrix;

// Vectors
template <typename T, std::size_t N>
class Vector;

// Quaternions
template <typename T>
class QuaternionT;

// Spherical
template <typename T>
class SphericalT;


} // /namespace Gs


#endif



// ================================================================================
