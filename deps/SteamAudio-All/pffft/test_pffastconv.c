/*
  Copyright (c) 2013 Julien Pommier.
  Copyright (c) 2019  Hayati Ayguen ( h_ayguen@web.de )
 */

#define _WANT_SNAN  1

#include "pffft.h"
#include "pffastconv.h"

#include <math.h>
#include <float.h>
#include <limits.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

#ifdef HAVE_SYS_TIMES
#  include <sys/times.h>
#  include <unistd.h>
#endif

/* benchmark duration: 250 ms */
#define BENCH_TEST_DURATION_IN_SEC      0.5

/* 
   vector support macros: the rest of the code is independant of
   SSE/Altivec/NEON -- adding support for other platforms with 4-element
   vectors should be limited to these macros 
*/
#if 0
#include "simd/pf_float.h"
#endif

#if defined(_MSC_VER)
#  define RESTRICT __restrict
#elif defined(__GNUC__)
#  define RESTRICT __restrict
#else
#  define RESTRICT
#endif


#if defined(_MSC_VER)
#pragma warning( disable : 4244 )
#endif


#ifdef SNANF
  #define INVALID_FLOAT_VAL  SNANF
#elif defined(SNAN)
  #define INVALID_FLOAT_VAL  SNAN
#elif defined(NAN)
  #define INVALID_FLOAT_VAL  NAN
#elif defined(INFINITY)
  #define INVALID_FLOAT_VAL  INFINITY
#else
  #define INVALID_FLOAT_VAL  FLT_MAX
#endif


#if defined(HAVE_SYS_TIMES)
  inline double uclock_sec(void) {
    static double ttclk = 0.;
    struct tms t;
    if (ttclk == 0.)
      ttclk = sysconf(_SC_CLK_TCK);
    times(&t);
    /* use only the user time of this process - not realtime, which depends on OS-scheduler .. */
    return ((double)t.tms_utime)) / ttclk;
  }
# else
  double uclock_sec(void)
{ return (double)clock()/(double)CLOCKS_PER_SEC; }
#endif



typedef int            (*pfnConvolution)  (void * setup, const float * X, int len, float *Y, const float *Yref, int applyFlush);
typedef void*          (*pfnConvSetup)    (float *Hfwd, int Nf, int * BlkLen, int flags);
typedef pfnConvolution (*pfnGetConvFnPtr) (void * setup);
typedef void           (*pfnConvDestroy)  (void * setup);


struct ConvSetup
{
  pfnConvolution pfn;
  int N;
  int B;
  float * H;
  int flags;
};


void * convSetupRev( float * H, int N, int * BlkLen, int flags )
{
  struct ConvSetup * s = pffastconv_malloc( sizeof(struct ConvSetup) );
  int i, Nr = N;
  if (flags & PFFASTCONV_CPLX_INP_OUT)
    Nr *= 2;
  Nr += 4;
  s->pfn = NULL;
  s->N = N;
  s->B = *BlkLen;
  s->H = pffastconv_malloc((unsigned)Nr * sizeof(float));
  s->flags = flags;
  memset(s->H, 0, (unsigned)Nr * sizeof(float));
  if (flags & PFFASTCONV_CPLX_INP_OUT)
  {
    for ( i = 0; i < N; ++i ) {
      s->H[2*(N-1 -i)  ] = H[i];
      s->H[2*(N-1 -i)+1] = H[i];
    }
    /* simpler detection of overruns */
    s->H[ 2*N    ] = INVALID_FLOAT_VAL;
    s->H[ 2*N +1 ] = INVALID_FLOAT_VAL;
    s->H[ 2*N +2 ] = INVALID_FLOAT_VAL;
    s->H[ 2*N +3 ] = INVALID_FLOAT_VAL;
  }
  else
  {
    for ( i = 0; i < N; ++i )
      s->H[ N-1 -i ] = H[i];
    /* simpler detection of overruns */
    s->H[ N    ] = INVALID_FLOAT_VAL;
    s->H[ N +1 ] = INVALID_FLOAT_VAL;
    s->H[ N +2 ] = INVALID_FLOAT_VAL;
    s->H[ N +3 ] = INVALID_FLOAT_VAL;
  }
  return s;
}

void convDestroyRev( void * setup )
{
  struct ConvSetup * s = (struct ConvSetup*)setup;
  pffastconv_free(s->H);
  pffastconv_free(setup);
}


pfnConvolution ConvGetFnPtrRev( void * setup )
{
  struct ConvSetup * s = (struct ConvSetup*)setup;
  if (!s)
    return NULL;
  return s->pfn;
}


void convSimdDestroy( void * setup )
{
  convDestroyRev(setup);
}


void * fastConvSetup( float * H, int N, int * BlkLen, int flags )
{
  void * p = pffastconv_new_setup( H, N, BlkLen, flags );
  if (!p)
    printf("fastConvSetup(N = %d, *BlkLen = %d, flags = %d) = NULL\n", N, *BlkLen, flags);
  return p;
}


void fastConvDestroy( void * setup )
{
  pffastconv_destroy_setup( (PFFASTCONV_Setup*)setup );
}



