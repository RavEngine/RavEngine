
file(GLOB SRC "${CMAKE_CURRENT_LIST_DIR}/../tools/importlib/*.cpp" "${CMAKE_CURRENT_LIST_DIR}/../tools/importlib/*.hpp" "${CMAKE_CURRENT_LIST_DIR}/../tools/importlib/*.h")
add_library(rve_importlib ${SRC})
target_compile_features(rve_importlib PRIVATE cxx_std_20)
target_link_libraries(rve_importlib PRIVATE assimp fmt glm)
target_include_directories(rve_importlib 
	PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../include/RavEngine" "${CMAKE_CURRENT_LIST_DIR}/../tools/importlib/RavEngine" "${CMAKE_CURRENT_LIST_DIR}/../deps/parallel-hashmap/parallel_hashmap"
	PUBLIC "${CMAKE_CURRENT_LIST_DIR}/../tools/importlib"
)

macro(make_importer dir)

	file(GLOB SRC "${CMAKE_CURRENT_LIST_DIR}/../tools/${dir}/*.cpp" "${CMAKE_CURRENT_LIST_DIR}/../tools/${dir}/*.hpp" "${CMAKE_CURRENT_LIST_DIR}/../tools/${dir}/*.h")
	add_executable(${dir} ${SRC})
	target_compile_features(${dir} PRIVATE cxx_std_20)
	target_include_directories(${dir} PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../include/RavEngine" "${CMAKE_CURRENT_LIST_DIR}/../deps/parallel-hashmap/parallel_hashmap")

	set_target_properties(${dir} PROPERTIES XCODE_GENERATE_SCHEME ON)
	target_compile_definitions(${dir} PRIVATE CXXOPTS_NO_RTTI=1)

	set_target_properties(${dir}
		PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${dir}"
		LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${dir}"
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${dir}"
	)
endmacro()

make_importer(rvemc)
target_link_libraries(rvemc PRIVATE assimp cxxopts simdjson fmt glm rve_importlib)

make_importer(rveskc)
target_link_libraries(rveskc PRIVATE assimp cxxopts simdjson fmt glm rve_importlib)

make_importer(rveac)
target_link_libraries(rveac PRIVATE assimp cxxopts simdjson fmt glm rve_importlib)

