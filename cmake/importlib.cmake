
file(GLOB SRC "${CMAKE_CURRENT_LIST_DIR}/../tools/importlib/*.cpp" "${CMAKE_CURRENT_LIST_DIR}/../tools/importlib/*.hpp" "${CMAKE_CURRENT_LIST_DIR}/../tools/importlib/*.h")
add_library(rve_importlib ${SRC})
target_compile_features(rve_importlib PRIVATE cxx_std_20)
target_link_libraries(rve_importlib PRIVATE assimp fmt glm)
target_include_directories(rve_importlib PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../include/RavEngine" "${CMAKE_CURRENT_LIST_DIR}/../deps/parallel-hashmap/parallel_hashmap")

