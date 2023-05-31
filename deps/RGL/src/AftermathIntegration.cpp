#if RGL_AFTERMATH_AVAILABLE
#include <GFSDK_Aftermath.h>
#include <GFSDK_Aftermath_GpuCrashDump.h>
#include <GFSDK_Aftermath_GpuCrashDumpDecoding.h>
#include "AftermathIntegration.hpp"
#include <filesystem>
#include <format>
#include <fstream>

// In addition to RGL library copyright:
//*********************************************************
//
// Copyright (c) 2019-2022, NVIDIA CORPORATION. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//
//*********************************************************


namespace RGL {

    void AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_Result result) {
        if (result != GFSDK_Aftermath_Result::GFSDK_Aftermath_Result_Success) {
            throw std::runtime_error(std::format("Aftermath failure: {}",(int)result));
        }
    }

    void AftermathShaderDebugInfoLookupCallback(const GFSDK_Aftermath_ShaderDebugInfoIdentifier* pIdentifier,
        PFN_GFSDK_Aftermath_SetData setShaderDebugInfo,
        void* pUserData) {

    }

    void AftermathShaderLookupCallback(
        const GFSDK_Aftermath_ShaderBinaryHash* pShaderHash,
        PFN_GFSDK_Aftermath_SetData setShaderBinary,
        void* pUserData)
    {

    }

    void AftermathShaderSourceDebugInfoLookupCallback(
        const GFSDK_Aftermath_ShaderDebugName* pShaderDebugName,
        PFN_GFSDK_Aftermath_SetData setShaderBinary,
        void* pUserData)
    {

    }

    void AftermathShaderDebugInfoCallback(const void* pShaderDebugInfo, const uint32_t shaderDebugInfoSize, void* pUserData) {

    }

    void AftermathCrashDumpDescriptionCallback(PFN_GFSDK_Aftermath_AddGpuCrashDumpDescription addValue, void* pUserData) {
        addValue(0, "RGL Aftermath crash dump");    //TODO: add more data
    }

	void AftermathCrashDumpCallback(const void* pGpuCrashDump, const uint32_t gpuCrashDumpSize, void* pUserData) {
        // Create a GPU crash dump decoder object for the GPU crash dump.
        GFSDK_Aftermath_GpuCrashDump_Decoder decoder = {};
        AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GpuCrashDump_CreateDecoder(
            GFSDK_Aftermath_Version_API,
            pGpuCrashDump,
            gpuCrashDumpSize,
            &decoder));

        // Use the decoder object to read basic information, like application
        // name, PID, etc. from the GPU crash dump.
        GFSDK_Aftermath_GpuCrashDump_BaseInfo baseInfo = {};
        AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GpuCrashDump_GetBaseInfo(decoder, &baseInfo));

        // Create a unique file name for writing the crash dump data to a file.
        // Note: due to an Nsight Aftermath bug (will be fixed in an upcoming
        // driver release) we may see redundant crash dumps. As a workaround,
        // attach a unique count to each generated file name.
        static int count = 0;
        const std::string baseFileName =
            std::string("Aftermath")
            + "-"
            + std::to_string(baseInfo.pid)
            + "-"
            + std::to_string(++count);

        // Write the crash dump data to a file using the .nv-gpudmp extension
        // registered with Nsight Graphics.
        const std::string crashDumpFileName = baseFileName + ".nv-gpudmp";
        std::ofstream dumpFile(crashDumpFileName, std::ios::out | std::ios::binary);
        if (dumpFile)
        {
            dumpFile.write((const char*)pGpuCrashDump, gpuCrashDumpSize);
            dumpFile.close();
        }

        // Decode the crash dump to a JSON string.
        // Step 1: Generate the JSON and get the size.
        uint32_t jsonSize = 0;
        AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GpuCrashDump_GenerateJSON(
            decoder,
            GFSDK_Aftermath_GpuCrashDumpDecoderFlags_ALL_INFO,
            GFSDK_Aftermath_GpuCrashDumpFormatterFlags_NONE,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            &jsonSize));
        // Step 2: Allocate a buffer and fetch the generated JSON.
        std::vector<char> json(jsonSize);
        AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GpuCrashDump_GetJSON(
            decoder,
            uint32_t(json.size()),
            json.data()));

        // Write the crash dump data as JSON to a file.
        const std::string jsonFileName = crashDumpFileName + ".json";
        std::ofstream jsonFile(jsonFileName, std::ios::out | std::ios::binary);
        if (jsonFile)
        {
            // Write the JSON to the file (excluding string termination)
            jsonFile.write(json.data(), json.size() - 1);
            jsonFile.close();
        }

        // Destroy the GPU crash dump decoder object.
        AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GpuCrashDump_DestroyDecoder(decoder));
	}


	void InitializeAftermath()
	{
		GFSDK_Aftermath_EnableGpuCrashDumps(GFSDK_Aftermath_Version_API, GFSDK_Aftermath_GpuCrashDumpWatchedApiFlags_DX, GFSDK_Aftermath_GpuCrashDumpFeatureFlags_Default, AftermathCrashDumpCallback, nullptr, nullptr, nullptr,nullptr);
	}
	void DeinitAftermath()
	{
		GFSDK_Aftermath_DisableGpuCrashDumps();
	}
}
#else

namespace RGL {
    // stub these
    void InitializeAftermath()
    {
       
    }
    void DeinitAftermath()
    {
    }
}
#endif