int slow_conv_R(void * setup, const float * input, int len, float *output, const float *Yref, int applyFlush)
{
  struct ConvSetup * p = (struct ConvSetup*)setup;
  const float * RESTRICT X = input;
  const float * RESTRICT Hrev = p->H;
  float * RESTRICT Y = output;
  const int Nr = ((p->flags & PFFASTCONV_CPLX_INP_OUT) ? 2 : 1) * p->N;
  const int lenNr = ((p->flags & PFFASTCONV_CPLX_INP_OUT) ? 2 : 1) * (len - p->N);
  int i, j;
  (void)Yref;
  (void)applyFlush;

  if (p->flags & PFFASTCONV_CPLX_INP_OUT)
  {
    for ( i = 0; i <= lenNr; i += 2 )
    {
      float sumRe = 0.0F, sumIm = 0.0F;
      for ( j = 0; j < Nr; j += 2 )
      {
        sumRe += X[i+j  ] * Hrev[j];
        sumIm += X[i+j+1] * Hrev[j+1];
      }
      Y[i  ] = sumRe;
      Y[i+1] = sumIm;
    }
    return i/2;
  }
  else
  {
    for ( i = 0; i <= lenNr; ++i )
    {
      float sum = 0.0F;
      for (j = 0; j < Nr; ++j )
        sum += X[i+j]   * Hrev[j];
      Y[i] = sum;
    }
    return i;
  }
}



int slow_conv_A(void * setup, const float * input, int len, float *output, const float *Yref, int applyFlush)
{
  float sum[4];
  struct ConvSetup * p = (struct ConvSetup*)setup;
  const float * RESTRICT X = input;
  const float * RESTRICT Hrev = p->H;
  float * RESTRICT Y = output;
  const int Nr = ((p->flags & PFFASTCONV_CPLX_INP_OUT) ? 2 : 1) * p->N;
  const int lenNr = ((p->flags & PFFASTCONV_CPLX_INP_OUT) ? 2 : 1) * (len - p->N);
  int i, j;
  (void)Yref;
  (void)applyFlush;

  if (p->flags & PFFASTCONV_CPLX_INP_OUT)
  {
    if ( (Nr & 3) == 0 )
    {
      for ( i = 0; i <= lenNr; i += 2 )
      {
        sum[0] = sum[1] = sum[2] = sum[3] = 0.0F;
        for (j = 0; j < Nr; j += 4 )
        {
          sum[0] += X[i+j]   * Hrev[j];
          sum[1] += X[i+j+1] * Hrev[j+1];
          sum[2] += X[i+j+2] * Hrev[j+2];
          sum[3] += X[i+j+3] * Hrev[j+3];
        }
        Y[i  ] = sum[0] + sum[2];
        Y[i+1] = sum[1] + sum[3];
      }
    }
    else
    {
      const int M = Nr & (~3);
      for ( i = 0; i <= lenNr; i += 2 )
      {
        float tailSumRe = 0.0F, tailSumIm = 0.0F;
        sum[0] = sum[1] = sum[2] = sum[3] = 0.0F;
        for (j = 0; j < M; j += 4 )
        {
          sum[0] += X[i+j  ] * Hrev[j  ];
          sum[1] += X[i+j+1] * Hrev[j+1];
          sum[2] += X[i+j+2] * Hrev[j+2];
          sum[3] += X[i+j+3] * Hrev[j+3];
        }
        for ( ; j < Nr; j += 2 ) {
          tailSumRe += X[i+j  ] * Hrev[j  ];
          tailSumIm += X[i+j+1] * Hrev[j+1];
        }
        Y[i  ] = ( sum[0] + sum[2] ) + tailSumRe;
        Y[i+1] = ( sum[1] + sum[3] ) + tailSumIm;
      }
    }
    return i/2;
  }
  else
  {
    if ( (Nr & 3) == 0 )
    {
      for ( i = 0; i <= lenNr; ++i )
      {
        sum[0] = sum[1] = sum[2] = sum[3] = 0.0F;
        for (j = 0; j < Nr; j += 4 )
        {
          sum[0] += X[i+j]   * Hrev[j];
          sum[1] += X[i+j+1] * Hrev[j+1];
          sum[2] += X[i+j+2] * Hrev[j+2];
          sum[3] += X[i+j+3] * Hrev[j+3];
        }
        Y[i] = sum[0] + sum[1] + sum[2] + sum[3];
      }
      return i;
    }
    else
    {
      const int M = Nr & (~3);
      /* printf("A: Nr = %d, M = %d, H[M] = %f, H[M+1] = %f, H[M+2] = %f, H[M+3] = %f\n", Nr, M, Hrev[M], Hrev[M+1], Hrev[M+2], Hrev[M+3] ); */
      for ( i = 0; i <= lenNr; ++i )
      {
        float tailSum = 0.0;
        sum[0] = sum[1] = sum[2] = sum[3] = 0.0F;
        for (j = 0; j < M; j += 4 )
        {
          sum[0] += X[i+j]   * Hrev[j];
          sum[1] += X[i+j+1] * Hrev[j+1];
          sum[2] += X[i+j+2] * Hrev[j+2];
          sum[3] += X[i+j+3] * Hrev[j+3];
        }
        for ( ; j < Nr; ++j )
          tailSum += X[i+j] * Hrev[j];
        Y[i] = (sum[0] + sum[1]) + (sum[2] + sum[3]) + tailSum;
      }
      return i;
    }
  }
}


