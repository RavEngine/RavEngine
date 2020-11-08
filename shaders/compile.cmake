
# default shader
declare_shader("default" "${CMAKE_CURRENT_LIST_DIR}/vs_default.glsl" "${CMAKE_CURRENT_LIST_DIR}/fs_default.glsl" "${CMAKE_CURRENT_LIST_DIR}/default_varying.def.hlsl")

# debug wireframes shader
declare_shader("debug" "${CMAKE_CURRENT_LIST_DIR}/vs_debug.glsl" "${CMAKE_CURRENT_LIST_DIR}/fs_debug.glsl" "${CMAKE_CURRENT_LIST_DIR}/debug_varying.def.hlsl")


