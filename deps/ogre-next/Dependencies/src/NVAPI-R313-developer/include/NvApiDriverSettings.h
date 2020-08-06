/***************************************************************************\
|*                                                                           *|
|*      Copyright NVIDIA Corporation.  All rights reserved.                  *|
|*                                                                           *|
|*   NOTICE TO USER:                                                         *|
|*                                                                           *|
|*   This source code is subject to NVIDIA ownership rights under U.S.       *|
|*   and international Copyright laws.  Users and possessors of this         *|
|*   source code are hereby granted a nonexclusive, royalty-free             *|
|*   license to use this code in individual and commercial software.         *|
|*                                                                           *|
|*   NVIDIA MAKES NO REPRESENTATION ABOUT THE SUITABILITY OF THIS SOURCE     *|
|*   CODE FOR ANY PURPOSE. IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR         *|
|*   IMPLIED WARRANTY OF ANY KIND. NVIDIA DISCLAIMS ALL WARRANTIES WITH      *|
|*   REGARD TO THIS SOURCE CODE, INCLUDING ALL IMPLIED WARRANTIES OF         *|
|*   MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR          *|
|*   PURPOSE. IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY SPECIAL,            *|
|*   INDIRECT, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES          *|
|*   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN      *|
|*   AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING     *|
|*   OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOURCE      *|
|*   CODE.                                                                   *|
|*                                                                           *|
|*   U.S. Government End Users. This source code is a "commercial item"      *|
|*   as that term is defined at 48 C.F.R. 2.101 (OCT 1995), consisting       *|
|*   of "commercial computer  software" and "commercial computer software    *|
|*   documentation" as such terms are used in 48 C.F.R. 12.212 (SEPT 1995)   *|
|*   and is provided to the U.S. Government only as a commercial end item.   *|
|*   Consistent with 48 C.F.R.12.212 and 48 C.F.R. 227.7202-1 through        *|
|*   227.7202-4 (JUNE 1995), all U.S. Government End Users acquire the       *|
|*   source code with only those rights set forth herein.                    *|
|*                                                                           *|
|*   Any use of this source code in individual and commercial software must  *|
|*   include, in the user documentation and internal comments to the code,   *|
|*   the above Disclaimer and U.S. Government End Users Notice.              *|
|*                                                                           *|
|*                                                                           *|
\***************************************************************************/

#ifndef _NVAPI_DRIVER_SETTINGS_H_
#define _NVAPI_DRIVER_SETTINGS_H_

#define OGL_AA_LINE_GAMMA_STRING                   L"Antialiasing - Line gamma"
#define OGL_DEEP_COLOR_SCANOUT_STRING              L"Deep color for 3D applications"
#define OGL_DEFAULT_SWAP_INTERVAL_STRING           L"Controls the number of vblank signals from the display to wait before rendering a frame (SwapInterval) on OpenGL. In order to force VSYNC ON or OFF, use VSYNCMODE."
#define OGL_DEFAULT_SWAP_INTERVAL_FRACTIONAL_STRING L"Controls if we evaluate the current scan line for a (un)synced flip with negative intervals. A value in the range of 0 - 100%"
#define OGL_DEFAULT_SWAP_INTERVAL_SIGN_STRING      L"Controls if the number of SwapIntervals set is treated as negative or positive values on OpenGL."
#define OGL_EVENT_LOG_SEVERITY_THRESHOLD_STRING    L"Event Log Severity Threshold. This controls which events are logged."
#define OGL_EXTENSION_STRING_VERSION_STRING        L"Extension String version"
#define OGL_FORCE_BLIT_STRING                      L"Buffer-flipping mode"
#define OGL_FORCE_STEREO_STRING                    L"Force Stereo shuttering"
#define OGL_IMPLICIT_GPU_AFFINITY_STRING           L"Preferred OpenGL GPU"
#define OGL_MAX_FRAMES_ALLOWED_STRING              L"Maximum frames allowed"
#define OGL_MULTIMON_STRING                        L"Multi-display/mixed-GPU acceleration"
#define OGL_OVERLAY_PIXEL_TYPE_STRING              L"Exported Overlay pixel types"
#define OGL_OVERLAY_SUPPORT_STRING                 L"Enable overlay"
#define OGL_QUALITY_ENHANCEMENTS_STRING            L"High level control of the rendering quality on OpenGL"
#define OGL_SINGLE_BACKDEPTH_BUFFER_STRING         L"Unified back/depth buffer"
#define OGL_THREAD_CONTROL_STRING                  L"Threaded optimization"
#define OGL_TRIPLE_BUFFER_STRING                   L"Triple buffering"
#define OGL_VIDEO_EDITING_MODE_STRING              L"controls video-editing mode for OpenGL"
#define AA_BEHAVIOR_FLAGS_STRING                   L"Antialiasing - Behavior Flags"
#define AA_MODE_ALPHATOCOVERAGE_STRING             L"Antialiasing - Transparency Multisampling"
#define AA_MODE_GAMMACORRECTION_STRING             L"Antialiasing - Gamma correction"
#define AA_MODE_METHOD_STRING                      L"Antialiasing - Setting"
#define AA_MODE_REPLAY_STRING                      L"Antialiasing - Transparency Supersampling"
#define AA_MODE_SELECTOR_STRING                    L"Antialiasing - Mode"
#define AA_MODE_SELECTOR_SLIAA_STRING              L"Antialiasing - SLI AA"
#define ANISO_MODE_LEVEL_STRING                    L"Anisotropic filtering setting"
#define ANISO_MODE_SELECTOR_STRING                 L"Anisotropic filtering mode"
#define APPLICATION_PROFILE_NOTIFICATION_TIMEOUT_STRING L"Application Profile Notification Popup Timeout"
#define APPLICATION_STEAM_ID_STRING                L"This ID is used to identify which applications are installed"
#define CPL_HIDDEN_PROFILE_STRING                  L"Do not display this profile in the Control Panel"
#define CUDA_EXCLUDED_GPUS_STRING                  L"List of Universal GPU ids"
#define D3DOGL_GPU_MAX_POWER_STRING                L"Maximum Allowed GPU Power"
#define EXPORT_PERF_COUNTERS_STRING                L"Export Performance Counters"
#define FXAA_ALLOW_STRING                          L"NVIDIA Predefined FXAA Usage"
#define FXAA_ENABLE_STRING                         L"Toggle FXAA on or off"
#define FXAA_INDICATOR_ENABLE_STRING               L"Toggle FXAA Indicator on or off"
#define MCSFRSHOWSPLIT_STRING                      L"Show the SLI on-screen indicator"
#define OPTIMUS_DEBUG_STRING                       L"Debug bits for optimus"
#define OPTIMUS_MAXAA_STRING                       L"Maximum AA samples allowed for a given application"
#define PHYSXINDICATOR_STRING                      L"Display the PhysX indicator"
#define PREFERRED_PSTATE_STRING                    L"Power management mode"
#define PS_FRAMERATE_LIMITER_STRING                L"Frame Rate Limiter"
#define SHIM_IGPU_TRANSCODING_STRING               L"iGPU transcoding"
#define SHIM_MAXRES_STRING                         L"Maximum resolution allowed for a given application"
#define SHIM_MCCOMPAT_STRING                       L"Optimus flags for enabled applications"
#define SHIM_RENDERING_MODE_STRING                 L"Enable application for Optimus"
#define SHIM_RENDERING_OPTIONS_STRING              L"Shim Rendering Mode Options per application for Optimus"
#define SLI_GPU_COUNT_STRING                       L"Number of GPUs to use on SLI rendering mode"
#define SLI_PREDEFINED_GPU_COUNT_STRING            L"NVIDIA predefined number of GPUs to use on SLI rendering mode"
#define SLI_PREDEFINED_GPU_COUNT_DX10_STRING       L"NVIDIA predefined number of GPUs to use on SLI rendering mode on DirectX 10"
#define SLI_PREDEFINED_MODE_STRING                 L"NVIDIA predefined SLI mode"
#define SLI_PREDEFINED_MODE_DX10_STRING            L"NVIDIA predefined SLI mode on DirectX 10"
#define SLI_RENDERING_MODE_STRING                  L"SLI rendering mode"
#define VSYNCSMOOTHAFR_STRING                      L"Flag to control smooth AFR behavior"
#define VSYNC_BEHAVIOR_FLAGS_STRING                L"Vsync - Behavior Flags"
#define WKS_API_STEREO_EYES_EXCHANGE_STRING        L"Stereo - Swap eyes"
#define WKS_API_STEREO_MODE_STRING                 L"Stereo - Display mode"
#define WKS_FEATURE_SUPPORT_CONTROL_STRING         L""
#define WKS_STEREO_DONGLE_SUPPORT_STRING           L"Stereo - Dongle Support"
#define WKS_STEREO_SUPPORT_STRING                  L"Stereo - Enable"
#define AO_MODE_STRING                             L"Ambient Occlusion"
#define AO_MODE_ACTIVE_STRING                      L"NVIDIA Predefined Ambient Occlusion Usage"
#define AUTO_LODBIASADJUST_STRING                  L"Texture filtering - Driver Controlled LOD Bias"
#define ICAFE_LOGO_CONFIG_STRING                   L"ICafe Settings"
#define LODBIASADJUST_STRING                       L"Texture filtering - LOD Bias"
#define PRERENDERLIMIT_STRING                      L"Maximum pre-rendered frames"
#define PS_DYNAMIC_TILING_STRING                   L"Dynamic tiling"
#define PS_TEXFILTER_ANISO_OPTS2_STRING            L"Texture filtering - Anisotropic sample optimization"
#define PS_TEXFILTER_BILINEAR_IN_ANISO_STRING      L"Texture filtering - Anisotropic filter optimization"
#define PS_TEXFILTER_DISABLE_TRILIN_SLOPE_STRING   L"Texture filtering - Trilinear optimization"
#define PS_TEXFILTER_NO_NEG_LODBIAS_STRING         L"Texture filtering - Negative LOD bias"
#define QUALITY_ENHANCEMENTS_STRING                L"Texture filtering - Quality"
#define REFRESH_RATE_OVERRIDE_STRING               L"Preferred refresh rate"
#define SET_POWER_THROTTLE_FOR_PCIe_COMPLIANCE_STRING L"PowerThrottle"
#define SET_VAB_DATA_STRING                        L"VAB Default Data"
#define VSYNCMODE_STRING                           L"Vertical Sync"
#define VSYNCTEARCONTROL_STRING                    L"Vertical Sync Tear Control"

