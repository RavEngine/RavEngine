#the RavEngine Resource Packer

cmake_minimum_required(VERSION 3.17)
project(rscpack)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable("${PROJECT_NAME}" "main.cpp")
target_include_directories("${PROJECT_NAME}" PRIVATE "../miniz-cpp/" "../cxxopts/include")