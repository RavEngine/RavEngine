#!/bin/sh
set -e

if ! test -d "src"; then
    echo "Please run this in the project root directory."
    exit 1
fi

for src in src/sfizz/OpcodeCleanup.re; do
  re2c -o "${src%.re}.cpp" "$src"
done
