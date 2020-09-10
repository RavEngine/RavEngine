#!/bin/bash

# Compile
mkdir -p build
cd build || exit 1
cmake .. || exit 1
make -j4 || exit 1
cd ..

# Unit test
(cd build && valgrind --track-origins=yes ./Zipper-test) || exit 1
