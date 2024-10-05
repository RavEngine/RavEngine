#pragma once

#if ((_WIN32 && !defined(_M_ARM64)))
#define XR_AVAILABLE 1
#else
#define XR_AVAILABLE 0
#endif