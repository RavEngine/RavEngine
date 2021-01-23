# Libnyquist


Platform | Build Status |
-------- | ------------ |
Microsoft VS2017 x64 | [![Build status](https://ci.appveyor.com/api/projects/status/2xeuyuxy618ndf4r?svg=true)](https://ci.appveyor.com/project/ddiakopoulos/libnyquist) |
Clang (OSX) & GCC (Linux) | [![Build Status](https://travis-ci.org/ddiakopoulos/libnyquist.svg?branch=master)](https://travis-ci.org/ddiakopoulos/libnyquist) |

Libnyquist is a small C++11 library for reading sampled audio data from disk or memory. It is intended to be used an audio loading frontend for games, audio sequencers, music players, and more.

The library does not include patent or license encumbered formats (such as AAC). For portability, libnyquist does not link against platform-specific APIs like Windows Media Foundation or CoreAudio, and instead bundles the source code of reference decoders as an implementation detail.

Libnyquist is meant to be statically linked, which is not the case with other popular libraries like libsndfile (which is licensed under the LGPL). Furthermore, the library is not concerned with supporting very rare encodings (for instance, A-law PCM or the SND format). 
 
While untested, there are no technical conditions that preclude compilation on other platforms with C++11 support (Android NDK r10e+, Linux, iOS, etc).

## Format Support

Regardless of input bit depth, the library produces a channel-interleaved float vector, normalized between [-1.0,+1.0]. At present, the library does not provide comprehensive resampling functionality. 

* Wave (+ IMA-ADPCM encoding)
* MP3
* Ogg Vorbis
* Ogg Opus
* FLAC
* WavPack
* Musepack

## Known Issues & Bugs
* See the Github issue tracker. 

## License
This library is released under the simplied 2 clause BSD license. All included dependencies have been released under identical or similarly permissive licenses.
