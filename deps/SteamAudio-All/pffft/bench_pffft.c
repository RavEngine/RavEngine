/*
  Copyright (c) 2013 Julien Pommier.
  Copyright (c) 2019 Hayati Ayguen ( h_ayguen@web.de )

  Small test & bench for PFFFT, comparing its performance with the scalar FFTPACK, FFTW, Intel MKL, and Apple vDSP

  How to build: 

  on linux, with fftw3:
  gcc -o test_pffft -DHAVE_FFTW -msse -mfpmath=sse -O3 -Wall -W pffft.c test_pffft.c fftpack.c -L/usr/local/lib -I/usr/local/include/ -lfftw3f -lm

  on macos, without fftw3:
  clang -o test_pffft -DHAVE_VECLIB -O3 -Wall -W pffft.c test_pffft.c fftpack.c -L/usr/local/lib -I/usr/local/include/ -framework Accelerate

  on macos, with fftw3:
  clang -o test_pffft -DHAVE_FFTW -DHAVE_VECLIB -O3 -Wall -W pffft.c test_pffft.c fftpack.c -L/usr/local/lib -I/usr/local/include/ -lfftw3f -framework Accelerate

  as alternative: replace clang by gcc.

  on macos, with fftw3 and Intel MKL:
  clang -o test_pffft -I /opt/intel/mkl/include -DHAVE_FFTW -DHAVE_VECLIB -DHAVE_MKL  -O3 -Wall -W pffft.c test_pffft.c fftpack.c -L/usr/local/lib -I/usr/local/include/ -lfftw3f -framework Accelerate /opt/intel/mkl/lib/libmkl_{intel_lp64,sequential,core}.a

  on windows, with visual c++:
  cl /Ox -D_USE_MATH_DEFINES /arch:SSE test_pffft.c pffft.c fftpack.c
  
  build without SIMD instructions:
  gcc -o test_pffft -DPFFFT_SIMD_DISABLE -O3 -Wall -W pffft.c test_pffft.c fftpack.c -lm

 */

#define CONCAT_TOKENS(A, B)  A ## B
#define CONCAT_THREE_TOKENS(A, B, C)  A ## B ## C

#ifdef PFFFT_ENABLE_FLOAT
#include "pffft.h"

typedef float pffft_scalar;
typedef PFFFT_Setup PFFFT_SETUP;
#define PFFFT_FUNC(F)  CONCAT_TOKENS(pffft_, F)

#else
/*
Note: adapted for double precision dynamic range version.
*/
#include "pffft_double.h"

typedef double pffft_scalar;
typedef PFFFTD_Setup PFFFT_SETUP;
#define PFFFT_FUNC(F)  CONCAT_TOKENS(pffftd_, F)
#endif

#ifdef HAVE_FFTPACK
#include "fftpack.h"
#endif

#ifdef PFFFT_ENABLE_FLOAT

#ifdef HAVE_GREEN_FFTS
#include "fftext.h"
#endif

#ifdef HAVE_KISS_FFT
#include <kiss_fft.h>
#include <kiss_fftr.h>
#endif

#endif

#ifdef HAVE_POCKET_FFT
#include <pocketfft_double.h>
#include <pocketfft_single.h>
#endif

#ifdef PFFFT_ENABLE_FLOAT
  #define POCKFFTR_PRE(R)   CONCAT_TOKENS(rffts, R)
  #define POCKFFTC_PRE(R)   CONCAT_TOKENS(cffts, R)
  #define POCKFFTR_MID(L,R) CONCAT_THREE_TOKENS(L, rffts, R)
  #define POCKFFTC_MID(L,R) CONCAT_THREE_TOKENS(L, cffts, R)
#else
  #define POCKFFTR_PRE(R)   CONCAT_TOKENS(rfft, R)
  #define POCKFFTC_PRE(R)   CONCAT_TOKENS(cfft, R)
  #define POCKFFTR_MID(L,R) CONCAT_THREE_TOKENS(L, rfft, R)
  #define POCKFFTC_MID(L,R) CONCAT_THREE_TOKENS(L, cfft, R)
#endif



#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

#ifdef HAVE_SYS_TIMES
#  include <sys/times.h>
#  include <unistd.h>
#endif

#ifdef HAVE_VECLIB
#  include <Accelerate/Accelerate.h>
#endif

#ifdef HAVE_FFTW
#  include <fftw3.h>

#ifdef PFFFT_ENABLE_FLOAT
typedef fftwf_plan FFTW_PLAN;
typedef fftwf_complex FFTW_COMPLEX;
#define FFTW_FUNC(F)  CONCAT_TOKENS(fftwf_, F)
#else
typedef fftw_plan FFTW_PLAN;
typedef fftw_complex FFTW_COMPLEX;
#define FFTW_FUNC(F)  CONCAT_TOKENS(fftw_, F)
#endif

#endif /* HAVE_FFTW */

#ifdef HAVE_MKL
#  include <mkl/mkl_dfti.h>
#endif

#ifndef M_LN2
  #define M_LN2   0.69314718055994530942  /* log_e 2 */
#endif


#define NUM_FFT_ALGOS  10
enum {
  ALGO_FFTPACK = 0,
  ALGO_VECLIB,
  ALGO_FFTW_ESTIM,
  ALGO_FFTW_AUTO,
  ALGO_GREEN,
  ALGO_KISS,
  ALGO_POCKET,
  ALGO_MKL,
  ALGO_PFFFT_U, /* = 8 */
  ALGO_PFFFT_O  /* = 9 */
};

#define NUM_TYPES      7
enum {
  TYPE_PREP = 0,         /* time for preparation in ms */
  TYPE_DUR_NS = 1,       /* time per fft in ns */
  TYPE_DUR_FASTEST = 2,  /* relative time to fastest */
  TYPE_REL_PFFFT = 3,    /* relative time to ALGO_PFFFT */
  TYPE_ITER = 4,         /* # of iterations in measurement */
  TYPE_MFLOPS = 5,       /* MFlops/sec */
  TYPE_DUR_TOT = 6       /* test duration in sec */
};
/* double tmeas[NUM_TYPES][NUM_FFT_ALGOS]; */

const char * algoName[NUM_FFT_ALGOS] = {
  "FFTPack      ",
  "vDSP (vec)   ",
  "FFTW F(estim)",
  "FFTW F(auto) ",
  "Green        ",
  "Kiss         ",
  "Pocket       ",
  "Intel MKL    ",
  "PFFFT-U(simd)",  /* unordered */
  "PFFFT (simd) "   /* ordered */
};


int compiledInAlgo[NUM_FFT_ALGOS] = {
#ifdef HAVE_FFTPACK
  1, /* "FFTPack    " */
#else
  0, /* "FFTPack    " */
#endif
#if defined(HAVE_VECLIB) && defined(PFFFT_ENABLE_FLOAT)
  1, /* "vDSP (vec) " */
#else
  0,
#endif
#if defined(HAVE_FFTW)
  1, /* "FFTW(estim)" */
  1, /* "FFTW (auto)" */
#else
  0, 0,
#endif
#if defined(HAVE_GREEN_FFTS) && defined(PFFFT_ENABLE_FLOAT)
  1, /* "Green      " */
#else
  0,
#endif
#if defined(HAVE_KISS_FFT) && defined(PFFFT_ENABLE_FLOAT)
  1, /* "Kiss       " */
#else
  0,
#endif
#if defined(HAVE_POCKET_FFT)
  1, /* "Pocket     " */
#else
  0,
#endif
#if defined(HAVE_MKL)
  1, /* "Intel MKL  " */
#else
  0,
#endif
  1, /* "PFFFT_U    " */
  1  /* "PFFFT_O    " */
};

