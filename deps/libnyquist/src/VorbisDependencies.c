/*
Copyright (c) 2019, Dimitri Diakopoulos All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#if (_MSC_VER)
    #pragma warning (push)
    #pragma warning (disable: 181 111 4267 4996 4244 4701 4702 4133 4100 4127 4206 4312 4505 4365 4005 4013 4334)
#endif

#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wconversion"
    #pragma clang diagnostic ignored "-Wshadow"
    #pragma clang diagnostic ignored "-Wdeprecated-register"
#endif

#include "libvorbis/include/vorbis/vorbisenc.h"
#include "libvorbis/include/vorbis/codec.h"
#include "libvorbis/include/vorbis/vorbisfile.h"

#include "libogg/src/bitwise.c"
#include "libogg/src/framing.c"

#include "libvorbis/src/analysis.c"
#include "libvorbis/src/bitrate.c"
#include "libvorbis/src/block.c"
#include "libvorbis/src/codebook.c"
#include "libvorbis/src/envelope.c"
#include "libvorbis/src/floor0.c"
#include "libvorbis/src/floor1.c"
#include "libvorbis/src/info.c"
#include "libvorbis/src/lpc.c"
#include "libvorbis/src/lsp.c"
#include "libvorbis/src/mapping0.c"
#include "libvorbis/src/psy.c"
#include "libvorbis/src/registry.c"
#include "libvorbis/src/res0.c"
#include "libvorbis/src/sharedbook.c"
#include "libvorbis/src/smallft.c"
#include "libvorbis/src/synthesis.c"
#include "libvorbis/src/vorbisenc.c"
#include "libvorbis/src/vorbisfile.c"
#include "libvorbis/src/window.c"
#include "libvorbis/src/mdct.c"

#ifdef __clang__
    #pragma clang diagnostic pop
#endif

#if (_MSC_VER)
    #pragma warning (pop)
#endif
