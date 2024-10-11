
project(meshoptimizer)

file(GLOB SRC "src/*.cpp" "src/*.h")
add_library(${PROJECT_NAME} ${SRC})
target_include_directories(${PROJECT_NAME} 
    PRIVATE src/
    PUBLIC src/
)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)