// Copyright (c) 2013 Steinwurf ApS
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#pragma once

#include <string>

namespace cpuid
{
/// Here we define the STEINWURF_CPUID_VERSION this should be updated on each
/// release
#define STEINWURF_CPUID_VERSION v6_3_1

inline namespace STEINWURF_CPUID_VERSION
{
/// @return The version of the library as string
std::string version();
}
}
