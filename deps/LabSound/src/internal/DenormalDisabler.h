// License: BSD 2 Clause
// Copyright (C) 2011, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef DenormalDisabler_h
#define DenormalDisabler_h

#include "LabSound/core/Macros.h"

// Deal with denormals. They can very seriously impact performance on x86.

// Define HAVE_DENORMAL if we support flushing denormals to zero.
#if defined(LABSOUND_PLATFORM_WINDOWS)
#define HAVE_DENORMAL
#endif
#include <float.h>

#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
#define HAVE_DENORMAL
#endif

namespace lab
{

#ifdef HAVE_DENORMAL
class DenormalDisabler
{
public:
    DenormalDisabler()
        : m_savedCSR(0)
    {
#if defined(LABSOUND_PLATFORM_WINDOWS)
        // Save the current state, and set mode to flush denormals.
        //
        // http://stackoverflow.com/questions/637175/possible-bug-in-controlfp-s-may-not-restore-control-word-correctly
        _controlfp_s(&m_savedCSR, 0, 0);
        unsigned int unused;
        _controlfp_s(&unused, _DN_FLUSH, _MCW_DN);
#else
        m_savedCSR = getCSR();
        setCSR(m_savedCSR | 0x8040);
#endif
    }

    ~DenormalDisabler()
    {
#if defined(LABSOUND_PLATFORM_WINDOWS)
        unsigned int unused;
        _controlfp_s(&unused, m_savedCSR, _MCW_DN);
#else
        setCSR(m_savedCSR);
#endif
    }

    // This is a nop if we can flush denormals to zero in hardware.
    static inline float flushDenormalFloatToZero(float f)
    {
#if defined(LABSOUND_PLATFORM_WINDOWS) && (!_M_IX86_FP)
        // For systems using x87 instead of sse, there's no hardware support
        // to flush denormals automatically. Hence, we need to flush
        // denormals to zero manually.
        return (fabs(f) < FLT_MIN) ? 0.0f : f;
#else
        return f;
#endif
    }

private:
#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
    inline int getCSR()
    {
        int result;
        asm volatile("stmxcsr %0"
                     : "=m"(result));
        return result;
    }

    inline void setCSR(int a)
    {
        int temp = a;
        asm volatile("ldmxcsr %0"
                     :
                     : "m"(temp));
    }

#endif

    unsigned int m_savedCSR;
};

#else
// FIXME: add implementations for other architectures and compilers
class DenormalDisabler
{
public:
    DenormalDisabler() {}

    // Assume the worst case that other architectures and compilers
    // need to flush denormals to zero manually.
    static inline float flushDenormalFloatToZero(float f)
    {
        return (fabs(f) < FLT_MIN) ? 0.0f : f;
    }
};

#endif

}  // lab

#undef HAVE_DENORMAL
#endif  // DenormalDisabler_h
