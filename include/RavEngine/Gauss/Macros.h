/*
 * Macros.h
 * 
 * This file is part of the "GaussianLib" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef GS_MACROS_H
#define GS_MACROS_H


#include "Config.h"


#define GS_TOSTRING_PRIMARY(x)  #x
#define GS_TOSTRING(x)          GS_TOSTRING_PRIMARY(x)
#define GS_FILE_LINE            __FILE__ " (" GS_TOSTRING(__LINE__) "): "

#ifdef GS_ROW_VECTORS

#define GS_ASSERT_MxN_MATRIX(info, T, n, m)                 \
    static_assert(                                          \
        T::rows >= m && T::columns >= n,                    \
        info " requires at least a " #m "x" #n " matrix"    \
    )

#else

#define GS_ASSERT_MxN_MATRIX(info, T, m, n)                 \
    static_assert(                                          \
        T::rows >= m && T::columns >= n,                    \
        info " requires at least a " #m "x" #n " matrix"    \
    )

#endif


#endif



// ================================================================================
