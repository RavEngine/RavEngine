/*
 *
 * Copyright (c) 2023 The Khronos Group Inc.
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
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
 *
 * Author: Charles Giessen <charles@lunarg.com>
 *
 */

#include "settings.h"

#include "allocation.h"
#include "cJSON.h"
#include "loader.h"
#include "loader_environment.h"
#include "loader_windows.h"
#include "log.h"
#include "stack_allocation.h"
#include "vk_loader_platform.h"

loader_platform_thread_mutex global_loader_settings_lock;
loader_settings global_loader_settings;

void free_layer_configuration(const struct loader_instance* inst, loader_settings_layer_configuration* layer_configuration) {
    loader_instance_heap_free(inst, layer_configuration->name);
    layer_configuration->name = NULL;
    loader_instance_heap_free(inst, layer_configuration->path);
    layer_configuration->path = NULL;
}

void free_loader_settings(const struct loader_instance* inst, loader_settings* settings) {
    if (NULL != settings->layer_configurations) {
        for (uint32_t i = 0; i < settings->layer_configuration_count; i++) {
            free_layer_configuration(inst, &settings->layer_configurations[i]);
        }
    }
    loader_instance_heap_free(inst, settings->layer_configurations);
    loader_instance_heap_free(inst, settings->settings_file_path);
    settings->layer_configurations = NULL;
    memset(settings, 0, sizeof(loader_settings));
}

loader_settings_layer_control parse_control_string(char* control_string) {
    loader_settings_layer_control layer_control = LOADER_SETTINGS_LAYER_CONTROL_DEFAULT;
    if (strcmp(control_string, "auto") == 0)
        layer_control = LOADER_SETTINGS_LAYER_CONTROL_DEFAULT;
    else if (strcmp(control_string, "on") == 0)
        layer_control = LOADER_SETTINGS_LAYER_CONTROL_ON;
    else if (strcmp(control_string, "off") == 0)
        layer_control = LOADER_SETTINGS_LAYER_CONTROL_OFF;
    else if (strcmp(control_string, "unordered_layer_location") == 0)
        layer_control = LOADER_SETTINGS_LAYER_UNORDERED_LAYER_LOCATION;
    return layer_control;
}

const char* loader_settings_layer_control_to_string(loader_settings_layer_control control) {
    switch (control) {
        case (LOADER_SETTINGS_LAYER_CONTROL_DEFAULT):
            return "auto";
        case (LOADER_SETTINGS_LAYER_CONTROL_ON):
            return "on";
        case (LOADER_SETTINGS_LAYER_CONTROL_OFF):
            return "off";
        case (LOADER_SETTINGS_LAYER_UNORDERED_LAYER_LOCATION):
            return "unordered_layer_location";
        default:
            return "UNKNOWN_LAYER_CONTROl";
    }
}

uint32_t parse_log_filters_from_strings(struct loader_string_list* log_filters) {
    uint32_t filters = 0;
    for (uint32_t i = 0; i < log_filters->count; i++) {
        if (strcmp(log_filters->list[i], "all") == 0)
            filters |= VULKAN_LOADER_INFO_BIT | VULKAN_LOADER_WARN_BIT | VULKAN_LOADER_PERF_BIT | VULKAN_LOADER_ERROR_BIT |
                       VULKAN_LOADER_DEBUG_BIT | VULKAN_LOADER_LAYER_BIT | VULKAN_LOADER_DRIVER_BIT | VULKAN_LOADER_VALIDATION_BIT;
        else if (strcmp(log_filters->list[i], "info") == 0)
            filters |= VULKAN_LOADER_INFO_BIT;
        else if (strcmp(log_filters->list[i], "warn") == 0)
            filters |= VULKAN_LOADER_WARN_BIT;
        else if (strcmp(log_filters->list[i], "perf") == 0)
            filters |= VULKAN_LOADER_PERF_BIT;
        else if (strcmp(log_filters->list[i], "error") == 0)
            filters |= VULKAN_LOADER_ERROR_BIT;
        else if (strcmp(log_filters->list[i], "debug") == 0)
            filters |= VULKAN_LOADER_DEBUG_BIT;
        else if (strcmp(log_filters->list[i], "layer") == 0)
            filters |= VULKAN_LOADER_LAYER_BIT;
        else if (strcmp(log_filters->list[i], "driver") == 0)
            filters |= VULKAN_LOADER_DRIVER_BIT;
        else if (strcmp(log_filters->list[i], "validation") == 0)
            filters |= VULKAN_LOADER_VALIDATION_BIT;
    }
    return filters;
}