enum ESetting {
    OGL_AA_LINE_GAMMA_ID                          = 0x2089BF6C,
    OGL_DEEP_COLOR_SCANOUT_ID                     = 0x2097C2F6,
    OGL_DEFAULT_SWAP_INTERVAL_ID                  = 0x206A6582,
    OGL_DEFAULT_SWAP_INTERVAL_FRACTIONAL_ID       = 0x206C4581,
    OGL_DEFAULT_SWAP_INTERVAL_SIGN_ID             = 0x20655CFA,
    OGL_EVENT_LOG_SEVERITY_THRESHOLD_ID           = 0x209DF23E,
    OGL_EXTENSION_STRING_VERSION_ID               = 0x20FF7493,
    OGL_FORCE_BLIT_ID                             = 0x201F619F,
    OGL_FORCE_STEREO_ID                           = 0x204D9A0C,
    OGL_IMPLICIT_GPU_AFFINITY_ID                  = 0x20D0F3E6,
    OGL_MAX_FRAMES_ALLOWED_ID                     = 0x208E55E3,
    OGL_MULTIMON_ID                               = 0x200AEBFC,
    OGL_OVERLAY_PIXEL_TYPE_ID                     = 0x209AE66F,
    OGL_OVERLAY_SUPPORT_ID                        = 0x206C28C4,
    OGL_QUALITY_ENHANCEMENTS_ID                   = 0x20797D6C,
    OGL_SINGLE_BACKDEPTH_BUFFER_ID                = 0x20A29055,
    OGL_THREAD_CONTROL_ID                         = 0x20C1221E,
    OGL_TRIPLE_BUFFER_ID                          = 0x20FDD1F9,
    OGL_VIDEO_EDITING_MODE_ID                     = 0x20EE02B4,
    AA_BEHAVIOR_FLAGS_ID                          = 0x10ECDB82,
    AA_MODE_ALPHATOCOVERAGE_ID                    = 0x10FC2D9C,
    AA_MODE_GAMMACORRECTION_ID                    = 0x107D639D,
    AA_MODE_METHOD_ID                             = 0x10D773D2,
    AA_MODE_REPLAY_ID                             = 0x10D48A85,
    AA_MODE_SELECTOR_ID                           = 0x107EFC5B,
    AA_MODE_SELECTOR_SLIAA_ID                     = 0x107AFC5B,
    ANISO_MODE_LEVEL_ID                           = 0x101E61A9,
    ANISO_MODE_SELECTOR_ID                        = 0x10D2BB16,
    APPLICATION_PROFILE_NOTIFICATION_TIMEOUT_ID   = 0x104554B6,
    APPLICATION_STEAM_ID_ID                       = 0x107CDDBC,
    CPL_HIDDEN_PROFILE_ID                         = 0x106D5CFF,
    CUDA_EXCLUDED_GPUS_ID                         = 0x10354FF8,
    D3DOGL_GPU_MAX_POWER_ID                       = 0x10D1EF29,
    EXPORT_PERF_COUNTERS_ID                       = 0x108F0841,
    FXAA_ALLOW_ID                                 = 0x1034CB89,
    FXAA_ENABLE_ID                                = 0x1074C972,
    FXAA_INDICATOR_ENABLE_ID                      = 0x1068FB9C,
    MCSFRSHOWSPLIT_ID                             = 0x10287051,
    OPTIMUS_DEBUG_ID                              = 0x10F9DC03,
    OPTIMUS_MAXAA_ID                              = 0x10F9DC83,
    PHYSXINDICATOR_ID                             = 0x1094F16F,
    PREFERRED_PSTATE_ID                           = 0x1057EB71,
    PS_FRAMERATE_LIMITER_ID                       = 0x10834FEE,
    SHIM_IGPU_TRANSCODING_ID                      = 0x10F9DC85,
    SHIM_MAXRES_ID                                = 0x10F9DC82,
    SHIM_MCCOMPAT_ID                              = 0x10F9DC80,
    SHIM_RENDERING_MODE_ID                        = 0x10F9DC81,
    SHIM_RENDERING_OPTIONS_ID                     = 0x10F9DC84,
    SLI_GPU_COUNT_ID                              = 0x1033DCD1,
    SLI_PREDEFINED_GPU_COUNT_ID                   = 0x1033DCD2,
    SLI_PREDEFINED_GPU_COUNT_DX10_ID              = 0x1033DCD3,
    SLI_PREDEFINED_MODE_ID                        = 0x1033CEC1,
    SLI_PREDEFINED_MODE_DX10_ID                   = 0x1033CEC2,
    SLI_RENDERING_MODE_ID                         = 0x1033CED1,
    VSYNCSMOOTHAFR_ID                             = 0x101AE763,
    VSYNC_BEHAVIOR_FLAGS_ID                       = 0x10FDEC23,
    WKS_API_STEREO_EYES_EXCHANGE_ID               = 0x11AE435C,
    WKS_API_STEREO_MODE_ID                        = 0x11E91A61,
    WKS_FEATURE_SUPPORT_CONTROL_ID                = 0x11D9DC84,
    WKS_STEREO_DONGLE_SUPPORT_ID                  = 0x112493BD,
    WKS_STEREO_SUPPORT_ID                         = 0x11AA9E99,
    AO_MODE_ID                                    = 0x00667329,
    AO_MODE_ACTIVE_ID                             = 0x00664339,
    AUTO_LODBIASADJUST_ID                         = 0x00638E8F,
    ICAFE_LOGO_CONFIG_ID                          = 0x00DB1337,
    LODBIASADJUST_ID                              = 0x00738E8F,
    PRERENDERLIMIT_ID                             = 0x007BA09E,
    PS_DYNAMIC_TILING_ID                          = 0x00E5C6C0,
    PS_TEXFILTER_ANISO_OPTS2_ID                   = 0x00E73211,
    PS_TEXFILTER_BILINEAR_IN_ANISO_ID             = 0x0084CD70,
    PS_TEXFILTER_DISABLE_TRILIN_SLOPE_ID          = 0x002ECAF2,
    PS_TEXFILTER_NO_NEG_LODBIAS_ID                = 0x0019BB68,
    QUALITY_ENHANCEMENTS_ID                       = 0x00CE2691,
    REFRESH_RATE_OVERRIDE_ID                      = 0x0064B541,
    SET_POWER_THROTTLE_FOR_PCIe_COMPLIANCE_ID     = 0x00AE785C,
    SET_VAB_DATA_ID                               = 0x00AB8687,
    VSYNCMODE_ID                                  = 0x00A879CF,
    VSYNCTEARCONTROL_ID                           = 0x005A375C,
    TOTAL_DWORD_SETTING_NUM = 74,
    TOTAL_WSTRING_SETTING_NUM = 4,
    TOTAL_SETTING_NUM = 78,
    INVALID_SETTING_ID = 0xFFFFFFFF
};

enum EValues_OGL_AA_LINE_GAMMA {
    OGL_AA_LINE_GAMMA_DISABLED                           = 0x10,
    OGL_AA_LINE_GAMMA_ENABLED                            = 0x23,
    OGL_AA_LINE_GAMMA_MIN                                = 1,
    OGL_AA_LINE_GAMMA_MAX                                = 100,
    OGL_AA_LINE_GAMMA_NUM_VALUES = 4,
    OGL_AA_LINE_GAMMA_DEFAULT = OGL_AA_LINE_GAMMA_DISABLED
};

