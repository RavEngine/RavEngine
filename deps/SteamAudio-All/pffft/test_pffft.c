/*
  Copyright (c) 2013 Julien Pommier.

  Small test for PFFFT

  How to build: 

  on linux, with fftw3:
  gcc -o test_pffft -DHAVE_FFTW -msse -mfpmath=sse -O3 -Wall -W pffft.c test_pffft.c fftpack.c -L/usr/local/lib -I/usr/local/include/ -lfftw3f -lm

  on macos, without fftw3:
  clang -o test_pffft -DHAVE_VECLIB -O3 -Wall -W pffft.c test_pffft.c fftpack.c -L/usr/local/lib -I/usr/local/include/ -framework Accelerate

  on macos, with fftw3:
  clang -o test_pffft -DHAVE_FFTW -DHAVE_VECLIB -O3 -Wall -W pffft.c test_pffft.c fftpack.c -L/usr/local/lib -I/usr/local/include/ -lfftw3f -framework Accelerate

  as alternative: replace clang by gcc.

  on windows, with visual c++:
  cl /Ox -D_USE_MATH_DEFINES /arch:SSE test_pffft.c pffft.c fftpack.c
  
  build without SIMD instructions:
  gcc -o test_pffft -DPFFFT_SIMD_DISABLE -O3 -Wall -W pffft.c test_pffft.c fftpack.c -lm

 */

#ifdef PFFFT_ENABLE_FLOAT
#include "pffft.h"

typedef float pffft_scalar;
#else
/*
Note: adapted for double precision dynamic range version.
*/
#include "pffft_double.h"

typedef double pffft_scalar;
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

/* define own constants required to turn off g++ extensions .. */
#ifndef M_PI
  #define M_PI    3.14159265358979323846  /* pi */
#endif

/* EXPECTED_DYN_RANGE in dB:
 * single precision float has 24 bits mantissa
 * => 24 Bits * 6 dB = 144 dB
 * allow a few dB tolerance (even 144 dB looks good on my PC)
 */
#ifdef PFFFT_ENABLE_FLOAT
#define EXPECTED_DYN_RANGE  140.0
#else
#define EXPECTED_DYN_RANGE  215.0
#endif

/* maximum allowed phase error in degree */
#define DEG_ERR_LIMIT   1E-4

/* maximum allowed magnitude error in amplitude (of 1.0 or 1.1) */
#define MAG_ERR_LIMIT  1E-6


#define PRINT_SPEC  0

#define PWR2LOG(PWR)  ( (PWR) < 1E-30 ? 10.0*log10(1E-30) : 10.0*log10(PWR) )



