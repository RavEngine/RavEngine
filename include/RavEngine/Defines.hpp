#pragma once

#if ((_WIN32 && !defined(_M_ARM64)))
#define XR_AVAILABLE 1
#else
#define XR_AVAILABLE 0
#endif

#if __aarch64__ || __arm__
#define RVE_TBDR 1
#else
#define RVE_TBDR 0
#endif
