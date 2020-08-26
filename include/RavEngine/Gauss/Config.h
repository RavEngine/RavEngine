/*
 * Config.h
 * 
 * This file is part of the "GaussianLib" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef GS_CONFIG_H
#define GS_CONFIG_H


#ifdef _DEBUG
#   define GS_ENABLE_ASSERT
#endif

//! Uses double precision floating-points for real types.
//#define GS_REAL_DOUBLE

//! Uses a runtime exception instead of an assert.
//#define GS_ASSERT_EXCEPTION

//! Enables the swizzle operator in the vector classes. If undefined, swizzle operator is disabled (default).
//#define GS_ENABLE_SWIZZLE_OPERATOR

//! Enables the inverse matrix operator, i.e. allows "A^-1" expressions as shortcut for "A.Inverse()".
//#define GS_ENABLE_INVERSE_OPERATOR

//! Disables automatic data initialization. If undefined, automatic initialization is enabled (default).
//#define GS_DISABLE_AUTO_INIT

//! Enables row-major storage. If undefined, column-major storage is used (default).
//#define GS_ROW_MAJOR_STORAGE

//! Enables row vectors. If undefined, column vectors are used (default).
//#define GS_ROW_VECTORS


#endif



// ================================================================================
