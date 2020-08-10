set(TARGET PerlinNoise)

add_library(${TARGET} STATIC

    Include/noise1234.h
    Include/sdnoise1234.h
    Include/simplexnoise1234.h

    Sources/noise1234.cpp
    Sources/sdnoise1234.c
    Sources/simplexnoise1234.cpp
)

target_include_directories(${TARGET}
    PRIVATE
        Sources
    PUBLIC
        Include
)

set_target_properties(${TARGET}
    PROPERTIES
    FOLDER Externals
)