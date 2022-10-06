// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "st_audiofile.h"
#include <stdio.h>
#include <inttypes.h>

int main(int argc, char* argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Please indicate a sound file.\n");
        return 1;
    }

    const char* filename = argv[1];

    st_audio_file* af = st_open_file(filename);
    if (!af) {
        fprintf(stderr, "Could not open the sound file.\n");
        return 1;
    }

    printf("File name   : %s\n", filename);
    printf("File type   : %s\n", st_get_type_string(af));
    printf("Channels    : %u\n", st_get_channels(af));
    printf("Sample rate : %f\n", st_get_sample_rate(af));
    printf("Frames      : %" PRIu64 "\n", st_get_frame_count(af));

    return 0;
}