int slow_conv_B(void * setup, const float * input, int len, float *output, const float *Yref, int applyFlush)
{
  float sum[4];
  struct ConvSetup * p = (struct ConvSetup*)setup;
  (void)Yref;
  (void)applyFlush;
  if (p->flags & PFFASTCONV_SYMMETRIC)
  {
    const float * RESTRICT X = input;
    const float * RESTRICT Hrev = p->H;
    float * RESTRICT Y = output;
    const int Nr = ((p->flags & PFFASTCONV_CPLX_INP_OUT) ? 2 : 1) * p->N;
    const int lenNr = ((p->flags & PFFASTCONV_CPLX_INP_OUT) ? 2 : 1) * (len - p->N);
    const int h = Nr / 2 -4;
    const int E = Nr -4;
    int i, j;

    if (p->flags & PFFASTCONV_CPLX_INP_OUT)
    {
      for ( i = 0; i <= lenNr; i += 2 )
      {
        const int k = i + E;
        sum[0] = sum[1] = sum[2] = sum[3] = 0.0F;
        for (j = 0; j <= h; j += 4 )
        {
          sum[0] += Hrev[j  ] * ( X[i+j  ] + X[k-j+2] );
          sum[1] += Hrev[j+1] * ( X[i+j+1] + X[k-j+3] );
          sum[2] += Hrev[j+2] * ( X[i+j+2] + X[k-j  ] );
          sum[3] += Hrev[j+3] * ( X[i+j+3] + X[k-j+1] );
        }
        Y[i  ] = sum[0] + sum[2];
        Y[i+1] = sum[1] + sum[3];
      }
      return i/2;
    }
    else
    {
      for ( i = 0; i <= lenNr; ++i )
      {
        const int k = i + E;
        sum[0] = sum[1] = sum[2] = sum[3] = 0.0F;
        for (j = 0; j <= h; j += 4 )
        {
          sum[0] += Hrev[j  ] * ( X[i+j  ] + X[k-j+3] );
          sum[1] += Hrev[j+1] * ( X[i+j+1] + X[k-j+2] );
          sum[2] += Hrev[j+2] * ( X[i+j+2] + X[k-j+1] );
          sum[3] += Hrev[j+3] * ( X[i+j+3] + X[k-j  ] );
        }
        Y[i] = sum[0] + sum[1] + sum[2] + sum[3];
      }
      return i;
    }
  }
  else
  {
    const float * RESTRICT X = input;
    const float * RESTRICT Hrev = p->H;
    float * RESTRICT Y = output;
    const int Nr = ((p->flags & PFFASTCONV_CPLX_INP_OUT) ? 2 : 1) * p->N;
    const int lenNr = ((p->flags & PFFASTCONV_CPLX_INP_OUT) ? 2 : 1) * (len - p->N);
    int i, j;

    if (p->flags & PFFASTCONV_CPLX_INP_OUT)
    {
      for ( i = 0; i <= lenNr; i += 2 )
      {
        sum[0] = sum[1] = sum[2] = sum[3] = 0.0F;
        for (j = 0; j < Nr; j += 4 )
        {
          sum[0] += X[i+j]   * Hrev[j];
          sum[1] += X[i+j+1] * Hrev[j+1];
          sum[2] += X[i+j+2] * Hrev[j+2];
          sum[3] += X[i+j+3] * Hrev[j+3];
        }
        Y[i  ] = sum[0] + sum[2];
        Y[i+1] = sum[1] + sum[3];
      }
      return i/2;
    }
    else
    {
      if ( (Nr & 3) == 0 )
      {
        for ( i = 0; i <= lenNr; ++i )
        {
          sum[0] = sum[1] = sum[2] = sum[3] = 0.0F;
          for (j = 0; j < Nr; j += 4 )
          {
            sum[0] += X[i+j]   * Hrev[j];
            sum[1] += X[i+j+1] * Hrev[j+1];
            sum[2] += X[i+j+2] * Hrev[j+2];
            sum[3] += X[i+j+3] * Hrev[j+3];
          }
          Y[i] = (sum[0] + sum[1]) + (sum[2] + sum[3]);
        }
        return i;
      }
      else
      {
        const int M = Nr & (~3);
        /* printf("B: Nr = %d\n", Nr ); */
        for ( i = 0; i <= lenNr; ++i )
        {
          float tailSum = 0.0;
          sum[0] = sum[1] = sum[2] = sum[3] = 0.0F;
          for (j = 0; j < M; j += 4 )
          {
            sum[0] += X[i+j]   * Hrev[j];
            sum[1] += X[i+j+1] * Hrev[j+1];
            sum[2] += X[i+j+2] * Hrev[j+2];
            sum[3] += X[i+j+3] * Hrev[j+3];
          }
          for ( ; j < Nr; ++j )
            tailSum += X[i+j] * Hrev[j];
          Y[i] = (sum[0] + sum[1]) + (sum[2] + sum[3]) + tailSum;
        }
        return i;
      }
    }
  }

}


int fast_conv(void * setup, const float * X, int len, float *Y, const float *Yref, int applyFlush)
{
  (void)Yref;
  return pffastconv_apply( (PFFASTCONV_Setup*)setup, X, len, Y, applyFlush );
}



void printFirst( const float * V, const char * st, const int N, const int perLine )
{
  (void)V;  (void)st;  (void)N;  (void)perLine;
  return;
#if 0
  int i;
  for ( i = 0; i < N; ++i )
  {
    if ( (i % perLine) == 0 )
      printf("\n%s[%d]", st, i);
    printf("\t%.1f", V[i]);
  }
  printf("\n");
#endif
}



#define NUMY       15