const char * algoTableHeader[NUM_FFT_ALGOS][2] = {
{ "| real FFTPack ", "| cplx FFTPack " },
{ "|  real   vDSP ", "|  cplx   vDSP " },
{ "|real FFTWestim", "|cplx FFTWestim" },
{ "|real FFTWauto ", "|cplx FFTWauto " },
{ "|  real  Green ", "|  cplx  Green " },
{ "|  real   Kiss ", "|  cplx   Kiss " },
{ "|  real Pocket ", "|  cplx Pocket " },
{ "|  real   MKL  ", "|  cplx   MKL  " },
{ "| real PFFFT-U ", "| cplx PFFFT-U " },
{ "|  real  PFFFT ", "|  cplx  PFFFT " } };

const char * typeText[NUM_TYPES] = {
  "preparation in ms",
  "time per fft in ns",
  "relative to fastest",
  "relative to pffft",
  "measured_num_iters",
  "mflops",
  "test duration in sec"
};

const char * typeFilenamePart[NUM_TYPES] = {
  "1-preparation-in-ms",
  "2-timePerFft-in-ns",
  "3-rel-fastest",
  "4-rel-pffft",
  "5-num-iter",
  "6-mflops",
  "7-duration-in-sec"
};

#define SAVE_ALL_TYPES  0

const int saveType[NUM_TYPES] = {
  1, /* "1-preparation-in-ms" */
  0, /* "2-timePerFft-in-ns"  */
  0, /* "3-rel-fastest"       */
  1, /* "4-rel-pffft"         */
  1, /* "5-num-iter"          */
  1, /* "6-mflops"            */
  1, /* "7-duration-in-sec"   */
};


#define MAX(x,y) ((x)>(y)?(x):(y))
#define MIN(x,y) ((x)<(y)?(x):(y))

unsigned Log2(unsigned v) {
  /* we don't need speed records .. obvious way is good enough */
  /* https://graphics.stanford.edu/~seander/bithacks.html#IntegerLogObvious */
  /* Find the log base 2 of an integer with the MSB N set in O(N) operations (the obvious way):
   * unsigned v: 32-bit word to find the log base 2 of */
  unsigned r = 0; /* r will be lg(v) */
  while (v >>= 1)
  {
    r++;
  }
  return r;
}


double frand() {
  return rand()/(double)RAND_MAX;
}

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


/* compare results with the regular fftpack */
int pffft_validate_N(int N, int cplx) {

#ifdef HAVE_FFTPACK

  int Nfloat = N*(cplx?2:1);
  int Nbytes = Nfloat * sizeof(pffft_scalar);
  pffft_scalar *ref, *in, *out, *tmp, *tmp2;
  PFFFT_SETUP *s = PFFFT_FUNC(new_setup)(N, cplx ? PFFFT_COMPLEX : PFFFT_REAL);
  int pass;


  if (!s) { printf("Skipping N=%d, not supported\n", N); return 0; }
  ref = PFFFT_FUNC(aligned_malloc)(Nbytes);
  in = PFFFT_FUNC(aligned_malloc)(Nbytes);
  out = PFFFT_FUNC(aligned_malloc)(Nbytes);
  tmp = PFFFT_FUNC(aligned_malloc)(Nbytes);
  tmp2 = PFFFT_FUNC(aligned_malloc)(Nbytes);

  for (pass=0; pass < 2; ++pass) {
    float ref_max = 0;
    int k;
    /* printf("N=%d pass=%d cplx=%d\n", N, pass, cplx); */
    /* compute reference solution with FFTPACK */
    if (pass == 0) {
      fftpack_real *wrk = malloc(2*Nbytes+15*sizeof(pffft_scalar));
      for (k=0; k < Nfloat; ++k) {
        ref[k] = in[k] = (float)( frand()*2-1 );
        out[k] = 1e30F;
      }
      if (!cplx) {
        rffti(N, wrk);
        rfftf(N, ref, wrk);
        /* use our ordering for real ffts instead of the one of fftpack */
        {
          float refN=ref[N-1];
          for (k=N-2; k >= 1; --k) ref[k+1] = ref[k]; 
          ref[1] = refN;
        }
      } else {
        cffti(N, wrk);
        cfftf(N, ref, wrk);
      }
      free(wrk);
    }

    for (k = 0; k < Nfloat; ++k) ref_max = MAX(ref_max, (float)( fabs(ref[k]) ));


    /* pass 0 : non canonical ordering of transform coefficients */
    if (pass == 0) {
      /* test forward transform, with different input / output */
      PFFFT_FUNC(transform)(s, in, tmp, 0, PFFFT_FORWARD);
      memcpy(tmp2, tmp, Nbytes);
      memcpy(tmp, in, Nbytes);
      PFFFT_FUNC(transform)(s, tmp, tmp, 0, PFFFT_FORWARD);
      for (k = 0; k < Nfloat; ++k) {
        assert(tmp2[k] == tmp[k]);
      }

      /* test reordering */
      PFFFT_FUNC(zreorder)(s, tmp, out, PFFFT_FORWARD);
      PFFFT_FUNC(zreorder)(s, out, tmp, PFFFT_BACKWARD);
      for (k = 0; k < Nfloat; ++k) {
        assert(tmp2[k] == tmp[k]);
      }
      PFFFT_FUNC(zreorder)(s, tmp, out, PFFFT_FORWARD);
    } else {
      /* pass 1 : canonical ordering of transform coeffs. */
      PFFFT_FUNC(transform_ordered)(s, in, tmp, 0, PFFFT_FORWARD);
      memcpy(tmp2, tmp, Nbytes);
      memcpy(tmp, in, Nbytes);
      PFFFT_FUNC(transform_ordered)(s, tmp, tmp, 0, PFFFT_FORWARD);
      for (k = 0; k < Nfloat; ++k) {
        assert(tmp2[k] == tmp[k]);
      }
      memcpy(out, tmp, Nbytes);
    }

    {
      for (k=0; k < Nfloat; ++k) {
        if (!(fabs(ref[k] - out[k]) < 1e-3*ref_max)) {
          printf("%s forward PFFFT mismatch found for N=%d\n", (cplx?"CPLX":"REAL"), N);
          return 1;
        }
      }

      if (pass == 0) PFFFT_FUNC(transform)(s, tmp, out, 0, PFFFT_BACKWARD);
      else   PFFFT_FUNC(transform_ordered)(s, tmp, out, 0, PFFFT_BACKWARD);
      memcpy(tmp2, out, Nbytes);
      memcpy(out, tmp, Nbytes);
      if (pass == 0) PFFFT_FUNC(transform)(s, out, out, 0, PFFFT_BACKWARD);
      else   PFFFT_FUNC(transform_ordered)(s, out, out, 0, PFFFT_BACKWARD);
      for (k = 0; k < Nfloat; ++k) {
        assert(tmp2[k] == out[k]);
        out[k] *= 1.f/N;
      }
      for (k = 0; k < Nfloat; ++k) {
        if (fabs(in[k] - out[k]) > 1e-3 * ref_max) {
          printf("pass=%d, %s IFFFT does not match for N=%d\n", pass, (cplx?"CPLX":"REAL"), N); break;
          return 1;
        }
      }
    }

    /* quick test of the circular convolution in fft domain */
    {
      float conv_err = 0, conv_max = 0;

      PFFFT_FUNC(zreorder)(s, ref, tmp, PFFFT_FORWARD);
      memset(out, 0, Nbytes);
      PFFFT_FUNC(zconvolve_accumulate)(s, ref, ref, out, 1.0);
      PFFFT_FUNC(zreorder)(s, out, tmp2, PFFFT_FORWARD);
      
      for (k=0; k < Nfloat; k += 2) {
        float ar = tmp[k], ai=tmp[k+1];
        if (cplx || k > 0) {
          tmp[k] = ar*ar - ai*ai;
          tmp[k+1] = 2*ar*ai;
        } else {
          tmp[0] = ar*ar;
          tmp[1] = ai*ai;
        }
      }
      
      for (k=0; k < Nfloat; ++k) {
        float d = fabs(tmp[k] - tmp2[k]), e = fabs(tmp[k]);
        if (d > conv_err) conv_err = d;
        if (e > conv_max) conv_max = e;
      }
      if (conv_err > 1e-5*conv_max) {
        printf("zconvolve error ? %g %g\n", conv_err, conv_max);
        return 1;
      }
    }

  }

  printf("%s PFFFT is OK for N=%d\n", (cplx?"CPLX":"REAL"), N); fflush(stdout);

  PFFFT_FUNC(destroy_setup)(s);
  PFFFT_FUNC(aligned_free)(ref);
  PFFFT_FUNC(aligned_free)(in);
  PFFFT_FUNC(aligned_free)(out);
  PFFFT_FUNC(aligned_free)(tmp);
  PFFFT_FUNC(aligned_free)(tmp2);
  return 0;

#else
  return 2;
#endif /* HAVE_FFTPACK */
}