enum EValues_OGL_DEEP_COLOR_SCANOUT {
    OGL_DEEP_COLOR_SCANOUT_DISABLE                       = 0,
    OGL_DEEP_COLOR_SCANOUT_ENABLE                        = 1,
    OGL_DEEP_COLOR_SCANOUT_NUM_VALUES = 2,
    OGL_DEEP_COLOR_SCANOUT_DEFAULT = OGL_DEEP_COLOR_SCANOUT_ENABLE
};

enum EValues_OGL_DEFAULT_SWAP_INTERVAL {
    OGL_DEFAULT_SWAP_INTERVAL_TEAR                       = 0,
    OGL_DEFAULT_SWAP_INTERVAL_VSYNC_ONE                  = 1,
    OGL_DEFAULT_SWAP_INTERVAL_VSYNC                      = 1,
    OGL_DEFAULT_SWAP_INTERVAL_VALUE_MASK                 = 0x0000FFFF,
    OGL_DEFAULT_SWAP_INTERVAL_FORCE_MASK                 = 0xF0000000,
    OGL_DEFAULT_SWAP_INTERVAL_FORCE_OFF                  = 0xF0000000,
    OGL_DEFAULT_SWAP_INTERVAL_FORCE_ON                   = 0x10000000,
    OGL_DEFAULT_SWAP_INTERVAL_APP_CONTROLLED             = 0x00000000,
    OGL_DEFAULT_SWAP_INTERVAL_DISABLE                    = 0xffffffff,
    OGL_DEFAULT_SWAP_INTERVAL_NUM_VALUES = 9,
    OGL_DEFAULT_SWAP_INTERVAL_DEFAULT = OGL_DEFAULT_SWAP_INTERVAL_VSYNC_ONE
};

enum EValues_OGL_DEFAULT_SWAP_INTERVAL_FRACTIONAL {
    OGL_DEFAULT_SWAP_INTERVAL_FRACTIONAL_ZERO_SCANLINES  = 0,
    OGL_DEFAULT_SWAP_INTERVAL_FRACTIONAL_ONE_FULL_FRAME_OF_SCANLINES = 100,
    OGL_DEFAULT_SWAP_INTERVAL_FRACTIONAL_NUM_VALUES = 2,
    OGL_DEFAULT_SWAP_INTERVAL_FRACTIONAL_DEFAULT = 0
};

enum EValues_OGL_DEFAULT_SWAP_INTERVAL_SIGN {
    OGL_DEFAULT_SWAP_INTERVAL_SIGN_POSITIVE              = 0,
    OGL_DEFAULT_SWAP_INTERVAL_SIGN_NEGATIVE              = 1,
    OGL_DEFAULT_SWAP_INTERVAL_SIGN_NUM_VALUES = 2,
    OGL_DEFAULT_SWAP_INTERVAL_SIGN_DEFAULT = OGL_DEFAULT_SWAP_INTERVAL_SIGN_POSITIVE
};

enum EValues_OGL_EVENT_LOG_SEVERITY_THRESHOLD {
    OGL_EVENT_LOG_SEVERITY_THRESHOLD_DISABLE             = 0,
    OGL_EVENT_LOG_SEVERITY_THRESHOLD_CRITICAL            = 1,
    OGL_EVENT_LOG_SEVERITY_THRESHOLD_WARNING             = 2,
    OGL_EVENT_LOG_SEVERITY_THRESHOLD_INFORMATION         = 3,
    OGL_EVENT_LOG_SEVERITY_THRESHOLD_ALL                 = 4,
    OGL_EVENT_LOG_SEVERITY_THRESHOLD_NUM_VALUES = 5,
    OGL_EVENT_LOG_SEVERITY_THRESHOLD_DEFAULT = OGL_EVENT_LOG_SEVERITY_THRESHOLD_CRITICAL
};

enum EValues_OGL_FORCE_BLIT {
    OGL_FORCE_BLIT_ON                                    = 1,
    OGL_FORCE_BLIT_OFF                                   = 0,
    OGL_FORCE_BLIT_NUM_VALUES = 2,
    OGL_FORCE_BLIT_DEFAULT = OGL_FORCE_BLIT_OFF
};

enum EValues_OGL_FORCE_STEREO {
    OGL_FORCE_STEREO_OFF                                 = 0,
    OGL_FORCE_STEREO_ON                                  = 1,
    OGL_FORCE_STEREO_NUM_VALUES = 2,
    OGL_FORCE_STEREO_DEFAULT = OGL_FORCE_STEREO_OFF
};

#define    OGL_IMPLICIT_GPU_AFFINITY_ENV_VAR                    L"OGL_DEFAULT_RENDERING_GPU"
#define    OGL_IMPLICIT_GPU_AFFINITY_AUTOSELECT                 L"autoselect"
#define    OGL_IMPLICIT_GPU_AFFINITY_NUM_VALUES 1
#define    OGL_IMPLICIT_GPU_AFFINITY_DEFAULT OGL_IMPLICIT_GPU_AFFINITY_AUTOSELECT

enum EValues_OGL_MULTIMON {
    OGL_MULTIMON_SINGLE_MONITOR                          = 0,
    OGL_MULTIMON_COMPATIBILITY_LCD                       = 1,
    OGL_MULTIMON_COMPATIBILITY_GCD                       = 2,
    OGL_MULTIMON_PERFORMANCE_LCD                         = 3,
    OGL_MULTIMON_PERFORMANCE_GCD                         = 4,
    OGL_MULTIMON_EXTENDED_SINGLE_MONITOR                 = 5,
    OGL_MULTIMON_PERFORMANCE_QUADRO                      = 6,
    OGL_MULTIMON_MULTIMON_BUFFER                         = 7,
    OGL_MULTIMON_NUM_VALUES = 8,
    OGL_MULTIMON_DEFAULT = OGL_MULTIMON_PERFORMANCE_LCD
};

enum EValues_OGL_OVERLAY_PIXEL_TYPE {
    OGL_OVERLAY_PIXEL_TYPE_NONE                          = 0x0,
    OGL_OVERLAY_PIXEL_TYPE_CI                            = 0x1,
    OGL_OVERLAY_PIXEL_TYPE_RGBA                          = 0x2,
    OGL_OVERLAY_PIXEL_TYPE_CI_AND_RGBA                   = 0x3,
    OGL_OVERLAY_PIXEL_TYPE_NUM_VALUES = 4,
    OGL_OVERLAY_PIXEL_TYPE_DEFAULT = OGL_OVERLAY_PIXEL_TYPE_CI
};

enum EValues_OGL_OVERLAY_SUPPORT {
    OGL_OVERLAY_SUPPORT_OFF                              = 0,
    OGL_OVERLAY_SUPPORT_ON                               = 1,
    OGL_OVERLAY_SUPPORT_FORCE_SW                         = 2,
    OGL_OVERLAY_SUPPORT_NUM_VALUES = 3,
    OGL_OVERLAY_SUPPORT_DEFAULT = OGL_OVERLAY_SUPPORT_OFF
};

enum EValues_OGL_QUALITY_ENHANCEMENTS {
    OGL_QUALITY_ENHANCEMENTS_HQUAL                       = -10,
    OGL_QUALITY_ENHANCEMENTS_QUAL                        = 0,
    OGL_QUALITY_ENHANCEMENTS_PERF                        = 10,
    OGL_QUALITY_ENHANCEMENTS_HPERF                       = 20,
    OGL_QUALITY_ENHANCEMENTS_NUM_VALUES = 4,
    OGL_QUALITY_ENHANCEMENTS_DEFAULT = OGL_QUALITY_ENHANCEMENTS_QUAL
};

enum EValues_OGL_SINGLE_BACKDEPTH_BUFFER {
    OGL_SINGLE_BACKDEPTH_BUFFER_DISABLE                  = 0x0,
    OGL_SINGLE_BACKDEPTH_BUFFER_ENABLE                   = 0x1,
    OGL_SINGLE_BACKDEPTH_BUFFER_USE_HW_DEFAULT           = 0xffffffff,
    OGL_SINGLE_BACKDEPTH_BUFFER_NUM_VALUES = 3,
    OGL_SINGLE_BACKDEPTH_BUFFER_DEFAULT = OGL_SINGLE_BACKDEPTH_BUFFER_DISABLE
};

enum EValues_OGL_THREAD_CONTROL {
    OGL_THREAD_CONTROL_ENABLE                            = 0x00000001,
    OGL_THREAD_CONTROL_DISABLE                           = 0x00000002,
    OGL_THREAD_CONTROL_DUMP_STATS                        = 0x00000004,
    OGL_THREAD_CONTROL_IGNORE_GET_ERROR                  = 0x00000008,
    OGL_THREAD_CONTROL_NUM_VALUES = 4,
    OGL_THREAD_CONTROL_DEFAULT = 0
};