int test(int N, int cplx, int useOrdered) {
  int Nfloat = (cplx ? N*2 : N);
#ifdef PFFFT_ENABLE_FLOAT
  pffft_scalar *X = pffft_aligned_malloc((unsigned)Nfloat * sizeof(pffft_scalar));
  pffft_scalar *Y = pffft_aligned_malloc((unsigned)Nfloat * sizeof(pffft_scalar));
  pffft_scalar *R = pffft_aligned_malloc((unsigned)Nfloat * sizeof(pffft_scalar));
  pffft_scalar *Z = pffft_aligned_malloc((unsigned)Nfloat * sizeof(pffft_scalar));
  pffft_scalar *W = pffft_aligned_malloc((unsigned)Nfloat * sizeof(pffft_scalar));
#else
  pffft_scalar *X = pffftd_aligned_malloc((unsigned)Nfloat * sizeof(pffft_scalar));
  pffft_scalar *Y = pffftd_aligned_malloc((unsigned)Nfloat * sizeof(pffft_scalar));
  pffft_scalar *R = pffftd_aligned_malloc((unsigned)Nfloat * sizeof(pffft_scalar));
  pffft_scalar *Z = pffftd_aligned_malloc((unsigned)Nfloat * sizeof(pffft_scalar));
  pffft_scalar *W = pffftd_aligned_malloc((unsigned)Nfloat * sizeof(pffft_scalar));
#endif
  pffft_scalar amp = (pffft_scalar)1.0;
  double freq, dPhi, phi, phi0;
  double pwr, pwrCar, pwrOther, err, errSum, mag, expextedMag;
  int k, j, m, iter, kmaxOther, retError = 0;

#ifdef PFFFT_ENABLE_FLOAT
  assert( pffft_is_power_of_two(N) );
  PFFFT_Setup *s = pffft_new_setup(N, cplx ? PFFFT_COMPLEX : PFFFT_REAL);
#else
  assert( pffftd_is_power_of_two(N) );
  PFFFTD_Setup *s = pffftd_new_setup(N, cplx ? PFFFT_COMPLEX : PFFFT_REAL);
#endif
  assert(s);
  if (!s) {
    printf("Error setting up PFFFT!\n");
    return 1;
  }

  for ( k = m = 0; k < (cplx? N : (1 + N/2) ); k += N/16, ++m )
  {
    amp = (pffft_scalar)( ( (m % 3) == 0 ) ? 1.0 : 1.1 );
    freq = (k < N/2) ? ((double)k / N) : ((double)(k-N) / N);
    dPhi = 2.0 * M_PI * freq;
    if ( dPhi < 0.0 )
      dPhi += 2.0 * M_PI;

    iter = -1;
    while (1)
    {
      ++iter;

      if (iter)
        printf("bin %d: dphi = %f for freq %f\n", k, dPhi, freq);

      /* generate cosine carrier as time signal - start at defined phase phi0 */
      phi = phi0 = (m % 4) * 0.125 * M_PI;  /* have phi0 < 90 deg to be normalized */
      for ( j = 0; j < N; ++j )
      {
        if (cplx) {
          X[2*j] = amp * (pffft_scalar)cos(phi);  /* real part */
          X[2*j+1] = amp * (pffft_scalar)sin(phi);  /* imag part */
        }
        else
          X[j] = amp * (pffft_scalar)cos(phi);  /* only real part */

        /* phase increment .. stay normalized - cos()/sin() might degrade! */
        phi += dPhi;
        if ( phi >= M_PI )
          phi -= 2.0 * M_PI;
      }

      /* forward transform from X --> Y  .. using work buffer W */
#ifdef PFFFT_ENABLE_FLOAT
      if ( useOrdered )
        pffft_transform_ordered(s, X, Y, W, PFFFT_FORWARD );
      else
      {
        pffft_transform(s, X, R, W, PFFFT_FORWARD );  /* use R for reordering */
        pffft_zreorder(s, R, Y, PFFFT_FORWARD ); /* reorder into Y[] for power calculations */
      }
#else
      if ( useOrdered )
        pffftd_transform_ordered(s, X, Y, W, PFFFT_FORWARD );
      else
      {
        pffftd_transform(s, X, R, W, PFFFT_FORWARD );  /* use R for reordering */
        pffftd_zreorder(s, R, Y, PFFFT_FORWARD ); /* reorder into Y[] for power calculations */
      }
#endif

      pwrOther = -1.0;
      pwrCar = 0;


      /* for positive frequencies: 0 to 0.5 * samplerate */
      /* and also for negative frequencies: -0.5 * samplerate to 0 */
      for ( j = 0; j < ( cplx ? N : (1 + N/2) ); ++j )
      {
        if (!cplx && !j)  /* special treatment for DC for real input */
          pwr = Y[j]*Y[j];
        else if (!cplx && j == N/2)  /* treat 0.5 * samplerate */
          pwr = Y[1] * Y[1];  /* despite j (for freq calculation) we have index 1 */
        else
          pwr = Y[2*j] * Y[2*j] + Y[2*j+1] * Y[2*j+1];
        if (iter || PRINT_SPEC)
          printf("%s fft %d:  pwr[j = %d] = %g == %f dB\n", (cplx ? "cplx":"real"), N, j, pwr, PWR2LOG(pwr) );
        if (k == j)
          pwrCar = pwr;
        else if ( pwr > pwrOther ) {
          pwrOther = pwr;
          kmaxOther = j;
        }
      }

      if ( PWR2LOG(pwrCar) - PWR2LOG(pwrOther) < EXPECTED_DYN_RANGE ) {
        printf("%s fft %d amp %f iter %d:\n", (cplx ? "cplx":"real"), N, amp, iter);
        printf("  carrier power  at bin %d: %g == %f dB\n", k, pwrCar, PWR2LOG(pwrCar) );
        printf("  carrier mag || at bin %d: %g\n", k, sqrt(pwrCar) );
        printf("  max other pwr  at bin %d: %g == %f dB\n", kmaxOther, pwrOther, PWR2LOG(pwrOther) );
        printf("  dynamic range: %f dB\n\n", PWR2LOG(pwrCar) - PWR2LOG(pwrOther) );
        retError = 1;
        if ( iter == 0 )
          continue;
      }

      if ( k > 0 && k != N/2 )
      {
        phi = atan2( Y[2*k+1], Y[2*k] );
        if ( fabs( phi - phi0) > DEG_ERR_LIMIT * M_PI / 180.0 )
        {
        retError = 1;
        printf("%s fft %d  bin %d amp %f : phase mismatch! phase = %f deg   expected = %f deg\n",
            (cplx ? "cplx":"real"), N, k, amp, phi * 180.0 / M_PI, phi0 * 180.0 / M_PI );
        }
      }

      expextedMag = cplx ? amp : ( (k == 0 || k == N/2) ? amp : (amp/2) );
      mag = sqrt(pwrCar) / N;
      if ( fabs(mag - expextedMag) > MAG_ERR_LIMIT )
      {
        retError = 1;
        printf("%s fft %d  bin %d amp %f : mag = %g   expected = %g\n", (cplx ? "cplx":"real"), N, k, amp, mag, expextedMag );
      }


      /* now convert spectrum back */
#ifdef PFFFT_ENABLE_FLOAT
      if (useOrdered)
        pffft_transform_ordered(s, Y, Z, W, PFFFT_BACKWARD);
      else
        pffft_transform(s, R, Z, W, PFFFT_BACKWARD);
#else
      if (useOrdered)
        pffftd_transform_ordered(s, Y, Z, W, PFFFT_BACKWARD);
      else
        pffftd_transform(s, R, Z, W, PFFFT_BACKWARD);
#endif

      errSum = 0.0;
      for ( j = 0; j < (cplx ? (2*N) : N); ++j )
      {
        /* scale back */
        Z[j] /= N;
        /* square sum errors over real (and imag parts) */
        err = (X[j]-Z[j]) * (X[j]-Z[j]);
        errSum += err;
      }

      if ( errSum > N * 1E-7 )
      {
        retError = 1;
        printf("%s fft %d  bin %d : inverse FFT doesn't match original signal! errSum = %g ; mean err = %g\n", (cplx ? "cplx":"real"), N, k, errSum, errSum / N);
      }

      break;
    }

  }
#ifdef PFFFT_ENABLE_FLOAT
  pffft_destroy_setup(s);
  pffft_aligned_free(X);
  pffft_aligned_free(Y);
  pffft_aligned_free(Z);
  pffft_aligned_free(R);
  pffft_aligned_free(W);
#else
  pffftd_destroy_setup(s);
  pffftd_aligned_free(X);
  pffftd_aligned_free(Y);
  pffftd_aligned_free(Z);
  pffftd_aligned_free(R);
  pffftd_aligned_free(W);
#endif

  return retError;
}

