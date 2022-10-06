#!/bin/sh
script_dir="$(dirname "$0")"
cmake $@ -D CMAKE_C_COMPILER=clang -D CMAKE_CXX_COMPILER=clang++ -D CMAKE_BUILD_TYPE=Debug -D SFIZZ_TESTS=ON -D SFIZZ_BENCHMARKS=ON -S "$script_dir/.." -B .
