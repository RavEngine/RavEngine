// License: BSD 2 Clause
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef labsound_mixing_h
#define labsound_mixing_h

#include <stdint.h>

namespace lab
{

enum class ChannelInterpretation
{
    Speakers,
    Discrete,
};

enum class Channel : int
{
    First = 0,
    Left = 0,
    Right = 1,
    Center = 2,  // center and mono are the same
    Mono = 2,
    LFE = 3,
    SurroundLeft = 4,
    SurroundRight = 5,
    BackLeft = 6,
    BackRight = 7
};

namespace Channels
{
    enum : uint32_t
    {
        Mono = 1,
        Stereo = 2,
        Quad = 4,
        Surround_5_0 = 5,
        Surround_5_1 = 6,
        Surround_7_1 = 8
    };
};

enum class ChannelCountMode
{
    Max,
    ClampedMax,
    Explicit,
    End
};

}  // end namespace lab

#endif  // end labsound_mixing_h
