
project(MiniVulkanSDK)

set(BUILD_TESTS OFF)
add_subdirectory(Vulkan-Headers EXCLUDE_FROM_ALL)

set(LOADER_CODEGEN OFF)
add_subdirectory(Vulkan-Loader EXCLUDE_FROM_ALL)

if (NOT TARGET SPIRV-Headers)
	add_subdirectory(SPIRV-Headers EXCLUDE_FROM_ALL)
endif()

set(VVL_CODEGEN OFF)
set("SPIRV-Headers_DIR" "${CMAKE_CURRENT_LIST_DIR}")
set("SPIRV-Tools-opt_DIR" "${CMAKE_CURRENT_LIST_DIR}")
set("SPIRV-Tools_DIR" "${CMAKE_CURRENT_LIST_DIR}")
set("VulkanHeaders_DIR" "${CMAKE_CURRENT_LIST_DIR}")
set(CMAKE_ASM_MASM_COMPILER_WORKS OFF)
set(USE_MASM OFF)
set(USE_GAS OFF)
set(ASSEMBLER_WORKS OFF)
set(USE_ASSEMBLY_FALLBACK ON)
add_subdirectory(Vulkan-ValidationLayers EXCLUDE_FROM_ALL)

set(Vulkan_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/Vulkan-Headers/include" "${CMAKE_CURRENT_LIST_DIR}/Vulkan-ValidationLayers/layers/vulkan/generated" PARENT_SCOPE)


add_library(${PROJECT_NAME} INTERFACE)
target_link_libraries(${PROJECT_NAME} INTERFACE Vulkan::Headers vulkan vvl)
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})

INSTALL(TARGETS vulkan EXPORT vulkan DESTINATION "${CMAKE_INSTALL_PREFIX}")

