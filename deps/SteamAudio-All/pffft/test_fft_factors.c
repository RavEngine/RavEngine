
#ifdef PFFFT_ENABLE_FLOAT
#include "pffft.h"
#endif


#ifdef PFFFT_ENABLE_DOUBLE
#include "pffft_double.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>



#ifdef PFFFT_ENABLE_FLOAT
int test_float(int TL)
{
  PFFFT_Setup * S;

  for (int dir_i = 0; dir_i <= 1; ++dir_i)
  {
    for (int cplx_i = 0; cplx_i <= 1; ++cplx_i)
    {
      const pffft_direction_t dir = (!dir_i) ? PFFFT_FORWARD : PFFFT_BACKWARD;
      const pffft_transform_t cplx = (!cplx_i) ? PFFFT_REAL : PFFFT_COMPLEX;
      const int N_min = pffft_min_fft_size(cplx);
      const int N_max = N_min * 11 + N_min;
      int NTL = pffft_nearest_transform_size(TL, cplx, (!dir_i));
      double near_off = (NTL - TL) * 100.0 / (double)TL;

      fprintf(stderr, "testing float, %s, %s ..\tminimum transform %d; nearest transform for %d is %d (%.2f%% off)\n",
          (!dir_i) ? "FORWARD" : "BACKWARD", (!cplx_i) ? "REAL" : "COMPLEX", N_min, TL, NTL, near_off );

      for (int N = (N_min/2); N <= N_max; N += (N_min/2))
      {
        int R = N, f2 = 0, f3 = 0, f5 = 0, tmp_f;
        const int factorizable = pffft_is_valid_size(N, cplx);
        while (R >= 5*N_min && (R % 5) == 0) {  R /= 5; ++f5; }
        while (R >= 3*N_min && (R % 3) == 0) {  R /= 3; ++f3; }
        while (R >= 2*N_min && (R % 2) == 0) {  R /= 2; ++f2; }
        tmp_f = (R == N_min) ? 1 : 0;
        assert( factorizable == tmp_f );

        S = pffft_new_setup(N, cplx);

        if ( S && !factorizable )
        {
          fprintf(stderr, "fft setup successful, but NOT factorizable into min(=%d), 2^%d, 3^%d, 5^%d for N = %d (R = %d)\n", N_min, f2, f3, f5, N, R);
          return 1;
        }
        else if ( !S && factorizable)
        {
          fprintf(stderr, "fft setup UNsuccessful, but factorizable into min(=%d), 2^%d, 3^%d, 5^%d for N = %d (R = %d)\n", N_min, f2, f3, f5, N, R);
          return 1;
        }
        
        if (S)
          pffft_destroy_setup(S);
      }

    }
  }
  return 0;
}

#endif


#ifdef PFFFT_ENABLE_DOUBLE
int test_double(int TL)
{
  PFFFTD_Setup * S;
  for (int dir_i = 0; dir_i <= 1; ++dir_i)
  {
    for (int cplx_i = 0; cplx_i <= 1; ++cplx_i)
    {
      const pffft_direction_t dir = (!dir_i) ? PFFFT_FORWARD : PFFFT_BACKWARD;
      const pffft_transform_t cplx = (!cplx_i) ? PFFFT_REAL : PFFFT_COMPLEX;
      const int N_min = pffftd_min_fft_size(cplx);
      const int N_max = N_min * 11 + N_min;
      int NTL = pffftd_nearest_transform_size(TL, cplx, (!dir_i));
      double near_off = (NTL - TL) * 100.0 / (double)TL;

      fprintf(stderr, "testing double, %s, %s ..\tminimum transform %d; nearest transform for %d is %d (%.2f%% off)\n",
          (!dir_i) ? "FORWARD" : "BACKWARD", (!cplx_i) ? "REAL" : "COMPLEX", N_min, TL, NTL, near_off );

      for (int N = (N_min/2); N <= N_max; N += (N_min/2))
      {
        int R = N, f2 = 0, f3 = 0, f5 = 0, tmp_f;
        const int factorizable = pffftd_is_valid_size(N, cplx);
        while (R >= 5*N_min && (R % 5) == 0) {  R /= 5; ++f5; }
        while (R >= 3*N_min && (R % 3) == 0) {  R /= 3; ++f3; }
        while (R >= 2*N_min && (R % 2) == 0) {  R /= 2; ++f2; }
        tmp_f = (R == N_min) ? 1 : 0;
        assert( factorizable == tmp_f );

        S = pffftd_new_setup(N, cplx);

        if ( S && !factorizable )
        {
          fprintf(stderr, "fft setup successful, but NOT factorizable into min(=%d), 2^%d, 3^%d, 5^%d for N = %d (R = %d)\n", N_min, f2, f3, f5, N, R);
          return 1;
        }
        else if ( !S && factorizable)
        {
          fprintf(stderr, "fft setup UNsuccessful, but factorizable into min(=%d), 2^%d, 3^%d, 5^%d for N = %d (R = %d)\n", N_min, f2, f3, f5, N, R);
          return 1;
        }
        
        if (S)
          pffftd_destroy_setup(S);
      }

    }
  }
  return 0;
}

#endif



int main(int argc, char *argv[])
{
  int N = (1 < argc) ? atoi(argv[1]) : 2;

  int r = 0;
#ifdef PFFFT_ENABLE_FLOAT
  r = test_float(N);
  if (r)
    return r;
#endif

#ifdef PFFFT_ENABLE_DOUBLE
  r = test_double(N);
#endif

  return r;
}