int pffft_validate(int cplx) {
  static int Ntest[] = { 16, 32, 64, 96, 128, 160, 192, 256, 288, 384, 5*96, 512, 576, 5*128, 800, 864, 1024, 2048, 2592, 4000, 4096, 12000, 36864, 0};
  int k, r;
  for (k = 0; Ntest[k]; ++k) {
    int N = Ntest[k];
    if (N == 16 && !cplx) continue;
    r = pffft_validate_N(N, cplx);
    if (r)
      return r;
  }
  return 0;
}

int array_output_format = 1;


void print_table(const char *txt, FILE *tableFile) {
  fprintf(stdout, "%s", txt);
  if (tableFile && tableFile != stdout)
    fprintf(tableFile, "%s", txt);
}

void print_table_flops(float mflops, FILE *tableFile) {
  fprintf(stdout, "|%11.0f   ", mflops);
  if (tableFile && tableFile != stdout)
    fprintf(tableFile, "|%11.0f   ", mflops);
}

void print_table_fftsize(int N, FILE *tableFile) {
  fprintf(stdout, "|%9d  ", N);
  if (tableFile && tableFile != stdout)
    fprintf(tableFile, "|%9d  ", N);
}

double show_output(const char *name, int N, int cplx, float flops, float t0, float t1, int max_iter, FILE *tableFile) {
  double T = (double)(t1-t0)/2/max_iter * 1e9;
  float mflops = flops/1e6/(t1 - t0 + 1e-16);
  if (array_output_format) {
    if (flops != -1)
      print_table_flops(mflops, tableFile);
    else
      print_table("|        n/a   ", tableFile);
  } else {
    if (flops != -1) {
      printf("N=%5d, %s %16s : %6.0f MFlops [t=%6.0f ns, %d runs]\n", N, (cplx?"CPLX":"REAL"), name, mflops, (t1-t0)/2/max_iter * 1e9, max_iter);
    }
  }
  fflush(stdout);
  return T;
}

double cal_benchmark(int N, int cplx) {
  const int log2N = Log2(N);
  int Nfloat = (cplx ? N*2 : N);
  int Nbytes = Nfloat * sizeof(pffft_scalar);
  pffft_scalar *X = PFFFT_FUNC(aligned_malloc)(Nbytes), *Y = PFFFT_FUNC(aligned_malloc)(Nbytes), *Z = PFFFT_FUNC(aligned_malloc)(Nbytes);
  double t0, t1, tstop, T, nI;
  int k, iter;

  assert( PFFFT_FUNC(is_power_of_two)(N) );
  for (k = 0; k < Nfloat; ++k) {
    X[k] = sqrtf(k+1);
  }

  /* PFFFT-U (unordered) benchmark */
  PFFFT_SETUP *s = PFFFT_FUNC(new_setup)(N, cplx ? PFFFT_COMPLEX : PFFFT_REAL);
  assert(s);
  iter = 0;
  t0 = uclock_sec();
  tstop = t0 + 0.25;  /* benchmark duration: 250 ms */
  do {
    for ( k = 0; k < 512; ++k ) {
      PFFFT_FUNC(transform)(s, X, Z, Y, PFFFT_FORWARD);
      PFFFT_FUNC(transform)(s, X, Z, Y, PFFFT_BACKWARD);
      ++iter;
    }
    t1 = uclock_sec();
  } while ( t1 < tstop );
  PFFFT_FUNC(destroy_setup)(s);
  PFFFT_FUNC(aligned_free)(X);
  PFFFT_FUNC(aligned_free)(Y);
  PFFFT_FUNC(aligned_free)(Z);

  T = ( t1 - t0 );  /* duration per fft() */
  nI = ((double)iter) * ( log2N * N );  /* number of iterations "normalized" to O(N) = N*log2(N) */
  return (nI / T);    /* normalized iterations per second */
}



