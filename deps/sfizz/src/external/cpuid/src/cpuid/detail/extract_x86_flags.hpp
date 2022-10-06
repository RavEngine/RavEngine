// Copyright (c) 2013 Steinwurf ApS
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#pragma once

#include <cstdint>

#include "cpuinfo_impl.hpp"

namespace cpuid
{
inline namespace STEINWURF_CPUID_VERSION
{
void extract_x86_flags(cpuinfo::impl& info, uint32_t ecx, uint32_t edx)
{
    // Instruction set flags

    info.m_has_fpu = (edx & (1 << 0)) != 0;
    info.m_has_mmx = (edx & (1 << 23)) != 0;
    info.m_has_sse = (edx & (1 << 25)) != 0;
    info.m_has_sse2 = (edx & (1 << 26)) != 0;
    info.m_has_sse3 = (ecx & (1 << 0)) != 0;
    info.m_has_ssse3 = (ecx & (1 << 9)) != 0;
    info.m_has_sse4_1 = (ecx & (1 << 19)) != 0;
    info.m_has_sse4_2 = (ecx & (1 << 20)) != 0;
    info.m_has_pclmulqdq = (ecx & (1 << 1)) != 0;
    info.m_has_avx = (ecx & (1 << 28)) != 0;
}

void extract_x86_extended_flags(cpuinfo::impl& info, uint32_t ebx)
{
    // Extended instruction set flags

    info.m_has_avx2 = (ebx & (1 << 5)) != 0;
}
}
}
