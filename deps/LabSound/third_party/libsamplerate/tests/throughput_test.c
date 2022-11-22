/*
** Copyright (c) 2004-2016, Erik de Castro Lopo <erikd@mega-nerd.com>
** All rights reserved.
**
** This code is released under 2-clause BSD license. Please see the
** file at : https://github.com/libsndfile/libsamplerate/blob/master/COPYING
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <math.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include <samplerate.h>

#include "util.h"

#define BUFFER_LEN	(1<<16)

static float input [BUFFER_LEN] ;
static float output [BUFFER_LEN] ;

static long
throughput_test (int converter, long best_throughput)
{	SRC_DATA src_data ;
	clock_t start_time, clock_time ;
	double duration ;
	long total_frames = 0, throughput ;
	int error ;

	printf ("    %-30s    ", src_get_name (converter)) ;
	fflush (stdout) ;

	src_data.data_in = input ;
	src_data.input_frames = ARRAY_LEN (input) ;

	src_data.data_out = output ;
	src_data.output_frames = ARRAY_LEN (output) ;

	src_data.src_ratio = 0.99 ;

#ifdef _WIN32
	Sleep (2000) ;
#else
	sleep (2) ;
#endif

	start_time = clock () ;

	do
	{
		if ((error = src_simple (&src_data, converter, 1)) != 0)
		{	puts (src_strerror (error)) ;
			exit (1) ;
			} ;

		total_frames += src_data.output_frames_gen ;

		clock_time = clock () - start_time ;
#ifdef __GNU__ /* Clock resolution is 10ms on GNU/Hurd */
		duration = (10000.0 * clock_time) / CLOCKS_PER_SEC ;
#else
		duration = (1.0 * clock_time) / CLOCKS_PER_SEC ;
#endif
	}
	while (duration < 3.0) ;

	if (src_data.input_frames_used != ARRAY_LEN (input))
	{	printf ("\n\nLine %d : input frames used %ld should be %d\n", __LINE__, src_data.input_frames_used, ARRAY_LEN (input)) ;
		exit (1) ;
		} ;

	if (fabs (src_data.src_ratio * src_data.input_frames_used - src_data.output_frames_gen) > 2)
	{	printf ("\n\nLine %d : input / output length mismatch.\n\n", __LINE__) ;
		printf ("    input len  : %d\n", ARRAY_LEN (input)) ;
		printf ("    output len : %ld (should be %g +/- 2)\n\n", src_data.output_frames_gen,
				floor (0.5 + src_data.src_ratio * src_data.input_frames_used)) ;
		exit (1) ;
		} ;

	throughput = lrint (floor (total_frames / duration)) ;

	if (best_throughput == 0)
	{	best_throughput = MAX (throughput, best_throughput) ;
		printf ("%5.2f          %10ld\n", duration, throughput) ;
		}
	else
	{	best_throughput = MAX (throughput, best_throughput) ;
		printf ("%5.2f          %10ld       %10ld\n", duration, throughput, best_throughput) ;
		}


	return best_throughput ;
} /* throughput_test */

static void
single_run (void)
{

	printf ("\n    CPU name : %s\n", get_cpu_name ()) ;

	puts (
		"\n"
		"    Converter                        Duration        Throughput\n"
		"    -----------------------------------------------------------"
		) ;

	throughput_test (SRC_ZERO_ORDER_HOLD, 0) ;
	throughput_test (SRC_LINEAR, 0) ;
#ifdef ENABLE_SINC_FAST_CONVERTER
	throughput_test (SRC_SINC_FASTEST, 0) ;
#endif
#ifdef ENABLE_SINC_MEDIUM_CONVERTER
	throughput_test (SRC_SINC_MEDIUM_QUALITY, 0) ;
#endif
#ifdef ENABLE_SINC_BEST_CONVERTER
	throughput_test (SRC_SINC_BEST_QUALITY, 0) ;
#endif

	puts ("") ;
	return ;
} /* single_run */

