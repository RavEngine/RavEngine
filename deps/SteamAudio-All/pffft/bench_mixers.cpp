/*
  Copyright (c) 2020  Hayati Ayguen ( h_ayguen@web.de )

  bench for mixer algorithm/implementations

 */

#include <pf_mixer.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

#include "papi_perf_counter.h"

#if defined(__linux__)
#define HAVE_SYS_TIMES
#endif

#ifdef HAVE_SYS_TIMES
#  include <sys/times.h>
#  include <unistd.h>
#endif

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>
#endif

#define BENCH_REF_TRIG_FUNC       1
#define BENCH_OUT_OF_PLACE_ALGOS  0
#define BENCH_INPLACE_ALGOS       1

#define SAVE_BY_DEFAULT  0
#define SAVE_LIMIT_MSPS           16

#if 0
  #define BENCH_FILE_SHIFT_MATH_CC           "/home/ayguen/WindowsDesktop/mixer_test/A_shift_math_cc.bin"
  #define BENCH_FILE_ADD_FAST_CC             "/home/ayguen/WindowsDesktop/mixer_test/C_shift_addfast_cc.bin"
  #define BENCH_FILE_ADD_FAST_INP_C          "/home/ayguen/WindowsDesktop/mixer_test/C_shift_addfast_inp_c.bin"
  #define BENCH_FILE_UNROLL_INP_C            "/home/ayguen/WindowsDesktop/mixer_test/D_shift_unroll_inp_c.bin"
  #define BENCH_FILE_LTD_UNROLL_INP_C        "/home/ayguen/WindowsDesktop/mixer_test/E_shift_limited_unroll_inp_c.bin"
  #define BENCH_FILE_LTD_UNROLL_A_SSE_INP_C  "/home/ayguen/WindowsDesktop/mixer_test/F_shift_limited_unroll_A_sse_inp_c.bin"
  #define BENCH_FILE_LTD_UNROLL_B_SSE_INP_C  "/home/ayguen/WindowsDesktop/mixer_test/G_shift_limited_unroll_B_sse_inp_c.bin"
  #define BENCH_FILE_LTD_UNROLL_C_SSE_INP_C  "/home/ayguen/WindowsDesktop/mixer_test/H_shift_limited_unroll_C_sse_inp_c.bin"
  #define BENCH_FILE_REC_OSC_CC              ""
  #define BENCH_FILE_REC_OSC_INP_C           "/home/ayguen/WindowsDesktop/mixer_test/I_shift_recursive_osc_inp_c.bin"
  #define BENCH_FILE_REC_OSC_SSE_INP_C       "/home/ayguen/WindowsDesktop/mixer_test/J_shift_recursive_osc_sse_inp_c.bin"
#else
  #define BENCH_FILE_SHIFT_MATH_CC           ""
  #define BENCH_FILE_ADD_FAST_CC             ""
  #define BENCH_FILE_ADD_FAST_INP_C          ""
  #define BENCH_FILE_UNROLL_INP_C            ""
  #define BENCH_FILE_LTD_UNROLL_INP_C        ""
  #define BENCH_FILE_LTD_UNROLL_A_SSE_INP_C  ""
  #define BENCH_FILE_LTD_UNROLL_B_SSE_INP_C  ""
  #define BENCH_FILE_LTD_UNROLL_C_SSE_INP_C  ""
  #define BENCH_FILE_REC_OSC_CC              ""
  #define BENCH_FILE_REC_OSC_INP_C           ""
  #define BENCH_FILE_REC_OSC_SSE_INP_C       ""
#endif



#if defined(HAVE_SYS_TIMES)
    static double ttclk = 0.;

    static double uclock_sec(int find_start)
    {
        struct tms t0, t;
        if (ttclk == 0.)
        {
            ttclk = sysconf(_SC_CLK_TCK);
            fprintf(stderr, "sysconf(_SC_CLK_TCK) => %f\n", ttclk);
        }
        times(&t);
        if (find_start)
        {
            t0 = t;
            while (t0.tms_utime == t.tms_utime)
                times(&t);
        }
        /* use only the user time of this process - not realtime, which depends on OS-scheduler .. */
        return ((double)t.tms_utime) / ttclk;
    }

