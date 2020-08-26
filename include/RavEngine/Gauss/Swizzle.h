/*
 * Swizzle.h
 * 
 * This file is part of the "GaussianLib" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef GS_SWIZZLE_H
#define GS_SWIZZLE_H


#include "Decl.h"
#include "Config.h"


#define GS_DEF_SWIZZLE_REF2(v0, v1)     \
    Vector<T, 2> v0##v1() const         \
    {                                   \
        return Vector<T, 2>(v0, v1);    \
    }

#define GS_DEF_SWIZZLE_REF3(v0, v1, v2)     \
    Vector<T, 3> v0##v1##v2() const         \
    {                                       \
        return Vector<T, 3>(v0, v1, v2);    \
    }

#define GS_DEF_SWIZZLE_REF4(v0, v1, v2, v3)     \
    Vector<T, 4> v0##v1##v2##v3() const         \
    {                                           \
        return Vector<T, 4>(v0, v1, v2, v3);    \
    }


#endif



// ================================================================================
