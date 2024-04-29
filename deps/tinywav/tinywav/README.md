# TinyWav

![](https://img.shields.io/github/license/mhroth/tinywav)
![](https://img.shields.io/badge/C-C99_std-blue.svg?style=flat&logo=c)
![](https://img.shields.io/badge/dependencies-<stdio.h>-blue)
![](https://img.shields.io/badge/external_dependencies-none-blue)

A minimal C library for reading and writing (32-bit float or 16-bit int) WAV audio files. Designed for maximum portability.

* TinyWav takes and provides audio samples in configurable channel formats (interleaved, split, inline). WAV files always store samples in interleaved format.
* TinyWav is minimal: it can only read/write RIFF WAV files with sample format `float32` or `int16`.
* TinyWav does not allocate any memory on the heap. It uses `alloca` internally, which allocates on the stack. In practice, this restricts the block size to "reasonable" values, so watch out for stack overflows.

**CI/CD**: To guarantee portability, TinyWav is built and tested on several platforms, compilers & architectures:

![](https://img.shields.io/badge/macos-Clang_14-teal)
![](https://img.shields.io/badge/linux-GCC_7-teal)
![](https://img.shields.io/badge/linux-GCC_11-teal)
![](https://img.shields.io/badge/windows-MSVC_VS2022_(x64)-teal)
![](https://img.shields.io/badge/windows-MSVC_VS2022_(arm64)_[build_only]-teal)
[![Build & Test](https://github.com/mhroth/tinywav/actions/workflows/workflow.yml/badge.svg?branch=master)](https://github.com/mhroth/tinywav/actions/workflows/workflow.yml)

## Code Example
### Writing

```c
#include "tinywav.h"

#define NUM_CHANNELS 2
#define SAMPLE_RATE 48000
#define BLOCK_SIZE 480

TinyWav tw;
tinywav_open_write(&tw,
    NUM_CHANNELS,
    SAMPLE_RATE,
    TW_FLOAT32, // the output samples will be 32-bit floats. TW_INT16 is also supported
    TW_INLINE,  // the samples to be written will be assumed to be inlined in a single buffer.
                // Other options include TW_INTERLEAVED and TW_SPLIT
    "path/to/output.wav" // the output path
);

for (int i = 0; i < 100; i++) {
  // NOTE: samples are always expected in float32 format, 
  // regardless of file sample format

  float samples[BLOCK_SIZE * NUM_CHANNELS];
  tinywav_write_f(&tw, samples, BLOCK_SIZE);
  
  ...
}

tinywav_close_write(&tw);
```

### Reading
```c
#include "tinywav.h"

#define NUM_CHANNELS 2
#define SAMPLE_RATE 48000
#define BLOCK_SIZE 480

TinyWav tw;
tinywav_open_read(&tw, 
	"path/to/input.wav",
	TW_SPLIT // the samples will be delivered by the read function in split format
);

for (int i = 0; i < 100; i++) {
  // samples are always provided in float32 format, 
  // regardless of file sample format
  float samples[NUM_CHANNELS * BLOCK_SIZE];
  
  // Split buffer requires pointers to channel buffers
  float* samplePtrs[NUM_CHANNELS];
  for (int j = 0; j < NUM_CHANNELS; ++j) {
    samplePtrs[j] = samples + j*BLOCK_SIZE;
  }

  tinywav_read_f(&tw, samplePtrs, BLOCK_SIZE);
  
  ...
}

tinywav_close_read(&tw);
```

## Running the Tests

TinyWav relies on Cmake for its test builds. The automated tests are based on the [Catch2](https://github.com/catchorg/Catch2) C++ test framework.

To run the tests, run one of the generator scripts in the `test/scripts` directory to generate a Xcode/VS project - or directly use Cmake in the commandline:

```bash
mkdir -p test/scripts/build
cd test/scripts/build
cmake ../../..
make
ctest # or run ./TinyWavTest directly
```

> **NOTE**: The Git repository uses Git LFS to store some reference wav files as binaries. Make sure you have Git LFS installed/enabled/pulled before running the tests.

## License
TinyWav is published under the [ISC license](http://opensource.org/licenses/ISC). Please see the `LICENSE` file included in this repository, also reproduced below. In short, you are welcome to use this code for any purpose, including commercial and closed-source use.

```
Copyright (c) 2015-2023, Martin Roth <mhroth@gmail.com>

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
```
