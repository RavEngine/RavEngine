// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

// We include the ETW headers in a special way:
#include <windows.h>
#include <evntrace.h>

VOID NTAPI EtwEnableCallback(__in LPCGUID, __in ULONG, __in UCHAR, __in ULONGLONG, __in ULONGLONG,
    __in_opt PEVENT_FILTER_DESCRIPTOR, __inout_opt PVOID);
#define MCGEN_PRIVATE_ENABLE_CALLBACK_V2 EtwEnableCallback

// This redefinition is necessary because this lib is linked into d3d12.dll, which is itself an ETW provider.
// If the name is left the same, then we run into compile-time confusion between d3d12's own copy of the
// McGenControlCallbackV2 inline function and that of WinPixEventEncoder.lib (which is the binary into which
// this code is compiled within the OS). The result of that confusion is that d3d12's ETW callback is
// called from PIX's enablement macros.
#define McGenControlCallbackV2 PixMcGenControlCallbackV2

#include <PIXETW.h>

