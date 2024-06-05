// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <lib/IncludePixEtw.h>
#include <lib/WinPixEventRuntime.h>

#include <atomic>

// Suppress "warning C4447: 'main' signature found without threading mode"
// warning.  This doesn't apply to us as we're not a WinRT dll.
#pragma warning(disable:4447)

static std::atomic<bool> g_detaching;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        WinPixEventRuntime::Initialize();
        EventRegisterMicrosoft_Graphics_Tools_PixMarkers();
        break;

    case DLL_PROCESS_DETACH:
        g_detaching = true;
        WinPixEventRuntime::Shutdown();
        EventUnregisterMicrosoft_Graphics_Tools_PixMarkers();
        break;
    }

    return TRUE;
}

//
// ETW calls this callback when the provider is enabled / disabled etc.
//
VOID NTAPI EtwEnableCallback(
    __in LPCGUID,
    __in ULONG controlCode,
    __in UCHAR,
    __in ULONGLONG,
    __in ULONGLONG,
    __in_opt PEVENT_FILTER_DESCRIPTOR,
    __inout_opt PVOID)
{
    // Don't try and process ETW commands while we're shutting down
    if (g_detaching)
        return;

    switch (controlCode)
    {
    case EVENT_CONTROL_CODE_ENABLE_PROVIDER:
        WinPixEventRuntime::EnableCapture();
        break;
    case EVENT_CONTROL_CODE_DISABLE_PROVIDER:
        WinPixEventRuntime::DisableCapture();
        break;
    case EVENT_CONTROL_CODE_CAPTURE_STATE:  
        WinPixEventRuntime::FlushCapture();
        break;
    }
}

