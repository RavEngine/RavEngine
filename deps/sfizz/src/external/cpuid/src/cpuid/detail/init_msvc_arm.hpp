// Copyright (c) 2013 Steinwurf ApS
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#pragma once

#include "cpuinfo_impl.hpp"

namespace cpuid
{
inline namespace STEINWURF_CPUID_VERSION
{
void init_cpuinfo(cpuinfo::impl& info)
{
    // Visual Studio 2012 (and above) guarantees the NEON capability when
    // compiling for Windows Phone 8 (and above)

#if defined(PLATFORM_WINDOWS_PHONE)
    info.m_has_neon = true;
#else
    info.m_has_neon = false;
#endif
}
}
}
