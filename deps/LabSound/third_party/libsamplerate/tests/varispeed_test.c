/*
** Copyright (c) 2006-2016, Erik de Castro Lopo <erikd@mega-nerd.com>
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
#include <math.h>
#include <string.h>

#include <samplerate.h>

#include "util.h"

#if HAVE_FFTW3
#include <fftw3.h>
#else
#define fftw_cleanup()
#endif

#define	BUFFER_LEN		(1 << 14)

static void varispeed_test (int converter, double target_snr) ;
static void varispeed_bounds_test (int converter) ;
static void set_ratio_test (int converter, int channels, double initial_ratio, double second_ratio) ;

int
main (void)
{
	puts ("\n    Varispeed SNR test") ;
	printf ("        Zero Order Hold interpolator    : ") ;
	fflush (stdout) ;
	varispeed_test (SRC_ZERO_ORDER_HOLD, 10.0) ;
	puts ("ok") ;

	printf ("        Linear interpolator             : ") ;
	fflush (stdout) ;
	varispeed_test (SRC_LINEAR, 10.0) ;
	puts ("ok") ;

#ifdef ENABLE_SINC_FAST_CONVERTER
	printf ("        Sinc interpolator               : ") ;
	fflush (stdout) ;
	varispeed_test (SRC_SINC_FASTEST, 115.0) ;
	puts ("ok") ;
#endif

	puts ("\n    Varispeed bounds test") ;
	printf ("        Zero Order Hold interpolator    : ") ;
	fflush (stdout) ;
	varispeed_bounds_test (SRC_ZERO_ORDER_HOLD) ;
	puts ("ok") ;

	printf ("        Linear interpolator             : ") ;
	fflush (stdout) ;
	varispeed_bounds_test (SRC_LINEAR) ;
	puts ("ok") ;

#ifdef ENABLE_SINC_FAST_CONVERTER
	printf ("        Sinc interpolator               : ") ;
	fflush (stdout) ;
	varispeed_bounds_test (SRC_SINC_FASTEST) ;
	puts ("ok") ;
#endif

	fftw_cleanup () ;
	puts ("") ;

	return 0 ;
} /* main */

static void
varispeed_test (int converter, double target_snr)
{	static float input [BUFFER_LEN], output [BUFFER_LEN] ;
	double sine_freq, snr ;

	SRC_STATE	*src_state ;
	SRC_DATA	src_data ;

	int input_len, error ;

	memset (input, 0, sizeof (input)) ;

	input_len = ARRAY_LEN (input) / 2 ;

	sine_freq = 0.0111 ;
	gen_windowed_sines (1, &sine_freq, 1.0, input, input_len) ;

	/* Perform sample rate conversion. */
	if ((src_state = src_new (converter, 1, &error)) == NULL)
	{	printf ("\n\nLine %d : src_new () failed : %s\n\n", __LINE__, src_strerror (error)) ;
		exit (1) ;
		} ;

	src_data.end_of_input = 1 ;

	src_data.data_in = input ;
	src_data.input_frames = input_len ;

	src_data.src_ratio = 3.0 ;

	src_data.data_out = output ;
	src_data.output_frames = ARRAY_LEN (output) ;

	if ((error = src_set_ratio (src_state, 1.0 / src_data.src_ratio)))
	{	printf ("\n\nLine %d : %s\n\n", __LINE__, src_strerror (error)) ;
		exit (1) ;
		} ;

	if ((error = src_process (src_state, &src_data)))
	{	printf ("\n\nLine %d : %s\n\n", __LINE__, src_strerror (error)) ;
		printf ("  src_data.input_frames  : %ld\n", src_data.input_frames) ;
		printf ("  src_data.output_frames : %ld\n\n", src_data.output_frames) ;
		exit (1) ;
		} ;

	if (src_data.input_frames_used != input_len)
	{	printf ("\n\nLine %d : unused input.\n", __LINE__) ;
		printf ("\tinput_len         : %d\n", input_len) ;
		printf ("\tinput_frames_used : %ld\n\n", src_data.input_frames_used) ;
		exit (1) ;
		} ;

	/* Copy the last output to the input. */
	memcpy (input, output, sizeof (input)) ;
	reverse_data (input, src_data.output_frames_gen) ;

	if ((error = src_reset (src_state)))
	{	printf ("\n\nLine %d : %s\n\n", __LINE__, src_strerror (error)) ;
		exit (1) ;
		} ;

	src_data.end_of_input = 1 ;

	src_data.data_in = input ;
	input_len = src_data.input_frames = src_data.output_frames_gen ;

	src_data.data_out = output ;
	src_data.output_frames = ARRAY_LEN (output) ;

	if ((error = src_set_ratio (src_state, 1.0 / src_data.src_ratio)))
	{	printf ("\n\nLine %d : %s\n\n", __LINE__, src_strerror (error)) ;
		exit (1) ;
		} ;

	if ((error = src_process (src_state, &src_data)))
	{	printf ("\n\nLine %d : %s\n\n", __LINE__, src_strerror (error)) ;
		printf ("  src_data.input_frames  : %ld\n", src_data.input_frames) ;
		printf ("  src_data.output_frames : %ld\n\n", src_data.output_frames) ;
		exit (1) ;
		} ;

	if (src_data.input_frames_used != input_len)
	{	printf ("\n\nLine %d : unused input.\n", __LINE__) ;
		printf ("\tinput_len         : %d\n", input_len) ;
		printf ("\tinput_frames_used : %ld\n\n", src_data.input_frames_used) ;
		exit (1) ;
		} ;

	src_state = src_delete (src_state) ;

	snr = calculate_snr (output, src_data.output_frames_gen, 1) ;

	if (target_snr > snr)
	{	printf ("\n\nLine %d : snr (%3.1f) does not meet target (%3.1f)\n\n", __LINE__, snr, target_snr) ;
		save_oct_float ("varispeed.mat", input, src_data.input_frames, output, src_data.output_frames_gen) ;
		exit (1) ;
		} ;

	return ;
} /* varispeed_test */

