// Copyright (c) 2013 Steinwurf ApS
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#pragma once

#include "../cpuinfo.hpp"

namespace cpuid
{
inline namespace STEINWURF_CPUID_VERSION
{

struct cpuinfo::impl
{
    impl() :
        m_has_fpu(false), m_has_mmx(false), m_has_sse(false), m_has_sse2(false),
        m_has_sse3(false), m_has_ssse3(false), m_has_sse4_1(false),
        m_has_sse4_2(false), m_has_pclmulqdq(false), m_has_avx(false),
        m_has_avx2(false), m_has_neon(false)
    {
    }

    bool m_has_fpu;
    bool m_has_mmx;
    bool m_has_sse;
    bool m_has_sse2;
    bool m_has_sse3;
    bool m_has_ssse3;
    bool m_has_sse4_1;
    bool m_has_sse4_2;
    bool m_has_pclmulqdq;
    bool m_has_avx;
    bool m_has_avx2;
    bool m_has_neon;
};
}
}
