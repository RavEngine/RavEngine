set(TARGET STB)

add_library(${TARGET} INTERFACE)

target_sources(${TARGET} INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/stb_image.h"
)

target_include_directories(${TARGET} INTERFACE
    .
)
