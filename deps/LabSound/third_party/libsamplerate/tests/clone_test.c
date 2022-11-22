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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <samplerate.h>

#include "util.h"

#define	BUFFER_LEN		(1 << 16)
#define NUM_CHANNELS 	2
#define FRAMES_PER_PASS (BUFFER_LEN >> 1)

static void
clone_test (int converter)
{	static float input_serial [BUFFER_LEN * NUM_CHANNELS], input_interleaved [BUFFER_LEN * NUM_CHANNELS] ;
	static float output [BUFFER_LEN * NUM_CHANNELS], output_cloned [BUFFER_LEN * NUM_CHANNELS] ;
	double sine_freq ;

	SRC_STATE* src_state ;
	SRC_STATE* src_state_cloned ;
	SRC_DATA src_data, src_data_cloned ;

	int error, frame, ch, idx ;

	printf ("        clone_test          (%-28s) ....... ", src_get_name (converter)) ;
	fflush (stdout) ;

	memset (input_serial, 0, sizeof (input_serial)) ;
	memset (input_interleaved, 0, sizeof (input_interleaved)) ;
	memset (output, 0, sizeof (output)) ;
	memset (output_cloned, 0, sizeof (output_cloned)) ;

	/* Fill input buffer with an n-channel interleaved sine wave */
	sine_freq = 0.0111 ;
	gen_windowed_sines (1, &sine_freq, 1.0, input_serial, BUFFER_LEN) ;
	gen_windowed_sines (1, &sine_freq, 1.0, input_serial + BUFFER_LEN, BUFFER_LEN) ;
	interleave_data (input_serial, input_interleaved, BUFFER_LEN, NUM_CHANNELS) ;

	if ((src_state = src_new (converter, NUM_CHANNELS, &error)) == NULL)
	{	printf ("\n\nLine %d : src_new() failed : %s\n\n", __LINE__, src_strerror (error)) ;
		exit (1) ;
		} ;

	/* Perform initial pass using first half of buffer so that src_state has non-trivial state */
	src_data.src_ratio = 1.1 ;
	src_data.input_frames = FRAMES_PER_PASS ;
	src_data.output_frames = FRAMES_PER_PASS ;
	src_data.data_in = input_interleaved ;
	src_data.data_out = output ;
	src_data.output_frames_gen = 0 ;

	if ((error = src_process (src_state, &src_data)))
	{	printf ("\n\nLine %d : %s\n\n", __LINE__, src_strerror (error)) ;
		printf ("  src_data.input_frames  : %ld\n", src_data.input_frames) ;
		printf ("  src_data.output_frames : %ld\n\n", src_data.output_frames) ;
		exit (1) ;
		} ;

	/* Clone handle */
	if ((src_state_cloned = src_clone (src_state, &error)) == NULL)
	{	printf ("\n\nLine %d : src_clone() failed : %s\n\n", __LINE__, src_strerror (error)) ;
		exit (1) ;
		} ;

	/* Process second half of buffer with both handles */
	src_data.data_in = input_interleaved + FRAMES_PER_PASS ;

	src_data_cloned = src_data ;
	src_data_cloned.data_out = output_cloned ;

	if ((error = src_process (src_state, &src_data)))
	{	printf ("\n\nLine %d : %s\n\n", __LINE__, src_strerror (error)) ;
		printf ("  src_data.input_frames  : %ld\n", src_data.input_frames) ;
		printf ("  src_data.output_frames : %ld\n\n", src_data.output_frames) ;
		exit (1) ;
		} ;

	if ((error = src_process (src_state_cloned, &src_data_cloned)))
	{	printf ("\n\nLine %d : %s\n\n", __LINE__, src_strerror (error)) ;
		printf ("  src_data.input_frames  : %ld\n", src_data.input_frames) ;
		printf ("  src_data.output_frames : %ld\n\n", src_data.output_frames) ;
		exit (1) ;
		} ;

	/* Check that both handles generated the same number of output frames */
	if (src_data.output_frames_gen != src_data_cloned.output_frames_gen)
	{	printf ("\n\nLine %d : cloned output_frames_gen (%ld) != original (%ld)\n\n", __LINE__,
				src_data_cloned.output_frames_gen, src_data.output_frames_gen) ;
		exit (1) ;
		} ;

	for (ch = 0 ; ch < NUM_CHANNELS ; ch++)
	{	for (frame = 0 ; frame < src_data.output_frames_gen ; frame++)
		{	idx = ch * NUM_CHANNELS + ch ;
			if (output[idx] != output_cloned[idx])
			{	printf ("\n\nLine %d : cloned data does not equal original data at channel %d, frame %d\n\n",
						__LINE__, ch, frame) ;
				exit (1) ;
				} ;
			} ;
		} ;

	src_delete (src_state) ;
	src_delete (src_state_cloned) ;

	puts ("ok") ;
} /* clone_test */

int
main (void)
{
	puts("");

	clone_test (SRC_ZERO_ORDER_HOLD) ;
	clone_test (SRC_LINEAR) ;
#ifdef ENABLE_SINC_FAST_CONVERTER
	clone_test (SRC_SINC_FASTEST) ;
#endif
	puts("");

	return 0 ;
} /* main */