enum EValues_OGL_TRIPLE_BUFFER {
    OGL_TRIPLE_BUFFER_DISABLED                           = 0x00000000,
    OGL_TRIPLE_BUFFER_ENABLED                            = 0x00000001,
    OGL_TRIPLE_BUFFER_NUM_VALUES = 2,
    OGL_TRIPLE_BUFFER_DEFAULT = OGL_TRIPLE_BUFFER_DISABLED
};

enum EValues_OGL_VIDEO_EDITING_MODE {
    OGL_VIDEO_EDITING_MODE_DISABLE                       = 0x00000000,
    OGL_VIDEO_EDITING_MODE_ENABLE                        = 0x00000001,
    OGL_VIDEO_EDITING_MODE_NUM_VALUES = 2,
    OGL_VIDEO_EDITING_MODE_DEFAULT = OGL_VIDEO_EDITING_MODE_DISABLE
};

enum EValues_AA_BEHAVIOR_FLAGS {
    AA_BEHAVIOR_FLAGS_NONE                               = 0x00000000,
    AA_BEHAVIOR_FLAGS_TREAT_OVERRIDE_AS_APP_CONTROLLED   = 0x00000001,
    AA_BEHAVIOR_FLAGS_TREAT_OVERRIDE_AS_ENHANCE          = 0x00000002,
    AA_BEHAVIOR_FLAGS_DISABLE_OVERRIDE                   = 0x00000003,
    AA_BEHAVIOR_FLAGS_TREAT_ENHANCE_AS_APP_CONTROLLED    = 0x00000004,
    AA_BEHAVIOR_FLAGS_TREAT_ENHANCE_AS_OVERRIDE          = 0x00000008,
    AA_BEHAVIOR_FLAGS_DISABLE_ENHANCE                    = 0x0000000c,
    AA_BEHAVIOR_FLAGS_MAP_VCAA_TO_MULTISAMPLING          = 0x00010000,
    AA_BEHAVIOR_FLAGS_SLI_DISABLE_TRANSPARENCY_SUPERSAMPLING = 0x00020000,
    AA_BEHAVIOR_FLAGS_DISABLE_CPLAA                      = 0x00040000,
    AA_BEHAVIOR_FLAGS_SKIP_RT_DIM_CHECK_FOR_ENHANCE      = 0x00080000,
    AA_BEHAVIOR_FLAGS_DISABLE_SLIAA                      = 0x00100000,
    AA_BEHAVIOR_FLAGS_DEFAULT                            = 0x00000000,
    AA_BEHAVIOR_FLAGS_AA_RT_BPP_DIV_4                    = 0xf0000000,
    AA_BEHAVIOR_FLAGS_AA_RT_BPP_DIV_4_SHIFT              = 28,
    AA_BEHAVIOR_FLAGS_NON_AA_RT_BPP_DIV_4                = 0x0f000000,
    AA_BEHAVIOR_FLAGS_NON_AA_RT_BPP_DIV_4_SHIFT          = 24,
    AA_BEHAVIOR_FLAGS_MASK                               = 0xff1f000f,
    AA_BEHAVIOR_FLAGS_NUM_VALUES = 18,
};

enum EValues_AA_MODE_ALPHATOCOVERAGE {
    AA_MODE_ALPHATOCOVERAGE_MODE_MASK                    = 0x00000004,
    AA_MODE_ALPHATOCOVERAGE_MODE_OFF                     = 0x00000000,
    AA_MODE_ALPHATOCOVERAGE_MODE_ON                      = 0x00000004,
    AA_MODE_ALPHATOCOVERAGE_MODE_MAX                     = 0x00000004,
    AA_MODE_ALPHATOCOVERAGE_NUM_VALUES = 4,
    AA_MODE_ALPHATOCOVERAGE_DEFAULT = 0x00000000
};

enum EValues_AA_MODE_GAMMACORRECTION {
    AA_MODE_GAMMACORRECTION_MASK                         = 0x00000003,
    AA_MODE_GAMMACORRECTION_OFF                          = 0x00000000,
    AA_MODE_GAMMACORRECTION_ON_IF_FOS                    = 0x00000001,
    AA_MODE_GAMMACORRECTION_ON_ALWAYS                    = 0x00000002,
    AA_MODE_GAMMACORRECTION_MAX                          = 0x00000002,
    AA_MODE_GAMMACORRECTION_DEFAULT                      = 0x00000000,
    AA_MODE_GAMMACORRECTION_DEFAULT_TESLA                = 0x00000002,
    AA_MODE_GAMMACORRECTION_DEFAULT_FERMI                = 0x00000002,
    AA_MODE_GAMMACORRECTION_NUM_VALUES = 8,
};

enum EValues_AA_MODE_METHOD {
    AA_MODE_METHOD_NONE                                  = 0x0,
    AA_MODE_METHOD_SUPERSAMPLE_2X_H                      = 0x1,
    AA_MODE_METHOD_SUPERSAMPLE_2X_V                      = 0x2,
    AA_MODE_METHOD_SUPERSAMPLE_1_5X1_5                   = 0x2,
    AA_MODE_METHOD_FREE_0x03                             = 0x3,
    AA_MODE_METHOD_FREE_0x04                             = 0x4,
    AA_MODE_METHOD_SUPERSAMPLE_4X                        = 0x5,
    AA_MODE_METHOD_SUPERSAMPLE_4X_BIAS                   = 0x6,
    AA_MODE_METHOD_SUPERSAMPLE_4X_GAUSSIAN               = 0x7,
    AA_MODE_METHOD_FREE_0x08                             = 0x8,
    AA_MODE_METHOD_FREE_0x09                             = 0x9,
    AA_MODE_METHOD_SUPERSAMPLE_9X                        = 0xA,
    AA_MODE_METHOD_SUPERSAMPLE_9X_BIAS                   = 0xB,
    AA_MODE_METHOD_SUPERSAMPLE_16X                       = 0xC,
    AA_MODE_METHOD_SUPERSAMPLE_16X_BIAS                  = 0xD,
    AA_MODE_METHOD_MULTISAMPLE_2X_DIAGONAL               = 0xE,
    AA_MODE_METHOD_MULTISAMPLE_2X_QUINCUNX               = 0xF,
    AA_MODE_METHOD_MULTISAMPLE_4X                        = 0x10,
    AA_MODE_METHOD_FREE_0x11                             = 0x11,
    AA_MODE_METHOD_MULTISAMPLE_4X_GAUSSIAN               = 0x12,
    AA_MODE_METHOD_MIXEDSAMPLE_4X_SKEWED_4TAP            = 0x13,
    AA_MODE_METHOD_FREE_0x14                             = 0x14,
    AA_MODE_METHOD_FREE_0x15                             = 0x15,
    AA_MODE_METHOD_MIXEDSAMPLE_6X                        = 0x16,
    AA_MODE_METHOD_MIXEDSAMPLE_6X_SKEWED_6TAP            = 0x17,
    AA_MODE_METHOD_MIXEDSAMPLE_8X                        = 0x18,
    AA_MODE_METHOD_MIXEDSAMPLE_8X_SKEWED_8TAP            = 0x19,
    AA_MODE_METHOD_MIXEDSAMPLE_16X                       = 0x1a,
    AA_MODE_METHOD_MULTISAMPLE_4X_GAMMA                  = 0x1b,
    AA_MODE_METHOD_MULTISAMPLE_16X                       = 0x1c,
    AA_MODE_METHOD_VCAA_32X_8v24                         = 0x1d,
    AA_MODE_METHOD_CORRUPTION_CHECK                      = 0x1e,
    AA_MODE_METHOD_6X_CT                                 = 0x1f,
    AA_MODE_METHOD_MULTISAMPLE_2X_DIAGONAL_GAMMA         = 0x20,
    AA_MODE_METHOD_SUPERSAMPLE_4X_GAMMA                  = 0x21,
    AA_MODE_METHOD_MULTISAMPLE_4X_FOSGAMMA               = 0x22,
    AA_MODE_METHOD_MULTISAMPLE_2X_DIAGONAL_FOSGAMMA      = 0x23,
    AA_MODE_METHOD_SUPERSAMPLE_4X_FOSGAMMA               = 0x24,
    AA_MODE_METHOD_MULTISAMPLE_8X                        = 0x25,
    AA_MODE_METHOD_VCAA_8X_4v4                           = 0x26,
    AA_MODE_METHOD_VCAA_16X_4v12                         = 0x27,
    AA_MODE_METHOD_VCAA_16X_8v8                          = 0x28,
    AA_MODE_METHOD_MIXEDSAMPLE_32X                       = 0x29,
    AA_MODE_METHOD_SUPERVCAA_64X_4v12                    = 0x2a,
    AA_MODE_METHOD_SUPERVCAA_64X_8v8                     = 0x2b,
    AA_MODE_METHOD_MIXEDSAMPLE_64X                       = 0x2c,
    AA_MODE_METHOD_MIXEDSAMPLE_128X                      = 0x2d,
    AA_MODE_METHOD_COUNT                                 = 0x2e,
    AA_MODE_METHOD_METHOD_MASK                           = 0x0000ffff,
    AA_MODE_METHOD_METHOD_MAX                            = 0xf1c57815,
    AA_MODE_METHOD_NUM_VALUES = 50,
    AA_MODE_METHOD_DEFAULT = AA_MODE_METHOD_NONE
};

