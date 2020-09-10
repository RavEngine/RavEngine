#!/bin/bash

export PATH="/mingw64/bin:/usr/local/bin:/usr/bin:/bin:/c/WINDOWS/system32:/c/WINDOWS:/c/WINDOWS/System32/Wbem:/c/WINDOWS/System32/WindowsPowerShell/v1.0"
bf=$(cygpath ${APPVEYOR_BUILD_FOLDER})
cd "$bf" || (echo "Cannot go to directory $bf"; return 1)

# Compile
mkdir -p build
cd build || exit 1
cmake .. || exit 1
make -j4 || exit 1
cd ..

# Unit test
(cd build && ./Zipper-test) || exit 1
