# libsodium-cmake

[![Travis build status](https://travis-ci.com/robinlinden/libsodium-cmake.svg?branch=master)](https://travis-ci.com/robinlinden/libsodium-cmake)
[![Windows CI](https://github.com/robinlinden/libsodium-cmake/workflows/Windows%20CI/badge.svg)](https://github.com/robinlinden/libsodium-cmake/actions?query=workflow%3A%22Windows+CI%22)

## Description

CMakeWrapper for [libsodium](https://github.com/jedisct1/libsodium), the modern, portable, easy to use crypto library.

This wrapper is written with a few goals in mind:
1. It should be easy to build
1. It should be obvious that libsodium's source code hasn't been touched
1. It should be easy to integrate into projects

## Building

Clone!

`git clone --recursive https://github.com/robinlinden/libsodium-cmake.git`

Build!

```sh
mkdir build && cd build
cmake ..
make
make test
```

## Using in your project

```cmake
include(FetchContent)

FetchContent_Declare(Sodium
    GIT_REPOSITORY https://github.com/robinlinden/libsodium-cmake.git
)
set(SODIUM_DISABLE_TESTS ON)
FetchContent_MakeAvailable(Sodium)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        sodium
)
```
