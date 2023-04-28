#!/bin/bash
cd "$(dirname "$0")"

# folders
find . -name .git -type d -exec rm -rf {} \;

# submodule files
find . -name .git -exec rm {} \;
