/*
 *
 * Copyright (c) 2014-2022 The Khronos Group Inc.
 * Copyright (c) 2014-2022 Valve Corporation
 * Copyright (c) 2014-2022 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: Jon Ashburn <jon@lunarg.com>
 * Author: Courtney Goeltzenleuchter <courtney@LunarG.com>
 * Author: Chia-I Wu <olvaffe@gmail.com>
 * Author: Chia-I Wu <olv@lunarg.com>
 * Author: Mark Lobodzinski <mark@LunarG.com>
 * Author: Lenny Komow <lenny@lunarg.com>
 * Author: Charles Giessen <charles@lunarg.com>
 *
 */

#include "log.h"

#include <stdio.h>
#include <stdarg.h>

#include "debug_utils.h"
#include "loader_common.h"
#include "loader_environment.h"
#include "settings.h"
#include "vk_loader_platform.h"

uint32_t g_loader_debug = 0;

void loader_init_global_debug_level(void) {
    char *env, *orig;

    if (g_loader_debug > 0) return;

    g_loader_debug = 0;

    // Parse comma-separated debug options
    orig = env = loader_getenv("VK_LOADER_DEBUG", NULL);
    while (env) {
        char *p = strchr(env, ',');
        size_t len;

        if (p) {
            len = p - env;
        } else {
            len = strlen(env);
        }

        if (len > 0) {
            if (strncmp(env, "all", len) == 0) {
                g_loader_debug = ~0u;
            } else if (strncmp(env, "warn", len) == 0) {
                g_loader_debug |= VULKAN_LOADER_WARN_BIT;
            } else if (strncmp(env, "info", len) == 0) {
                g_loader_debug |= VULKAN_LOADER_INFO_BIT;
            } else if (strncmp(env, "perf", len) == 0) {
                g_loader_debug |= VULKAN_LOADER_PERF_BIT;
            } else if (strncmp(env, "error", len) == 0) {
                g_loader_debug |= VULKAN_LOADER_ERROR_BIT;
            } else if (strncmp(env, "debug", len) == 0) {
                g_loader_debug |= VULKAN_LOADER_DEBUG_BIT;
            } else if (strncmp(env, "layer", len) == 0) {
                g_loader_debug |= VULKAN_LOADER_LAYER_BIT;
            } else if (strncmp(env, "driver", len) == 0 || strncmp(env, "implem", len) == 0 || strncmp(env, "icd", len) == 0) {
                g_loader_debug |= VULKAN_LOADER_DRIVER_BIT;
            }
        }

        if (!p) break;

        env = p + 1;
    }

    loader_free_getenv(orig, NULL);
}

void loader_set_global_debug_level(uint32_t new_loader_debug) { g_loader_debug = new_loader_debug; }

uint32_t loader_get_global_debug_level(void) { return g_loader_debug; }