#elif defined(WIN32)
    // https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getprocesstimes
    double uclock_sec(int find_start)
    {
        FILETIME a, b, c, d;
        if (GetProcessTimes(GetCurrentProcess(), &a, &b, &c, &d) != 0)
        {
            //  Returns total user time.
            //  Can be tweaked to include kernel times as well.
            return
                (double)(d.dwLowDateTime |
                    ((unsigned long long)d.dwHighDateTime << 32)) * 0.0000001;
        }
        else {
            //  Handle error
            return 0;
        }
    }

#else
    double uclock_sec(int find_start)
    { return (double)clock()/(double)CLOCKS_PER_SEC; }
#endif


void save(complexf * d, int B, int N, const char * fn)
{
    if (!fn || !fn[0])
    {
        if (! SAVE_BY_DEFAULT)
            return;
        fn = "/dev/shm/bench.bin";
    }
    FILE * f = fopen(fn, "wb");
    if (!f) {
        fprintf(stderr, "error writing result to %s\n", fn);
        return;
    }
    if ( N >= SAVE_LIMIT_MSPS * 1024 * 1024 )
        N = SAVE_LIMIT_MSPS * 1024 * 1024;
    for (int off = 0; off + B <= N; off += B)
    {
        fwrite(d+off, sizeof(complexf), B, f);
    }
    fclose(f);
}


double bench_core_shift_math_cc(
        const int B, const int N, const bool ignore_time,
        const complexf *input,
        complexf *output,
        int &iters_out, int &off_out
        )
{
    const double t0 = uclock_sec(1);
    const double tstop = t0 + 0.5;  /* benchmark duration: 500 ms */
    double t1;
    float phase = 0.0F;
    int off = 0, iter = 0;
    papi_perf_counter perf_counter(1);

    do {
        // work
        phase = shift_math_cc(input+off, output+off, B, -0.0009F, phase);
        off += B;
        ++iter;
        t1 = uclock_sec(0);
    } while ( off + B < N && (ignore_time || t1 < tstop) );

    iters_out = iter;
    off_out = off;
    return t1 - t0;
}

double bench_shift_math_cc(const int B, const int N, const bool ignore_time) {
    int iter, off;
    complexf *input = (complexf *)malloc(N * sizeof(complexf));
    complexf *output = (complexf *)malloc(N * sizeof(complexf));
    shift_recursive_osc_t gen_state;
    shift_recursive_osc_conf_t gen_conf;

    shift_recursive_osc_init(0.001F, 0.0F, &gen_conf, &gen_state);
    gen_recursive_osc_c(input, N, &gen_conf, &gen_state);

    double T = bench_core_shift_math_cc(B, N, ignore_time, input, output,  iter, off);

    save(output, B, off, BENCH_FILE_SHIFT_MATH_CC);

    free(input);
    free(output);
    printf("processed %f Msamples in %f ms\n", off * 1E-6, T*1E3);
    double nI = ((double)iter) * B;  /* number of iterations "normalized" to O(N) = N */
    return (nI / T);    /* normalized iterations per second */
}


double bench_shift_table_cc(int B, int N) {
    double t0, t1, tstop, T, nI;
    int iter, off;
    int table_size=65536;
    float phase = 0.0F;
    complexf *input = (complexf *)malloc(N * sizeof(complexf));
    complexf *output = (complexf *)malloc(N * sizeof(complexf));
    shift_recursive_osc_t gen_state;
    shift_recursive_osc_conf_t gen_conf;

    shift_table_data_t table_data = shift_table_init(table_size);

    shift_recursive_osc_init(0.001F, 0.0F, &gen_conf, &gen_state);
    gen_recursive_osc_c(input, N, &gen_conf, &gen_state);

    iter = 0;
    off = 0;
    t0 = uclock_sec(1);
    tstop = t0 + 0.5;  /* benchmark duration: 500 ms */
    do {
        // work
        phase = shift_table_cc(input+off, output+off, B, -0.0009F, table_data, phase);

        off += B;
        ++iter;
        t1 = uclock_sec(0);
    } while ( t1 < tstop && off + B < N );

    save(output, B, off, NULL);
    free(input);
    free(output);
    T = ( t1 - t0 );  /* duration per fft() */
    printf("processed %f Msamples in %f ms\n", off * 1E-6, T*1E3);
    nI = ((double)iter) * B;  /* number of iterations "normalized" to O(N) = N */
    return (nI / T);    /* normalized iterations per second */
}


