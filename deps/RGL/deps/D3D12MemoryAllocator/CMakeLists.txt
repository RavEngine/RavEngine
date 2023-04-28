project(D3D12MemoryAllocator)

add_library(${PROJECT_NAME} 
	src/D3D12MemAlloc.cpp 
	src/D3D12MemAlloc.natvis 
	include/D3D12MemAlloc.h
)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_14)
target_include_directories(${PROJECT_NAME} PUBLIC "include/")