#include "../hrtf/mysofa.h"
#include "../hrtf/tools.h"
#include "tests.h"
#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

void test_loudness() {
  struct MYSOFA_HRTF *hrtf;
  int err = 0;
  float factor;

  hrtf = mysofa_load("tests/Pulse.sofa", &err);
  if (!hrtf) {
    CU_FAIL_FATAL("Error reading file.");
    return;
  }

  factor = mysofa_loudness(hrtf);
#ifdef VDEBUG
  printf("loudness of Pulse.sofa %f\n", factor);
#endif
  CU_ASSERT(fequals(factor, 1));
  mysofa_free(hrtf);

  hrtf = mysofa_load("tests/MIT_KEMAR_normal_pinna.old.sofa", &err);
  if (!hrtf) {
    CU_FAIL_FATAL("Error reading file.");
  }

  factor = mysofa_loudness(hrtf);
#ifdef VDEBUG
  printf("loudness of MIT_KEMAR_normal_pinna.old.sofa %f\n", factor);
#endif
  CU_ASSERT(fequals(factor, 1.116589));

  factor = mysofa_loudness(hrtf);
#ifdef VDEBUG
  printf("loudness of MIT_KEMAR_normal_pinna.old.sofa after normalization %f\n",
         factor);
#endif
  CU_ASSERT(fequals(factor, 1.));

  mysofa_free(hrtf);
}