double bench_shift_addfast(int B, int N) {
    double t0, t1, tstop, T, nI;
    int iter, off;
    float phase = 0.0F;
    complexf *input = (complexf *)malloc(N * sizeof(complexf));
    complexf *output = (complexf *)malloc(N * sizeof(complexf));
    shift_recursive_osc_t gen_state;
    shift_recursive_osc_conf_t gen_conf;
    shift_addfast_data_t state = shift_addfast_init(-0.0009F);

    shift_recursive_osc_init(0.001F, 0.0F, &gen_conf, &gen_state);
    gen_recursive_osc_c(input, N, &gen_conf, &gen_state);

    iter = 0;
    off = 0;
    t0 = uclock_sec(1);
    tstop = t0 + 0.5;  /* benchmark duration: 500 ms */
    do {
        // work
        phase = shift_addfast_cc(input+off, output+off, B, &state, phase);

        off += B;
        ++iter;
        t1 = uclock_sec(0);
    } while ( t1 < tstop && off + B < N );

    save(output, B, off, BENCH_FILE_ADD_FAST_CC);

    free(input);
    free(output);
    T = ( t1 - t0 );  /* duration per fft() */
    printf("processed %f Msamples in %f ms\n", off * 1E-6, T*1E3);
    nI = ((double)iter) * B;  /* number of iterations "normalized" to O(N) = N */
    return (nI / T);    /* normalized iterations per second */
}


double bench_core_shift_addfast_inplace(
        const int B, const int N, const bool ignore_time,
        complexf *data,
        shift_addfast_data_t &state,
        int &iters_out, int &off_out
        )
{
    const double t0 = uclock_sec(1);
    const double tstop = t0 + 0.5;  /* benchmark duration: 500 ms */
    double t1;
    float phase = 0.0F;
    int off = 0, iter = 0;
    papi_perf_counter perf_counter(1);

    do {
        // work
        phase = shift_addfast_inp_c(data+off, B, &state, phase);
        off += B;
        ++iter;
        t1 = uclock_sec(0);
    } while ( off + B < N && (ignore_time || t1 < tstop) );

    iters_out = iter;
    off_out = off;
    return t1 - t0;
}

double bench_shift_addfast_inp(int B, int N, const bool ignore_time) {
    complexf *input = (complexf *)malloc(N * sizeof(complexf));
    shift_recursive_osc_t gen_state;
    shift_recursive_osc_conf_t gen_conf;
    shift_addfast_data_t state = shift_addfast_init(-0.0009F);
    int iter, off;

    shift_recursive_osc_init(0.001F, 0.0F, &gen_conf, &gen_state);
    gen_recursive_osc_c(input, N, &gen_conf, &gen_state);

    double T = bench_core_shift_addfast_inplace(
                B, N, ignore_time, input, state,
                iter, off
                );

    save(input, B, off, BENCH_FILE_ADD_FAST_INP_C);

    free(input);
    printf("processed %f Msamples in %f ms\n", off * 1E-6, T*1E3);
    double nI = ((double)iter) * B;  /* number of iterations "normalized" to O(N) = N */
    return (nI / T);    /* normalized iterations per second */
}


double bench_shift_unroll_oop(int B, int N) {
    double t0, t1, tstop, T, nI;
    int iter, off;
    float phase = 0.0F;
    complexf *input = (complexf *)malloc(N * sizeof(complexf));
    complexf *output = (complexf *)malloc(N * sizeof(complexf));
    shift_recursive_osc_t gen_state;
    shift_recursive_osc_conf_t gen_conf;
    shift_unroll_data_t state = shift_unroll_init(-0.0009F, B);

    shift_recursive_osc_init(0.001F, 0.0F, &gen_conf, &gen_state);
    gen_recursive_osc_c(input, N, &gen_conf, &gen_state);

    iter = 0;
    off = 0;
    t0 = uclock_sec(1);
    tstop = t0 + 0.5;  /* benchmark duration: 500 ms */
    do {
        // work
        phase = shift_unroll_cc(input+off, output+off, B, &state, phase);

        off += B;
        ++iter;
        t1 = uclock_sec(0);
    } while ( t1 < tstop && off + B < N );

    save(output, B, off, NULL);
    free(input);
    free(output);
    T = ( t1 - t0 );  /* duration per fft() */
    printf("processed %f Msamples in %f ms\n", off * 1E-6, T*1E3);
    nI = ((double)iter) * B;  /* number of iterations "normalized" to O(N) = N */
    return (nI / T);    /* normalized iterations per second */
}


