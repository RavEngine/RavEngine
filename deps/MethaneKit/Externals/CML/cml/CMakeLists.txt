set(TARGET CML)

add_library(${TARGET} INTERFACE)

target_include_directories(${TARGET} INTERFACE
    ..
)