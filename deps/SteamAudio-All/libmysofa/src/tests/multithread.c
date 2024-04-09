#include "../hrtf/mysofa.h"
#include "../hrtf/tools.h"
#include "json.h"
#include <float.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

/* #define VDEBUG */

struct MYSOFA_EASY *easy;
int filterlength;

void *thread(void *arg) {
  int i;
  int err = 0;
  int filters = 0;
  float theta, r;
  float *coordinates;
  float *ir;
  float *delays;
  int count;
  float sdiff1, sdiff2, diff1, diff2;

  for (i = 0; i < 10; i++) {

    filters = 0;
    for (theta = -90.; theta <= 90.; theta += 5.) {
      r = round(cos(theta * M_PI / 180.) * 120.);
      if (r == 0.)
        r = 1;
      filters += r;
    }

    sdiff1 = sdiff2 = 0;
    err = 0;
    count = 0;
    coordinates = malloc(filters * sizeof(float) * 3);
    ir = malloc(filters * easy->hrtf->N * sizeof(float) * 2);
    delays = malloc(filters * 2 * sizeof(float));

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
        diff2 =
            fabs(fmod(theta + 360, 360) - delays[2 * count + 1] * 48000 * 2);
        if (diff1 > 5 || diff2 > 5)
          err++;
        else {
          sdiff1 += diff1;
          sdiff2 += diff2;
        }

#ifdef VDEBUG
        printf("diff %f %f\t", diff1, diff2);
        printf("delays %f %f %f %f\n", phi * (360. / r), fmod(theta + 360, 360),
               delays[2 * count] * 48000 * 2,
               delays[2 * count + 1] * 48000 * 2);
#endif
        count++;
      }
    }
    if (count != 0)
      err = err * 100. / count;
    if (count != err) {
      sdiff1 = sdiff1 / (count - err);
      sdiff2 = sdiff2 / (count - err);
    }
#ifdef VDEBUG
    if (count != 0 && count != err)
      printf("errors %f%% diffs %f %f\n", err * 100. / count,
             sdiff1 / (count - err), sdiff2 / (count - err));
#endif
    if (!(err < 31.7 && sdiff1 < 1.67 && sdiff2 < 1.43)) {
      abort();
    }

    free(coordinates);
    free(ir);
    free(delays);
  }

  return NULL;
}

void *timer(void *arg) {
  sleep(600);
  abort();
}

#define THREADS (20)

int main() {
  int t;
  int err = 0;

  easy = mysofa_open("tests/tester.sofa", 48000, &filterlength, &err);
  if (!easy) {
    abort();
  }
  mysofa_minphase(easy->hrtf, 0.01);

  // start multithread
  pthread_t threads[THREADS];

  for (t = 0; t < THREADS; t++) {
    if (pthread_create(threads + t, NULL, thread, NULL))
      abort();
  }

  // start watchdog
  pthread_t watchdog;
  if (pthread_create(&watchdog, NULL, timer, NULL))
    abort();

  // end multithread
  for (t = 0; t < THREADS; t++) {
    if (pthread_join(threads[t], NULL))
      abort();
  }

  pthread_cancel(watchdog);

  mysofa_close(easy);

  printf("ALL GOOD\n");
  return 0;
}