double bench_core_shift_unroll_inplace(
        const int B, const int N, const bool ignore_time,
        complexf *data,
        shift_unroll_data_t &state,
        int &iters_out, int &off_out
        )
{
    const double t0 = uclock_sec(1);
    const double tstop = t0 + 0.5;  /* benchmark duration: 500 ms */
    double t1;
    float phase = 0.0F;
    int off = 0, iter = 0;
    papi_perf_counter perf_counter(1);

    do {
        // work
        phase = shift_unroll_inp_c(data+off, B, &state, phase);
        off += B;
        ++iter;
        t1 = uclock_sec(0);
    } while ( off + B < N && (ignore_time || t1 < tstop) );

    iters_out = iter;
    off_out = off;
    return t1 - t0;
}

double bench_shift_unroll_inp(const int B, const int N, const bool ignore_time) {
    complexf *input = (complexf *)malloc(N * sizeof(complexf));
    shift_recursive_osc_t gen_state;
    shift_recursive_osc_conf_t gen_conf;
    shift_unroll_data_t state = shift_unroll_init(-0.0009F, B);
    int iter, off;

    shift_recursive_osc_init(0.001F, 0.0F, &gen_conf, &gen_state);
    gen_recursive_osc_c(input, N, &gen_conf, &gen_state);

    double T = bench_core_shift_unroll_inplace(
                B, N, ignore_time, input, state,
                iter, off
                );

    save(input, B, off, BENCH_FILE_UNROLL_INP_C);

    free(input);
    shift_unroll_deinit(&state);
    printf("processed %f Msamples in %f ms\n", off * 1E-6, T*1E3);
    double nI = ((double)iter) * B;  /* number of iterations "normalized" to O(N) = N */
    return (nI / T);    /* normalized iterations per second */
}



double bench_shift_limited_unroll_oop(int B, int N) {
    double t0, t1, tstop, T, nI;
    int iter, off;
    complexf *input = (complexf *)malloc(N * sizeof(complexf));
    complexf *output = (complexf *)malloc(N * sizeof(complexf));
    shift_recursive_osc_t gen_state;
    shift_recursive_osc_conf_t gen_conf;
    shift_limited_unroll_data_t state = shift_limited_unroll_init(-0.0009F);

    shift_recursive_osc_init(0.001F, 0.0F, &gen_conf, &gen_state);
    gen_recursive_osc_c(input, N, &gen_conf, &gen_state);

    iter = 0;
    off = 0;
    t0 = uclock_sec(1);
    tstop = t0 + 0.5;  /* benchmark duration: 500 ms */
    do {
        // work
        shift_limited_unroll_cc(input+off, output+off, B, &state);

        off += B;
        ++iter;
        t1 = uclock_sec(0);
    } while ( t1 < tstop && off + B < N );

    save(output, B, off, NULL);
    free(input);
    free(output);
    T = ( t1 - t0 );  /* duration per fft() */
    printf("processed %f Msamples in %f ms\n", off * 1E-6, T*1E3);
    nI = ((double)iter) * B;  /* number of iterations "normalized" to O(N) = N */
    return (nI / T);    /* normalized iterations per second */
}


double bench_core_shift_limited_unroll_inplace(
        const int B, const int N, const bool ignore_time,
        complexf *data,
        shift_limited_unroll_data_t &state,
        int &iters_out, int &off_out
        )
{
    const double t0 = uclock_sec(1);
    const double tstop = t0 + 0.5;  /* benchmark duration: 500 ms */
    double t1;
    int off = 0, iter = 0;
    papi_perf_counter perf_counter(1);

    do {
        // work
        shift_limited_unroll_inp_c(data+off, B, &state);
        off += B;
        ++iter;
        t1 = uclock_sec(0);
    } while ( off + B < N && (ignore_time || t1 < tstop) );

    iters_out = iter;
    off_out = off;
    return t1 - t0;
}