enum EValues_AA_MODE_REPLAY {
    AA_MODE_REPLAY_SAMPLES_MASK                          = 0x00000070,
    AA_MODE_REPLAY_SAMPLES_ONE                           = 0x00000000,
    AA_MODE_REPLAY_SAMPLES_TWO                           = 0x00000010,
    AA_MODE_REPLAY_SAMPLES_FOUR                          = 0x00000020,
    AA_MODE_REPLAY_SAMPLES_EIGHT                         = 0x00000030,
    AA_MODE_REPLAY_SAMPLES_MAX                           = 0x00000030,
    AA_MODE_REPLAY_MODE_MASK                             = 0x0000000f,
    AA_MODE_REPLAY_MODE_OFF                              = 0x00000000,
    AA_MODE_REPLAY_MODE_ALPHA_TEST                       = 0x00000001,
    AA_MODE_REPLAY_MODE_PIXEL_KILL                       = 0x00000002,
    AA_MODE_REPLAY_MODE_DYN_BRANCH                       = 0x00000004,
    AA_MODE_REPLAY_MODE_OPTIMAL                          = 0x00000004,
    AA_MODE_REPLAY_MODE_ALL                              = 0x00000008,
    AA_MODE_REPLAY_MODE_MAX                              = 0x0000000f,
    AA_MODE_REPLAY_TRANSPARENCY                          = 0x00000023,
    AA_MODE_REPLAY_DISALLOW_TRAA                         = 0x00000100,
    AA_MODE_REPLAY_TRANSPARENCY_DEFAULT                  = 0x00000000,
    AA_MODE_REPLAY_TRANSPARENCY_DEFAULT_TESLA            = 0x00000000,
    AA_MODE_REPLAY_TRANSPARENCY_DEFAULT_FERMI            = 0x00000000,
    AA_MODE_REPLAY_NUM_VALUES = 19,
    AA_MODE_REPLAY_DEFAULT = 0x00000000
};

enum EValues_AA_MODE_SELECTOR {
    AA_MODE_SELECTOR_MASK                                = 0x00000003,
    AA_MODE_SELECTOR_APP_CONTROL                         = 0x00000000,
    AA_MODE_SELECTOR_OVERRIDE                            = 0x00000001,
    AA_MODE_SELECTOR_ENHANCE                             = 0x00000002,
    AA_MODE_SELECTOR_MAX                                 = 0x00000002,
    AA_MODE_SELECTOR_NUM_VALUES = 5,
    AA_MODE_SELECTOR_DEFAULT = AA_MODE_SELECTOR_APP_CONTROL
};

enum EValues_AA_MODE_SELECTOR_SLIAA {
    AA_MODE_SELECTOR_SLIAA_DISABLED                      = 0,
    AA_MODE_SELECTOR_SLIAA_ENABLED                       = 1,
    AA_MODE_SELECTOR_SLIAA_NUM_VALUES = 2,
    AA_MODE_SELECTOR_SLIAA_DEFAULT = AA_MODE_SELECTOR_SLIAA_DISABLED
};

enum EValues_ANISO_MODE_LEVEL {
    ANISO_MODE_LEVEL_MASK                                = 0x0000ffff,
    ANISO_MODE_LEVEL_NONE_POINT                          = 0x00000000,
    ANISO_MODE_LEVEL_NONE_LINEAR                         = 0x00000001,
    ANISO_MODE_LEVEL_MAX                                 = 0x00000010,
    ANISO_MODE_LEVEL_DEFAULT                             = 0x00000001,
    ANISO_MODE_LEVEL_NUM_VALUES = 5,
};

enum EValues_ANISO_MODE_SELECTOR {
    ANISO_MODE_SELECTOR_MASK                             = 0x0000000f,
    ANISO_MODE_SELECTOR_APP                              = 0x00000000,
    ANISO_MODE_SELECTOR_USER                             = 0x00000001,
    ANISO_MODE_SELECTOR_COND                             = 0x00000002,
    ANISO_MODE_SELECTOR_MAX                              = 0x00000002,
    ANISO_MODE_SELECTOR_DEFAULT                          = 0x00000000,
    ANISO_MODE_SELECTOR_NUM_VALUES = 6,
};

enum EValues_APPLICATION_PROFILE_NOTIFICATION_TIMEOUT {
    APPLICATION_PROFILE_NOTIFICATION_TIMEOUT_DISABLED    = 0,
    APPLICATION_PROFILE_NOTIFICATION_TIMEOUT_NINE_SECONDS = 9,
    APPLICATION_PROFILE_NOTIFICATION_TIMEOUT_FIFTEEN_SECONDS = 15,
    APPLICATION_PROFILE_NOTIFICATION_TIMEOUT_THIRTY_SECONDS = 30,
    APPLICATION_PROFILE_NOTIFICATION_TIMEOUT_ONE_MINUTE  = 60,
    APPLICATION_PROFILE_NOTIFICATION_TIMEOUT_TWO_MINUTES = 120,
    APPLICATION_PROFILE_NOTIFICATION_TIMEOUT_NUM_VALUES = 6,
    APPLICATION_PROFILE_NOTIFICATION_TIMEOUT_DEFAULT = APPLICATION_PROFILE_NOTIFICATION_TIMEOUT_DISABLED
};

enum EValues_CPL_HIDDEN_PROFILE {
    CPL_HIDDEN_PROFILE_DISABLED                          = 0,
    CPL_HIDDEN_PROFILE_ENABLED                           = 1,
    CPL_HIDDEN_PROFILE_NUM_VALUES = 2,
    CPL_HIDDEN_PROFILE_DEFAULT = CPL_HIDDEN_PROFILE_DISABLED
};

#define    CUDA_EXCLUDED_GPUS_NONE                              L"none"
#define    CUDA_EXCLUDED_GPUS_NUM_VALUES 1
#define    CUDA_EXCLUDED_GPUS_DEFAULT CUDA_EXCLUDED_GPUS_NONE

#define    D3DOGL_GPU_MAX_POWER_DEFAULTPOWER                    L"0"
#define    D3DOGL_GPU_MAX_POWER_NUM_VALUES 1
#define    D3DOGL_GPU_MAX_POWER_DEFAULT D3DOGL_GPU_MAX_POWER_DEFAULTPOWER

enum EValues_EXPORT_PERF_COUNTERS {
    EXPORT_PERF_COUNTERS_OFF                             = 0x00000000,
    EXPORT_PERF_COUNTERS_ON                              = 0x00000001,
    EXPORT_PERF_COUNTERS_NUM_VALUES = 2,
    EXPORT_PERF_COUNTERS_DEFAULT = EXPORT_PERF_COUNTERS_OFF
};

enum EValues_FXAA_ALLOW {
    FXAA_ALLOW_DISALLOWED                                = 0,
    FXAA_ALLOW_ALLOWED                                   = 1,
    FXAA_ALLOW_NUM_VALUES = 2,
    FXAA_ALLOW_DEFAULT = FXAA_ALLOW_ALLOWED
};

enum EValues_FXAA_ENABLE {
    FXAA_ENABLE_OFF                                      = 0,
    FXAA_ENABLE_ON                                       = 1,
    FXAA_ENABLE_NUM_VALUES = 2,
    FXAA_ENABLE_DEFAULT = FXAA_ENABLE_OFF
};

enum EValues_FXAA_INDICATOR_ENABLE {
    FXAA_INDICATOR_ENABLE_OFF                            = 0,
    FXAA_INDICATOR_ENABLE_ON                             = 1,
    FXAA_INDICATOR_ENABLE_NUM_VALUES = 2,
    FXAA_INDICATOR_ENABLE_DEFAULT = FXAA_INDICATOR_ENABLE_OFF
};

enum EValues_MCSFRSHOWSPLIT {
    MCSFRSHOWSPLIT_DISABLED                              = 0x34534064,
    MCSFRSHOWSPLIT_ENABLED                               = 0x24545582,
    MCSFRSHOWSPLIT_NUM_VALUES = 2,
    MCSFRSHOWSPLIT_DEFAULT = MCSFRSHOWSPLIT_DISABLED
};

enum EValues_OPTIMUS_DEBUG {
    OPTIMUS_DEBUG_NULL_RENDER_TRANSPORT                  = 0x00000001,
    OPTIMUS_DEBUG_NULL_DISPLAY_TRANSPORT                 = 0x00000002,
    OPTIMUS_DEBUG_NUM_VALUES = 2,
    OPTIMUS_DEBUG_DEFAULT = 0
};