void benchmark_ffts(int N, int cplx, int withFFTWfullMeas, double iterCal, double tmeas[NUM_TYPES][NUM_FFT_ALGOS], int haveAlgo[NUM_FFT_ALGOS], FILE *tableFile ) {
  const int log2N = Log2(N);
  int nextPow2N = PFFFT_FUNC(next_power_of_two)(N);
  int log2NextN = Log2(nextPow2N);
  int pffftPow2N = nextPow2N;

  int Nfloat = (cplx ? MAX(nextPow2N, pffftPow2N)*2 : MAX(nextPow2N, pffftPow2N));
  int Nmax, k, iter;
  int Nbytes = Nfloat * sizeof(pffft_scalar);

  pffft_scalar *X = PFFFT_FUNC(aligned_malloc)(Nbytes + sizeof(pffft_scalar)), *Y = PFFFT_FUNC(aligned_malloc)(Nbytes + 2*sizeof(pffft_scalar) ), *Z = PFFFT_FUNC(aligned_malloc)(Nbytes);
  double te, t0, t1, tstop, flops, Tfastest;

  const double max_test_duration = 0.150;   /* test duration 150 ms */
  double numIter = max_test_duration * iterCal / ( log2N * N );  /* number of iteration for max_test_duration */
  const int step_iter = MAX(1, ((int)(0.01 * numIter)) );  /* one hundredth */
  int max_iter = MAX(1, ((int)numIter) );  /* minimum 1 iteration */

  const float checkVal = 12345.0F;

  /* printf("benchmark_ffts(N = %d, cplx = %d): Nfloat = %d, X_mem = 0x%p, X = %p\n", N, cplx, Nfloat, X_mem, X); */

  memset( X, 0, Nfloat * sizeof(pffft_scalar) );
  if ( Nfloat < 32 ) {
    for (k = 0; k < Nfloat; k += 4)
      X[k] = sqrtf(k+1);
  } else {
    for (k = 0; k < Nfloat; k += (Nfloat/16) )
      X[k] = sqrtf(k+1);
  }

  for ( k = 0; k < NUM_TYPES; ++k )
  {
    for ( iter = 0; iter < NUM_FFT_ALGOS; ++iter )
      tmeas[k][iter] = 0.0;
  }


  /* FFTPack benchmark */
  Nmax = (cplx ? N*2 : N);
  X[Nmax] = checkVal;
#ifdef HAVE_FFTPACK
  {
    fftpack_real *wrk = malloc(2*Nbytes + 15*sizeof(pffft_scalar));
    te = uclock_sec();
    if (cplx) cffti(N, wrk);
    else      rffti(N, wrk);
    t0 = uclock_sec();
    tstop = t0 + max_test_duration;
    max_iter = 0;
    do {
      for ( k = 0; k < step_iter; ++k ) {
        if (cplx) {
          assert( X[Nmax] == checkVal );
          cfftf(N, X, wrk);
          assert( X[Nmax] == checkVal );
          cfftb(N, X, wrk);
          assert( X[Nmax] == checkVal );
        } else {
          assert( X[Nmax] == checkVal );
          rfftf(N, X, wrk);
          assert( X[Nmax] == checkVal );
          rfftb(N, X, wrk);
          assert( X[Nmax] == checkVal );
        }
        ++max_iter;
      }
      t1 = uclock_sec();
    } while ( t1 < tstop );

    free(wrk);

    flops = (max_iter*2) * ((cplx ? 5 : 2.5)*N*log((double)N)/M_LN2); /* see http://www.fftw.org/speed/method.html */
    tmeas[TYPE_ITER][ALGO_FFTPACK] = max_iter;
    tmeas[TYPE_MFLOPS][ALGO_FFTPACK] = flops/1e6/(t1 - t0 + 1e-16);
    tmeas[TYPE_DUR_TOT][ALGO_FFTPACK] = t1 - t0;
    tmeas[TYPE_DUR_NS][ALGO_FFTPACK] = show_output("FFTPack", N, cplx, flops, t0, t1, max_iter, tableFile);
    tmeas[TYPE_PREP][ALGO_FFTPACK] = (t0 - te) * 1e3;
    haveAlgo[ALGO_FFTPACK] = 1;
  }
#endif

#if defined(HAVE_VECLIB) && defined(PFFFT_ENABLE_FLOAT)
  Nmax = (cplx ? nextPow2N*2 : nextPow2N);
  X[Nmax] = checkVal;
  te = uclock_sec();
  if ( 1 || PFFFT_FUNC(is_power_of_two)(N) ) {
    FFTSetup setup;

    setup = vDSP_create_fftsetup(log2NextN, FFT_RADIX2);
    DSPSplitComplex zsamples;
    zsamples.realp = &X[0];
    zsamples.imagp = &X[Nfloat/2];
    t0 = uclock_sec();
    tstop = t0 + max_test_duration;
    max_iter = 0;
    do {
      for ( k = 0; k < step_iter; ++k ) {
        if (cplx) {
          assert( X[Nmax] == checkVal );
          vDSP_fft_zip(setup, &zsamples, 1, log2NextN, kFFTDirection_Forward);
          assert( X[Nmax] == checkVal );
          vDSP_fft_zip(setup, &zsamples, 1, log2NextN, kFFTDirection_Inverse);
          assert( X[Nmax] == checkVal );
        } else {
          assert( X[Nmax] == checkVal );
          vDSP_fft_zrip(setup, &zsamples, 1, log2NextN, kFFTDirection_Forward); 
          assert( X[Nmax] == checkVal );
          vDSP_fft_zrip(setup, &zsamples, 1, log2NextN, kFFTDirection_Inverse);
          assert( X[Nmax] == checkVal );
        }
        ++max_iter;
      }
      t1 = uclock_sec();
    } while ( t1 < tstop );

    vDSP_destroy_fftsetup(setup);
    flops = (max_iter*2) * ((cplx ? 5 : 2.5)*N*log((double)N)/M_LN2); /* see http://www.fftw.org/speed/method.html */
    tmeas[TYPE_ITER][ALGO_VECLIB] = max_iter;
    tmeas[TYPE_MFLOPS][ALGO_VECLIB] = flops/1e6/(t1 - t0 + 1e-16);
    tmeas[TYPE_DUR_TOT][ALGO_VECLIB] = t1 - t0;
    tmeas[TYPE_DUR_NS][ALGO_VECLIB] = show_output("vDSP", N, cplx, flops, t0, t1, max_iter, tableFile);
    tmeas[TYPE_PREP][ALGO_VECLIB] = (t0 - te) * 1e3;
    haveAlgo[ALGO_VECLIB] = 1;
  } else {
    show_output("vDSP", N, cplx, -1, -1, -1, -1, tableFile);
  }
#endif

#if defined(HAVE_FFTW)
  Nmax = (cplx ? N*2 : N);
  X[Nmax] = checkVal;
  {
    /* int flags = (N <= (256*1024) ? FFTW_MEASURE : FFTW_ESTIMATE);  measure takes a lot of time on largest ffts */
    int flags = FFTW_ESTIMATE;
    te = uclock_sec();

    FFTW_PLAN planf, planb;
    FFTW_COMPLEX *in = (FFTW_COMPLEX*) FFTW_FUNC(malloc)(sizeof(FFTW_COMPLEX) * N);
    FFTW_COMPLEX *out = (FFTW_COMPLEX*) FFTW_FUNC(malloc)(sizeof(FFTW_COMPLEX) * N);
    memset(in, 0, sizeof(FFTW_COMPLEX) * N);
    if (cplx) {
      planf = FFTW_FUNC(plan_dft_1d)(N, in, out, FFTW_FORWARD, flags);
      planb = FFTW_FUNC(plan_dft_1d)(N, in, out, FFTW_BACKWARD, flags);
    } else {
      planf = FFTW_FUNC(plan_dft_r2c_1d)(N, (pffft_scalar*)in, out, flags);
      planb = FFTW_FUNC(plan_dft_c2r_1d)(N, in, (pffft_scalar*)out, flags);
    }

    t0 = uclock_sec();
    tstop = t0 + max_test_duration;
    max_iter = 0;
    do {
      for ( k = 0; k < step_iter; ++k ) {
        assert( X[Nmax] == checkVal );
        FFTW_FUNC(execute)(planf);
        assert( X[Nmax] == checkVal );
        FFTW_FUNC(execute)(planb);
        assert( X[Nmax] == checkVal );
        ++max_iter;
      }
      t1 = uclock_sec();
    } while ( t1 < tstop );

    FFTW_FUNC(destroy_plan)(planf);
    FFTW_FUNC(destroy_plan)(planb);
    FFTW_FUNC(free)(in); FFTW_FUNC(free)(out);

    flops = (max_iter*2) * ((cplx ? 5 : 2.5)*N*log((double)N)/M_LN2); /* see http://www.fftw.org/speed/method.html */
    tmeas[TYPE_ITER][ALGO_FFTW_ESTIM] = max_iter;
    tmeas[TYPE_MFLOPS][ALGO_FFTW_ESTIM] = flops/1e6/(t1 - t0 + 1e-16);
    tmeas[TYPE_DUR_TOT][ALGO_FFTW_ESTIM] = t1 - t0;
    tmeas[TYPE_DUR_NS][ALGO_FFTW_ESTIM] = show_output((flags == FFTW_MEASURE ? algoName[ALGO_FFTW_AUTO] : algoName[ALGO_FFTW_ESTIM]), N, cplx, flops, t0, t1, max_iter, tableFile);
    tmeas[TYPE_PREP][ALGO_FFTW_ESTIM] = (t0 - te) * 1e3;
    haveAlgo[ALGO_FFTW_ESTIM] = 1;
  }
  Nmax = (cplx ? N*2 : N);
  X[Nmax] = checkVal;
  do {
    /* int flags = (N <= (256*1024) ? FFTW_MEASURE : FFTW_ESTIMATE);  measure takes a lot of time on largest ffts */
    /* int flags = FFTW_MEASURE; */
#if ( defined(__arm__) || defined(__aarch64__) || defined(__arm64__) )
    int limitFFTsize = 31;  /* takes over a second on Raspberry Pi 3 B+ -- and much much more on higher ffts sizes! */
#else
    int limitFFTsize = 2400;  /* take over a second on i7 for fft size 2400 */
#endif
    int flags = (N < limitFFTsize ? FFTW_MEASURE : (withFFTWfullMeas ? FFTW_MEASURE : FFTW_ESTIMATE));

    if (flags == FFTW_ESTIMATE) {
      show_output((flags == FFTW_MEASURE ? algoName[ALGO_FFTW_AUTO] : algoName[ALGO_FFTW_ESTIM]), N, cplx, -1, -1, -1, -1, tableFile);
      /* copy values from estimation */
      tmeas[TYPE_ITER][ALGO_FFTW_AUTO] = tmeas[TYPE_ITER][ALGO_FFTW_ESTIM];
      tmeas[TYPE_DUR_TOT][ALGO_FFTW_AUTO] = tmeas[TYPE_DUR_TOT][ALGO_FFTW_ESTIM];
      tmeas[TYPE_DUR_NS][ALGO_FFTW_AUTO] = tmeas[TYPE_DUR_NS][ALGO_FFTW_ESTIM];
      tmeas[TYPE_PREP][ALGO_FFTW_AUTO] = tmeas[TYPE_PREP][ALGO_FFTW_ESTIM];
    } else {
      te = uclock_sec();
      FFTW_PLAN planf, planb;
      FFTW_COMPLEX *in = (FFTW_COMPLEX*) FFTW_FUNC(malloc)(sizeof(FFTW_COMPLEX) * N);
      FFTW_COMPLEX *out = (FFTW_COMPLEX*) FFTW_FUNC(malloc)(sizeof(FFTW_COMPLEX) * N);
      memset(in, 0, sizeof(FFTW_COMPLEX) * N);
      if (cplx) {
        planf = FFTW_FUNC(plan_dft_1d)(N, in, out, FFTW_FORWARD, flags);
        planb = FFTW_FUNC(plan_dft_1d)(N, in, out, FFTW_BACKWARD, flags);
      } else {
        planf = FFTW_FUNC(plan_dft_r2c_1d)(N, (pffft_scalar*)in, out, flags);
        planb = FFTW_FUNC(plan_dft_c2r_1d)(N, in, (pffft_scalar*)out, flags);
      }

      t0 = uclock_sec();
      tstop = t0 + max_test_duration;
      max_iter = 0;
      do {
        for ( k = 0; k < step_iter; ++k ) {
          assert( X[Nmax] == checkVal );
          FFTW_FUNC(execute)(planf);
          assert( X[Nmax] == checkVal );
          FFTW_FUNC(execute)(planb);
          assert( X[Nmax] == checkVal );
          ++max_iter;
        }
        t1 = uclock_sec();
      } while ( t1 < tstop );

      FFTW_FUNC(destroy_plan)(planf);
      FFTW_FUNC(destroy_plan)(planb);
      FFTW_FUNC(free)(in); FFTW_FUNC(free)(out);

      flops = (max_iter*2) * ((cplx ? 5 : 2.5)*N*log((double)N)/M_LN2); /* see http://www.fftw.org/speed/method.html */
      tmeas[TYPE_ITER][ALGO_FFTW_AUTO] = max_iter;
      tmeas[TYPE_MFLOPS][ALGO_FFTW_AUTO] = flops/1e6/(t1 - t0 + 1e-16);
      tmeas[TYPE_DUR_TOT][ALGO_FFTW_AUTO] = t1 - t0;
      tmeas[TYPE_DUR_NS][ALGO_FFTW_AUTO] = show_output((flags == FFTW_MEASURE ? algoName[ALGO_FFTW_AUTO] : algoName[ALGO_FFTW_ESTIM]), N, cplx, flops, t0, t1, max_iter, tableFile);
      tmeas[TYPE_PREP][ALGO_FFTW_AUTO] = (t0 - te) * 1e3;
      haveAlgo[ALGO_FFTW_AUTO] = 1;
    }
  } while (0);
#else
  (void)withFFTWfullMeas;
#endif

#if defined(HAVE_GREEN_FFTS) && defined(PFFFT_ENABLE_FLOAT)
  Nmax = (cplx ? nextPow2N*2 : nextPow2N);
  X[Nmax] = checkVal;
  if ( 1 || PFFFT_FUNC(is_power_of_two)(N) )
  {
    te = uclock_sec();
    fftInit(log2NextN);

    t0 = uclock_sec();
    tstop = t0 + max_test_duration;
    max_iter = 0;
    do {
      for ( k = 0; k < step_iter; ++k ) {
        if (cplx) {
          assert( X[Nmax] == checkVal );
          ffts(X, log2NextN, 1);
          assert( X[Nmax] == checkVal );
          iffts(X, log2NextN, 1);
          assert( X[Nmax] == checkVal );
        } else {
          rffts(X, log2NextN, 1);
          riffts(X, log2NextN, 1);
        }

        ++max_iter;
      }
      t1 = uclock_sec();
    } while ( t1 < tstop );

    fftFree();

    flops = (max_iter*2) * ((cplx ? 5 : 2.5)*N*log((double)N)/M_LN2); /* see http://www.fftw.org/speed/method.html */
    tmeas[TYPE_ITER][ALGO_GREEN] = max_iter;
    tmeas[TYPE_MFLOPS][ALGO_GREEN] = flops/1e6/(t1 - t0 + 1e-16);
    tmeas[TYPE_DUR_TOT][ALGO_GREEN] = t1 - t0;
    tmeas[TYPE_DUR_NS][ALGO_GREEN] = show_output("Green", N, cplx, flops, t0, t1, max_iter, tableFile);
    tmeas[TYPE_PREP][ALGO_GREEN] = (t0 - te) * 1e3;
    haveAlgo[ALGO_GREEN] = 1;
  } else {
    show_output("Green", N, cplx, -1, -1, -1, -1, tableFile);
  }
#endif

#if defined(HAVE_KISS_FFT) && defined(PFFFT_ENABLE_FLOAT)
  Nmax = (cplx ? nextPow2N*2 : nextPow2N);
  X[Nmax] = checkVal;
  if ( 1 || PFFFT_FUNC(is_power_of_two)(N) )
  {
    kiss_fft_cfg stf;
    kiss_fft_cfg sti;
    kiss_fftr_cfg stfr;
    kiss_fftr_cfg stir;

    te = uclock_sec();
    if (cplx) {
      stf = kiss_fft_alloc(nextPow2N, 0, 0, 0);
      sti = kiss_fft_alloc(nextPow2N, 1, 0, 0);
    } else {
      stfr = kiss_fftr_alloc(nextPow2N, 0, 0, 0);
      stir = kiss_fftr_alloc(nextPow2N, 1, 0, 0);
    }

    t0 = uclock_sec();
    tstop = t0 + max_test_duration;
    max_iter = 0;
    do {
      for ( k = 0; k < step_iter; ++k ) {
        if (cplx) {
          assert( X[Nmax] == checkVal );
          kiss_fft(stf, (const kiss_fft_cpx *)X, (kiss_fft_cpx *)Y);
          assert( X[Nmax] == checkVal );
          kiss_fft(sti, (const kiss_fft_cpx *)Y, (kiss_fft_cpx *)X);
          assert( X[Nmax] == checkVal );
        } else {
          assert( X[Nmax] == checkVal );
          kiss_fftr(stfr, X, (kiss_fft_cpx *)Y);
          assert( X[Nmax] == checkVal );
          kiss_fftri(stir, (const kiss_fft_cpx *)Y, X);
          assert( X[Nmax] == checkVal );
        }
        ++max_iter;
      }
      t1 = uclock_sec();
    } while ( t1 < tstop );

    kiss_fft_cleanup();

    flops = (max_iter*2) * ((cplx ? 5 : 2.5)*N*log((double)N)/M_LN2); /* see http://www.fftw.org/speed/method.html */
    tmeas[TYPE_ITER][ALGO_KISS] = max_iter;
    tmeas[TYPE_MFLOPS][ALGO_KISS] = flops/1e6/(t1 - t0 + 1e-16);
    tmeas[TYPE_DUR_TOT][ALGO_KISS] = t1 - t0;
    tmeas[TYPE_DUR_NS][ALGO_KISS] = show_output("Kiss", N, cplx, flops, t0, t1, max_iter, tableFile);
    tmeas[TYPE_PREP][ALGO_KISS] = (t0 - te) * 1e3;
    haveAlgo[ALGO_KISS] = 1;
  } else {
    show_output("Kiss", N, cplx, -1, -1, -1, -1, tableFile);
  }
#endif

#if defined(HAVE_POCKET_FFT)

  Nmax = (cplx ? nextPow2N*2 : nextPow2N);
  X[Nmax] = checkVal;
  if ( 1 || PFFFT_FUNC(is_power_of_two)(N) )
  {
    POCKFFTR_PRE(_plan) planr;
    POCKFFTC_PRE(_plan) planc;

    te = uclock_sec();
    if (cplx) {
      planc = POCKFFTC_MID(make_,_plan)(nextPow2N);
    } else {
      planr = POCKFFTR_MID(make_,_plan)(nextPow2N);
    }

    t0 = uclock_sec();
    tstop = t0 + max_test_duration;
    max_iter = 0;
    do {
      for ( k = 0; k < step_iter; ++k ) {
        if (cplx) {
          assert( X[Nmax] == checkVal );
          memcpy(Y, X, 2*nextPow2N * sizeof(pffft_scalar) );
          POCKFFTC_PRE(_forward)(planc, Y, 1.);
          assert( X[Nmax] == checkVal );
          memcpy(X, Y, 2*nextPow2N * sizeof(pffft_scalar) );
          POCKFFTC_PRE(_backward)(planc, X, 1./nextPow2N);
          assert( X[Nmax] == checkVal );
        } else {
          assert( X[Nmax] == checkVal );
          memcpy(Y, X, nextPow2N * sizeof(pffft_scalar) );
          POCKFFTR_PRE(_forward)(planr, Y, 1.);
          assert( X[Nmax] == checkVal );
          memcpy(X, Y, nextPow2N * sizeof(pffft_scalar) );
          POCKFFTR_PRE(_backward)(planr, X, 1./nextPow2N);
          assert( X[Nmax] == checkVal );
        }
        ++max_iter;
      }
      t1 = uclock_sec();
    } while ( t1 < tstop );

    if (cplx) {
      POCKFFTC_MID(destroy_,_plan)(planc);
    } else {
      POCKFFTR_MID(destroy_,_plan)(planr);
    }

    flops = (max_iter*2) * ((cplx ? 5 : 2.5)*N*log((double)N)/M_LN2); /* see http://www.fftw.org/speed/method.html */
    tmeas[TYPE_ITER][ALGO_POCKET] = max_iter;
    tmeas[TYPE_MFLOPS][ALGO_POCKET] = flops/1e6/(t1 - t0 + 1e-16);
    tmeas[TYPE_DUR_TOT][ALGO_POCKET] = t1 - t0;
    tmeas[TYPE_DUR_NS][ALGO_POCKET] = show_output("Pocket", N, cplx, flops, t0, t1, max_iter, tableFile);
    tmeas[TYPE_PREP][ALGO_POCKET] = (t0 - te) * 1e3;
    haveAlgo[ALGO_POCKET] = 1;
  } else {
    show_output("Pocket", N, cplx, -1, -1, -1, -1, tableFile);
  }
#endif


#if defined(HAVE_MKL)
  {
    DFTI_DESCRIPTOR_HANDLE fft_handle;
    MKL_LONG mkl_status, mkl_ret;
    te = uclock_sec();
    if (sizeof(float) == sizeof(pffft_scalar))
      mkl_status = DftiCreateDescriptor(&fft_handle, DFTI_SINGLE, (cplx ? DFTI_COMPLEX : DFTI_REAL), 1, N);
    else if (sizeof(double) == sizeof(pffft_scalar))
      mkl_status = DftiCreateDescriptor(&fft_handle, DFTI_DOUBLE, (cplx ? DFTI_COMPLEX : DFTI_REAL), 1, N);
    else
      mkl_status = 1;

    while (mkl_status == 0) {
      mkl_ret = DftiSetValue(fft_handle, DFTI_PLACEMENT, DFTI_NOT_INPLACE);
      if (mkl_ret) {
        DftiFreeDescriptor(&fft_handle);
        mkl_status = 1;
        break;
      }
      mkl_ret = DftiCommitDescriptor(fft_handle);
      if (mkl_ret) {
        DftiFreeDescriptor(&fft_handle);
        mkl_status = 1;
        break;
      }
      break;
    }

    if (mkl_status == 0) {
      t0 = uclock_sec();
      tstop = t0 + max_test_duration;
      max_iter = 0;

      do {
        for ( k = 0; k < step_iter; ++k ) {
          assert( X[Nmax] == checkVal );
          DftiComputeForward(fft_handle, &X[0], &Y[0]);
          assert( X[Nmax] == checkVal );
          DftiComputeBackward(fft_handle, &X[0], &Y[0]);
          assert( X[Nmax] == checkVal );
          ++max_iter;
        }
        t1 = uclock_sec();
      } while ( t1 < tstop );

      DftiFreeDescriptor(&fft_handle);

      flops = (max_iter*2) * ((cplx ? 5 : 2.5)*N*log((double)N)/M_LN2); /* see http://www.fftw.org/speed/method.html */
      tmeas[TYPE_ITER][ALGO_MKL] = max_iter;
      tmeas[TYPE_MFLOPS][ALGO_MKL] = flops/1e6/(t1 - t0 + 1e-16);
      tmeas[TYPE_DUR_TOT][ALGO_MKL] = t1 - t0;
      tmeas[TYPE_DUR_NS][ALGO_MKL] = show_output("MKL", N, cplx, flops, t0, t1, max_iter, tableFile);
      tmeas[TYPE_PREP][ALGO_MKL] = (t0 - te) * 1e3;
      haveAlgo[ALGO_MKL] = 1;
    } else {
      show_output("MKL", N, cplx, -1, -1, -1, -1, tableFile);
    }
  }
#endif

  /* PFFFT-U (unordered) benchmark */
  Nmax = (cplx ? pffftPow2N*2 : pffftPow2N);
  X[Nmax] = checkVal;
  if ( pffftPow2N >= PFFFT_FUNC(min_fft_size)(cplx ? PFFFT_COMPLEX : PFFFT_REAL) )
  {
    te = uclock_sec();
    PFFFT_SETUP *s = PFFFT_FUNC(new_setup)(pffftPow2N, cplx ? PFFFT_COMPLEX : PFFFT_REAL);
    if (s) {
      t0 = uclock_sec();
      tstop = t0 + max_test_duration;
      max_iter = 0;
      do {
        for ( k = 0; k < step_iter; ++k ) {
          assert( X[Nmax] == checkVal );
          PFFFT_FUNC(transform)(s, X, Z, Y, PFFFT_FORWARD);
          assert( X[Nmax] == checkVal );
          PFFFT_FUNC(transform)(s, X, Z, Y, PFFFT_BACKWARD);
          assert( X[Nmax] == checkVal );
          ++max_iter;
        }
        t1 = uclock_sec();
      } while ( t1 < tstop );

      PFFFT_FUNC(destroy_setup)(s);

      flops = (max_iter*2) * ((cplx ? 5 : 2.5)*N*log((double)N)/M_LN2); /* see http://www.fftw.org/speed/method.html */
      tmeas[TYPE_ITER][ALGO_PFFFT_U] = max_iter;
      tmeas[TYPE_MFLOPS][ALGO_PFFFT_U] = flops/1e6/(t1 - t0 + 1e-16);
      tmeas[TYPE_DUR_TOT][ALGO_PFFFT_U] = t1 - t0;
      tmeas[TYPE_DUR_NS][ALGO_PFFFT_U] = show_output("PFFFT-U", N, cplx, flops, t0, t1, max_iter, tableFile);
      tmeas[TYPE_PREP][ALGO_PFFFT_U] = (t0 - te) * 1e3;
      haveAlgo[ALGO_PFFFT_U] = 1;
    }
  } else {
    show_output("PFFFT-U", N, cplx, -1, -1, -1, -1, tableFile);
  }


  if ( pffftPow2N >= PFFFT_FUNC(min_fft_size)(cplx ? PFFFT_COMPLEX : PFFFT_REAL) )
  {
    te = uclock_sec();
    PFFFT_SETUP *s = PFFFT_FUNC(new_setup)(pffftPow2N, cplx ? PFFFT_COMPLEX : PFFFT_REAL);
    if (s) {
      t0 = uclock_sec();
      tstop = t0 + max_test_duration;
      max_iter = 0;
      do {
        for ( k = 0; k < step_iter; ++k ) {
          assert( X[Nmax] == checkVal );
          PFFFT_FUNC(transform_ordered)(s, X, Z, Y, PFFFT_FORWARD);
          assert( X[Nmax] == checkVal );
          PFFFT_FUNC(transform_ordered)(s, X, Z, Y, PFFFT_BACKWARD);
          assert( X[Nmax] == checkVal );
          ++max_iter;
        }
        t1 = uclock_sec();
      } while ( t1 < tstop );

      PFFFT_FUNC(destroy_setup)(s);

      flops = (max_iter*2) * ((cplx ? 5 : 2.5)*N*log((double)N)/M_LN2); /* see http://www.fftw.org/speed/method.html */
      tmeas[TYPE_ITER][ALGO_PFFFT_O] = max_iter;
      tmeas[TYPE_MFLOPS][ALGO_PFFFT_O] = flops/1e6/(t1 - t0 + 1e-16);
      tmeas[TYPE_DUR_TOT][ALGO_PFFFT_O] = t1 - t0;
      tmeas[TYPE_DUR_NS][ALGO_PFFFT_O] = show_output("PFFFT", N, cplx, flops, t0, t1, max_iter, tableFile);
      tmeas[TYPE_PREP][ALGO_PFFFT_O] = (t0 - te) * 1e3;
      haveAlgo[ALGO_PFFFT_O] = 1;
    }
  } else {
    show_output("PFFFT", N, cplx, -1, -1, -1, -1, tableFile);
  }

  if (!array_output_format)
  {
    printf("prepare/ms:     ");
    for ( iter = 0; iter < NUM_FFT_ALGOS; ++iter )
    {
      if ( haveAlgo[iter] && tmeas[TYPE_DUR_NS][iter] > 0.0 ) {
        printf("%s %.3f    ", algoName[iter], tmeas[TYPE_PREP][iter] );
      }
    }
    printf("\n");
  }
  Tfastest = 0.0;
  for ( iter = 0; iter < NUM_FFT_ALGOS; ++iter )
  {
    if ( Tfastest == 0.0 || ( tmeas[TYPE_DUR_NS][iter] != 0.0 && tmeas[TYPE_DUR_NS][iter] < Tfastest ) )
      Tfastest = tmeas[TYPE_DUR_NS][iter];
  }
  if ( Tfastest > 0.0 )
  {
    if (!array_output_format)
      printf("relative fast:  ");
    for ( iter = 0; iter < NUM_FFT_ALGOS; ++iter )
    {
      if ( haveAlgo[iter] && tmeas[TYPE_DUR_NS][iter] > 0.0 ) {
        tmeas[TYPE_DUR_FASTEST][iter] = tmeas[TYPE_DUR_NS][iter] / Tfastest;
        if (!array_output_format)
          printf("%s %.3f    ", algoName[iter], tmeas[TYPE_DUR_FASTEST][iter] );
      }
    }
    if (!array_output_format)
      printf("\n");
  }

  {
    if (!array_output_format)
      printf("relative pffft: ");
    for ( iter = 0; iter < NUM_FFT_ALGOS; ++iter )
    {
      if ( haveAlgo[iter] && tmeas[TYPE_DUR_NS][iter] > 0.0 ) {
        tmeas[TYPE_REL_PFFFT][iter] = tmeas[TYPE_DUR_NS][iter] / tmeas[TYPE_DUR_NS][ALGO_PFFFT_O];
        if (!array_output_format)
          printf("%s %.3f    ", algoName[iter], tmeas[TYPE_REL_PFFFT][iter] );
      }
    }
    if (!array_output_format)
      printf("\n");
  }

  if (!array_output_format) {
    printf("--\n");
  }

  PFFFT_FUNC(aligned_free)(X);
  PFFFT_FUNC(aligned_free)(Y);
  PFFFT_FUNC(aligned_free)(Z);
}


