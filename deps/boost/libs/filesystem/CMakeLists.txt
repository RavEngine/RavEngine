cmake_minimum_required(VERSION 3.5)
project(boost_filesystem)

add_library(boost_filesystem
    src/codecvt_error_category.cpp
    src/exception.cpp
    src/operations.cpp
    src/directory.cpp
    src/path.cpp
    src/path_traits.cpp
    src/portability.cpp
    src/unique_path.cpp
    src/utf8_codecvt_facet.cpp
    src/windows_file_codecvt.cpp
)

add_library(Boost::filesystem ALIAS boost_filesystem)

target_include_directories(boost_filesystem PUBLIC ../../)
target_include_directories(boost_filesystem PRIVATE src)