enum EValues_OPTIMUS_MAXAA {
    OPTIMUS_MAXAA_MIN                                    = 0,
    OPTIMUS_MAXAA_MAX                                    = 16,
    OPTIMUS_MAXAA_NUM_VALUES = 2,
    OPTIMUS_MAXAA_DEFAULT = 0
};

enum EValues_PHYSXINDICATOR {
    PHYSXINDICATOR_DISABLED                              = 0x34534064,
    PHYSXINDICATOR_ENABLED                               = 0x24545582,
    PHYSXINDICATOR_NUM_VALUES = 2,
    PHYSXINDICATOR_DEFAULT = PHYSXINDICATOR_DISABLED
};

enum EValues_PREFERRED_PSTATE {
    PREFERRED_PSTATE_ADAPTIVE                            = 0x00000000,
    PREFERRED_PSTATE_PREFER_MAX                          = 0x00000001,
    PREFERRED_PSTATE_DRIVER_CONTROLLED                   = 0x00000002,
    PREFERRED_PSTATE_PREFER_MIN                          = 0x00000003,
    PREFERRED_PSTATE_MIN                                 = 0x00000000,
    PREFERRED_PSTATE_MAX                                 = 0x00000003,
    PREFERRED_PSTATE_NUM_VALUES = 6,
    PREFERRED_PSTATE_DEFAULT = PREFERRED_PSTATE_ADAPTIVE
};

enum EValues_PS_FRAMERATE_LIMITER {
    PS_FRAMERATE_LIMITER_DISABLED                        = 0x00000000,
    PS_FRAMERATE_LIMITER_FPS_20                          = 0x00000014,
    PS_FRAMERATE_LIMITER_FPS_30                          = 0x0000001e,
    PS_FRAMERATE_LIMITER_FPS_40                          = 0x00000028,
    PS_FRAMERATE_LIMITER_FPSMASK                         = 0x000000ff,
    PS_FRAMERATE_LIMITER_GPS_WEB                         = 0x00080000,
    PS_FRAMERATE_LIMITER_FORCE_OPTIMUS_POLICY            = 0x00100000,
    PS_FRAMERATE_LIMITER_DISALLOWED                      = 0x00200000,
    PS_FRAMERATE_LIMITER_THRESHOLD                       = 0x00000000,
    PS_FRAMERATE_LIMITER_TEMPERATURE                     = 0x02000000,
    PS_FRAMERATE_LIMITER_POWER                           = 0x04000000,
    PS_FRAMERATE_LIMITER_MODEMASK                        = 0x0f000000,
    PS_FRAMERATE_LIMITER_ACCURATE                        = 0x10000000,
    PS_FRAMERATE_LIMITER_ALLOW_WINDOWED                  = 0x20000000,
    PS_FRAMERATE_LIMITER_FORCEON                         = 0x40000000,
    PS_FRAMERATE_LIMITER_ENABLED                         = 0x80000000,
    PS_FRAMERATE_LIMITER_MASK                            = 0xff3800ff,
    PS_FRAMERATE_LIMITER_NUM_VALUES = 17,
    PS_FRAMERATE_LIMITER_DEFAULT = PS_FRAMERATE_LIMITER_DISABLED
};

enum EValues_SHIM_IGPU_TRANSCODING {
    SHIM_IGPU_TRANSCODING_DISABLE                        = 0x00000000,
    SHIM_IGPU_TRANSCODING_ENABLE                         = 0x00000001,
    SHIM_IGPU_TRANSCODING_NUM_VALUES = 2,
    SHIM_IGPU_TRANSCODING_DEFAULT = SHIM_IGPU_TRANSCODING_DISABLE
};

enum EValues_SHIM_MCCOMPAT {
    SHIM_MCCOMPAT_INTEGRATED                             = 0x00000000,
    SHIM_MCCOMPAT_ENABLE                                 = 0x00000001,
    SHIM_MCCOMPAT_USER_EDITABLE                          = 0x00000002,
    SHIM_MCCOMPAT_MASK                                   = 0x00000003,
    SHIM_MCCOMPAT_VIDEO_MASK                             = 0x00000004,
    SHIM_MCCOMPAT_VARYING_BIT                            = 0x00000008,
    SHIM_MCCOMPAT_AUTO_SELECT                            = 0x00000010,
    SHIM_MCCOMPAT_OVERRIDE_BIT                           = 0x80000000,
    SHIM_MCCOMPAT_NUM_VALUES = 8,
    SHIM_MCCOMPAT_DEFAULT = SHIM_MCCOMPAT_AUTO_SELECT
};

enum EValues_SHIM_RENDERING_MODE {
    SHIM_RENDERING_MODE_INTEGRATED                       = 0x00000000,
    SHIM_RENDERING_MODE_ENABLE                           = 0x00000001,
    SHIM_RENDERING_MODE_USER_EDITABLE                    = 0x00000002,
    SHIM_RENDERING_MODE_MASK                             = 0x00000003,
    SHIM_RENDERING_MODE_VIDEO_MASK                       = 0x00000004,
    SHIM_RENDERING_MODE_VARYING_BIT                      = 0x00000008,
    SHIM_RENDERING_MODE_AUTO_SELECT                      = 0x00000010,
    SHIM_RENDERING_MODE_OVERRIDE_BIT                     = 0x80000000,
    SHIM_RENDERING_MODE_NUM_VALUES = 8,
    SHIM_RENDERING_MODE_DEFAULT = SHIM_RENDERING_MODE_AUTO_SELECT
};

enum EValues_SHIM_RENDERING_OPTIONS {
    SHIM_RENDERING_OPTIONS_DEFAULT_RENDERING_MODE        = 0x00000000,
    SHIM_RENDERING_OPTIONS_DISABLE_ASYNC_PRESENT         = 0x00000001,
    SHIM_RENDERING_OPTIONS_EHSHELL_DETECT                = 0x00000002,
    SHIM_RENDERING_OPTIONS_FLASHPLAYER_HOST_DETECT       = 0x00000004,
    SHIM_RENDERING_OPTIONS_VIDEO_DRM_APP_DETECT          = 0x00000008,
    SHIM_RENDERING_OPTIONS_IGNORE_OVERRIDES              = 0x00000010,
    SHIM_RENDERING_OPTIONS_CHILDPROCESS_DETECT           = 0x00000020,
    SHIM_RENDERING_OPTIONS_ENABLE_DWM_ASYNC_PRESENT      = 0x00000040,
    SHIM_RENDERING_OPTIONS_PARENTPROCESS_DETECT          = 0x00000080,
    SHIM_RENDERING_OPTIONS_ALLOW_INHERITANCE             = 0x00000100,
    SHIM_RENDERING_OPTIONS_DISABLE_WRAPPERS              = 0x00000200,
    SHIM_RENDERING_OPTIONS_DISABLE_DXGI_WRAPPERS         = 0x00000400,
    SHIM_RENDERING_OPTIONS_PRUNE_UNSUPPORTED_FORMATS     = 0x00000800,
    SHIM_RENDERING_OPTIONS_ENABLE_ALPHA_FORMAT           = 0x00001000,
    SHIM_RENDERING_OPTIONS_IGPU_TRANSCODING              = 0x00002000,
    SHIM_RENDERING_OPTIONS_DISABLE_CUDA                  = 0x00004000,
    SHIM_RENDERING_OPTIONS_ALLOW_CP_CAPS_FOR_VIDEO       = 0x00008000,
    SHIM_RENDERING_OPTIONS_ENABLE_NEW_HOOKING            = 0x00010000,
    SHIM_RENDERING_OPTIONS_DISABLE_DURING_SECURE_BOOT    = 0x00020000,
    SHIM_RENDERING_OPTIONS_NUM_VALUES = 19,
    SHIM_RENDERING_OPTIONS_DEFAULT = SHIM_RENDERING_OPTIONS_DEFAULT_RENDERING_MODE
};

enum EValues_SLI_GPU_COUNT {
    SLI_GPU_COUNT_AUTOSELECT                             = 0x00000000,
    SLI_GPU_COUNT_ONE                                    = 0x00000001,
    SLI_GPU_COUNT_TWO                                    = 0x00000002,
    SLI_GPU_COUNT_THREE                                  = 0x00000003,
    SLI_GPU_COUNT_FOUR                                   = 0x00000004,
    SLI_GPU_COUNT_NUM_VALUES = 5,
    SLI_GPU_COUNT_DEFAULT = SLI_GPU_COUNT_AUTOSELECT
};

