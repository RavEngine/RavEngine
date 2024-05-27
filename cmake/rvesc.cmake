

file(GLOB RVESC_SRC "${CMAKE_CURRENT_LIST_DIR}/../tools/rvesc/*.cpp" "${CMAKE_CURRENT_LIST_DIR}/../tools/rvesc/*.hpp" "${CMAKE_CURRENT_LIST_DIR}/../tools/rvesc/*.h")
add_executable(rvesc ${RVESC_SRC})
target_compile_features(rvesc PUBLIC cxx_std_20)

target_link_libraries(rvesc PUBLIC librglc cxxopts simdjson)