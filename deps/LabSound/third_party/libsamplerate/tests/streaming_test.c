/*
** Copyright (c) 2002-2016, Erik de Castro Lopo <erikd@mega-nerd.com>
** All rights reserved.
**
** This code is released under 2-clause BSD license. Please see the
** file at : https://github.com/libsndfile/libsamplerate/blob/master/COPYING
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <samplerate.h>

#include "util.h"

#define	BUFFER_LEN		(1<<15)

#define BLOCK_LEN		100

static void stream_test (int converter, double ratio) ;

int
main (void)
{	static double src_ratios [] =
	{	0.3, 0.9, 1.1, 3.0
	} ;

	int k ;

	puts ("\n    Zero Order Hold interpolator:") ;
	for (k = 0 ; k < ARRAY_LEN (src_ratios) ; k++)
		stream_test (SRC_ZERO_ORDER_HOLD, src_ratios [k]) ;

	puts ("\n    Linear interpolator:") ;
	for (k = 0 ; k < ARRAY_LEN (src_ratios) ; k++)
		stream_test (SRC_LINEAR, src_ratios [k]) ;

	puts ("\n    Sinc interpolator:") ;
	for (k = 0 ; k < ARRAY_LEN (src_ratios) ; k++)
		stream_test (SRC_SINC_FASTEST, src_ratios [k]) ;

	puts ("") ;

	return 0 ;
} /* main */

static void
stream_test (int converter, double src_ratio)
{	static float input [BUFFER_LEN], output [BUFFER_LEN] ;

	SRC_STATE	*src_state ;
	SRC_DATA	src_data ;

	int input_len, output_len, current_in, current_out ;
	int error, terminate ;

	printf ("\tstreaming_test   (SRC ratio = %6.4f) ........... ", src_ratio) ;
	fflush (stdout) ;

	/* Calculate maximun input and output lengths. */
	if (src_ratio >= 1.0)
	{	output_len = BUFFER_LEN ;
		input_len = (int) floor (BUFFER_LEN / src_ratio) ;
		}
	else
	{	input_len = BUFFER_LEN ;
		output_len = (int) floor (BUFFER_LEN * src_ratio) ;
		} ;

	/* Reduce input_len by 10 so output is longer than necessary. */
	input_len -= 10 ;

	if (output_len > BUFFER_LEN)
	{	printf ("\n\nLine %d : output_len > BUFFER_LEN\n\n", __LINE__) ;
		exit (1) ;
		} ;

	current_in = current_out = 0 ;

	/* Perform sample rate conversion. */
	if ((src_state = src_new (converter, 1, &error)) == NULL)
	{	printf ("\n\nLine %d : src_new() failed : %s\n\n", __LINE__, src_strerror (error)) ;
		exit (1) ;
		} ;

	src_data.end_of_input = 0 ; /* Set this later. */

	src_data.data_in = input ;
	src_data.input_frames = BLOCK_LEN ;

	src_data.src_ratio = src_ratio ;

	src_data.data_out = output ;
	src_data.output_frames = BLOCK_LEN ;

	while (1)
	{	if ((error = src_process (src_state, &src_data)))
		{	printf ("\n\nLine %d : %s\n\n", __LINE__, src_strerror (error)) ;

printf ("src_data.input_frames  : %ld\n", src_data.input_frames) ;
printf ("src_data.output_frames : %ld\n", src_data.output_frames) ;

			exit (1) ;
			} ;

		if (src_data.end_of_input && src_data.output_frames_gen == 0)
			break ;

		current_in	+= src_data.input_frames_used ;
		current_out += src_data.output_frames_gen ;

		src_data.data_in	+= src_data.input_frames_used ;
		src_data.data_out	+= src_data.output_frames_gen ;

		src_data.input_frames	= MIN (BLOCK_LEN, input_len - current_in) ;
		src_data.output_frames	= MIN (BLOCK_LEN, output_len - current_out) ;

		src_data.end_of_input = (current_in >= input_len) ? 1 : 0 ;
		} ;

	src_state = src_delete (src_state) ;

	terminate = (int) ceil ((src_ratio >= 1.0) ? src_ratio : 1.0 / src_ratio) ;

	if (fabs (current_out - src_ratio * input_len) > 2 * terminate)
	{	printf ("\n\nLine %d : bad output data length %d should be %d.\n", __LINE__,
					current_out, (int) floor (src_ratio * input_len)) ;
		printf ("\tsrc_ratio  : %.4f\n", src_ratio) ;
		printf ("\tinput_len  : %d\n\toutput_len : %d\n\n", input_len, output_len) ;
		exit (1) ;
		} ;

	if (current_in != input_len)
	{	printf ("\n\nLine %d : unused input.\n", __LINE__) ;
		printf ("\tinput_len         : %d\n", input_len) ;
		printf ("\tinput_frames_used : %d\n\n", current_in) ;
		exit (1) ;
		} ;

	puts ("ok") ;

	return ;
} /* stream_test */