static void
varispeed_bounds_test (int converter)
{	double ratios [] = { 0.1, 0.01, 20 } ;
	int chan, r1, r2 ;

	for (chan = 1 ; chan <= 9 ; chan ++)
		for (r1 = 0 ; r1 < ARRAY_LEN (ratios) ; r1++)
			for (r2 = 0 ; r2 < ARRAY_LEN (ratios) ; r2 ++)
				if (r1 != r2)
					set_ratio_test (converter, chan, ratios [r1], ratios [r2]) ;
} /* varispeed_bounds_test */

static void
set_ratio_test (int converter, int channels, double initial_ratio, double second_ratio)
{	const int total_input_frames = BUFFER_LEN ;
	/* Maximum upsample ratio is 20, use a value beigger. */
	const int total_output_frames = 25 * BUFFER_LEN ;

	/* Interested in array boundary conditions, so all zero data here is fine. */
	float *input = calloc (total_input_frames * channels, sizeof (float)) ;
	float *output = calloc (total_output_frames * channels, sizeof (float)) ;

	char details [128] ;

	const int max_loop_count = 100000 ;
	const int chunk_size = 128 ;

	SRC_STATE *src_state ;
	SRC_DATA src_data ;

	int error, k, total_frames_used, total_frames_gen ;

	snprintf (details, sizeof (details), "%d channels, ratio %g -> %g", channels, initial_ratio, second_ratio) ;

	if ((src_state = src_new (converter, channels, &error)) == NULL)
	{	printf ("\n\nLine %d : src_new () failed : %s\n\n", __LINE__, src_strerror (error)) ;
		exit (1) ;
		} ;

	total_frames_used = 0 ;
	total_frames_gen = 0 ;

	memset (&src_data, 0, sizeof (src_data)) ;
	src_data.end_of_input = 0 ;
	src_data.src_ratio = initial_ratio ;
	src_data.data_in = input ;
	src_data.data_out = output ;
	src_data.input_frames = chunk_size ;
	src_data.output_frames = total_output_frames ;

	/* Use a max_loop_count here to enable the detection of infinite loops
	** (due to end of input not being detected.
	*/
	for (k = 0 ; k < max_loop_count ; k ++)
	{	if (k == 1)
		{	/* Hard switch to second_ratio after processing one chunk. */
			src_data.src_ratio = second_ratio ;
			if ((error = src_set_ratio (src_state, second_ratio)))
			{	printf ("\n\nLine %d : %s : %s\n\n", __LINE__, details, src_strerror (error)) ;
				exit (1) ;
				} ;
			} ;

		if ((error = src_process (src_state, &src_data)) != 0)
		{	printf ("\n\nLine %d : %s : %s\n\n", __LINE__, details, src_strerror (error)) ;
			exit (1) ;
			} ;

		if (src_data.end_of_input && src_data.output_frames_gen == 0)
			break ;

		total_frames_used	+= src_data.input_frames_used ;
		total_frames_gen 	+= src_data.output_frames_gen ;

		src_data.data_in	+= src_data.input_frames_used * channels ;
		src_data.data_out	+= src_data.output_frames_gen * channels ;

		src_data.input_frames	= total_input_frames - total_frames_used ;
		src_data.output_frames	= total_output_frames - total_frames_gen ;

		src_data.end_of_input = total_frames_used >= total_input_frames ? 1 : 0 ;
		} ;

	ASSERT (k < max_loop_count) ;
	ASSERT (total_frames_gen > 0) ;

	for (k = 0 ; k < total_frames_gen * channels ; k ++)
		ASSERT (! isnan (output [k])) ;

	src_state = src_delete (src_state) ;

	free (input) ;
	free (output) ;

	return ;
} /* set_ratio_test */
