set(TARGET Catch)

add_library(${TARGET} INTERFACE)

target_sources(${TARGET} INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/catch2/catch.hpp
)

target_include_directories(${TARGET} INTERFACE
    .
)