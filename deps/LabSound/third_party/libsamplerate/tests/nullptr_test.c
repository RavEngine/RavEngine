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
#define NUM_CHANNELS 	1

static void
nullptr_test (int converter)
{	static float input [BUFFER_LEN * NUM_CHANNELS] ;
	static float output [BUFFER_LEN * NUM_CHANNELS] ;

	SRC_STATE* src_state ;
	SRC_DATA src_data, src_data2 ;

	int error ;

	printf ("        nullptr_test        (%-28s) ....... ", src_get_name (converter)) ;
	fflush (stdout) ;

	memset (input, 0, sizeof (input)) ;
	memset (output, 0, sizeof (output)) ;

	if ((src_state = src_new (converter, NUM_CHANNELS, &error)) == NULL)
	{	printf ("\n\nLine %d : src_new() failed : %s\n\n", __LINE__, src_strerror (error)) ;
		exit (1) ;
		} ;

	src_data.src_ratio = 1.1 ;
	src_data.input_frames = BUFFER_LEN ;
	src_data.output_frames = BUFFER_LEN ;
	src_data.data_in = input ;
	src_data.data_out = output ;
	src_data.output_frames_gen = 0 ;

	if ((error = src_process (src_state, &src_data)))
	{	printf ("\n\nLine %d : %s\n\n", __LINE__, src_strerror (error)) ;
		exit (1) ;
		} ;

	//Input is zero-length
	src_data2 = src_data;
	src_data2.data_in = NULL;
	src_data2.input_frames = 0;

	if ((error = src_process (src_state, &src_data2)))
	{	printf ("\n\nLine %d : %s\n\n", __LINE__, src_strerror (error)) ;
		exit (1) ;
		} ;

	//Output is zero-length
	src_data2 = src_data;
	src_data2.data_out = NULL;
	src_data2.output_frames = 0;

	if ((error = src_process (src_state, &src_data2)))
	{	printf ("\n\nLine %d : %s\n\n", __LINE__, src_strerror (error)) ;
		exit (1) ;
		} ;

	//Input and output are zero-length
	src_data2 = src_data;
	src_data2.data_in = NULL;
	src_data2.data_out = NULL;
	src_data2.input_frames = 0;
	src_data2.output_frames = 0;

	if ((error = src_process (src_state, &src_data2)))
	{	printf ("\n\nLine %d : %s\n\n", __LINE__, src_strerror (error)) ;
		exit (1) ;
		} ;


	src_state = src_delete (src_state) ;

	puts ("ok") ;
} /* nullptr_test */

int
main (void)
{
	puts("");

	nullptr_test (SRC_ZERO_ORDER_HOLD) ;
	nullptr_test (SRC_LINEAR) ;
#ifdef ENABLE_SINC_FAST_CONVERTER
	nullptr_test (SRC_SINC_FASTEST) ;
#endif
	puts("");

	return 0 ;
} /* main */
