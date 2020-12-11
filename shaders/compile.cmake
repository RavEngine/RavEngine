
# debug wireframes shader
declare_shader("debug" "${CMAKE_CURRENT_LIST_DIR}/vs_debug.glsl" "${CMAKE_CURRENT_LIST_DIR}/fs_debug.glsl" "${CMAKE_CURRENT_LIST_DIR}/debug_varying.def.hlsl")

# deferred geometry material
declare_shader("pbrmaterial" "${CMAKE_CURRENT_LIST_DIR}/vs_deferred_geo.glsl" "${CMAKE_CURRENT_LIST_DIR}/fs_deferred_geo.glsl" "${CMAKE_CURRENT_LIST_DIR}/deferred_varying.def.hlsl")

declare_shader("deferred_blit" "${CMAKE_CURRENT_LIST_DIR}/deferred_blit.vsh" "${CMAKE_CURRENT_LIST_DIR}/deferred_blit.fsh" "${CMAKE_CURRENT_LIST_DIR}/blit_varying.def.hlsl")

# lighting shaders
declare_shader("pointlightvolume" "${CMAKE_CURRENT_LIST_DIR}/pointlight.vsh" "${CMAKE_CURRENT_LIST_DIR}/pointlight.fsh" "${CMAKE_CURRENT_LIST_DIR}/pointlight_varying.def.hlsl")
declare_shader("ambientlightvolume" "${CMAKE_CURRENT_LIST_DIR}/ambientlight.vsh" "${CMAKE_CURRENT_LIST_DIR}/ambientlight.fsh" "${CMAKE_CURRENT_LIST_DIR}/ambientlight_varying.def.hlsl")
