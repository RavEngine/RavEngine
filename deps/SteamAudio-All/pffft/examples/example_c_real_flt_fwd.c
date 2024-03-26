
#include "pffft.h"

#include <stdio.h>
#include <stdlib.h>


void c_forward_real_float(const int transformLen)
{
  printf("running %s()\n", __FUNCTION__);

  /* first check - might be skipped */
  if (transformLen < pffft_min_fft_size(PFFFT_REAL))
  {
    fprintf(stderr, "Error: minimum FFT transformation length is %d\n", pffft_min_fft_size(PFFFT_REAL));
    return;
  }

  /* instantiate FFT and prepare transformation for length N */
  PFFFT_Setup *ffts = pffft_new_setup(transformLen, PFFFT_REAL);

  /* one more check */
  if (!ffts)
  {
    fprintf(stderr,
            "Error: transformation length %d is not decomposable into small prime factors. "
            "Next valid transform size is: %d ; next power of 2 is: %d\n",
            transformLen,
            pffft_nearest_transform_size(transformLen, PFFFT_REAL, 1),
            pffft_next_power_of_two(transformLen) );
    return;
  }

  /* allocate aligned vectors for input X and output Y */
  float *X = (float*)pffft_aligned_malloc(transformLen * sizeof(float));
  float *Y = (float*)pffft_aligned_malloc(transformLen * sizeof(float));  /* complex: re/im interleaved */
  float *W = (float*)pffft_aligned_malloc(transformLen * sizeof(float));

  /* prepare some input data */
  for (int k = 0; k < transformLen; k += 2)
  {
    X[k] = k;
    X[k+1] = -1-k;
  }

  /* do the forward transform; write complex spectrum result into Y */
  pffft_transform_ordered(ffts, X, Y, W, PFFFT_FORWARD);

  /* print spectral output */
  printf("output should be complex spectrum with %d complex bins\n", transformLen /2);
  for (int k = 0; k < transformLen; k += 2)
    printf("Y[%d] = %f + i * %f\n", k/2, Y[k], Y[k+1]);

  pffft_aligned_free(W);
  pffft_aligned_free(Y);
  pffft_aligned_free(X);
  pffft_destroy_setup(ffts);
}


int main(int argc, char *argv[])
{
  int N = (1 < argc) ? atoi(argv[1]) : 32;
  c_forward_real_float(N);
  return 0;
}
