#!/bin/sh
script_dir="$(dirname "$0")"
echo "DON'T USE: libc++ is not instrumented with msan..."
cmake $@ -D CMAKE_C_COMPILER=clang -D CMAKE_CXX_COMPILER=clang++ -D CMAKE_CXX_FLAGS="-fsanitize=memory -fsanitize=undefined -fsanitize-memory-track-origins" -D CMAKE_BUILD_TYPE=Release -S "$script_dir/.." -B .
