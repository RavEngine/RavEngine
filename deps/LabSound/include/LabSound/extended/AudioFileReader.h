// SPDX-License-Identifier: BSD-2-Clause
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef AudioFileReader_H
#define AudioFileReader_H

#include "LabSound/core/AudioBus.h"
#include "LabSound/extended/AudioContextLock.h"

#include <memory>
#include <stdint.h>
#include <string>
#include <vector>

namespace lab
{
// Performs filesystem i/o to decode a file (using libnyquist)
std::shared_ptr<AudioBus> MakeBusFromFile(const char * filePath, bool mixToMono);
std::shared_ptr<AudioBus> MakeBusFromFile(const std::string & path, bool mixToMono);

// Performs filesystem i/o to decode a file (using libnyquist)
// Resamples the data to the specified rate. A typical application is to resample an audio
// file to the device rate; perhaps from 44.1 to 48 khz. Resampling at load time saves
// run time overhead.
std::shared_ptr<AudioBus> MakeBusFromFile(const char * filePath, bool mixToMono, float targetSampleRate);
std::shared_ptr<AudioBus> MakeBusFromFile(const std::string & path, bool mixToMono, float targetSampleRate);


// Loads and decodes a raw binary memory chunk making use of magic numbers to determine filetype.
std::shared_ptr<AudioBus> MakeBusFromMemory(const std::vector<uint8_t> & buffer, bool mixToMono);

// Loads and decodes a raw binary memory chunk where the file extension (mp3, wav, ogg, etc) is aleady known.
std::shared_ptr<AudioBus> MakeBusFromMemory(const std::vector<uint8_t> & buffer, const std::string & extension, bool mixToMono);
}

#endif