void loader_log(const struct loader_instance *inst, VkFlags msg_type, int32_t msg_code, const char *format, ...) {
    (void)msg_code;
    char msg[512];

    va_list ap;
    va_start(ap, format);
    int ret = vsnprintf(msg, sizeof(msg), format, ap);
    if ((ret >= (int)sizeof(msg)) || ret < 0) {
        msg[sizeof(msg) - 1] = '\0';
    }
    va_end(ap);

    if (inst) {
        VkDebugUtilsMessageSeverityFlagBitsEXT severity = 0;
        VkDebugUtilsMessageTypeFlagsEXT type;
        VkDebugUtilsMessengerCallbackDataEXT callback_data;
        VkDebugUtilsObjectNameInfoEXT object_name;

        if ((msg_type & VULKAN_LOADER_INFO_BIT) != 0) {
            severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
        } else if ((msg_type & VULKAN_LOADER_WARN_BIT) != 0) {
            severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        } else if ((msg_type & VULKAN_LOADER_ERROR_BIT) != 0) {
            severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        } else if ((msg_type & VULKAN_LOADER_DEBUG_BIT) != 0) {
            severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
        } else if ((msg_type & VULKAN_LOADER_LAYER_BIT) != 0 || (msg_type & VULKAN_LOADER_DRIVER_BIT) != 0) {
            // Just driver or just layer bit should be treated as an info message in debug utils.
            severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
        }

        if ((msg_type & VULKAN_LOADER_PERF_BIT) != 0) {
            type = VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        } else if ((msg_type & VULKAN_LOADER_VALIDATION_BIT) != 0) {
            // For loader logging, if it's a validation message, we still want to also keep the general flag as well
            // so messages of type validation can still be triggered for general message callbacks.
            type = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        } else {
            type = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
        }

        callback_data.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT;
        callback_data.pNext = NULL;
        callback_data.flags = 0;
        callback_data.pMessageIdName = "Loader Message";
        callback_data.messageIdNumber = 0;
        callback_data.pMessage = msg;
        callback_data.queueLabelCount = 0;
        callback_data.pQueueLabels = NULL;
        callback_data.cmdBufLabelCount = 0;
        callback_data.pCmdBufLabels = NULL;
        callback_data.objectCount = 1;
        callback_data.pObjects = &object_name;
        object_name.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        object_name.pNext = NULL;
        object_name.objectType = VK_OBJECT_TYPE_INSTANCE;
        object_name.objectHandle = (uint64_t)(uintptr_t)inst;
        object_name.pObjectName = NULL;

        util_SubmitDebugUtilsMessageEXT(inst, severity, type, &callback_data);
    }

    // Exit early if the current instance settings do not ask for logging to stderr
    if (inst && inst->settings.settings_active && 0 == (msg_type & inst->settings.debug_level)) {
        return;
    } else {
        // Check the global settings and if that doesn't say to skip, check the environment variable
        if (0 == (msg_type & g_loader_debug)) return;
    }

    // Only need enough space to create the filter description header for log messages
    // Also use the same header for all output
    char cmd_line_msg[64];
    size_t cmd_line_size = sizeof(cmd_line_msg);
    size_t num_used = 1;

    cmd_line_msg[0] = '\0';

// Helper macro which strncat's the given string literal, then updates num_used & cmd_line_end
// Assumes that we haven't used the entire buffer - must manually check this when adding new filter types
// We concat at the end of cmd_line_msg, so that strncat isn't a victim of Schlemiel the Painter
// We write to the end - 1 of cmd_line_msg, as the end is actually a null terminator
#define STRNCAT_TO_BUFFER(string_literal_to_cat)                                             \
    strncat(cmd_line_msg + (num_used - 1), string_literal_to_cat, cmd_line_size - num_used); \
    num_used += sizeof(string_literal_to_cat) - 1;  // subtract one to remove the null terminator in the string literal

    if ((msg_type & VULKAN_LOADER_ERROR_BIT) != 0) {
        STRNCAT_TO_BUFFER("ERROR");
    } else if ((msg_type & VULKAN_LOADER_WARN_BIT) != 0) {
        STRNCAT_TO_BUFFER("WARNING");
    } else if ((msg_type & VULKAN_LOADER_INFO_BIT) != 0) {
        STRNCAT_TO_BUFFER("INFO");
    } else if ((msg_type & VULKAN_LOADER_DEBUG_BIT) != 0) {
        STRNCAT_TO_BUFFER("DEBUG");
    }

    if ((msg_type & VULKAN_LOADER_PERF_BIT) != 0) {
        if (num_used > 1) {
            STRNCAT_TO_BUFFER(" | ");
        }
        STRNCAT_TO_BUFFER("PERF");
    }
    if ((msg_type & VULKAN_LOADER_DRIVER_BIT) != 0) {
        if (num_used > 1) {
            STRNCAT_TO_BUFFER(" | ");
        }
        STRNCAT_TO_BUFFER("DRIVER");
    }
    if ((msg_type & VULKAN_LOADER_LAYER_BIT) != 0) {
        if (num_used > 1) {
            STRNCAT_TO_BUFFER(" | ");
        }
        STRNCAT_TO_BUFFER("LAYER");
    }

    // Add a ": " to separate the filters from the message
    STRNCAT_TO_BUFFER(": ");
#undef STRNCAT_TO_BUFFER

    // Justifies the output to at least 19 spaces
    if (num_used < 19) {
        const char *space_buffer = "                   ";
        // Only write (19 - num_used) spaces
        strncat(cmd_line_msg + (num_used - 1), space_buffer, 19 - num_used);
        num_used += sizeof(space_buffer) - (num_used - 1);
    }
    // Assert that we didn't write more than what is available in cmd_line_msg
    assert(cmd_line_size > num_used);

    fputs(cmd_line_msg, stderr);
    fputs(msg, stderr);
    fputc('\n', stderr);
#if defined(WIN32)
    OutputDebugString(cmd_line_msg);
    OutputDebugString(msg);
    OutputDebugString("\n");
#endif
}

void loader_log_asm_function_not_supported(const struct loader_instance *inst, VkFlags msg_type, int32_t msg_code,
                                           const char *func_name) {
    loader_log(inst, msg_type, msg_code, "Function %s not supported for this physical device", func_name);
}
