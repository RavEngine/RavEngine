# NVIDIA PhysX SDK for Windows

## Location of Binaries:

* SDK DLLs/LIBs: bin/
* PhysX Device DLLs: bin/

## Required packages to generate projects:

* Microsoft Windows 10 (or later)
* CMake, minimum version 3.21
* Python, minimum version 3.5
* NVIDIA CUDA Toolkit 12.8 (Not required for CPU only builds)

Compilers and C++ Standard:
  * Microsoft Visual Studio 2017, 2019, 2022 (64 bit)
  * Tested with C++14 standard

## PhysX GPU Acceleration:

* Running GPU-accelerated simulations requires a CUDA toolkit 12.8 compatible display driver. The corresponding driver version can be found [here](https://docs.nvidia.com/cuda/cuda-toolkit-release-notes/index.html#cuda-major-component-versions__table-cuda-toolkit-driver-versions).
* Volta, CUDA ARCH 7.0 GPU or higher

## Generating solutions for Visual Studio:

* Solutions are generated through a script in the physx root directory: generate_projects.bat
* The generate_projects.bat script expects a preset name as a parameter; if a parameter is not provided, it lists the available presets.
* The generated solutions are in the folder compiler/"preset name".