static void
multi_run (int run_count)
{	long zero_order_hold = 0, linear = 0 ;

#ifdef ENABLE_SINC_FAST_CONVERTER
	long sinc_fastest = 0 ;
#endif

#ifdef ENABLE_SINC_MEDIUM_CONVERTER
	long sinc_medium = 0 ;
#endif

#ifdef ENABLE_SINC_BEST_CONVERTER
	long sinc_best = 0 ;
#endif
	int k ;

	puts (
		"\n"
		"    Converter                        Duration        Throughput      Best Throughput\n"
		"    --------------------------------------------------------------------------------"
		) ;

	for (k = 0 ; k < run_count ; k++)
	{	zero_order_hold =	throughput_test (SRC_ZERO_ORDER_HOLD, zero_order_hold) ;
		linear =			throughput_test (SRC_LINEAR, linear) ;
#ifdef ENABLE_SINC_FAST_CONVERTER
		sinc_fastest =		throughput_test (SRC_SINC_FASTEST, sinc_fastest) ;
#endif
#ifdef ENABLE_SINC_MEDIUM_CONVERTER
		sinc_medium =		throughput_test (SRC_SINC_MEDIUM_QUALITY, sinc_medium) ;
#endif
#ifdef ENABLE_SINC_BEST_CONVERTER
		sinc_best =			throughput_test (SRC_SINC_BEST_QUALITY, sinc_best) ;
#endif
		puts ("") ;

		/* Let the CPU cool down. We might be running on a laptop. */
#ifdef _WIN32
		Sleep (10000) ;
#else
		sleep (10) ;
#endif
		} ;

	printf ("\n    CPU name : %s\n", get_cpu_name ()) ;

	puts (
		"\n"
		"    Converter                        Best Throughput\n"
		"    ------------------------------------------------"
		) ;
	printf ("    %-30s    %10ld\n", src_get_name (SRC_ZERO_ORDER_HOLD), zero_order_hold) ;
	printf ("    %-30s    %10ld\n", src_get_name (SRC_LINEAR), linear) ;
#ifdef ENABLE_SINC_FAST_CONVERTER
	printf ("    %-30s    %10ld\n", src_get_name (SRC_SINC_FASTEST), sinc_fastest) ;
#endif
#ifdef ENABLE_SINC_MEDIUM_CONVERTER
	printf ("    %-30s    %10ld\n", src_get_name (SRC_SINC_MEDIUM_QUALITY), sinc_medium) ;
#endif
#ifdef ENABLE_SINC_BEST_CONVERTER
	printf ("    %-30s    %10ld\n", src_get_name (SRC_SINC_BEST_QUALITY), sinc_best) ;
#endif

	puts ("") ;
} /* multi_run */

static void
usage_exit (const char * argv0)
{	const char * cptr ;

	if ((cptr = strrchr (argv0, '/')) != NULL)
		argv0 = cptr ;

	printf (
		"Usage :\n"
	 	"    %s                 - Single run of the throughput test.\n"
		"    %s --best-of N     - Do N runs of test a print bext result.\n"
		"\n",
		argv0, argv0) ;

	exit (0) ;
} /* usage_exit */

int
main (int argc, char ** argv)
{	double freq ;

	memset (input, 0, sizeof (input)) ;
	freq = 0.01 ;
	gen_windowed_sines (1, &freq, 1.0, input, BUFFER_LEN) ;

	if (argc == 1)
		single_run () ;
	else if (argc == 3 && strcmp (argv [1], "--best-of") == 0)
	{	int run_count = atoi (argv [2]) ;

		if (run_count < 1 || run_count > 20)
		{	printf ("Please be sensible. Run count should be in range (1, 10].\n") ;
			exit (1) ;
			} ;

		multi_run (run_count) ;
		}
	else
		usage_exit (argv [0]) ;

	puts (
		"            Duration is in seconds.\n"
		"            Throughput is in samples/sec (more is better).\n"
		) ;

	return 0 ;
} /* main */