double bench_shift_limited_unroll_inp(const int B, const int N, const bool ignore_time) {
    complexf *input = (complexf *)malloc(N * sizeof(complexf));
    shift_recursive_osc_t gen_state;
    shift_recursive_osc_conf_t gen_conf;
    shift_limited_unroll_data_t state = shift_limited_unroll_init(-0.0009F);
    int iter, off;

    shift_recursive_osc_init(0.001F, 0.0F, &gen_conf, &gen_state);
    gen_recursive_osc_c(input, N, &gen_conf, &gen_state);

    double T = bench_core_shift_limited_unroll_inplace(
                B, N, ignore_time, input, state,
                iter, off
                );

    save(input, B, off, BENCH_FILE_LTD_UNROLL_INP_C);

    free(input);
    printf("processed %f Msamples in %f ms\n", off * 1E-6, T*1E3);
    double nI = ((double)iter) * B;  /* number of iterations "normalized" to O(N) = N */
    return (nI / T);    /* normalized iterations per second */
}


double bench_core_shift_limited_unroll_A_sse_inplace(
        const int B, const int N, const bool ignore_time,
        complexf *data,
        shift_limited_unroll_A_sse_data_t &state,
        int &iters_out, int &off_out
        )
{
    const double t0 = uclock_sec(1);
    const double tstop = t0 + 0.5;  /* benchmark duration: 500 ms */
    double t1;
    int off = 0, iter = 0;
    papi_perf_counter perf_counter(1);

    do {
        // work
        shift_limited_unroll_A_sse_inp_c(data+off, B, &state);
        off += B;
        ++iter;
        t1 = uclock_sec(0);
    } while ( off + B < N && (ignore_time || t1 < tstop) );

    iters_out = iter;
    off_out = off;
    return t1 - t0;
}

double bench_shift_limited_unroll_A_sse_inp(const int B, const int N, const bool ignore_time) {
    complexf *input = (complexf *)malloc(N * sizeof(complexf));
    shift_recursive_osc_t gen_state;
    shift_recursive_osc_conf_t gen_conf;
    shift_limited_unroll_A_sse_data_t *state = (shift_limited_unroll_A_sse_data_t*)malloc(sizeof(shift_limited_unroll_A_sse_data_t));
    int iter, off;

    *state = shift_limited_unroll_A_sse_init(-0.0009F, 0.0F);

    shift_recursive_osc_init(0.001F, 0.0F, &gen_conf, &gen_state);
    gen_recursive_osc_c(input, N, &gen_conf, &gen_state);

    double T = bench_core_shift_limited_unroll_A_sse_inplace(
                B, N, ignore_time, input, *state,
                iter, off
                );

    save(input, B, off, BENCH_FILE_LTD_UNROLL_A_SSE_INP_C);

    free(input);
    free(state);
    printf("processed %f Msamples in %f ms\n", off * 1E-6, T*1E3);
    double nI = ((double)iter) * B;  /* number of iterations "normalized" to O(N) = N */
    return (nI / T);    /* normalized iterations per second */
}


double bench_core_shift_limited_unroll_B_sse_inplace(
        const int B, const int N, const bool ignore_time,
        complexf *data,
        shift_limited_unroll_B_sse_data_t &state,
        int &iters_out, int &off_out
        )
{
    const double t0 = uclock_sec(1);
    const double tstop = t0 + 0.5;  /* benchmark duration: 500 ms */
    double t1;
    int off = 0, iter = 0;
    papi_perf_counter perf_counter(1);

    do {
        // work
        shift_limited_unroll_B_sse_inp_c(data+off, B, &state);
        off += B;
        ++iter;
        t1 = uclock_sec(0);
    } while ( off + B < N && (ignore_time || t1 < tstop) );

    iters_out = iter;
    off_out = off;
    return t1 - t0;
}

