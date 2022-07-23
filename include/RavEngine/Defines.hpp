#pragma once

#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP)
#define _UWP 1   
#else
#define _UWP 0
#endif

#if ((_WIN32 && !_UWP && !defined(_M_ARM64)))
#define XR_AVAILABLE 1
#else
#define XR_AVAILABLE 0
#endif

#if XR_AVAILABLE
#define XR_CHECK(a) {auto XR_CHECK_RESULT = a; assert(XR_CHECK_RESULT == XR_SUCCESS);}
// for disabling a check
#define dXR_CHECK(a) 
#endif
