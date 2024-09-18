file(GLOB RVESC_TMP 
	"${CMAKE_CURRENT_LIST_DIR}/../tools/rvesc/*.fsh"
	"${CMAKE_CURRENT_LIST_DIR}/../tools/rvesc/*.vsh" 
	"${CMAKE_CURRENT_LIST_DIR}/../tools/rvesc/*.csh" 
	"${CMAKE_CURRENT_LIST_DIR}/../tools/rvesc/*.glsl"
)
cmrc_add_resource_library(rvesc_resources ${RVESC_TMP} WHENCE "${CMAKE_CURRENT_LIST_DIR}/../tools/rvesc/")

file(GLOB RVESC_SRC "${CMAKE_CURRENT_LIST_DIR}/../tools/rvesc/*.cpp" "${CMAKE_CURRENT_LIST_DIR}/../tools/rvesc/*.hpp" "${CMAKE_CURRENT_LIST_DIR}/../tools/rvesc/*.h")
add_executable(rvesc ${RVESC_SRC})
target_compile_features(rvesc PUBLIC cxx_std_20)

target_link_libraries(rvesc PUBLIC librglc cxxopts simdjson rvesc_resources fmt)
target_compile_definitions(rvesc PRIVATE CXXOPTS_NO_RTTI=1)