int test(int FILTERLEN, int convFlags, const int testOutLen, int printDbg, int printSpeed, int abortFirstFastAlgo, int printErrValues, int printAsCSV, int *pIsFirstFilterLen) {
  double t0, t1, tstop, td, tdref;
  float *X, *H;
  float *Y[NUMY];
  int64_t outN[NUMY];
  /* 256 KFloats or 16 MFloats data */
#if 1
  const int len = testOutLen ? (1 << 18) : (1 << 24);
#elif 0
  const int len = testOutLen ? (1 << 18) : (1 << 13);
#else
  const int len = testOutLen ? (1 << 18) : (1024);
#endif
  const int cplxFactor = ( convFlags & PFFASTCONV_CPLX_INP_OUT ) ? 2 : 1;
  const int lenC = len / cplxFactor;

  int yi, yc, posMaxErr;
  float yRangeMin, yRangeMax, yErrLimit, maxErr = 0.0;
  int i, j, numErrOverLimit, iter;
  int retErr = 0;

  /*                                  0               1               2               3                   4                   5                   6                   7                   8                      9,                   10,                  11,                   12,                   13                     */
  pfnConvSetup   aSetup[NUMY]     = { convSetupRev,   convSetupRev,   convSetupRev,   fastConvSetup,      fastConvSetup,      fastConvSetup,      fastConvSetup,      fastConvSetup,      fastConvSetup,         fastConvSetup,       fastConvSetup,       fastConvSetup,        fastConvSetup,        fastConvSetup,         };
  pfnConvDestroy aDestroy[NUMY]   = { convDestroyRev, convDestroyRev, convDestroyRev, fastConvDestroy,    fastConvDestroy,    fastConvDestroy,    fastConvDestroy,    fastConvDestroy,    fastConvDestroy,       fastConvDestroy,     fastConvDestroy,     fastConvDestroy,      fastConvDestroy,      fastConvDestroy,       };
  pfnGetConvFnPtr aGetFnPtr[NUMY] = { NULL,           NULL,           NULL,           NULL,               NULL,               NULL,               NULL,               NULL,               NULL,                  NULL,                NULL,                NULL,                 NULL,                 NULL,                  };
  pfnConvolution aConv[NUMY]      = { slow_conv_R,    slow_conv_A,    slow_conv_B,    fast_conv,          fast_conv,          fast_conv,          fast_conv,          fast_conv,          fast_conv,             fast_conv,           fast_conv,           fast_conv,            fast_conv,            fast_conv,             };
  const char * convText[NUMY]     = { "R(non-simd)",  "A(non-simd)",  "B(non-simd)",  "fast_conv_64",     "fast_conv_128",    "fast_conv_256",    "fast_conv_512",    "fast_conv_1K",     "fast_conv_2K",        "fast_conv_4K",      "fast_conv_8K",      "fast_conv_16K",      "fast_conv_32K",      "fast_conv_64K",       };
  int    aFastAlgo[NUMY]          = { 0,              0,              0,              1,                  1,                  1,                  1,                  1,                  1,                     1,                   1,                   1,                    1,                    1,                     };
  void * aSetupCfg[NUMY]          = { NULL,           NULL,           NULL,           NULL,               NULL,               NULL,               NULL,               NULL,               NULL,                  NULL,                NULL,                NULL,                 NULL,                 NULL,                  };
//int    aBlkLen[NUMY]            = { 1024,           1024,           1024,           64,                 128,                256,                512,                1024,               2048,                  4096,                8192,                16384,                32768,                65536,                 };
  int    aBlkLen[NUMY]            = { 8192,           8192,           8192,           64,                 128,                256,                512,                1024,               2048,                  4096,                8192,                16384,                32768,                65536,                 };
#if 1
  int    aRunAlgo[NUMY]           = { 1,              1,              1,              FILTERLEN<64,       FILTERLEN<128,      FILTERLEN<256,      FILTERLEN<512,      FILTERLEN<1024,     FILTERLEN<2048,        FILTERLEN<4096,      FILTERLEN<8192,      FILTERLEN<16384,      FILTERLEN<32768,      FILTERLEN<65536,       };
#elif 0
  int    aRunAlgo[NUMY]           = { 1,              0,              0,              0 && FILTERLEN<64,  1 && FILTERLEN<128, 1 && FILTERLEN<256, 0 && FILTERLEN<512, 0 && FILTERLEN<1024, 0 && FILTERLEN<2048,  0 && FILTERLEN<4096, 0 && FILTERLEN<8192, 0 && FILTERLEN<16384, 0 && FILTERLEN<32768, 0 && FILTERLEN<65536,  };
#else
  int    aRunAlgo[NUMY]           = { 1,              1,              1,              0 && FILTERLEN<64,  0 && FILTERLEN<128, 1 && FILTERLEN<256, 0 && FILTERLEN<512, 0 && FILTERLEN<1024, 0 && FILTERLEN<2048,  0 && FILTERLEN<4096, 0 && FILTERLEN<8192, 0 && FILTERLEN<16384, 0 && FILTERLEN<32768, 0 && FILTERLEN<65536,  };
#endif
  double aSpeedFactor[NUMY], aDuration[NUMY], procSmpPerSec[NUMY];
  int aNumIters[NUMY], aNumLoops[NUMY];

  X = pffastconv_malloc( (unsigned)(len+4) * sizeof(float) );
  for ( i=0; i < NUMY; ++i)
  {
    if ( 1 || i < 2 )
      Y[i] = pffastconv_malloc( (unsigned)len * sizeof(float) );
    else
      Y[i] = Y[1];

    Y[i][0] = 123.F;  /* test for pffft_zconvolve_no_accu() */
    aSpeedFactor[i] = -1.0;
    aDuration[i] = -1.0;
    procSmpPerSec[i] = -1.0;
    aNumIters[i] = 0;
    aNumLoops[i] = 0;
  }

  H = pffastconv_malloc((unsigned)FILTERLEN * sizeof(float));

  /* initialize input */
  if ( convFlags & PFFASTCONV_CPLX_INP_OUT )
  {
    for ( i = 0; i < lenC; ++i )
    {
      X[2*i  ] = (float)(i % 4093);  /* 4094 is a prime number. see https://en.wikipedia.org/wiki/List_of_prime_numbers */
      X[2*i+1] = (float)((i+2048) % 4093);
    }
  }
  else
  {
    for ( i = 0; i < len; ++i )
      X[i] = (float)(i % 4093);  /* 4094 is a prime number. see https://en.wikipedia.org/wiki/List_of_prime_numbers */
  }
  X[ len    ] = INVALID_FLOAT_VAL;
  X[ len +1 ] = INVALID_FLOAT_VAL;
  X[ len +2 ] = INVALID_FLOAT_VAL;
  X[ len +3 ] = INVALID_FLOAT_VAL;

  if (!testOutLen)
    printFirst( X, "X", 64, 8 );

  /* filter coeffs */
  memset( H, 0, FILTERLEN * sizeof(float) );
#if 1
  if ( convFlags & PFFASTCONV_SYMMETRIC )
  {
    const int half = FILTERLEN / 2;
    for ( j = 0; j < half; ++j ) {
      switch (j % 3) {
        case 0: H[j] = H[FILTERLEN-1-j] = -1.0F;  break;
        case 1: H[j] = H[FILTERLEN-1-j] =  1.0F;  break;
        case 2: H[j] = H[FILTERLEN-1-j] =  0.5F;  break;
      }
    }
  }
  else
  {
    for ( j = 0; j < FILTERLEN; ++j ) {
      switch (j % 3) {
        case 0: H[j] = -1.0F;  break;
        case 1: H[j] = 1.0F;   break;
        case 2: H[j] = 0.5F;   break;
      }
    }
  }
#else
  H[0] = 1.0F;
  H[FILTERLEN -1] = 1.0F;
#endif
  if (!testOutLen)
    printFirst( H, "H", FILTERLEN, 8 );

  if (!printAsCSV)
  {
    printf("\n");
    printf("filterLen = %d\t%s%s\t%s:\n", FILTERLEN,
      ((convFlags & PFFASTCONV_CPLX_INP_OUT)?"cplx":"real"),
      (convFlags & PFFASTCONV_CPLX_INP_OUT)?((convFlags & PFFASTCONV_CPLX_SINGLE_FFT)?" single":" 2x") : "",
      ((convFlags & PFFASTCONV_SYMMETRIC)?"symmetric":"non-sym") );
  }

  int hadFastAlgo = 0;

  while (1)
  {

    for ( yi = 0; yi < NUMY; ++yi )
    {
      if (!aRunAlgo[yi])
        continue;

      if ( aFastAlgo[yi] && abortFirstFastAlgo && hadFastAlgo )
      {
        aRunAlgo[yi] = 0;
        continue;
      }

      hadFastAlgo = hadFastAlgo | aFastAlgo[yi];

      aSetupCfg[yi] = aSetup[yi]( H, FILTERLEN, &aBlkLen[yi], convFlags );

      /* get effective apply function ptr */
      if ( aSetupCfg[yi] && aGetFnPtr[yi] )
        aConv[yi] = aGetFnPtr[yi]( aSetupCfg[yi] );

      if ( aSetupCfg[yi] && aConv[yi] )
      {
        if (testOutLen)
        {
          t0 = uclock_sec();
          outN[yi] = aConv[yi]( aSetupCfg[yi], X, lenC, Y[yi], Y[0], 1 /* applyFlush */ );
          t1 = uclock_sec();
          td = t1 - t0;
        }
        else
        {
          //const int blkLen = 4096;  /* required for 'fast_conv_4K' */
          const int blkLen = aBlkLen[yi];
          int64_t offC = 0, offS, Nout;
          int k;
          iter = 0;
          outN[yi] = 0;
          aNumLoops[yi] = 1;
          t0 = uclock_sec();
          tstop = t0 + BENCH_TEST_DURATION_IN_SEC;
          do
          {
            const int prev_iter = iter;
            for ( k = 0; k < 128 && offC +blkLen < lenC; ++k )
            {
              offS = cplxFactor * offC;
              Nout = aConv[yi]( aSetupCfg[yi], X +offS, blkLen, Y[yi] +offS, Y[0], 0 /* applyFlush */ );
              offC += Nout;
              ++iter;
              if ( !Nout )
                break;
            }
            //if ( !Nout )
            //  break;
            t1 = uclock_sec();
            if ( prev_iter == iter )    // restart from begin of input?
            {
                offC = 0;
                ++aNumLoops[yi];
            }
          } while ( t1 < tstop );
          outN[yi] = offC;
          td = t1 - t0;
          procSmpPerSec[yi] = cplxFactor * (double)outN[yi] * (1.0 / td);
          aNumIters[yi] = iter;
          aDuration[yi] = td;

          //printf("algo '%s':\t%.2f MSmp\tin\t%.1f ms\t= %g kSmpPerSec\t%d iters\t%.1f ms\n",
          //  convText[yi], (double)outN[yi]/(1000.0 * 1000.0), 1000.0 * aDuration[yi], procSmpPerSec[yi] * 0.001, aNumIters[yi], 1000.0 * td );
        }
      }
      else
      {
        outN[yi] = 0;
      }
      if ( yi == 0 ) {
        const float * Yvals = Y[0];
        const int64_t refOutLen = cplxFactor * outN[0];
        tdref = td;
        if (printDbg) {
          printf("convolution '%s' took: %f ms\n", convText[yi], td*1000.0);
          printf("  convolution '%s' output size %" PRId64 " == (cplx) len %d + %" PRId64 "\n", convText[yi], outN[yi], len / cplxFactor, outN[yi] - len / cplxFactor);
        }
        aSpeedFactor[yi] = 1.0;
        /*  */
        yRangeMin = FLT_MAX;
        yRangeMax = FLT_MIN;
        for ( i = 0; i < refOutLen; ++i )
        {
          if ( yRangeMax < Yvals[i] )  yRangeMax = Yvals[i];
          if ( yRangeMin > Yvals[i] )  yRangeMin = Yvals[i];
        }
        yErrLimit = fabsf(yRangeMax - yRangeMin) / ( 100.0F * 1000.0F );
        /* yErrLimit = 0.01F; */
        if (testOutLen) {
          if (1) {
            printf("reference output len = %" PRId64 " smp\n", outN[0]);
            printf("reference output range |%.1f ..%.1f| = %.1f ==> err limit = %f\n", yRangeMin, yRangeMax, yRangeMax - yRangeMin, yErrLimit);
          }
          printFirst( Yvals, "Yref", 64, 8 );
        }
      }
      else
      {
        aSpeedFactor[yi] = tdref / td;
        if (printDbg) {
          printf("\nconvolution '%s' took: %f ms == %f %% == %f X\n", convText[yi], td*1000.0, td * 100 / tdref, tdref / td);
          printf("  convolution '%s' output size %" PRId64 " == (cplx) len %d + %" PRId64 "\n", convText[yi], outN[yi], len / cplxFactor, outN[yi] - len / cplxFactor);
        }
      }
    }

    int iMaxSpeedSlowAlgo = -1;
    int iFirstFastAlgo = -1;
    int iMaxSpeedFastAlgo = -1;
    int iPrintedRefOutLen = 0;
    {
      for ( yc = 1; yc < NUMY; ++yc )
      {
        if (!aRunAlgo[yc])
          continue;
        if (aFastAlgo[yc]) {
          if ( iMaxSpeedFastAlgo < 0 || aSpeedFactor[yc] > aSpeedFactor[iMaxSpeedFastAlgo] )
            iMaxSpeedFastAlgo = yc;
            
          if (iFirstFastAlgo < 0)
            iFirstFastAlgo = yc;
        }
        else
        {
          if ( iMaxSpeedSlowAlgo < 0 || aSpeedFactor[yc] > aSpeedFactor[iMaxSpeedSlowAlgo] )
            iMaxSpeedSlowAlgo = yc;
        }
      }

      if (printSpeed)
      {
        if (testOutLen)
        {
          if (iMaxSpeedSlowAlgo >= 0 )
            printf("fastest slow algorithm is '%s' at speed %f X ; abs duration %f ms\n", convText[iMaxSpeedSlowAlgo], aSpeedFactor[iMaxSpeedSlowAlgo], 1000.0 * aDuration[iMaxSpeedSlowAlgo]);
          if (0 != iMaxSpeedSlowAlgo && aRunAlgo[0])
            printf("slow algorithm '%s' at speed %f X ; abs duration %f ms\n", convText[0], aSpeedFactor[0], 1000.0 * aDuration[0]);
          if (1 != iMaxSpeedSlowAlgo && aRunAlgo[1])
            printf("slow algorithm '%s' at speed %f X ; abs duration %f ms\n", convText[1], aSpeedFactor[1], 1000.0 * aDuration[1]);

          if (iFirstFastAlgo >= 0 && iFirstFastAlgo != iMaxSpeedFastAlgo && aRunAlgo[iFirstFastAlgo])
            printf("first   fast algorithm is '%s' at speed %f X ; abs duration %f ms\n", convText[iFirstFastAlgo],    aSpeedFactor[iFirstFastAlgo],    1000.0 * aDuration[iFirstFastAlgo]);
          if (iFirstFastAlgo >= 0 && iFirstFastAlgo+1 != iMaxSpeedFastAlgo && iFirstFastAlgo+1 < NUMY && aRunAlgo[iFirstFastAlgo+1])
            printf("2nd     fast algorithm is '%s' at speed %f X ; abs duration %f ms\n", convText[iFirstFastAlgo+1],  aSpeedFactor[iFirstFastAlgo+1],  1000.0 * aDuration[iFirstFastAlgo+1]);

          if ( 0 <= iMaxSpeedFastAlgo && iMaxSpeedFastAlgo < NUMY && aRunAlgo[iMaxSpeedFastAlgo] )
          {
            printf("fastest fast algorithm is '%s' at speed %f X ; abs duration %f ms\n", convText[iMaxSpeedFastAlgo], aSpeedFactor[iMaxSpeedFastAlgo], 1000.0 * aDuration[iMaxSpeedFastAlgo]);
            if ( 0 <= iMaxSpeedSlowAlgo && iMaxSpeedSlowAlgo < NUMY && aRunAlgo[iMaxSpeedSlowAlgo] )
              printf("fast / slow ratio: %f X\n", aSpeedFactor[iMaxSpeedFastAlgo] / aSpeedFactor[iMaxSpeedSlowAlgo] );
          }
          printf("\n");
        }
        else
        {
          // print columns in 1st line
          if (printAsCSV && *pIsFirstFilterLen)
          {
            printf("\n# filterLen, filterOrder, Re/Cx, type, sym, ");
            for ( yc = 0; yc < NUMY; ++yc )
            {
              if (!aRunAlgo[yc] || procSmpPerSec[yc] <= 0.0)
                continue;
              if (printAsCSV)
                printf("%s, ", convText[yc]);
            }
            *pIsFirstFilterLen = 0;
          }

          for ( yc = 0; yc < NUMY; ++yc )
          {
            if (!yc)
            {
              double filterExp = log10((double)FILTERLEN) / log10(2.0);
              printf("\n%5d, %5.1f, %s, %s, %s, ", FILTERLEN, filterExp,
                     ((convFlags & PFFASTCONV_CPLX_INP_OUT)?"cplx":"real"),
                     (convFlags & PFFASTCONV_CPLX_INP_OUT)?((convFlags & PFFASTCONV_CPLX_SINGLE_FFT)?" single":" 2x") : "",
                     ((convFlags & PFFASTCONV_SYMMETRIC)?"symmetric":"non-sym")
                     );
            }
            if (!aRunAlgo[yc] || procSmpPerSec[yc] <= 0.0)
              continue;
            if (printAsCSV)
              printf("%.0f, ", procSmpPerSec[yc] * 0.001);
            else
              printf("algo '%s':\t%.2f MSmp\tin\t%.1f ms\t= %g kSmpPerSec\t%d iters\t%d loops\n",
                     convText[yc], (double)outN[yc]/(1000.0 * 1000.0), 1000.0 * aDuration[yc], procSmpPerSec[yc] * 0.001, aNumIters[yc], aNumLoops[yc] );
          }
        }

      }
    }


    for ( yc = 1; yc < NUMY; ++yc )
    {
      const float * Yref;
      const float * Ycurr;
      int outMin;

      if (!aRunAlgo[yc])
        continue;

      if (printDbg)
        printf("\n");

      if ( outN[yc] == 0 )
      {
        if (!printAsCSV)
          printf("output size 0: '%s' not implemented\n", convText[yc]);
      }
      else if ( outN[0] != outN[yc] /* && aFastAlgo[yc] */ && testOutLen )
      {
        if (!iPrintedRefOutLen)
        {
          printf("reference output size = %" PRId64 ", delta to (cplx) input length = %" PRId64 " smp\n", outN[0], (len / cplxFactor) - outN[0]);
          iPrintedRefOutLen = 1;
        }
        printf("output size doesn't match!: ref (FILTERLEN %d) returned %" PRId64 " smp, '%s' returned %" PRId64 " smp : delta = %" PRId64 " smp\n",
          FILTERLEN, outN[0], convText[yc], outN[yc], outN[yc] - outN[0] );
        retErr = 1;
      }

      posMaxErr = 0;
      maxErr = -1.0;
      Yref = Y[0];
      Ycurr = Y[yc];
      outMin = ( outN[yc] < outN[0] ) ? outN[yc] : outN[0];
      numErrOverLimit = 0;
      for ( i = 0; i < outMin; ++i )
      {
        if ( numErrOverLimit < 6 && fabs(Ycurr[i] - Yref[i]) >= yErrLimit && printErrValues )
        {
          printf("algo '%s': at %d: ***ERROR*** = %f, errLimit = %f, ref = %f, actual = %f\n",
            convText[yc], i, fabs(Ycurr[i] - Yref[i]), yErrLimit, Yref[i], Ycurr[i] );
          ++numErrOverLimit;
        }

        if ( fabs(Ycurr[i] - Yref[i]) > maxErr )
        {
          maxErr = fabsf(Ycurr[i] - Yref[i]);
          posMaxErr = i;
        }
      }

      if ( printDbg || (iMaxSpeedSlowAlgo == i) || (iMaxSpeedFastAlgo == i) )
        printf("max difference for '%s' is %g at sample idx %d of max inp 4093-1 == %f %%\n", convText[yc], maxErr, posMaxErr, maxErr * 100.0 / 4092.0 );
    }

    break;
  }

  pffastconv_free(X);
  for ( i=0; i < NUMY; ++i)
  {
    if ( 1 || i < 2 )
      pffastconv_free( Y[i] );
    if (!aRunAlgo[i])
      continue;
    aDestroy[i]( aSetupCfg[i] );
  }

  pffastconv_free(H);

  return retErr;
}

