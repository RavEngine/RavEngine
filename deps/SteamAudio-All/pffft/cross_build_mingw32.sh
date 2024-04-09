#!/bin/bash

# requires debian/ubuntu packages: zip gcc-mingw-w64

if [ -z "$1" ]; then
  echo "usage: $0 <zip-post> <any other cmake options>"
  exit 1
fi

ZIP_POST="$1"
shift

CROSS="i686-w64-mingw32"
WN="w32"
TOOLCHAIN="mingw-w32-i686.cmake"

rm -rf build_${WN}_${ZIP_POST}
echo -e "\n\n********************************************************"
echo "start build of pffft_${WN}_${ZIP_POST}"
mkdir build_${WN}_${ZIP_POST} && \
cmake -S . -B build_${WN}_${ZIP_POST} \
  -DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN} \
  -DCMAKE_INSTALL_PREFIX=pffft_bin-${WN}_${ZIP_POST} \
  "$@" && \
cmake --build build_${WN}_${ZIP_POST}
