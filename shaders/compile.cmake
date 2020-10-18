
# default shader
declare_shader("default" "${CMAKE_CURRENT_LIST_DIR}/vs_cubes.sc" "${CMAKE_CURRENT_LIST_DIR}/fs_cubes.sc" "${CMAKE_CURRENT_LIST_DIR}/varying.def.sc")

# debug wireframes shader
declare_shader("debug" "${CMAKE_CURRENT_LIST_DIR}/vs_debug.sc" "${CMAKE_CURRENT_LIST_DIR}/fs_debug.sc" "${CMAKE_CURRENT_LIST_DIR}/debug_varying.def.sc")