enum EValues_SLI_PREDEFINED_GPU_COUNT {
    SLI_PREDEFINED_GPU_COUNT_AUTOSELECT                  = 0x00000000,
    SLI_PREDEFINED_GPU_COUNT_ONE                         = 0x00000001,
    SLI_PREDEFINED_GPU_COUNT_TWO                         = 0x00000002,
    SLI_PREDEFINED_GPU_COUNT_THREE                       = 0x00000003,
    SLI_PREDEFINED_GPU_COUNT_FOUR                        = 0x00000004,
    SLI_PREDEFINED_GPU_COUNT_NUM_VALUES = 5,
    SLI_PREDEFINED_GPU_COUNT_DEFAULT = SLI_PREDEFINED_GPU_COUNT_AUTOSELECT
};

enum EValues_SLI_PREDEFINED_GPU_COUNT_DX10 {
    SLI_PREDEFINED_GPU_COUNT_DX10_AUTOSELECT             = 0x00000000,
    SLI_PREDEFINED_GPU_COUNT_DX10_ONE                    = 0x00000001,
    SLI_PREDEFINED_GPU_COUNT_DX10_TWO                    = 0x00000002,
    SLI_PREDEFINED_GPU_COUNT_DX10_THREE                  = 0x00000003,
    SLI_PREDEFINED_GPU_COUNT_DX10_FOUR                   = 0x00000004,
    SLI_PREDEFINED_GPU_COUNT_DX10_NUM_VALUES = 5,
    SLI_PREDEFINED_GPU_COUNT_DX10_DEFAULT = SLI_PREDEFINED_GPU_COUNT_DX10_AUTOSELECT
};

enum EValues_SLI_PREDEFINED_MODE {
    SLI_PREDEFINED_MODE_AUTOSELECT                       = 0x00000000,
    SLI_PREDEFINED_MODE_FORCE_SINGLE                     = 0x00000001,
    SLI_PREDEFINED_MODE_FORCE_AFR                        = 0x00000002,
    SLI_PREDEFINED_MODE_FORCE_AFR2                       = 0x00000003,
    SLI_PREDEFINED_MODE_FORCE_SFR                        = 0x00000004,
    SLI_PREDEFINED_MODE_FORCE_AFR_OF_SFR__FALLBACK_3AFR  = 0x00000005,
    SLI_PREDEFINED_MODE_NUM_VALUES = 6,
    SLI_PREDEFINED_MODE_DEFAULT = SLI_PREDEFINED_MODE_AUTOSELECT
};

enum EValues_SLI_PREDEFINED_MODE_DX10 {
    SLI_PREDEFINED_MODE_DX10_AUTOSELECT                  = 0x00000000,
    SLI_PREDEFINED_MODE_DX10_FORCE_SINGLE                = 0x00000001,
    SLI_PREDEFINED_MODE_DX10_FORCE_AFR                   = 0x00000002,
    SLI_PREDEFINED_MODE_DX10_FORCE_AFR2                  = 0x00000003,
    SLI_PREDEFINED_MODE_DX10_FORCE_SFR                   = 0x00000004,
    SLI_PREDEFINED_MODE_DX10_FORCE_AFR_OF_SFR__FALLBACK_3AFR = 0x00000005,
    SLI_PREDEFINED_MODE_DX10_NUM_VALUES = 6,
    SLI_PREDEFINED_MODE_DX10_DEFAULT = SLI_PREDEFINED_MODE_DX10_AUTOSELECT
};

enum EValues_SLI_RENDERING_MODE {
    SLI_RENDERING_MODE_AUTOSELECT                        = 0x00000000,
    SLI_RENDERING_MODE_FORCE_SINGLE                      = 0x00000001,
    SLI_RENDERING_MODE_FORCE_AFR                         = 0x00000002,
    SLI_RENDERING_MODE_FORCE_AFR2                        = 0x00000003,
    SLI_RENDERING_MODE_FORCE_SFR                         = 0x00000004,
    SLI_RENDERING_MODE_FORCE_AFR_OF_SFR__FALLBACK_3AFR   = 0x00000005,
    SLI_RENDERING_MODE_NUM_VALUES = 6,
    SLI_RENDERING_MODE_DEFAULT = SLI_RENDERING_MODE_AUTOSELECT
};

enum EValues_VSYNCSMOOTHAFR {
    VSYNCSMOOTHAFR_OFF                                   = 0x00000000,
    VSYNCSMOOTHAFR_ON                                    = 0x00000001,
    VSYNCSMOOTHAFR_NUM_VALUES = 2,
    VSYNCSMOOTHAFR_DEFAULT = VSYNCSMOOTHAFR_OFF
};

enum EValues_VSYNC_BEHAVIOR_FLAGS {
    VSYNC_BEHAVIOR_FLAGS_NONE                            = 0x00000000,
    VSYNC_BEHAVIOR_FLAGS_DEFAULT                         = 0x00000000,
    VSYNC_BEHAVIOR_FLAGS_IGNORE_FLIPINTERVAL_MULTIPLE    = 0x00000001,
    VSYNC_BEHAVIOR_FLAGS_NUM_VALUES = 3,
};

enum EValues_WKS_API_STEREO_EYES_EXCHANGE {
    WKS_API_STEREO_EYES_EXCHANGE_OFF                     = 0,
    WKS_API_STEREO_EYES_EXCHANGE_ON                      = 1,
    WKS_API_STEREO_EYES_EXCHANGE_NUM_VALUES = 2,
    WKS_API_STEREO_EYES_EXCHANGE_DEFAULT = WKS_API_STEREO_EYES_EXCHANGE_OFF
};

enum EValues_WKS_API_STEREO_MODE {
    WKS_API_STEREO_MODE_SHUTTER_GLASSES                  = 0,
    WKS_API_STEREO_MODE_VERTICAL_INTERLACED              = 1,
    WKS_API_STEREO_MODE_TWINVIEW                         = 2,
    WKS_API_STEREO_MODE_NV17_SHUTTER_GLASSES_AUTO        = 3,
    WKS_API_STEREO_MODE_NV17_SHUTTER_GLASSES_DAC0        = 4,
    WKS_API_STEREO_MODE_NV17_SHUTTER_GLASSES_DAC1        = 5,
    WKS_API_STEREO_MODE_COLOR_LINE                       = 6,
    WKS_API_STEREO_MODE_COLOR_INTERLEAVED                = 7,
    WKS_API_STEREO_MODE_ANAGLYPH                         = 8,
    WKS_API_STEREO_MODE_HORIZONTAL_INTERLACED            = 9,
    WKS_API_STEREO_MODE_SIDE_FIELD                       = 10,
    WKS_API_STEREO_MODE_SUB_FIELD                        = 11,
    WKS_API_STEREO_MODE_CHECKERBOARD                     = 12,
    WKS_API_STEREO_MODE_INVERSE_CHECKERBOARD             = 13,
    WKS_API_STEREO_MODE_TRIDELITY_SL                     = 14,
    WKS_API_STEREO_MODE_TRIDELITY_MV                     = 15,
    WKS_API_STEREO_MODE_SEEFRONT                         = 16,
    WKS_API_STEREO_MODE_STEREO_MIRROR                    = 17,
    WKS_API_STEREO_MODE_FRAME_SEQUENTIAL                 = 18,
    WKS_API_STEREO_MODE_AUTODETECT_PASSIVE_MODE          = 19,
    WKS_API_STEREO_MODE_AEGIS_DT_FRAME_SEQUENTIAL        = 20,
    WKS_API_STEREO_MODE_OEM_EMITTER_FRAME_SEQUENTIAL     = 21,
    WKS_API_STEREO_MODE_USE_HW_DEFAULT                   = 0xffffffff,
    WKS_API_STEREO_MODE_DEFAULT_GL                       = 3,
    WKS_API_STEREO_MODE_NUM_VALUES = 24,
    WKS_API_STEREO_MODE_DEFAULT = WKS_API_STEREO_MODE_SHUTTER_GLASSES
};

enum EValues_WKS_FEATURE_SUPPORT_CONTROL {
    WKS_FEATURE_SUPPORT_CONTROL_OFF                      = 0x00000000,
    WKS_FEATURE_SUPPORT_CONTROL_SRS_1714_WIN8_STEREO     = 0x00000001,
    WKS_FEATURE_SUPPORT_CONTROL_WIN8_STEREO_EXPORT_IF_ENABLED = 0x00000002,
    WKS_FEATURE_SUPPORT_CONTROL_NUM_VALUES = 3,
    WKS_FEATURE_SUPPORT_CONTROL_DEFAULT = WKS_FEATURE_SUPPORT_CONTROL_SRS_1714_WIN8_STEREO
};

enum EValues_WKS_STEREO_DONGLE_SUPPORT {
    WKS_STEREO_DONGLE_SUPPORT_OFF                        = 0,
    WKS_STEREO_DONGLE_SUPPORT_DAC                        = 1,
    WKS_STEREO_DONGLE_SUPPORT_DLP                        = 2,
    WKS_STEREO_DONGLE_SUPPORT_NUM_VALUES = 3,
    WKS_STEREO_DONGLE_SUPPORT_DEFAULT = WKS_STEREO_DONGLE_SUPPORT_OFF
};

