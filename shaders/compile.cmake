
# default shader
declare_shader("default" "${CMAKE_CURRENT_LIST_DIR}/vs_default.glsl" "${CMAKE_CURRENT_LIST_DIR}/fs_default.glsl" "${CMAKE_CURRENT_LIST_DIR}/default_varying.def.hlsl")

# debug wireframes shader
declare_shader("debug" "${CMAKE_CURRENT_LIST_DIR}/vs_debug.glsl" "${CMAKE_CURRENT_LIST_DIR}/fs_debug.glsl" "${CMAKE_CURRENT_LIST_DIR}/debug_varying.def.hlsl")

# deferred geometry material
declare_shader("deferredGeometry" "${CMAKE_CURRENT_LIST_DIR}/vs_deferred_geo.glsl" "${CMAKE_CURRENT_LIST_DIR}/fs_deferred_geo.glsl" "${CMAKE_CURRENT_LIST_DIR}/deferred_varying.def.hlsl")
