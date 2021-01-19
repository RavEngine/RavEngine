
# debug wireframes shader
declare_shader("debug" "${CMAKE_CURRENT_LIST_DIR}/debug.vsh" "${CMAKE_CURRENT_LIST_DIR}/debug.fsh" "${CMAKE_CURRENT_LIST_DIR}/debug_varying.def.hlsl")

# deferred geometry material
declare_shader("pbrmaterial" "${CMAKE_CURRENT_LIST_DIR}/deferred_geo.vsh" "${CMAKE_CURRENT_LIST_DIR}/deferred_geo.fsh" "${CMAKE_CURRENT_LIST_DIR}/deferred_varying.def.hlsl")

declare_shader("deferred_blit" "${CMAKE_CURRENT_LIST_DIR}/deferred_blit.vsh" "${CMAKE_CURRENT_LIST_DIR}/deferred_blit.fsh" "${CMAKE_CURRENT_LIST_DIR}/deferred_blit_varying.def.hlsl")

# lighting shaders
declare_shader("pointlightvolume" "${CMAKE_CURRENT_LIST_DIR}/pointlight.vsh" "${CMAKE_CURRENT_LIST_DIR}/pointlight.fsh" "${CMAKE_CURRENT_LIST_DIR}/pointlight_varying.def.hlsl")
declare_shader("ambientlightvolume" "${CMAKE_CURRENT_LIST_DIR}/ambientlight.vsh" "${CMAKE_CURRENT_LIST_DIR}/ambientlight.fsh" "${CMAKE_CURRENT_LIST_DIR}/ambientlight_varying.def.hlsl")
declare_shader("directionallightvolume" "${CMAKE_CURRENT_LIST_DIR}/directionallight.vsh" "${CMAKE_CURRENT_LIST_DIR}/directionallight.fsh" "${CMAKE_CURRENT_LIST_DIR}/directionallight_varying.def.hlsl")
declare_shader("spotlightvolume" "${CMAKE_CURRENT_LIST_DIR}/spotlight.vsh" "${CMAKE_CURRENT_LIST_DIR}/spotlight.fsh" "${CMAKE_CURRENT_LIST_DIR}/spotlight_varying.def.hlsl")

# GUI rendering shaders
declare_shader("guishader" "${CMAKE_CURRENT_LIST_DIR}/gui.vsh" "${CMAKE_CURRENT_LIST_DIR}/gui.fsh" "${CMAKE_CURRENT_LIST_DIR}/gui.def.hlsl")