double bench_shift_limited_unroll_B_sse_inp(const int B, const int N, const bool ignore_time) {
    complexf *input = (complexf *)malloc(N * sizeof(complexf));
    shift_recursive_osc_t gen_state;
    shift_recursive_osc_conf_t gen_conf;
    shift_limited_unroll_B_sse_data_t *state = (shift_limited_unroll_B_sse_data_t*)malloc(sizeof(shift_limited_unroll_B_sse_data_t));
    int iter, off;

    *state = shift_limited_unroll_B_sse_init(-0.0009F, 0.0F);

    shift_recursive_osc_init(0.001F, 0.0F, &gen_conf, &gen_state);
    //shift_recursive_osc_init(0.0F, 0.0F, &gen_conf, &gen_state);
    gen_recursive_osc_c(input, N, &gen_conf, &gen_state);

    double T = bench_core_shift_limited_unroll_B_sse_inplace(
                B, N, ignore_time, input, *state,
                iter, off
                );

    save(input, B, off, BENCH_FILE_LTD_UNROLL_B_SSE_INP_C);
    
    free(input);
    free(state);
    printf("processed %f Msamples in %f ms\n", off * 1E-6, T*1E3);
    double nI = ((double)iter) * B;  /* number of iterations "normalized" to O(N) = N */
    return (nI / T);    /* normalized iterations per second */
}


double bench_core_shift_limited_unroll_C_sse_inplace(
        const int B, const int N, const bool ignore_time,
        complexf *data,
        shift_limited_unroll_C_sse_data_t &state,
        int &iters_out, int &off_out
        )
{
    const double t0 = uclock_sec(1);
    const double tstop = t0 + 0.5;  /* benchmark duration: 500 ms */
    double t1;
    int off = 0, iter = 0;
    papi_perf_counter perf_counter(1);

    do {
        // work
        shift_limited_unroll_C_sse_inp_c(data+off, B, &state);
        off += B;
        ++iter;
        t1 = uclock_sec(0);
    } while ( off + B < N && (ignore_time || t1 < tstop) );

    iters_out = iter;
    off_out = off;
    return t1 - t0;
}

double bench_shift_limited_unroll_C_sse_inp(const int B, const int N, const bool ignore_time) {
    complexf *input = (complexf *)malloc(N * sizeof(complexf));
    shift_recursive_osc_t gen_state;
    shift_recursive_osc_conf_t gen_conf;
    shift_limited_unroll_C_sse_data_t *state = (shift_limited_unroll_C_sse_data_t*)malloc(sizeof(shift_limited_unroll_C_sse_data_t));
    int iter, off;

    *state = shift_limited_unroll_C_sse_init(-0.0009F, 0.0F);

    shift_recursive_osc_init(0.001F, 0.0F, &gen_conf, &gen_state);
    gen_recursive_osc_c(input, N, &gen_conf, &gen_state);

    double T = bench_core_shift_limited_unroll_C_sse_inplace(
                B, N, ignore_time, input, *state,
                iter, off
                );

    save(input, B, off, BENCH_FILE_LTD_UNROLL_C_SSE_INP_C);

    free(input);
    free(state);
    printf("processed %f Msamples in %f ms\n", off * 1E-6, T*1E3);
    double nI = ((double)iter) * B;  /* number of iterations "normalized" to O(N) = N */
    return (nI / T);    /* normalized iterations per second */
}


double bench_shift_rec_osc_cc_oop(int B, int N) {
    double t0, t1, tstop, T, nI;
    int iter, off;
    complexf *input = (complexf *)malloc(N * sizeof(complexf));
    complexf *output = (complexf *)malloc(N * sizeof(complexf));
    shift_recursive_osc_t gen_state, shift_state;
    shift_recursive_osc_conf_t gen_conf, shift_conf;

    shift_recursive_osc_init(-0.0009F, 0.0F, &shift_conf, &shift_state);
    shift_recursive_osc_init(0.001F, 0.0F, &gen_conf, &gen_state);
    gen_recursive_osc_c(input, N, &gen_conf, &gen_state);

    iter = 0;
    off = 0;
    t0 = uclock_sec(1);
    tstop = t0 + 0.5;  /* benchmark duration: 500 ms */
    do {
        // work
        shift_recursive_osc_cc(input+off, output+off, B, &shift_conf, &shift_state);

        off += B;
        ++iter;
        t1 = uclock_sec(0);
    } while ( t1 < tstop && off + B < N );

    save(input, B, off, BENCH_FILE_REC_OSC_CC);

    save(output, B, off, NULL);
    free(input);
    free(output);
    T = ( t1 - t0 );  /* duration per fft() */
    printf("processed %f Msamples in %f ms\n", off * 1E-6, T*1E3);
    nI = ((double)iter) * B;  /* number of iterations "normalized" to O(N) = N */
    return (nI / T);    /* normalized iterations per second */
}


