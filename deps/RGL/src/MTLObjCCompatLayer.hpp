#pragma once
#include <objc/objc.h>

// Because Apple likes OBJC, we have to
// include this nasty hack that removes type information but retains size information
// when compiled as a standard c++ header instead of an OBJC++ header.
// Instead of using id<T>, use OBJC_ID(T). #import objc headers before #including this header. 

#ifdef __OBJC__
#define OBJC_ID(a) id< a >
#define APPLE_API_PTR(a) a *
#define APPLE_API_TYPE(a) a
#else
#define OBJC_ID(a) id
#define APPLE_API_PTR(a) void *
#define APPLE_API_TYPE(a) int

#endif
