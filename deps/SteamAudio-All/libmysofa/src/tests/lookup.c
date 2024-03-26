#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#ifdef __GNUC__
#define HAVE_GETTIMEOFDAY
#endif

#ifdef HAVE_GETTIMEOFDAY
#include <sys/time.h>
#endif

#include "../hrtf/tools.h"
#include "tests.h"

void test_lookup() {
  struct MYSOFA_HRTF *hrtf = NULL;
  int err = 0;
#ifdef HAVE_GETTIMEOFDAY
  struct timeval r1, r2;
#endif
  float duration1 = 0.f, duration2 = 0.f;
  float find[3];
  int j;
  struct MYSOFA_LOOKUP *lookup;

  hrtf = mysofa_load("tests/Pulse.sofa", &err);

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

#ifdef VDEBUG
  printf("r  %f %f\n", lookup->radius_min, lookup->radius_max);
#endif

  for (j = 0; j < 10000; j++) {
    int index = -1;
    float dmin = FLT_MAX;
    int i, lk;

    find[0] = rand() * (4. / RAND_MAX) - 2;
    find[1] = rand() * (4. / RAND_MAX) - 2;
    find[2] = rand() * (4. / RAND_MAX) - 2;

#ifdef HAVE_GETTIMEOFDAY
    gettimeofday(&r1, NULL);
#endif
    lk = mysofa_lookup(lookup, find);
#ifdef HAVE_GETTIMEOFDAY
    gettimeofday(&r2, NULL);
    duration1 = (r2.tv_sec - r1.tv_sec) * 1000000. + (r2.tv_usec - r1.tv_usec);

    gettimeofday(&r1, NULL);
#endif
    for (i = 0; i < hrtf->M; i++) {
      float r = distance(find, hrtf->SourcePosition.values + i * hrtf->C);
      if (r < dmin) {
        dmin = r;
        index = i;
      }
    }
#ifdef HAVE_GETTIMEOFDAY
    gettimeofday(&r2, NULL);
    duration2 = (r2.tv_sec - r1.tv_sec) * 1000000. + (r2.tv_usec - r1.tv_usec);
#endif

    CU_ASSERT(lk == index);
    if (lk != index) {
      printf("O(log n) %f %f %f -> %d %f \t\t", find[0], find[1], find[2], lk,
             distance(find, hrtf->SourcePosition.values + lk * hrtf->C));
      printf("O(n): %f %f %f -> %d %f\t%f%%\n", find[0], find[1], find[2],
             index, dmin, duration1 / duration2 * 100);
    }
  }

  mysofa_lookup_free(lookup);
  mysofa_free(hrtf);
}
