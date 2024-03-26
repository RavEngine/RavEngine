#include "../hrtf/mysofa.h"
#include "../hrtf/tools.h"
#include "json.h"
#include "tests.h"
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

/* #define VDEBUG */

void test_easy_open() {
  struct MYSOFA_EASY *easy;
  int err = 0;
  int filterlength;

  easy = mysofa_open("tests/MIT_KEMAR_normal_pinna.old.sofa", 8000.,
                     &filterlength, &err);
  if (!easy) {
    CU_FAIL_FATAL("Error reading file.");
    return;
  }

  mysofa_close(easy);
}

void test_easy() {
  struct MYSOFA_EASY *easy;
  int err = 0;
  int filterlength;
  int filters = 0;
  float theta, r;
  float *coordinates;
  float *ir;
  float *delays;
  int count = 0;
  FILE *file;
  float c[3];
  float l1, l2;
  float sdiff1, sdiff2, diff1, diff2;

  easy = mysofa_open("tests/tester.sofa", 48000, &filterlength, &err);
  if (!easy) {
    CU_FAIL_FATAL("Error reading file.");
    return;
  }
  mysofa_minphase(easy->hrtf, 0.01);

  for (filters = 0; filters < easy->hrtf->M; filters++) {
    c[0] = easy->hrtf->SourcePosition.values[filters * 3];
    c[1] = easy->hrtf->SourcePosition.values[filters * 3 + 1];
    c[2] = easy->hrtf->SourcePosition.values[filters * 3 + 2];
    mysofa_c2s(c);

#ifdef VDEBUG
    printf("in %d %f %f %f # %f %f %f\n", filters, c[0], c[1], c[2],
           easy->hrtf->SourcePosition.values[filters * 3],
           easy->hrtf->SourcePosition.values[filters * 3 + 1],
           easy->hrtf->SourcePosition.values[filters * 3 + 2]);
#endif
    c[0] = fmod(round(c[0] + 360), 360);
    c[1] = fmod(round(c[1] + 361), 360);
    l1 = round(easy->hrtf->DataDelay.values[filters * 2] * 48000 * 2);
    l2 = round(easy->hrtf->DataDelay.values[filters * 2 + 1] * 48000 * 2);

#ifdef VDEBUG
    /*   		printf("compare %d %f %f %f %f
     * %f\n",filters,c[0],c[1],c[2],l1,l2);
     */
#endif
    CU_ASSERT(
        !((fabs(c[0] - l1) > 2 || fabs(c[1] - l2) > 2) && !fequals(l2, 90)));
  }

  filters = 0;
  for (theta = -90.; theta <= 90.; theta += 5.) {
    r = round(cos(theta * M_PI / 180.) * 120.);
    if (r == 0.)
      r = 1;
    filters += r;
  }
#ifdef VDEBUG
  printf("Filters %d\n", filters);
#endif

  coordinates = malloc(filters * sizeof(float) * 3);
  ir = malloc(filters * easy->hrtf->N * sizeof(float) * 2);
  delays = malloc(filters * 2 * sizeof(float));

  sdiff1 = sdiff2 = 0;
  err = 0;
  for (theta = -90.; theta <= 90.; theta += 5.) {
    int r = round(cos(theta * M_PI / 180.) * 120.);
    int phi;
    if (r == 0)
      r = 1;
    for (phi = 0; phi < r; phi++) {
      coordinates[count * 3 + 0] = phi * (360. / r);
      coordinates[count * 3 + 1] = theta;
      coordinates[count * 3 + 2] = 1;
      mysofa_s2c(coordinates + count * 3);
#ifdef VDEBUG
      printf("req %f %f %d %f %f %f\n", phi * (360. / r), theta, count,
             coordinates[count * 3 + 0], coordinates[count * 3 + 1],
             coordinates[count * 3 + 2]);
#endif
      mysofa_getfilter_float(
          easy, coordinates[count * 3 + 0], coordinates[count * 3 + 1],
          coordinates[count * 3 + 2], ir + 2 * count * easy->hrtf->N,
          ir + (2 * count + 1) * easy->hrtf->N, &delays[2 * count],
          &delays[2 * count + 1]);
      diff1 = fabs(phi * (360. / r) - delays[2 * count] * 48000 * 2);
      diff2 = fabs(fmod(theta + 360, 360) - delays[2 * count + 1] * 48000 * 2);
      if (diff1 > 5 || diff2 > 5)
        err++;
      else {
        sdiff1 += diff1;
        sdiff2 += diff2;
      }

#ifdef VDEBUG
      printf("diff %f %f\t", diff1, diff2);
      printf("delays %f %f %f %f\n", phi * (360. / r), fmod(theta + 360, 360),
             delays[2 * count] * 48000 * 2, delays[2 * count + 1] * 48000 * 2);
#endif
      count++;
    }
  }
  err = err * 100. / count;
  sdiff1 = sdiff1 / (count - err);
  sdiff2 = sdiff2 / (count - err);

#ifdef VDEBUG
  printf("errors %f%% diffs %f %f\n", err * 100. / count,
         sdiff1 / (count - err), sdiff2 / (count - err));
#endif
  CU_ASSERT(err < 31.7);
  CU_ASSERT(sdiff1 < 1.67);
  CU_ASSERT(sdiff2 < 1.43);

  free(easy->hrtf->DataDelay.values);
  free(easy->hrtf->DataIR.values);
  free(easy->hrtf->SourcePosition.values);
  easy->hrtf->DataDelay.elements = filters * 2;
  easy->hrtf->DataDelay.values = delays;
  easy->hrtf->DataIR.elements = filters * 2 * easy->hrtf->N;
  easy->hrtf->DataIR.values = ir;
  easy->hrtf->SourcePosition.elements = filters * 3;
  easy->hrtf->SourcePosition.values = coordinates;
  easy->hrtf->M = filters;

  file = fopen("/tmp/easy.tmp.json", "w");
  CU_ASSERT(file != NULL);
  printJson(file, easy->hrtf, 0);
  fclose(file);

  mysofa_close(easy);
}

void test_easy_nonorm() {
  struct MYSOFA_EASY *easy;
  int err = 0;
  int filterlength;
  int filters = 0;
  float diff1, diff2;

  /* Test raw opening (no norm) and MxR delay .sofa */
  easy = mysofa_open_no_norm("tests/tester2.sofa", 48000., &filterlength, &err);
  if (!easy) {
    CU_FAIL_FATAL("Error reading file.");
    return;
  }

  err = 0;

  for (filters = 0; filters < easy->hrtf->M; filters++) {
    // see tester2.sofa file creation in tester2.m file
    diff1 = filters - easy->hrtf->DataDelay.values[easy->hrtf->R * filters];
    diff2 = filters + easy->hrtf->DataDelay.values[easy->hrtf->R * filters + 1];
    if (diff1 > 0.1 || diff2 > 0.1)
      err++;
#ifdef VDEBUG
    printf("delays: %f %f \t",
           easy->hrtf->DataDelay.values[easy->hrtf->R * filters],
           easy->hrtf->DataDelay.values[easy->hrtf->R * filters + 1]);
    printf("diff: %f %f %i\n", diff1, diff2, err);
#endif
  }
  CU_ASSERT(err < 1);

  mysofa_close(easy);
}
