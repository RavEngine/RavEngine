
# default shader
declare_shader("default" "${CMAKE_CURRENT_LIST_DIR}/vs_default.sc" "${CMAKE_CURRENT_LIST_DIR}/fs_default.sc" "${CMAKE_CURRENT_LIST_DIR}/default_varying.def.sc")

# debug wireframes shader
declare_shader("debug" "${CMAKE_CURRENT_LIST_DIR}/vs_debug.sc" "${CMAKE_CURRENT_LIST_DIR}/fs_debug.sc" "${CMAKE_CURRENT_LIST_DIR}/debug_varying.def.sc")