enum EValues_WKS_STEREO_SUPPORT {
    WKS_STEREO_SUPPORT_OFF                               = 0,
    WKS_STEREO_SUPPORT_ON                                = 1,
    WKS_STEREO_SUPPORT_NUM_VALUES = 2,
    WKS_STEREO_SUPPORT_DEFAULT = WKS_STEREO_SUPPORT_OFF
};

enum EValues_AO_MODE {
    AO_MODE_OFF                                          = 0,
    AO_MODE_LOW                                          = 1,
    AO_MODE_MEDIUM                                       = 2,
    AO_MODE_HIGH                                         = 3,
    AO_MODE_NUM_VALUES = 4,
    AO_MODE_DEFAULT = AO_MODE_OFF
};

enum EValues_AO_MODE_ACTIVE {
    AO_MODE_ACTIVE_DISABLED                              = 0,
    AO_MODE_ACTIVE_ENABLED                               = 1,
    AO_MODE_ACTIVE_NUM_VALUES = 2,
    AO_MODE_ACTIVE_DEFAULT = AO_MODE_ACTIVE_DISABLED
};

enum EValues_AUTO_LODBIASADJUST {
    AUTO_LODBIASADJUST_OFF                               = 0x00000000,
    AUTO_LODBIASADJUST_ON                                = 0x00000001,
    AUTO_LODBIASADJUST_NUM_VALUES = 2,
    AUTO_LODBIASADJUST_DEFAULT = AUTO_LODBIASADJUST_ON
};

enum EValues_LODBIASADJUST {
    LODBIASADJUST_MIN                                    = 0xffffff80,
    LODBIASADJUST_MAX                                    = 128,
    LODBIASADJUST_NUM_VALUES = 2,
    LODBIASADJUST_DEFAULT = 0
};

enum EValues_PRERENDERLIMIT {
    PRERENDERLIMIT_MIN                                   = 0x00,
    PRERENDERLIMIT_MAX                                   = 0xff,
    PRERENDERLIMIT_APP_CONTROLLED                        = 0x00,
    PRERENDERLIMIT_NUM_VALUES = 3,
    PRERENDERLIMIT_DEFAULT = PRERENDERLIMIT_APP_CONTROLLED
};

enum EValues_PS_DYNAMIC_TILING {
    PS_DYNAMIC_TILING_OFF                                = 0x82247787,
    PS_DYNAMIC_TILING_ON                                 = 0x74288976,
    PS_DYNAMIC_TILING_NUM_VALUES = 2,
    PS_DYNAMIC_TILING_DEFAULT = PS_DYNAMIC_TILING_OFF
};

enum EValues_PS_TEXFILTER_ANISO_OPTS2 {
    PS_TEXFILTER_ANISO_OPTS2_OFF                         = 0x00000000,
    PS_TEXFILTER_ANISO_OPTS2_ON                          = 0x00000001,
    PS_TEXFILTER_ANISO_OPTS2_NUM_VALUES = 2,
    PS_TEXFILTER_ANISO_OPTS2_DEFAULT = PS_TEXFILTER_ANISO_OPTS2_OFF
};

enum EValues_PS_TEXFILTER_BILINEAR_IN_ANISO {
    PS_TEXFILTER_BILINEAR_IN_ANISO_OFF                   = 0x00000000,
    PS_TEXFILTER_BILINEAR_IN_ANISO_ON                    = 0x00000001,
    PS_TEXFILTER_BILINEAR_IN_ANISO_NUM_VALUES = 2,
    PS_TEXFILTER_BILINEAR_IN_ANISO_DEFAULT = PS_TEXFILTER_BILINEAR_IN_ANISO_OFF
};

enum EValues_PS_TEXFILTER_DISABLE_TRILIN_SLOPE {
    PS_TEXFILTER_DISABLE_TRILIN_SLOPE_OFF                = 0x00000000,
    PS_TEXFILTER_DISABLE_TRILIN_SLOPE_ON                 = 0x00000001,
    PS_TEXFILTER_DISABLE_TRILIN_SLOPE_NUM_VALUES = 2,
    PS_TEXFILTER_DISABLE_TRILIN_SLOPE_DEFAULT = PS_TEXFILTER_DISABLE_TRILIN_SLOPE_OFF
};

enum EValues_PS_TEXFILTER_NO_NEG_LODBIAS {
    PS_TEXFILTER_NO_NEG_LODBIAS_OFF                      = 0x00000000,
    PS_TEXFILTER_NO_NEG_LODBIAS_ON                       = 0x00000001,
    PS_TEXFILTER_NO_NEG_LODBIAS_NUM_VALUES = 2,
    PS_TEXFILTER_NO_NEG_LODBIAS_DEFAULT = PS_TEXFILTER_NO_NEG_LODBIAS_OFF
};

enum EValues_QUALITY_ENHANCEMENTS {
    QUALITY_ENHANCEMENTS_HIGHQUALITY                     = 0xfffffff6,
    QUALITY_ENHANCEMENTS_QUALITY                         = 0x00000000,
    QUALITY_ENHANCEMENTS_PERFORMANCE                     = 0x0000000a,
    QUALITY_ENHANCEMENTS_HIGHPERFORMANCE                 = 0x00000014,
    QUALITY_ENHANCEMENTS_NUM_VALUES = 4,
    QUALITY_ENHANCEMENTS_DEFAULT = QUALITY_ENHANCEMENTS_QUALITY
};

enum EValues_REFRESH_RATE_OVERRIDE {
    REFRESH_RATE_OVERRIDE_APPLICATION_CONTROLLED         = 0,
    REFRESH_RATE_OVERRIDE_HIGHEST_AVAILABLE              = 1,
    REFRESH_RATE_OVERRIDE_NUM_VALUES = 2,
    REFRESH_RATE_OVERRIDE_DEFAULT = REFRESH_RATE_OVERRIDE_APPLICATION_CONTROLLED
};

enum EValues_SET_POWER_THROTTLE_FOR_PCIe_COMPLIANCE {
    SET_POWER_THROTTLE_FOR_PCIe_COMPLIANCE_OFF           = 0x00000000,
    SET_POWER_THROTTLE_FOR_PCIe_COMPLIANCE_ON            = 0x00000001,
    SET_POWER_THROTTLE_FOR_PCIe_COMPLIANCE_NUM_VALUES = 2,
    SET_POWER_THROTTLE_FOR_PCIe_COMPLIANCE_DEFAULT = SET_POWER_THROTTLE_FOR_PCIe_COMPLIANCE_OFF
};

enum EValues_SET_VAB_DATA {
    SET_VAB_DATA_ZERO                                    = 0x00000000,
    SET_VAB_DATA_UINT_ONE                                = 0x00000001,
    SET_VAB_DATA_FLOAT_ONE                               = 0x3f800000,
    SET_VAB_DATA_FLOAT_POS_INF                           = 0x7f800000,
    SET_VAB_DATA_FLOAT_NAN                               = 0x7fc00000,
    SET_VAB_DATA_USE_API_DEFAULTS                        = 0xffffffff,
    SET_VAB_DATA_NUM_VALUES = 6,
    SET_VAB_DATA_DEFAULT = SET_VAB_DATA_USE_API_DEFAULTS
};

enum EValues_VSYNCMODE {
    VSYNCMODE_PASSIVE                                    = 0x60925292,
    VSYNCMODE_FORCEOFF                                   = 0x08416747,
    VSYNCMODE_FORCEON                                    = 0x47814940,
    VSYNCMODE_FLIPINTERVAL2                              = 0x32610244,
    VSYNCMODE_FLIPINTERVAL3                              = 0x71271021,
    VSYNCMODE_FLIPINTERVAL4                              = 0x13245256,
    VSYNCMODE_NUM_VALUES = 6,
    VSYNCMODE_DEFAULT = VSYNCMODE_PASSIVE
};

enum EValues_VSYNCTEARCONTROL {
    VSYNCTEARCONTROL_DISABLE                             = 0x96861077,
    VSYNCTEARCONTROL_ENABLE                              = 0x99941284,
    VSYNCTEARCONTROL_NUM_VALUES = 2,
    VSYNCTEARCONTROL_DEFAULT = VSYNCTEARCONTROL_DISABLE
};



typedef struct _SettingDWORDNameString {
    NvU32 settingId;
    const wchar_t * settingNameString;
    NvU32 numSettingValues;
    NvU32 *settingValues;
    NvU32 defaultValue;
} SettingDWORDNameString;

typedef struct _SettingWSTRINGNameString {
    NvU32 settingId;
    const wchar_t * settingNameString;
    NvU32 numSettingValues;
    const wchar_t **settingValues;
    const wchar_t * defaultValue;
} SettingWSTRINGNameString;


#endif // _NVAPI_DRIVER_SETTINGS_H_

