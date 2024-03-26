#include "../hrtf/mysofa.h"
#include "../hrtf/tools.h"
#include "tests.h"
#include <float.h>
#include <math.h>
#include <stdio.h>

void test_interpolate() {
  struct MYSOFA_HRTF *hrtf = NULL;
  int err = 0;
  int i;
  float *fir;
  float delays[2];
  float *res;
  int neighborhood[6] = {-1, -1, -1, -1, -1, -1};
  float c[3];

  hrtf = mysofa_load("tests/MIT_KEMAR_normal_pinna.old.sofa", &err);

  if (!hrtf) {
    CU_FAIL_FATAL("Error reading file.");
    return;
  }

  mysofa_tocartesian(hrtf);

  fir = malloc(hrtf->N * hrtf->R * sizeof(float));

  res = mysofa_interpolate(hrtf, hrtf->SourcePosition.values, 0, neighborhood,
                           fir, delays);
  CU_ASSERT(res == hrtf->DataIR.values);
  CU_ASSERT(delays[0] == 0);
  CU_ASSERT(delays[1] == 0);

  c[0] = (hrtf->SourcePosition.values[0] + hrtf->SourcePosition.values[3]) / 2;
  c[1] = (hrtf->SourcePosition.values[1] + hrtf->SourcePosition.values[4]) / 2;
  c[2] = (hrtf->SourcePosition.values[2] + hrtf->SourcePosition.values[5]) / 2;
  neighborhood[0] = 1;

  res = mysofa_interpolate(hrtf, c, 0, neighborhood, fir, delays);
  CU_ASSERT(res == fir);
  CU_ASSERT(delays[0] == 0);
  CU_ASSERT(delays[1] == 0);

  for (i = 0; i < hrtf->N * hrtf->R; i++) {
#ifdef VDEBUG
    printf("%f %f %f\n", res[i], hrtf->DataIR.values[i],
           hrtf->DataIR.values[i + hrtf->N * hrtf->R]);
#endif
    CU_ASSERT(fequals(res[i], (hrtf->DataIR.values[i] +
                               hrtf->DataIR.values[i + hrtf->N * hrtf->R]) /
                                  2));
  }

  /* TODO add some tests... */

  mysofa_free(hrtf);
  free(fir);
}