double bench_core_shift_rec_osc_cc_inplace(
        const int B, const int N, const bool ignore_time,
        complexf *data,
        shift_recursive_osc_conf_t &conf, shift_recursive_osc_t &state,
        int &iters_out, int &off_out
        )
{
    const double t0 = uclock_sec(1);
    const double tstop = t0 + 0.5;  /* benchmark duration: 500 ms */
    double t1;
    int off = 0, iter = 0;
    papi_perf_counter perf_counter(1);

    do {
        // work
        shift_recursive_osc_inp_c(data+off, B, &conf, &state);
        off += B;
        ++iter;
        t1 = uclock_sec(0);
    } while ( off + B < N && (ignore_time || t1 < tstop) );

    iters_out = iter;
    off_out = off;
    return t1 - t0;
}

double bench_shift_rec_osc_cc_inp(const int B, const int N, const bool ignore_time) {
    complexf *input = (complexf *)malloc(N * sizeof(complexf));
    shift_recursive_osc_t gen_state, shift_state;
    shift_recursive_osc_conf_t gen_conf, shift_conf;
    int iter, off;

    shift_recursive_osc_init(0.001F, 0.0F, &gen_conf, &gen_state);
    gen_recursive_osc_c(input, N, &gen_conf, &gen_state);
    shift_recursive_osc_init(-0.0009F, 0.0F, &shift_conf, &shift_state);

    double T = bench_core_shift_rec_osc_cc_inplace(
                B, N, ignore_time, input, shift_conf, shift_state,
                iter, off
                );

    save(input, B, off, BENCH_FILE_REC_OSC_INP_C);
    free(input);
    printf("processed %f Msamples in %f ms\n", off * 1E-6, T*1E3);
    double nI = ((double)iter) * B;  /* number of iterations "normalized" to O(N) = N */
    return (nI / T);    /* normalized iterations per second */
}


double bench_core_shift_rec_osc_sse_c_inplace(
        const int B, const int N, const bool ignore_time,
        complexf *data,
        shift_recursive_osc_sse_conf_t &conf, shift_recursive_osc_sse_t &state,
        int &iters_out, int &off_out
        )
{
    const double t0 = uclock_sec(1);
    const double tstop = t0 + 0.5;  /* benchmark duration: 500 ms */
    double t1;
    int off = 0, iter = 0;
    papi_perf_counter perf_counter(1);

    do {
        // work
        shift_recursive_osc_sse_inp_c(data+off, B, &conf, &state);
        off += B;
        ++iter;
        t1 = uclock_sec(0);
    } while ( off + B < N && (ignore_time || t1 < tstop) );

    iters_out = iter;
    off_out = off;
    return t1 - t0;
}

double bench_shift_rec_osc_sse_c_inp(const int B, const int N, const bool ignore_time) {
    complexf *input = (complexf *)malloc(N * sizeof(complexf));
    shift_recursive_osc_t gen_state;
    shift_recursive_osc_conf_t gen_conf;

    shift_recursive_osc_sse_t *shift_state = (shift_recursive_osc_sse_t*)malloc(sizeof(shift_recursive_osc_sse_t));
    shift_recursive_osc_sse_conf_t shift_conf;
    int iter, off;

    shift_recursive_osc_init(0.001F, 0.0F, &gen_conf, &gen_state);
    gen_recursive_osc_c(input, N, &gen_conf, &gen_state);

    shift_recursive_osc_sse_init(-0.0009F, 0.0F, &shift_conf, shift_state);

    double T = bench_core_shift_rec_osc_sse_c_inplace(
                B, N, ignore_time, input, shift_conf, *shift_state,
                iter, off
                );

    save(input, B, off, BENCH_FILE_REC_OSC_SSE_INP_C);
    free(input);
    free(shift_state);
    printf("processed %f Msamples in %f ms\n", off * 1E-6, T*1E3);
    double nI = ((double)iter) * B;  /* number of iterations "normalized" to O(N) = N */
    return (nI / T);    /* normalized iterations per second */
}