bool parse_json_enable_disable_option(const struct loader_instance* inst, cJSON* object, const char* key) {
    char* str = NULL;
    VkResult res = loader_parse_json_string(object, key, &str);
    if (res != VK_SUCCESS || NULL == str) {
        return false;
    }
    bool enable = false;
    if (strcmp(str, "enabled") == 0) {
        enable = true;
    }
    loader_instance_heap_free(inst, str);
    return enable;
}

VkResult parse_layer_configuration(const struct loader_instance* inst, cJSON* layer_configuration_json,
                                   loader_settings_layer_configuration* layer_configuration) {
    char* control_string = NULL;
    VkResult res = loader_parse_json_string(layer_configuration_json, "control", &control_string);
    if (res != VK_SUCCESS) {
        goto out;
    }
    layer_configuration->control = parse_control_string(control_string);
    loader_instance_heap_free(inst, control_string);

    // If that is the only value - do no further parsing
    if (layer_configuration->control == LOADER_SETTINGS_LAYER_UNORDERED_LAYER_LOCATION) {
        goto out;
    }

    res = loader_parse_json_string(layer_configuration_json, "name", &(layer_configuration->name));
    if (res != VK_SUCCESS) {
        goto out;
    }

    res = loader_parse_json_string(layer_configuration_json, "path", &(layer_configuration->path));
    if (res != VK_SUCCESS) {
        goto out;
    }

    cJSON* treat_as_implicit_manifest = cJSON_GetObjectItem(layer_configuration_json, "treat_as_implicit_manifest");
    if (treat_as_implicit_manifest && treat_as_implicit_manifest->type == cJSON_True) {
        layer_configuration->treat_as_implicit_manifest = true;
    }
out:
    if (VK_SUCCESS != res) {
        free_layer_configuration(inst, layer_configuration);
    }
    return res;
}

