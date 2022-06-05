#pragma once

#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP)
#define _UWP 1   
#else
#define _UWP 0
#endif

#define XR_AVAILABLE ((_WIN32 && !_UWP && !defined(_M_ARM64)) || __linux__)