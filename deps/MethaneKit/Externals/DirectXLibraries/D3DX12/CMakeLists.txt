set(TARGET D3DX12)

add_library(${TARGET} INTERFACE)

target_sources(${TARGET} INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/d3dx12.h
)

target_include_directories(${TARGET}
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