/* small functions inside pffft.c that will detect (compiler) bugs with respect to simd instructions */
void validate_pffft_simd();
int  validate_pffft_simd_ex(FILE * DbgOut);


int main(int argc, char **argv)
{
  int result = 0;
  int i, k, M, flagsA, flagsB, flagsC, testOutLen, printDbg, printSpeed;
  int testOutLens = 1, benchConv = 1, quickTest = 0, slowTest = 0;
  int testReal = 1, testCplx = 1, testSymetric = 0, abortFirstFastAlgo = 1, printErrValues = 0, printAsCSV = 1;
  int isFirstFilterLen = 1;

  for ( i = 1; i < argc; ++i ) {

    if (!strcmp(argv[i], "--test-simd")) {
      int numErrs = validate_pffft_simd_ex(stdout);
      fprintf( ( numErrs != 0 ? stderr : stdout ), "validate_pffft_simd_ex() returned %d errors!\n", numErrs);
      return ( numErrs > 0 ? 1 : 0 );
    }

    if (!strcmp(argv[i], "--no-len")) {
      testOutLens = 0;
    }
    else if (!strcmp(argv[i], "--no-bench")) {
      benchConv = 0;
    }
    else if (!strcmp(argv[i], "--quick")) {
      quickTest = 1;
    }
    else if (!strcmp(argv[i], "--slow")) {
      slowTest = 1;
    }
    else if (!strcmp(argv[i], "--real")) {
      testCplx = 0;
    }
    else if (!strcmp(argv[i], "--cplx")) {
      testReal = 0;
    }
    else if (!strcmp(argv[i], "--sym")) {
      testSymetric = 1;
    }
    else /* if (!strcmp(argv[i], "--help")) */ {
      printf("usage: %s [--test-simd] [--no-len] [--no-bench] [--quick|--slow] [--real|--cplx] [--sym]\n", argv[0]);
      exit(1);
    }
  }


  if (testOutLens)
  {
    for ( k = 0; k < 3; ++k )
    {
      if ( (k == 0 && !testReal) || (k > 0 && !testCplx) )
        continue;
      printf("\n\n==========\n");
      printf("testing %s %s output lengths ..\n", (k == 0 ? "real" : "cplx"), ( k == 0 ? "" : (k==1 ? "2x" : "single") ) );
      printf("==========\n");
      flagsA = (k == 0) ? 0 : PFFASTCONV_CPLX_INP_OUT;
      flagsB = flagsA | ( testSymetric ? PFFASTCONV_SYMMETRIC : 0 );
      flagsC = flagsB | PFFASTCONV_CPLX_SINGLE_FFT;
      testOutLen = 1;
      printDbg = 0;
      printSpeed = 0;
      for ( M = 128 - 4; M <= (quickTest ? 128+16 : 256); ++M )
      {
        if ( (M % 16) != 0 && testSymetric )
          continue;
        result |= test(M, flagsB, testOutLen, printDbg, printSpeed, abortFirstFastAlgo, printErrValues, 0, &isFirstFilterLen);
      }
    }
  }

  if (benchConv)
  {
      printf("quickTest is %d\n", quickTest);
      printf("slowTest is %d\n", slowTest);

    for ( k = 0; k < 3; ++k )
    {
      if ( (k == 0 && !testReal) || (k > 0 && !testCplx) )
        continue;
      if (!printAsCSV)
      {
        printf("\n\n==========\n");
        printf("starting %s %s benchmark against linear convolutions ..\n", (k == 0 ? "real" : "cplx"), ( k == 0 ? "" : (k==1 ? "2x" : "single") ) );
        printf("==========\n");
      }
      flagsA = (k == 0) ? 0 : PFFASTCONV_CPLX_INP_OUT;
      flagsB = flagsA | ( testSymetric ? PFFASTCONV_SYMMETRIC : 0 );
      flagsC = flagsB | ( k == 2 ? PFFASTCONV_CPLX_SINGLE_FFT : 0 );
      testOutLen = 0;
      printDbg = 0;
      printSpeed = 1;
      if (!slowTest) {
        if (!quickTest) {
          result |= test(32, flagsC, testOutLen, printDbg, printSpeed, abortFirstFastAlgo, printErrValues, printAsCSV, &isFirstFilterLen);
          result |= test(32 + 16, flagsC, testOutLen, printDbg, printSpeed, abortFirstFastAlgo, printErrValues, printAsCSV, &isFirstFilterLen);
        }
        result |= test(64, flagsC, testOutLen, printDbg, printSpeed, abortFirstFastAlgo, printErrValues, printAsCSV, &isFirstFilterLen);
        if (!quickTest) {
          result |= test(64 + 32, flagsC, testOutLen, printDbg, printSpeed, abortFirstFastAlgo, printErrValues, printAsCSV, &isFirstFilterLen);
          result |= test(128, flagsC, testOutLen, printDbg, printSpeed, abortFirstFastAlgo, printErrValues, printAsCSV, &isFirstFilterLen);
        }
      }
      if (!quickTest) {
        result |= test(128+ 64, flagsC, testOutLen, printDbg, printSpeed, abortFirstFastAlgo, printErrValues, printAsCSV, &isFirstFilterLen);
        result |= test(256,     flagsC, testOutLen, printDbg, printSpeed, abortFirstFastAlgo, printErrValues, printAsCSV, &isFirstFilterLen);
        result |= test(256+128, flagsC, testOutLen, printDbg, printSpeed, abortFirstFastAlgo, printErrValues, printAsCSV, &isFirstFilterLen);
        result |= test(512,     flagsC, testOutLen, printDbg, printSpeed, abortFirstFastAlgo, printErrValues, printAsCSV, &isFirstFilterLen);
        result |= test(1024,    flagsC, testOutLen, printDbg, printSpeed, abortFirstFastAlgo, printErrValues, printAsCSV, &isFirstFilterLen);

        result |= test(2048,    flagsC, testOutLen, printDbg, printSpeed, abortFirstFastAlgo, printErrValues, printAsCSV, &isFirstFilterLen);
        result |= test(4096,    flagsC, testOutLen, printDbg, printSpeed, abortFirstFastAlgo, printErrValues, printAsCSV, &isFirstFilterLen);
        result |= test(8192,    flagsC, testOutLen, printDbg, printSpeed, abortFirstFastAlgo, printErrValues, printAsCSV, &isFirstFilterLen);
        result |= test(16384,   flagsC, testOutLen, printDbg, printSpeed, abortFirstFastAlgo, printErrValues, printAsCSV, &isFirstFilterLen);
        result |= test(32768,   flagsC, testOutLen, printDbg, printSpeed, abortFirstFastAlgo, printErrValues, printAsCSV, &isFirstFilterLen);
      }
      if (printAsCSV)
        printf("\n");
    }
  }

  return result;
}