int main(int argc, char **argv)
{
    double rt;

    // process up to 64 MSample (512 MByte) in blocks of 8 kSamples (=64 kByte)
    int B = 8 * 1024;
    int N = 64 * 1024 * 1024;
    int showUsage = 0;
    bool ignore_time = true;

    if (argc == 1)
        showUsage = 1;

    if (1 < argc)
        B = atoi(argv[1]);
    if (2 < argc)
        N = atoi(argv[2]) * 1024 * 1024;

    if ( !B || !N || showUsage )
    {
        fprintf(stderr, "%s [<blockLength in samples> [<total # of MSamples>] ]\n", argv[0]);
        if ( !B || !N )
            return 0;
    }

    fprintf(stderr, "processing up to N = %d MSamples with block length of %d samples\n",
        N / (1024 * 1024), B );


#if BENCH_REF_TRIG_FUNC
    printf("\nstarting bench of shift_math_cc (out-of-place) with trig functions ..\n");
    rt = bench_shift_math_cc(B, N, ignore_time);
    printf("  %f MSamples/sec\n\n", rt * 1E-6);
#endif

#if BENCH_OUT_OF_PLACE_ALGOS
    printf("starting bench of shift_table_cc (out-of-place) ..\n");
    rt = bench_shift_table_cc(B, N);
    printf("  %f MSamples/sec\n\n", rt * 1E-6);

    printf("starting bench of shift_addfast_cc (out-of-place) ..\n");
    rt = bench_shift_addfast(B, N);
    printf("  %f MSamples/sec\n\n", rt * 1E-6);

    printf("\nstarting bench of shift_unroll_cc (out-of-place) ..\n");
    rt = bench_shift_unroll_oop(B, N);
    printf("  %f MSamples/sec\n\n", rt * 1E-6);

    printf("\nstarting bench of shift_limited_unroll_cc (out-of-place) ..\n");
    rt = bench_shift_limited_unroll_oop(B, N);
    printf("  %f MSamples/sec\n\n", rt * 1E-6);

    printf("\nstarting bench of shift_recursive_osc_cc (out-of-place) ..\n");
    rt = bench_shift_rec_osc_cc_oop(B, N);
    printf("  %f MSamples/sec\n\n", rt * 1E-6);
#endif

#if BENCH_INPLACE_ALGOS

    printf("starting bench of shift_addfast_inp_c in-place ..\n");
    rt = bench_shift_addfast_inp(B, N, ignore_time);
    printf("  %f MSamples/sec\n\n", rt * 1E-6);

    printf("starting bench of shift_unroll_inp_c in-place ..\n");
    rt = bench_shift_unroll_inp(B, N, ignore_time);
    printf("  %f MSamples/sec\n\n", rt * 1E-6);

    printf("starting bench of shift_limited_unroll_inp_c in-place ..\n");
    rt = bench_shift_limited_unroll_inp(B, N, ignore_time);
    printf("  %f MSamples/sec\n\n", rt * 1E-6);

    if ( have_sse_shift_mixer_impl() )
    {
        printf("starting bench of shift_limited_unroll_A_sse_inp_c in-place ..\n");
        rt = bench_shift_limited_unroll_A_sse_inp(B, N, ignore_time);
        printf("  %f MSamples/sec\n\n", rt * 1E-6);

        printf("starting bench of shift_limited_unroll_B_sse_inp_c in-place ..\n");
        rt = bench_shift_limited_unroll_B_sse_inp(B, N, ignore_time);
        printf("  %f MSamples/sec\n\n", rt * 1E-6);

        printf("starting bench of shift_limited_unroll_C_sse_inp_c in-place ..\n");
        rt = bench_shift_limited_unroll_C_sse_inp(B, N, ignore_time);
        printf("  %f MSamples/sec\n\n", rt * 1E-6);
    }

    printf("starting bench of shift_recursive_osc_cc in-place ..\n");
    rt = bench_shift_rec_osc_cc_inp(B, N, ignore_time);
    printf("  %f MSamples/sec\n\n", rt * 1E-6);

    if ( have_sse_shift_mixer_impl() )
    {
        printf("starting bench of shift_recursive_osc_sse_c in-place ..\n");
        rt = bench_shift_rec_osc_sse_c_inp(B, N, ignore_time);
        printf("  %f MSamples/sec\n\n", rt * 1E-6);
    }
#endif

    return 0;
}