/* small functions inside pffft.c that will detect (compiler) bugs with respect to simd instructions */
void validate_pffft_simd();
int  validate_pffft_simd_ex(FILE * DbgOut);
void validate_pffftd_simd();
int  validate_pffftd_simd_ex(FILE * DbgOut);



int main(int argc, char **argv) {
  /* unfortunately, the fft size must be a multiple of 16 for complex FFTs 
     and 32 for real FFTs -- a lot of stuff would need to be rewritten to
     handle other cases (or maybe just switch to a scalar fft, I don't know..) */

#if 0  /* include powers of 2 ? */
#define NUMNONPOW2LENS  23
  int NnonPow2[NUMNONPOW2LENS] = {
    64, 96, 128, 160, 192,   256, 384, 5*96, 512, 5*128,
    3*256, 800, 1024, 2048, 2400,   4096, 8192, 9*1024, 16384, 32768,
    256*1024, 1024*1024, -1 };
#else
#define NUMNONPOW2LENS  11
  int NnonPow2[NUMNONPOW2LENS] = {
    96, 160, 192, 384, 5*96,   5*128,3*256, 800, 2400, 9*1024,
    -1 };
#endif

#define NUMPOW2FFTLENS  22
#define MAXNUMFFTLENS MAX( NUMPOW2FFTLENS, NUMNONPOW2LENS )
  int Npow2[NUMPOW2FFTLENS];  /* exp = 1 .. 21, -1 */
  const int *Nvalues = NULL;
  double tmeas[2][MAXNUMFFTLENS][NUM_TYPES][NUM_FFT_ALGOS];
  double iterCalReal = 0.0, iterCalCplx = 0.0;

  int benchReal=1, benchCplx=1, withFFTWfullMeas=0, outputTable2File=1, usePow2=1;
  int max_N = 1024 * 1024 * 2;
  int quicktest = 0;
  int realCplxIdx, typeIdx;
  int i, k;
  FILE *tableFile = NULL;

  int haveAlgo[NUM_FFT_ALGOS];
  char acCsvFilename[64];

  for ( k = 1; k <= NUMPOW2FFTLENS; ++k )
    Npow2[k-1] = (k == NUMPOW2FFTLENS) ? -1 : (1 << k);
  Nvalues = Npow2;  /* set default .. for comparisons .. */

  for ( i = 0; i < NUM_FFT_ALGOS; ++i )
    haveAlgo[i] = 0;

  printf("pffft architecture:    '%s'\n", PFFFT_FUNC(simd_arch)());
  printf("pffft SIMD size:       %d\n", PFFFT_FUNC(simd_size)());
  printf("pffft min real fft:    %d\n", PFFFT_FUNC(min_fft_size)(PFFFT_REAL));
  printf("pffft min complex fft: %d\n", PFFFT_FUNC(min_fft_size)(PFFFT_COMPLEX));
  printf("\n");

  for ( i = 1; i < argc; ++i ) {
    if (!strcmp(argv[i], "--array-format") || !strcmp(argv[i], "--table")) {
      array_output_format = 1;
    }
    else if (!strcmp(argv[i], "--no-tab")) {
      array_output_format = 0;
    }
    else if (!strcmp(argv[i], "--real")) {
      benchCplx = 0;
    }
    else if (!strcmp(argv[i], "--cplx")) {
      benchReal = 0;
    }
    else if (!strcmp(argv[i], "--fftw-full-measure")) {
      withFFTWfullMeas = 1;
    }
    else if (!strcmp(argv[i], "--non-pow2")) {
      Nvalues = NnonPow2;
      usePow2 = 0;
    }
    else if (!strcmp(argv[i], "--max-len") && i+1 < argc) {
      max_N = atoi(argv[i+1]);
      ++i;
    }
    else if (!strcmp(argv[i], "--quick")) {
      fprintf(stdout, "actived quicktest mode\n");
      quicktest = 1;
    }
    else if (!strcmp(argv[i], "--validate")) {
#ifdef HAVE_FFTPACK
      int r;
      fprintf(stdout, "validating PFFFT against %s FFTPACK ..\n", (benchCplx ? "complex" : "real"));
      r = pffft_validate(benchCplx);
      fprintf((r ? stderr : stderr), "pffft %s\n", (r ? "validation failed!" : "successful"));
      return r;
#else
      fprintf(stderr, "validation not available without FFTPACK!\n");
#endif
      return 0;
    }
    else /* if (!strcmp(argv[i], "--help")) */ {
      printf("usage: %s [--array-format|--table] [--no-tab] [--real|--cplx] [--validate] [--fftw-full-measure] [--non-pow2] [--max-len <N>] [--quick]\n", argv[0]);
      exit(0);
    }
  }

#ifdef HAVE_FFTW
#ifdef PFFFT_ENABLE_DOUBLE
  algoName[ALGO_FFTW_ESTIM] = "FFTW D(estim)";
  algoName[ALGO_FFTW_AUTO]  = "FFTW D(auto) ";
#endif

  if (withFFTWfullMeas)
  {
#ifdef PFFFT_ENABLE_FLOAT
    algoName[ALGO_FFTW_AUTO] = "FFTWF(meas)"; /* "FFTW (auto)" */
#else
    algoName[ALGO_FFTW_AUTO] = "FFTWD(meas)"; /* "FFTW (auto)" */
#endif
    algoTableHeader[ALGO_FFTW_AUTO][0] = "|real FFTWmeas "; /* "|real FFTWauto " */
    algoTableHeader[ALGO_FFTW_AUTO][1] = "|cplx FFTWmeas "; /* "|cplx FFTWauto " */
  }
#endif

  if ( PFFFT_FUNC(simd_size)() == 1 )
  {
    algoName[ALGO_PFFFT_U] = "PFFFTU scal-1";
    algoName[ALGO_PFFFT_O] = "PFFFT scal-1 ";
  }
  else if ( !strcmp(PFFFT_FUNC(simd_arch)(), "4xScalar") )
  {
    algoName[ALGO_PFFFT_U] = "PFFFT-U scal4";
    algoName[ALGO_PFFFT_O] = "PFFFT scal-4 ";
  }


  clock();
  /* double TClockDur = 1.0 / CLOCKS_PER_SEC;
  printf("clock() duration for CLOCKS_PER_SEC = %f sec = %f ms\n", TClockDur, 1000.0 * TClockDur );
  */

  /* calibrate test duration */
  if (!quicktest)
  {
    double t0, t1, dur;
    printf("calibrating fft benchmark duration at size N = 512 ..\n");
    t0 = uclock_sec();
    if (benchReal) {
      iterCalReal = cal_benchmark(512, 0 /* real fft */);
      printf("real fft iterCal = %f\n", iterCalReal);
    }
    if (benchCplx) {
      iterCalCplx = cal_benchmark(512, 1 /* cplx fft */);
      printf("cplx fft iterCal = %f\n", iterCalCplx);
    }
    t1 = uclock_sec();
    dur = t1 - t0;
    printf("calibration done in %f sec.\n\n", dur);
  }

  if (!array_output_format) {
    if (benchReal) {
      for (i=0; Nvalues[i] > 0 && Nvalues[i] <= max_N; ++i)
        benchmark_ffts(Nvalues[i], 0 /* real fft */, withFFTWfullMeas, iterCalReal, tmeas[0][i], haveAlgo, NULL);
    }
    if (benchCplx) {
      for (i=0; Nvalues[i] > 0 && Nvalues[i] <= max_N; ++i)
        benchmark_ffts(Nvalues[i], 1 /* cplx fft */, withFFTWfullMeas, iterCalCplx, tmeas[1][i], haveAlgo, NULL);
    }

  } else {

    if (outputTable2File) {
      tableFile = fopen( usePow2 ? "bench-fft-table-pow2.txt" : "bench-fft-table-non2.txt", "w");
    }
    /* print table headers */
    printf("table shows MFlops; higher values indicate faster computation\n\n");

    {
      print_table("| input len ", tableFile);
      for (realCplxIdx = 0; realCplxIdx < 2; ++realCplxIdx)
      {
        if ( (realCplxIdx == 0 && !benchReal) || (realCplxIdx == 1 && !benchCplx) )
          continue;
        for (k=0; k < NUM_FFT_ALGOS; ++k)
        {
          if ( compiledInAlgo[k] )
            print_table(algoTableHeader[k][realCplxIdx], tableFile);
        }
      }
      print_table("|\n", tableFile);
    }
    /* print table value seperators */
    {
      print_table("|----------", tableFile);
      for (realCplxIdx = 0; realCplxIdx < 2; ++realCplxIdx)
      {
        if ( (realCplxIdx == 0 && !benchReal) || (realCplxIdx == 1 && !benchCplx) )
          continue;
        for (k=0; k < NUM_FFT_ALGOS; ++k)
        {
          if ( compiledInAlgo[k] )
            print_table(":|-------------", tableFile);
        }
      }
      print_table(":|\n", tableFile);
    }

    for (i=0; Nvalues[i] > 0 && Nvalues[i] <= max_N; ++i) {
      {
        double t0, t1;
        print_table_fftsize(Nvalues[i], tableFile);
        t0 = uclock_sec();
        if (benchReal)
          benchmark_ffts(Nvalues[i], 0, withFFTWfullMeas, iterCalReal, tmeas[0][i], haveAlgo, tableFile);
        if (benchCplx)
          benchmark_ffts(Nvalues[i], 1, withFFTWfullMeas, iterCalCplx, tmeas[1][i], haveAlgo, tableFile);
        t1 = uclock_sec();
        print_table("|\n", tableFile);
        /* printf("all ffts for size %d took %f sec\n", Nvalues[i], t1-t0); */
        (void)t0;
        (void)t1;
      }
    }
    fprintf(stdout, " (numbers are given in MFlops)\n");
    if (outputTable2File) {
      fclose(tableFile);
    }
  }

  printf("\n");
  printf("now writing .csv files ..\n");

  for (realCplxIdx = 0; realCplxIdx < 2; ++realCplxIdx)
  {
    if ( (benchReal && realCplxIdx == 0) || (benchCplx && realCplxIdx == 1) )
    {
      for (typeIdx = 0; typeIdx < NUM_TYPES; ++typeIdx)
      {
        FILE *f = NULL;
        if ( !(SAVE_ALL_TYPES || saveType[typeIdx]) )
          continue;
        acCsvFilename[0] = 0;
#ifdef PFFFT_SIMD_DISABLE
        strcat(acCsvFilename, "scal-");
#else
        strcat(acCsvFilename, "simd-");
#endif
        strcat(acCsvFilename, (realCplxIdx == 0 ? "real-" : "cplx-"));
        strcat(acCsvFilename, ( usePow2 ? "pow2-" : "non2-"));
        assert( strlen(acCsvFilename) + strlen(typeFilenamePart[typeIdx]) + 5 < (sizeof(acCsvFilename) / sizeof(acCsvFilename[0])) );
        strcat(acCsvFilename, typeFilenamePart[typeIdx]);
        strcat(acCsvFilename, ".csv");
        f = fopen(acCsvFilename, "w");
        if (!f)
          continue;
        {
          fprintf(f, "size, log2, ");
          for (k=0; k < NUM_FFT_ALGOS; ++k)
            if ( haveAlgo[k] )
              fprintf(f, "%s, ", algoName[k]);
          fprintf(f, "\n");
        }
        for (i=0; Nvalues[i] > 0 && Nvalues[i] <= max_N; ++i)
        {
          {
            fprintf(f, "%d, %.3f, ", Nvalues[i], log10((double)Nvalues[i])/log10(2.0) );
            for (k=0; k < NUM_FFT_ALGOS; ++k)
              if ( haveAlgo[k] )
                fprintf(f, "%f, ", tmeas[realCplxIdx][i][typeIdx][k]);
            fprintf(f, "\n");
          }
        }
        fclose(f);
      }
    }
  }

  return 0;
}

