
#include "pffft_double.h"

#include <stdio.h>
#include <stdlib.h>


void c_forward_complex_double(const int transformLen)
{
  printf("running %s()\n", __FUNCTION__);

  /* first check - might be skipped */
  if (transformLen < pffftd_min_fft_size(PFFFT_COMPLEX))
  {
    fprintf(stderr, "Error: minimum FFT transformation length is %d\n", pffftd_min_fft_size(PFFFT_COMPLEX));
    return;
  }

  /* instantiate FFT and prepare transformation for length N */
  PFFFTD_Setup *ffts = pffftd_new_setup(transformLen, PFFFT_COMPLEX);

  /* one more check */
  if (!ffts)
  {
    fprintf(stderr,
            "Error: transformation length %d is not decomposable into small prime factors. "
            "Next valid transform size is: %d ; next power of 2 is: %d\n",
            transformLen,
            pffftd_nearest_transform_size(transformLen, PFFFT_COMPLEX, 1),
            pffftd_next_power_of_two(transformLen) );
    return;
  }

  /* allocate aligned vectors for input X and output Y */
  double *X = (double*)pffftd_aligned_malloc(transformLen * 2 * sizeof(double));  /* complex: re/im interleaved */
  double *Y = (double*)pffftd_aligned_malloc(transformLen * 2 * sizeof(double));  /* complex: re/im interleaved */
  double *W = (double*)pffftd_aligned_malloc(transformLen * 2 * sizeof(double));

  /* prepare some input data */
  for (int k = 0; k < 2 * transformLen; k += 4)
  {
    X[k] = k / 2;  /* real */
    X[k+1] = (k / 2) & 1;  /* imag */

    X[k+2] = -1 - k / 2;  /* real */
    X[k+3] = (k / 2) & 1;  /* imag */
  }

  /* do the forward transform; write complex spectrum result into Y */
  pffftd_transform_ordered(ffts, X, Y, W, PFFFT_FORWARD);

  /* print spectral output */
  printf("output should be complex spectrum with %d complex bins\n", transformLen);
  for (int k = 0; k < 2 * transformLen; k += 2)
    printf("Y[%d] = %f + i * %f\n", k/2, Y[k], Y[k+1]);

  pffftd_aligned_free(W);
  pffftd_aligned_free(Y);
  pffftd_aligned_free(X);
  pffftd_destroy_setup(ffts);
}


int main(int argc, char *argv[])
{
  int N = (1 < argc) ? atoi(argv[1]) : 16;
  c_forward_complex_double(N);
  return 0;
}
