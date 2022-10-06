// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "sfizz/FileMetadata.h"
#include "absl/strings/string_view.h"
#include <cstdio>

static void printWavetable(const sfz::WavetableInfo& wt)
{
    printf("Table size: %u\n", wt.tableSize);
    printf("Cross-table interpolation: %d\n", wt.crossTableInterpolation);
    printf("One-shot: %d\n", wt.oneShot);
}

static void usage(const char* argv0)
{
    fprintf(
        stderr,
        "Usage: %s <sound-file>\n",
        argv0);
}

int main(int argc, char *argv[])
{
    fs::path path;

    if (argc == 2)
        path = argv[1];
    else {
        usage(argv[0]);
        return 1;
    }

    sfz::WavetableInfo wt {};

    sfz::FileMetadataReader reader;
    if (!reader.open(path)) {
        fprintf(stderr, "Cannot open file\n");
        return 1;
    }
    if (!reader.extractWavetableInfo(wt)) {
        fprintf(stderr, "Cannot get wavetable info\n");
        return 1;
    }

    printWavetable(wt);

    return 0;
}
