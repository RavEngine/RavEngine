---
layout: default
---

# Compiling on Win32

You can use [CMake](https://cmake.org/) to generate Visual Studio project. The
configuration process is described [here](https://cmake.org/runningcmake/).

The libsamplerate library itself does not require any dependencies, but if you
want to build examples and tests, you will need the [libsndfile](https://github.com/libsndfile/libsndfile)
and [FFTW](http://www.fftw.org/) libraries. The easiest way to install them is
to use a package manager, such as [Vcpkg](https://github.com/microsoft/vcpkg). Their README.md contains detailed
installation instructions for supported platforms. Libsamplerate requires the
`libsndfile` and `fftw3` packages.