VkResult parse_layer_configurations(const struct loader_instance* inst, cJSON* settings_object, loader_settings* loader_settings) {
    VkResult res = VK_SUCCESS;

    cJSON* layer_configurations = cJSON_GetObjectItem(settings_object, "layers");
    if (NULL == layer_configurations) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    uint32_t layer_configurations_count = cJSON_GetArraySize(layer_configurations);
    if (layer_configurations_count == 0) {
        return VK_SUCCESS;
    }

    loader_settings->layer_configuration_count = layer_configurations_count;

    loader_settings->layer_configurations = loader_instance_heap_calloc(
        inst, sizeof(loader_settings_layer_configuration) * layer_configurations_count, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
    if (NULL == loader_settings->layer_configurations) {
        res = VK_ERROR_OUT_OF_HOST_MEMORY;
        goto out;
    }

    for (uint32_t i = 0; i < layer_configurations_count; i++) {
        cJSON* layer = cJSON_GetArrayItem(layer_configurations, i);
        if (NULL == layer) {
            res = VK_ERROR_INITIALIZATION_FAILED;
            goto out;
        }
        res = parse_layer_configuration(inst, layer, &(loader_settings->layer_configurations[i]));
        if (VK_SUCCESS != res) {
            goto out;
        }
    }
out:
    if (res != VK_SUCCESS) {
        if (loader_settings->layer_configurations) {
            for (uint32_t i = 0; i < layer_configurations_count; i++) {
                free_layer_configuration(inst, &loader_settings->layer_configurations[i]);
            }
        }
        loader_instance_heap_free(inst, &loader_settings->layer_configurations);
    }

    return res;
}

VkResult check_if_settings_path_exists(const struct loader_instance* inst, char* base, char* suffix, char** settings_file_path) {
    if (NULL == base || NULL == suffix) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    *settings_file_path = loader_instance_heap_calloc(inst, strlen(base) + strlen(suffix) + 1, VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
    if (NULL == *settings_file_path) {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    strncpy(*settings_file_path, base, strlen(base));
    strncat(*settings_file_path, suffix, strlen(suffix));

    if (!loader_platform_file_exists(*settings_file_path)) {
        loader_instance_heap_free(inst, *settings_file_path);
        *settings_file_path = NULL;
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    return VK_SUCCESS;
}
VkResult get_unix_settings_path(const struct loader_instance* inst, char** settings_file_path) {
    VkResult res =
        check_if_settings_path_exists(inst, loader_secure_getenv("HOME", inst),
                                      "/.local/share/vulkan/loader_settings.d/" VK_LOADER_SETTINGS_FILENAME, settings_file_path);
    if (res == VK_SUCCESS) {
        return res;
    }
    // If HOME isn't set, fallback to XDG_DATA_HOME
    res = check_if_settings_path_exists(inst, loader_secure_getenv("XDG_DATA_HOME", inst),
                                        "/vulkan/loader_settings.d/" VK_LOADER_SETTINGS_FILENAME, settings_file_path);
    if (res == VK_SUCCESS) {
        return res;
    }
    // if XDG_DATA_HOME isn't set, fallback to /etc.
    // note that the settings_fil_path_suffix stays the same since its the same layout as for XDG_DATA_HOME
    return check_if_settings_path_exists(inst, "/etc", "/vulkan/loader_settings.d/" VK_LOADER_SETTINGS_FILENAME,
                                         settings_file_path);
}

bool check_if_settings_are_equal(loader_settings* a, loader_settings* b) {
    // If either pointer is null, return true
    if (NULL == a || NULL == b) return false;
    bool are_equal = true;
    are_equal &= a->settings_active == b->settings_active;
    are_equal &= a->has_unordered_layer_location == b->has_unordered_layer_location;
    are_equal &= a->debug_level == b->debug_level;
    are_equal &= a->layer_configuration_count == b->layer_configuration_count;
    if (!are_equal) return false;
    for (uint32_t i = 0; i < a->layer_configuration_count && i < b->layer_configuration_count; i++) {
        if (a->layer_configurations[i].name && b->layer_configurations[i].name) {
            are_equal &= 0 == strcmp(a->layer_configurations[i].name, b->layer_configurations[i].name);
        } else {
            are_equal = false;
        }
        if (a->layer_configurations[i].path && b->layer_configurations[i].path) {
            are_equal &= 0 == strcmp(a->layer_configurations[i].path, b->layer_configurations[i].path);
        } else {
            are_equal = false;
        }
        are_equal &= a->layer_configurations[i].control == b->layer_configurations[i].control;
    }
    return are_equal;
}

void log_settings(const struct loader_instance* inst, loader_settings* settings) {
    if (settings == NULL) {
        return;
    }
    loader_log(inst, VULKAN_LOADER_INFO_BIT, 0, "Using layer configurations found in loader settings from %s",
               settings->settings_file_path);

    loader_log(inst, VULKAN_LOADER_DEBUG_BIT, 0, "Layer Configurations count = %d", settings->layer_configuration_count);
    for (uint32_t i = 0; i < settings->layer_configuration_count; i++) {
        loader_log(inst, VULKAN_LOADER_DEBUG_BIT, 0, "---- Layer Configuration [%d] ----", i);
        if (settings->layer_configurations[i].control != LOADER_SETTINGS_LAYER_UNORDERED_LAYER_LOCATION) {
            loader_log(inst, VULKAN_LOADER_DEBUG_BIT, 0, "Name: %s", settings->layer_configurations[i].name);
            loader_log(inst, VULKAN_LOADER_DEBUG_BIT, 0, "Path: %s", settings->layer_configurations[i].path);
        }
        loader_log(inst, VULKAN_LOADER_DEBUG_BIT, 0, "Control: %s",
                   loader_settings_layer_control_to_string(settings->layer_configurations[i].control));
    }
    loader_log(inst, VULKAN_LOADER_DEBUG_BIT, 0, "---------------------------------");
}

// Loads the vk_loader_settings.json file
// Returns VK_SUCCESS if it was found & was successfully parsed. Otherwise, it returns VK_ERROR_INITIALIZATION_FAILED if it
// wasn't found or failed to parse, and returns VK_ERROR_OUT_OF_HOST_MEMORY if it was unable to allocate enough memory.
VkResult get_loader_settings(const struct loader_instance* inst, loader_settings* loader_settings) {
    VkResult res = VK_SUCCESS;
    cJSON* json = NULL;
    char* file_format_version_string = NULL;
    char* settings_file_path = NULL;
#if defined(WIN32)
    res = windows_get_loader_settings_file_path(inst, &settings_file_path);
    if (res != VK_SUCCESS) {
        goto out;
    }

#elif COMMON_UNIX_PLATFORMS
    res = get_unix_settings_path(inst, &settings_file_path);
    if (res != VK_SUCCESS) {
        goto out;
    }
#else
#warning "Unsupported platform - must specify platform specific location for vk_loader_settings.json"
#endif

    res = loader_get_json(inst, settings_file_path, &json);
    // Make sure sure the top level json value is an object
    if (res != VK_SUCCESS || NULL == json || json->type != 6) {
        goto out;
    }

    res = loader_parse_json_string(json, "file_format_version", &file_format_version_string);
    if (res != VK_SUCCESS) {
        goto out;
    }
    uint32_t settings_array_count = 0;
    bool has_multi_setting_file = false;
    cJSON* settings_array = cJSON_GetObjectItem(json, "settings_array");
    cJSON* single_settings_object = cJSON_GetObjectItem(json, "settings");
    if (NULL != settings_array) {
        has_multi_setting_file = true;
        settings_array_count = cJSON_GetArraySize(settings_array);
    } else if (NULL != single_settings_object) {
        settings_array_count = 1;
    }

    // Corresponds to the settings object that has no app keys
    int global_settings_index = -1;
    // Corresponds to the settings object which has a matchign app key
    int index_to_use = -1;

    char current_process_path[1024];
    bool valid_exe_path = NULL != loader_platform_executable_path(current_process_path, 1024);

    for (int i = 0; i < (int)settings_array_count; i++) {
        if (has_multi_setting_file) {
            single_settings_object = cJSON_GetArrayItem(settings_array, i);
        }
        cJSON* app_keys = cJSON_GetObjectItem(single_settings_object, "app_keys");
        if (NULL == app_keys) {
            if (global_settings_index == -1) {
                global_settings_index = i;  // use the first 'global' settings that has no app keys as the global one
            }
            continue;
        } else if (valid_exe_path) {
            int app_key_count = cJSON_GetArraySize(app_keys);
            if (app_key_count == 0) {
                continue;  // empty array
            }
            for (int j = 0; j < app_key_count; j++) {
                cJSON* app_key_json = cJSON_GetArrayItem(app_keys, j);
                if (NULL == app_key_json) {
                    continue;
                }
                char* app_key = cJSON_Print(app_key_json);
                if (NULL == app_key) {
                    continue;
                }

                if (strcmp(current_process_path, app_key) == 0) {
                    index_to_use = i;
                }
                loader_instance_heap_free(inst, app_key);
                if (index_to_use == i) {
                    break;  // break only after freeing the app key
                }
            }
        }
    }

    // No app specific settings match - either use global settings or exit
    if (index_to_use == -1) {
        if (global_settings_index == -1) {
            goto out;  // No global settings were found - exit
        } else {
            index_to_use = global_settings_index;  // Global settings are present - use it
        }
    }

    // Now get the actual settings object to use - already have it if there is only one settings object
    // If there are multiple settings, just need to set single_settings_object to the desired settings object
    if (has_multi_setting_file) {
        single_settings_object = cJSON_GetArrayItem(settings_array, index_to_use);
        if (NULL == single_settings_object) {
            res = VK_ERROR_INITIALIZATION_FAILED;
            goto out;
        }
    }

    // optional
    cJSON* stderr_filter = cJSON_GetObjectItem(single_settings_object, "stderr_log");
    if (NULL != stderr_filter) {
        struct loader_string_list stderr_log = {0};
        res = loader_parse_json_array_of_strings(inst, single_settings_object, "stderr_log", &stderr_log);
        if (VK_ERROR_OUT_OF_HOST_MEMORY == res) {
            goto out;
        }
        loader_settings->debug_level = parse_log_filters_from_strings(&stderr_log);
        free_string_list(inst, &stderr_log);
    }

    // optional
    cJSON* logs_to_use = cJSON_GetObjectItem(single_settings_object, "log_locations");
    if (NULL != logs_to_use) {
        int log_count = cJSON_GetArraySize(logs_to_use);
        for (int i = 0; i < log_count; i++) {
            cJSON* log_element = cJSON_GetArrayItem(logs_to_use, i);
            // bool is_valid = true;
            if (NULL != log_element) {
                struct loader_string_list log_destinations = {0};
                res = loader_parse_json_array_of_strings(inst, log_element, "destinations", &log_destinations);
                if (res != VK_SUCCESS) {
                    // is_valid = false;
                }
                free_string_list(inst, &log_destinations);
                struct loader_string_list log_filters = {0};
                res = loader_parse_json_array_of_strings(inst, log_element, "filters", &log_filters);
                if (res != VK_SUCCESS) {
                    // is_valid = false;
                }
                free_string_list(inst, &log_filters);
            }
        }
    }

    res = parse_layer_configurations(inst, single_settings_object, loader_settings);
    if (res != VK_SUCCESS) {
        goto out;
    }

    // Determine if there exists a layer configuration indicating where to put layers not contained in the settings file
    // LOADER_SETTINGS_LAYER_UNORDERED_LAYER_LOCATION
    for (uint32_t i = 0; i < loader_settings->layer_configuration_count; i++) {
        if (loader_settings->layer_configurations[i].control == LOADER_SETTINGS_LAYER_UNORDERED_LAYER_LOCATION) {
            loader_settings->has_unordered_layer_location = true;
            break;
        }
    }

    loader_settings->settings_file_path = settings_file_path;
    settings_file_path = NULL;
    loader_settings->settings_active = true;
out:
    if (NULL != json) {
        cJSON_Delete(json);
    }

    loader_instance_heap_free(inst, settings_file_path);

    loader_instance_heap_free(inst, file_format_version_string);
    return res;
}

VkResult update_global_loader_settings(void) {
    loader_settings settings = {0};
    VkResult res = get_loader_settings(NULL, &settings);
    loader_platform_thread_lock_mutex(&global_loader_settings_lock);

    free_loader_settings(NULL, &global_loader_settings);
    if (res == VK_SUCCESS) {
        if (!check_if_settings_are_equal(&settings, &global_loader_settings)) {
            log_settings(NULL, &settings);
        }

        memcpy(&global_loader_settings, &settings, sizeof(loader_settings));
        if (global_loader_settings.settings_active) {
            loader_set_global_debug_level(global_loader_settings.debug_level);
        }
    }
    loader_platform_thread_unlock_mutex(&global_loader_settings_lock);
    return res;
}

void init_global_loader_settings(void) {
    loader_platform_thread_create_mutex(&global_loader_settings_lock);
    // Free out the global settings in case the process was loaded & unloaded
    free_loader_settings(NULL, &global_loader_settings);
}
void teardown_global_loader_settings(void) {
    free_loader_settings(NULL, &global_loader_settings);
    loader_platform_thread_delete_mutex(&global_loader_settings_lock);
}

bool should_skip_logging_global_messages(VkFlags msg_type) {
    loader_platform_thread_lock_mutex(&global_loader_settings_lock);
    bool should_skip = global_loader_settings.settings_active && 0 != (msg_type & global_loader_settings.debug_level);
    loader_platform_thread_unlock_mutex(&global_loader_settings_lock);
    return should_skip;
}

// Use this function to get the correct settings to use based on the context
// If inst is NULL - use the global settings and lock the mutex
// Else return the settings local to the instance - but do nto lock the mutex
const loader_settings* get_current_settings_and_lock(const struct loader_instance* inst) {
    if (inst) {
        return &inst->settings;
    }
    loader_platform_thread_lock_mutex(&global_loader_settings_lock);
    return &global_loader_settings;
}
// Release the global settings lock if we are using the global settings - aka if inst is NULL
void release_current_settings_lock(const struct loader_instance* inst) {
    if (inst == NULL) {
        loader_platform_thread_unlock_mutex(&global_loader_settings_lock);
    }
}

VkResult get_settings_layers(const struct loader_instance* inst, struct loader_layer_list* settings_layers,
                             bool* should_search_for_other_layers) {
    VkResult res = VK_SUCCESS;
    *should_search_for_other_layers = true;  // default to true

    const loader_settings* settings = get_current_settings_and_lock(inst);

    // Assume the list doesn't contain LOADER_SETTINGS_LAYER_UNORDERED_LAYER_LOCATION at first
    if (settings != NULL && settings->settings_active) {
        *should_search_for_other_layers = false;
    }

    for (uint32_t i = 0; i < settings->layer_configuration_count; i++) {
        loader_settings_layer_configuration* layer_config = &settings->layer_configurations[i];

        // If we encountered a layer that should be forced off, we add it to the settings_layers list but only
        // with the data required to compare it with layers not in the settings file (aka name and manifest path)
        if (layer_config->control == LOADER_SETTINGS_LAYER_CONTROL_OFF) {
            struct loader_layer_properties props = {0};
            props.settings_control_value = LOADER_SETTINGS_LAYER_CONTROL_OFF;
            strncpy(props.info.layerName, layer_config->name, VK_MAX_EXTENSION_NAME_SIZE);
            props.info.layerName[VK_MAX_EXTENSION_NAME_SIZE - 1] = '\0';
            res = loader_copy_to_new_str(inst, layer_config->path, &props.manifest_file_name);
            if (VK_ERROR_OUT_OF_HOST_MEMORY == res) {
                goto out;
            }
            res = loader_append_layer_property(inst, settings_layers, &props);
            if (VK_ERROR_OUT_OF_HOST_MEMORY == res) {
                loader_free_layer_properties(inst, &props);
                goto out;
            }
            continue;
        }

        // The special layer location that indicates where unordered layers should go only should have the
        // settings_control_value set - everythign else should be NULL
        if (layer_config->control == LOADER_SETTINGS_LAYER_UNORDERED_LAYER_LOCATION) {
            struct loader_layer_properties props = {0};
            props.settings_control_value = LOADER_SETTINGS_LAYER_UNORDERED_LAYER_LOCATION;
            res = loader_append_layer_property(inst, settings_layers, &props);
            if (VK_ERROR_OUT_OF_HOST_MEMORY == res) {
                loader_free_layer_properties(inst, &props);
                goto out;
            }
            *should_search_for_other_layers = true;
            continue;
        }

        if (layer_config->path == NULL) {
            continue;
        }

        cJSON* json = NULL;
        VkResult local_res = loader_get_json(inst, layer_config->path, &json);
        if (VK_ERROR_OUT_OF_HOST_MEMORY == local_res) {
            res = VK_ERROR_OUT_OF_HOST_MEMORY;
            goto out;
        } else if (VK_SUCCESS != local_res || NULL == json) {
            continue;
        }

        local_res =
            loader_add_layer_properties(inst, settings_layers, json, layer_config->treat_as_implicit_manifest, layer_config->path);
        cJSON_Delete(json);

        // If the error is anything other than out of memory we still want to try to load the other layers
        if (VK_ERROR_OUT_OF_HOST_MEMORY == local_res) {
            res = VK_ERROR_OUT_OF_HOST_MEMORY;
            goto out;
        }
        struct loader_layer_properties* newly_added_layer = &settings_layers->list[settings_layers->count - 1];
        newly_added_layer->settings_control_value = layer_config->control;
        // If the manifest file found has a name that differs from the one in the settings, remove this layer from consideration
        bool should_remove = false;
        if (strncmp(newly_added_layer->info.layerName, layer_config->name, VK_MAX_EXTENSION_NAME_SIZE) != 0) {
            should_remove = true;
            loader_remove_layer_in_list(inst, settings_layers, settings_layers->count - 1);
        }
        // Make sure the layer isn't already in the list
        for (uint32_t j = 0; settings_layers->count > 0 && j < settings_layers->count - 1; j++) {
            if (0 ==
                strncmp(settings_layers->list[j].info.layerName, newly_added_layer->info.layerName, VK_MAX_EXTENSION_NAME_SIZE)) {
                if (0 == (newly_added_layer->type_flags & VK_LAYER_TYPE_FLAG_META_LAYER) &&
                    strcmp(settings_layers->list[j].lib_name, newly_added_layer->lib_name) == 0) {
                    should_remove = true;
                    break;
                }
            }
        }
        if (should_remove) {
            loader_remove_layer_in_list(inst, settings_layers, settings_layers->count - 1);
        }
    }

out:
    release_current_settings_lock(inst);
    return res;
}

// Check if layers has an element with the same name.
// If layer_property is a regular layer, check if the lib_path is the same.
// If layer_property is a meta layer, just use the layerName
bool check_if_layer_is_in_list(struct loader_layer_list* layer_list, struct loader_layer_properties* layer_property) {
    // If the layer is a meta layer, just check against the name
    for (uint32_t i = 0; i < layer_list->count; i++) {
        if (0 == strncmp(layer_list->list[i].info.layerName, layer_property->info.layerName, VK_MAX_EXTENSION_NAME_SIZE)) {
            if (0 == (layer_property->type_flags & VK_LAYER_TYPE_FLAG_META_LAYER) &&
                strcmp(layer_list->list[i].lib_name, layer_property->lib_name) == 0) {
                return true;
            }
        }
    }
    return false;
}

VkResult combine_settings_layers_with_regular_layers(const struct loader_instance* inst, struct loader_layer_list* settings_layers,
                                                     struct loader_layer_list* regular_layers,
                                                     struct loader_layer_list* output_layers) {
    VkResult res = VK_SUCCESS;
    bool has_unordered_layer_location = false;
    uint32_t unordered_layer_location_index = 0;
    // Location to put layers that aren't known to the settings file
    // Find it here so we dont have to pass in a loader_settings struct
    for (uint32_t i = 0; i < settings_layers->count; i++) {
        if (settings_layers->list[i].settings_control_value == LOADER_SETTINGS_LAYER_UNORDERED_LAYER_LOCATION) {
            has_unordered_layer_location = true;
            unordered_layer_location_index = i;
            break;
        }
    }

    if (settings_layers->count == 0 && regular_layers->count == 0) {
        // No layers to combine
        goto out;
    } else if (settings_layers->count == 0) {
        // No settings layers - just copy regular to output_layers - memset regular layers to prevent double frees
        *output_layers = *regular_layers;
        memset(regular_layers, 0, sizeof(struct loader_layer_list));
        goto out;
    } else if (regular_layers->count == 0 || !has_unordered_layer_location) {
        // No regular layers or has_unordered_layer_location is false - just copy settings to output_layers -
        // memset settings layers to prevent double frees
        *output_layers = *settings_layers;
        memset(settings_layers, 0, sizeof(struct loader_layer_list));
        goto out;
    }

    res = loader_init_generic_list(inst, (struct loader_generic_list*)output_layers,
                                   (settings_layers->count + regular_layers->count) * sizeof(struct loader_layer_properties));
    if (VK_SUCCESS != res) {
        goto out;
    }

    // Insert the settings layers into output_layers up to unordered_layer_index
    for (uint32_t i = 0; i < unordered_layer_location_index; i++) {
        if (!check_if_layer_is_in_list(output_layers, &settings_layers->list[i])) {
            res = loader_append_layer_property(inst, output_layers, &settings_layers->list[i]);
            if (VK_SUCCESS != res) {
                goto out;
            }
        }
    }

    for (uint32_t i = 0; i < regular_layers->count; i++) {
        // Check if its already been put in the output_layers list as well as the remaining settings_layers
        bool regular_layer_is_ordered = check_if_layer_is_in_list(output_layers, &regular_layers->list[i]) ||
                                        check_if_layer_is_in_list(settings_layers, &regular_layers->list[i]);
        // If it isn't found, add it
        if (!regular_layer_is_ordered) {
            res = loader_append_layer_property(inst, output_layers, &regular_layers->list[i]);
            if (VK_SUCCESS != res) {
                goto out;
            }
        } else {
            // layer is already ordered and can be safely freed
            loader_free_layer_properties(inst, &regular_layers->list[i]);
        }
    }

    // Insert the rest of the settings layers into combined_layers from  unordered_layer_index to the end
    // start at one after the unordered_layer_index
    for (uint32_t i = unordered_layer_location_index + 1; i < settings_layers->count; i++) {
        res = loader_append_layer_property(inst, output_layers, &settings_layers->list[i]);
        if (VK_SUCCESS != res) {
            goto out;
        }
    }

out:
    if (res != VK_SUCCESS) {
        loader_delete_layer_list_and_properties(inst, output_layers);
    }

    return res;
}

VkResult enable_correct_layers_from_settings(const struct loader_instance* inst, const struct loader_envvar_filter* enable_filter,
                                             const struct loader_envvar_disable_layers_filter* disable_filter, uint32_t name_count,
                                             const char* const* names, const struct loader_layer_list* instance_layers,
                                             struct loader_pointer_layer_list* target_layer_list,
                                             struct loader_pointer_layer_list* activated_layer_list) {
    VkResult res = VK_SUCCESS;
    char* vk_instance_layers_env = loader_getenv(ENABLED_LAYERS_ENV, inst);
    if (vk_instance_layers_env != NULL) {
        loader_log(inst, VULKAN_LOADER_WARN_BIT | VULKAN_LOADER_LAYER_BIT, 0, "env var \'%s\' defined and adding layers \"%s\"",
                   ENABLED_LAYERS_ENV, names);
    }
    for (uint32_t i = 0; i < instance_layers->count; i++) {
        bool enable_layer = false;
        struct loader_layer_properties* props = &instance_layers->list[i];

        // Do not enable the layer if the settings have it set as off
        if (props->settings_control_value == LOADER_SETTINGS_LAYER_CONTROL_OFF) {
            continue;
        }
        // Force enable it based on settings
        if (props->settings_control_value == LOADER_SETTINGS_LAYER_CONTROL_ON) {
            enable_layer = true;
        }

        // Check if disable filter needs to skip the layer
        if (NULL != disable_filter &&
            (disable_filter->disable_all || disable_filter->disable_all_implicit ||
             check_name_matches_filter_environment_var(props->info.layerName, &disable_filter->additional_filters))) {
            continue;
        }
        // Check the enable filter
        if (!enable_layer && NULL != enable_filter &&
            check_name_matches_filter_environment_var(props->info.layerName, enable_filter)) {
            enable_layer = true;
        }

        if (!enable_layer && vk_instance_layers_env) {
            size_t vk_instance_layers_env_len = strlen(vk_instance_layers_env) + 1;
            char* name = loader_stack_alloc(vk_instance_layers_env_len);
            if (name != NULL) {
                strncpy(name, vk_instance_layers_env, vk_instance_layers_env_len);
                // First look for the old-fashion layers forced on with VK_INSTANCE_LAYERS
                while (name && *name) {
                    char* next = loader_get_next_path(name);

                    if (strlen(name) > 0) {
                        if (0 == strcmp(name, props->info.layerName)) {
                            enable_layer = true;
                            break;
                        }
                        name = next;
                    }
                }
            }
        }

        // Check if it should be enabled by the application
        if (!enable_layer) {
            for (uint32_t j = 0; j < name_count; j++) {
                if (strcmp(props->info.layerName, names[j]) == 0) {
                    enable_layer = true;
                    break;
                }
            }
        }

        // Check if its an implicit layers and thus enabled by default
        if (!enable_layer && (0 == (props->type_flags & VK_LAYER_TYPE_FLAG_EXPLICIT_LAYER)) &&
            loader_implicit_layer_is_enabled(inst, enable_filter, disable_filter, props)) {
            enable_layer = true;
        }

        if (enable_layer) {
            // Check if the layer is a meta layer reuse the existing function to add the meta layer
            if (props->type_flags & VK_LAYER_TYPE_FLAG_META_LAYER) {
                res = loader_add_meta_layer(inst, enable_filter, disable_filter, props, target_layer_list, activated_layer_list,
                                            instance_layers, NULL);
                if (res == VK_ERROR_OUT_OF_HOST_MEMORY) goto out;
            } else {
                res = loader_add_layer_properties_to_list(inst, target_layer_list, props);
                if (res != VK_SUCCESS) {
                    goto out;
                }
                res = loader_add_layer_properties_to_list(inst, activated_layer_list, props);
                if (res != VK_SUCCESS) {
                    goto out;
                }
            }
        }
    }
out:
    return res;
}
