# Copyright 2023 The Khronos Group Inc.
# Copyright 2023 Valve Corporation
# Copyright 2023 LunarG, Inc.
#
# SPDX-License-Identifier: Apache-2.0
target_include_directories(VulkanLayerSettings PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_sources(VulkanLayerSettings PRIVATE
	vulkan/layer/vk_layer_settings.h
	vulkan/layer/vk_layer_settings.hpp
)

set(CMAKE_FOLDER "${CMAKE_FOLDER}/VulkanUtilityHeaders")

add_library(VulkanUtilityHeaders INTERFACE)
add_library(Vulkan::UtilityHeaders ALIAS VulkanUtilityHeaders)

# https://cmake.org/cmake/help/latest/release/3.19.html#other
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.19")
	target_sources(VulkanUtilityHeaders PRIVATE
		vulkan/utility/vk_dispatch_table.h
		vulkan/vk_enum_string_helper.h
		vulkan/utility/vk_format_utils.h
		vulkan/utility/vk_struct_helper.hpp
	)
endif()

target_link_Libraries(VulkanUtilityHeaders INTERFACE Vulkan::Headers)

target_include_directories(VulkanUtilityHeaders INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

