#pragma once

/* for measurement of CPU cycles ..
 *
 * requires
 *   sudo apt-get install libpapi-dev papi-tools
 * on debian/ubuntu linux distributions
 *
 */

#ifdef HAVE_PAPI
#include <papi.h>
#endif

#include <stdio.h>


struct papi_perf_counter
{
    papi_perf_counter()
        : realTime(0.0F), processTime(0.0F), instructions(0LL), ipc(0.0F)
        , started(false), finished(false), print_at_destruction(false)
    { }

    papi_perf_counter(int _start, bool print_at_destruction_ = true)
        : print_at_destruction(print_at_destruction_)
    {
        (void)_start;
        start();
    }

    ~papi_perf_counter()
    {
        if (print_at_destruction)
            print(stderr);
    }

    bool start()
    {
        static bool reported_start_error = false;
#ifdef HAVE_PAPI
        int ret = PAPI_ipc(&realTime, &processTime, &instructions, &ipc);
        if (ret && !reported_start_error)
        {
            reported_start_error = true;
            fprintf(stderr, "papi_perf_counter::start(): PAPI_ipc() returned error %d\n", ret);
        }
#else
        if (!reported_start_error)
        {
            reported_start_error = true;
            fprintf(stderr, "papi_perf_counter::start(): no HAVE_PAPI\n");
        }
        int ret = 1;
#endif
        started = (!ret);
        finished = false;
        return started;
    }

    bool finish()
    {
        papi_perf_counter end(1, false);
        if (started && !finished && end.started)
        {
            realTime = end.realTime - realTime;
            processTime = end.processTime - processTime;
            instructions = end.instructions - instructions;
            ipc = end.ipc;
            finished = true;
            return true;
        }
        return false;
    }

    void print(FILE *f = stdout)
    {
        if (started && !finished)
            finish();
        if (!started || !finished)
            return;
        double cycles = instructions / ipc;
        fprintf(f, "real %g, process %g, instructions %lld, ins/cycle %f => cycles %g\n"
                , realTime, processTime, instructions, ipc, cycles
                );
        started = false;
    }

    float realTime;
    float processTime;
    long long instructions;
    float ipc;
    bool started;
    bool finished;
    bool print_at_destruction;
};

