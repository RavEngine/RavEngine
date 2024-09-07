
file(GLOB SRC "${CMAKE_CURRENT_LIST_DIR}/../tools/rveskc/*.cpp" "${CMAKE_CURRENT_LIST_DIR}/../tools/rveskc/*.hpp" "${CMAKE_CURRENT_LIST_DIR}/../tools/rveskc/*.h")
add_executable(rveskc ${SRC})
target_compile_features(rveskc PRIVATE cxx_std_20)
target_link_libraries(rveskc PRIVATE assimp cxxopts simdjson fmt glm rve_importlib)
target_include_directories(rveskc PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../include/RavEngine" "${CMAKE_CURRENT_LIST_DIR}/../deps/parallel-hashmap/parallel_hashmap")

set_target_properties(rveskc PROPERTIES XCODE_GENERATE_SCHEME ON)