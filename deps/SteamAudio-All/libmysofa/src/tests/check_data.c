#include "../hrtf/tools.h"
#include "tests.h"
#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static void check_data(char *filename) {
  struct MYSOFA_HRTF *hrtf;
  int err;

  FILE *fd = fopen(filename, "rb");
  ;
  if (!fd) {
    CU_FAIL_FATAL("Error opening file.");
    return;
  }

  if (fseek(fd, 0, SEEK_END) < 0) {
    CU_FAIL_FATAL("Error seeking file.");
    fclose(fd);
    return;
  }
  long size = ftell(fd);
  if (size < 0) {
    CU_FAIL_FATAL("Error geting file size.");
    fclose(fd);
    return;
  }
  if (fseek(fd, 0, SEEK_SET) < 0) {
    CU_FAIL_FATAL("Error seeking file.");
    fclose(fd);
    return;
  }

  char *data = malloc(size);
  int n = fread(data, 1, size, fd);
  if (n != size) {
    CU_FAIL_FATAL("Error loading file.");
    free(data);
    fclose(fd);
    return;
  }
  fclose(fd);

  hrtf = mysofa_load_data(data, size, &err);

  if (!hrtf) {
    CU_FAIL_FATAL("Error reading data.");
    free(data);
    return;
  }

  err = mysofa_check(hrtf);
  CU_ASSERT(err == MYSOFA_OK);

  mysofa_tocartesian(hrtf);

  mysofa_free(hrtf);

  free(data);
}

void test_check_data() { check_data("tests/Pulse.sofa"); }
