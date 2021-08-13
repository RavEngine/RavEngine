cmake_minimum_required(VERSION 3.13)
project("r8brain")

file(GLOB SRC 
	"r8brain-free-src/*.h" 
	"r8brain-free-src/pffft_double/*.h" 
	"r8brain-free-src/other/*.h" 
	"r8brain-free-src/dll/*.h"
)

add_library(${PROJECT_NAME} ${SRC} "r8brain-free-src/r8bbase.cpp" "r8brain-free-src/pffft_double/pffft_double.c")
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_include_directories(${PROJECT_NAME} PUBLIC "r8brain-free-src")