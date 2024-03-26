#include "../hrtf/mysofa.h"
#include "../hrtf/tools.h"
#include "tests.h"
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

void test_neighbors() {
  struct MYSOFA_HRTF *hrtf = NULL;
  int err = 0;
  struct MYSOFA_LOOKUP *lookup;
  struct MYSOFA_NEIGHBORHOOD *neighborhood;
  int i, j, *res;
  float c[3], C[3];
#ifdef VDEBUG
  const char *dir = "RLUDFB";
#endif

  hrtf = mysofa_load("tests/MIT_KEMAR_normal_pinna.old.sofa", &err);
  if (!hrtf) {
    CU_FAIL_FATAL("Error reading file.");
    return;
  }

  mysofa_tocartesian(hrtf);

  lookup = mysofa_lookup_init(hrtf);
  if (lookup == NULL) {
    CU_FAIL("Error sorting HRTF.");
    mysofa_free(hrtf);
    return;
  }

  neighborhood = mysofa_neighborhood_init(hrtf, lookup);

  if (neighborhood == NULL) {
    CU_FAIL("Error getting neighborhood.");
    mysofa_lookup_free(lookup);
    mysofa_free(hrtf);
    return;
  }

  for (i = 0; i < hrtf->M; i++) {
    memcpy(c, hrtf->SourcePosition.values + i * hrtf->C,
           sizeof(float) * hrtf->C);
    mysofa_c2s(c);
#ifdef VDEBUG
    printf("%4.0f %4.0f %5.2f\t", c[0], c[1], c[2]);
#endif
    res = mysofa_neighborhood(neighborhood, i);
    CU_ASSERT(res != NULL);
    for (j = 0; j < 6; j++) {
      if (res[j] >= 0) {
        memcpy(C, hrtf->SourcePosition.values + res[j] * hrtf->C,
               sizeof(float) * hrtf->C);
        mysofa_c2s(C);
#ifdef VDEBUG
        printf("\t%c %4.0f %4.0f %5.2f", dir[j], C[0], C[1], C[2]);
#endif
        switch (j) {
        case 0:
          if (C[0] < c[0])
            C[0] += 360;
          CU_ASSERT_FATAL(c[0] < C[0] && c[0] + 45 > C[0]);
          break;
        case 1:
          if (C[0] > c[0])
            C[0] -= 360;
          CU_ASSERT_FATAL(c[0] > C[0] && c[0] - 45 < C[0]);
          break;
        case 2:
          CU_ASSERT_FATAL(c[1] < C[1] || fequals(c[1], 90.f));
          break;
        case 3:
          CU_ASSERT_FATAL(c[1] > C[1]);
          break;
        }
      }
    }
#ifdef VDEBUG
    printf("\n");
#endif
  }

  mysofa_neighborhood_free(neighborhood);
  mysofa_lookup_free(lookup);
  mysofa_free(hrtf);
}
