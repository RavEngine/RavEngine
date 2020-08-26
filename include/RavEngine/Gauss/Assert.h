/*
 * Assert.h
 * 
 * This file is part of the "GaussianLib" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef GS_ASSERT_H
#define GS_ASSERT_H


#include "Config.h"
#include "Macros.h"

#include <cassert>


#ifdef GS_ENABLE_ASSERT
#   ifdef GS_ASSERT_EXCEPTION
#       include <stdexcept>
#       define GS_ASSERT(expr)                                  \
            if (!(expr))                                        \
            {                                                   \
                throw std::runtime_error(                       \
                    "assertion failed: (" #expr "), file "      \
                    __FILE__ ", line " GS_TOSTRING(__LINE__)    \
                );                                              \
            }
#   else
#       define GS_ASSERT(expr) assert((expr))
#   endif
#else
#   define GS_ASSERT(expr)
#endif


#endif



// ================================================================================
