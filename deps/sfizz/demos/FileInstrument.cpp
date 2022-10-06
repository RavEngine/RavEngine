// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "sfizz/FileMetadata.h"
#include "absl/strings/string_view.h"
#include <sndfile.hh>
#include <cstdio>

static const char* modeString(int mode, const char* valueFallback = nullptr)
{
    switch (mode) {
    case sfz::LoopNone:
        return "none";
    case sfz::LoopForward:
        return "forward";
    case sfz::LoopBackward:
        return "backward";
    case sfz::LoopAlternating:
        return "alternating";
    default:
        return valueFallback;
    }
}

template <class Instrument>
static void printInstrument(const Instrument& ins)
{
    printf("Gain: %d\n", ins.gain);
    printf("Base note: %d\n", ins.basenote);
    printf("Detune: %d\n", ins.detune);
    printf("Velocity: %d:%d\n", ins.velocity_lo, ins.velocity_hi);
    printf("Key: %d:%d\n", ins.key_lo, ins.key_hi);
    printf("Loop count: %d\n", ins.loop_count);

    for (unsigned i = 0, n = ins.loop_count; i < n; ++i) {
        printf("\nLoop %d:\n", i + 1);
        printf("\tMode: %s\n", modeString(ins.loops[i].mode, "(unknown)"));
        printf("\tStart: %u\n", ins.loops[i].start);
        printf("\tEnd: %u\n", ins.loops[i].end);
        printf("\tCount: %u\n", ins.loops[i].count);
    }
}

static void usage(const char* argv0)
{
    fprintf(
        stderr,
        "Usage: %s [-s|-f] <sound-file>\n"
        "    -s: extract the instrument using libsndfile\n"
        "    -f: extract the instrument using RIFF metadata\n",
        argv0);
}

enum FileMethod {
    kMethodSndfile,
    kMethodRiff,
};

int main(int argc, char *argv[])
{
    fs::path path;
    FileMethod method = kMethodSndfile;

    if (argc == 2) {
        path = argv[1];
    }
    else if (argc == 3) {
        absl::string_view flag = argv[1];
        if (flag == "-s")
            method = kMethodSndfile;
        else if (flag == "-f")
            method = kMethodRiff;
        else {
            usage(argv[0]);
            return 1;
        }
        path = argv[2];
    }
    else {
        usage(argv[0]);
        return 1;
    }

    if (method == kMethodRiff) {
        sfz::FileMetadataReader reader;
        if (!reader.open(path)) {
            fprintf(stderr, "Cannot open file\n");
            return 1;
        }
        sfz::InstrumentInfo ins {};
        if (!reader.extractInstrument(ins)) {
            fprintf(stderr, "Cannot get instrument\n");
            return 1;
        }
        printInstrument(ins);
    }
    else {
        SndfileHandle sndFile(path);
        if (!sndFile) {
            fprintf(stderr, "Cannot open file\n");
            return 1;
        }
        SF_INSTRUMENT ins {};
        if (sndFile.command(SFC_GET_INSTRUMENT, &ins, sizeof(ins)) != 1) {
            fprintf(stderr, "Cannot get instrument\n");
            return 1;
        }
        printInstrument(ins);
    }

    return 0;
}