/* small functions inside pffft.c that will detect (compiler) bugs with respect to simd instructions */
void validate_pffft_simd();
int  validate_pffft_simd_ex(FILE * DbgOut);
void validate_pffftd_simd();
int  validate_pffftd_simd_ex(FILE * DbgOut);



int main(int argc, char **argv)
{
  int N, result, resN, resAll, i, k, resNextPw2, resIsPw2, resFFT;

  int inp_power_of_two[] = { 1, 2, 3, 4, 5, 6, 7, 8,  9, 511, 512,  513 };
  int ref_power_of_two[] = { 1, 2, 4, 4, 8, 8, 8, 8, 16, 512, 512, 1024 };

  for ( i = 1; i < argc; ++i ) {

    if (!strcmp(argv[i], "--test-simd")) {
#ifdef PFFFT_ENABLE_FLOAT
      int numErrs = validate_pffft_simd_ex(stdout);
#else
      int numErrs = validate_pffftd_simd_ex(stdout);
#endif
      fprintf( ( numErrs != 0 ? stderr : stdout ), "validate_pffft_simd_ex() returned %d errors!\n", numErrs);
      return ( numErrs > 0 ? 1 : 0 );
    }
  }

  resNextPw2 = 0;
  resIsPw2 = 0;
  for ( k = 0; k < (sizeof(inp_power_of_two)/sizeof(inp_power_of_two[0])); ++k) {
#ifdef PFFFT_ENABLE_FLOAT
    N = pffft_next_power_of_two(inp_power_of_two[k]);
#else
    N = pffftd_next_power_of_two(inp_power_of_two[k]);
#endif
    if (N != ref_power_of_two[k]) {
      resNextPw2 = 1;
      printf("pffft_next_power_of_two(%d) does deliver %d, which is not reference result %d!\n",
        inp_power_of_two[k], N, ref_power_of_two[k] );
    }

#ifdef PFFFT_ENABLE_FLOAT
    result = pffft_is_power_of_two(inp_power_of_two[k]);
#else
    result = pffftd_is_power_of_two(inp_power_of_two[k]);
#endif
    if (inp_power_of_two[k] == ref_power_of_two[k]) {
      if (!result) {
        resIsPw2 = 1;
        printf("pffft_is_power_of_two(%d) delivers false; expected true!\n", inp_power_of_two[k]);
      }
    } else {
      if (result) {
        resIsPw2 = 1;
        printf("pffft_is_power_of_two(%d) delivers true; expected false!\n", inp_power_of_two[k]);
      }
    }
  }
  if (!resNextPw2)
    printf("tests for pffft_next_power_of_two() succeeded successfully.\n");
  if (!resIsPw2)
    printf("tests for pffft_is_power_of_two() succeeded successfully.\n");

  resFFT = 0;
  for ( N = 32; N <= 65536; N *= 2 )
  {
    result = test(N, 1 /* cplx fft */, 1 /* useOrdered */);
    resN = result;
    resFFT |= result;

    result = test(N, 0 /* cplx fft */, 1 /* useOrdered */);
    resN |= result;
    resFFT |= result;

    result = test(N, 1 /* cplx fft */, 0 /* useOrdered */);
    resN |= result;
    resFFT |= result;

    result = test(N, 0 /* cplx fft */, 0 /* useOrdered */);
    resN |= result;
    resFFT |= result;

    if (!resN)
      printf("tests for size %d succeeded successfully.\n", N);
  }

  if (!resFFT) {
#ifdef PFFFT_ENABLE_FLOAT
    printf("all pffft transform tests (FORWARD/BACKWARD, REAL/COMPLEX, float) succeeded successfully.\n");
#else
    printf("all pffft transform tests (FORWARD/BACKWARD, REAL/COMPLEX, double) succeeded successfully.\n");
#endif
  }

  resAll = resNextPw2 | resIsPw2 | resFFT;
  if (!resAll)
    printf("all tests succeeded successfully.\n");
  else
    printf("there are failed tests!\n");

  return resAll;
}

