/*

 Copyright 2016 Christian Hoene, Symonics GmbH

 */

#include "../hrtf/mysofa.h"
#include "../hrtf/tools.h"
#include "json.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct {
  int err;
  const char *name;
} errorNames[] = {
    {MYSOFA_OK, "OK"},
    {MYSOFA_INVALID_FORMAT, "invalid format"},
    {MYSOFA_INTERNAL_ERROR, "internal error"},
    {MYSOFA_UNSUPPORTED_FORMAT, "unsupported format"},
    {MYSOFA_NO_MEMORY, "no memory"},
    {MYSOFA_READ_ERROR, "read error"},
    {MYSOFA_INVALID_ATTRIBUTES, "invalid attributes"},
    {MYSOFA_INVALID_DIMENSIONS, "invalid dimensions"},
    {MYSOFA_INVALID_DIMENSION_LIST, "invalid dimension list"},
    {MYSOFA_INVALID_COORDINATE_TYPE, "invalid coordinate type"},
    {MYSOFA_ONLY_EMITTER_WITH_ECI_SUPPORTED,
     "only emitters with ECI dimensions supported"},
    {MYSOFA_ONLY_DELAYS_WITH_IR_OR_MR_SUPPORTED,
     "only delqys with IR or MR dimensions supported"},
    {MYSOFA_ONLY_THE_SAME_SAMPLING_RATE_SUPPORTED,
     "only the same sampling rate supported"},
    {MYSOFA_RECEIVERS_WITH_RCI_SUPPORTED, "receivers with RCI supported"},
    {MYSOFA_RECEIVERS_WITH_CARTESIAN_SUPPORTED,
     "receivers with cartesian coordinate system supported"},
    {MYSOFA_INVALID_RECEIVER_POSITIONS, "invalid receiver positions"},
    {MYSOFA_ONLY_SOURCES_WITH_MC_SUPPORTED,
     "only sources with MC dimensions supported"},
    {0, NULL}};

static const char *error2string(int error) {
  int i = 0;
  while (errorNames[i].name != NULL) {
    if (errorNames[i].err == error)
      return errorNames[i].name;
    i++;
  }
  return strerror(error);
}

static void usage(const char *exe) {
  fprintf(stderr,
          "Usage: %s [-s] [-c] [-o <outputfilename>] <FILE.SOFA>\n converts a "
          "sofa file to json "
          "output.\nAdd -s to sanitize the json output from netcdf fields.\n"
          "Add -c to check for a correct AES69-2015 format using libmysofa.\n",
          exe);
  exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
  struct MYSOFA_HRTF *hrtf = NULL;
  int err = 0;
  int sanitize = 0;
  int check = 0;
  char *filename, *output = NULL;
  int opt;
  FILE *out = stdout;

  while ((opt = getopt(argc, argv, "cso:")) != -1) {
    switch (opt) {
    case 's':
      sanitize = 1;
      break;
    case 'c':
      check = 1;
      break;
    case 'o':
      output = optarg;
      break;
    default: /* '?' */
      usage(argv[0]);
    }
  }

  if (optind + 1 != argc)
    usage(argv[0]);

  filename = argv[optind];

  if (output) {
    out = fopen(output, "w");
    if (out == NULL) {
      fprintf(stderr, "Cannot open output file %s.\n", output);
      return 1;
    }
  }

  hrtf = mysofa_load(filename, &err);
  if (!hrtf) {
    fprintf(stderr, "Error reading file %s. Error code: %d:%s\n", filename, err,
            error2string(err));
    if (output)
      fclose(out);
    return err;
  }

  printJson(out, hrtf, sanitize);

  if (output)
    fclose(out);

  mysofa_free(hrtf);

  if (check) {
    struct MYSOFA_EASY *hrtf2 = NULL;
    int filter_length;
    hrtf2 = mysofa_open(filename, 48000, &filter_length, &err);

    if (err != MYSOFA_OK) {
      fprintf(stderr, "Error checking file %s. Error code: %d:%s\n", filename,
              err, error2string(err));
      return err;
    }
    mysofa_close(hrtf2);
  }

  return 0;
}
