
file(GLOB SRC "${CMAKE_CURRENT_LIST_DIR}/../tools/rvemc/*.cpp" "${CMAKE_CURRENT_LIST_DIR}/../tools/rvemc/*.hpp" "${CMAKE_CURRENT_LIST_DIR}/../tools/rvemc/*.h")
add_executable(rvemc ${SRC})
target_compile_features(rvemc PRIVATE cxx_std_20)
target_link_libraries(rvemc PRIVATE assimp cxxopts simdjson fmt glm)

set_target_properties(rvemc PROPERTIES XCODE_GENERATE_SCHEME ON)