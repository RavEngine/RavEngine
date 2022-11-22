/*
** Copyright (c) 2003-2016, Erik de Castro Lopo <erikd@mega-nerd.com>
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

#define	BUFFER_LEN		10000

static void float_to_short_test (void) ;
static void short_to_float_test (void) ;

static void float_to_int_test (void) ;
static void int_to_float_test (void) ;

int
main (void)
{
	puts ("") ;

	float_to_short_test () ;
	short_to_float_test () ;

	float_to_int_test () ;
	int_to_float_test () ;

	puts ("") ;

	return 0 ;
} /* main */

/*=====================================================================================
*/

static void
float_to_short_test (void)
{
	static float fpos [] =
	{	0.95f, 0.99f, 1.0f, 1.01f, 1.1f, 2.0f, 11.1f, 111.1f, 2222.2f, 33333.3f,
		// Some "almost 1" as corner cases
		(float) (32767.0 / 32768.0), (float) ((32767.0 + 0.4) / 32768.0), (float) ((32767. + 0.5) / 32768.0),
		(float) ((32767.0 + 0.6) / 32768.0), (float) ((32767.0 + 0.9) / 32768.0),
		} ;
	static float fneg [] =
	{	-0.95f, -0.99f, -1.0f, -1.01f, -1.1f, -2.0f, -11.1f, -111.1f, -2222.2f, -33333.3f,
		// Some "almost 1" as corner cases
		(float) (-32767.0 / 32768.0), (float) (-(32767.0 + 0.4) / 32768.0), (float) (-(32767.0 + 0.5) / 32768.0),
		(float) (-(32767.0 + 0.6) / 32768.0), (float) (-(32767.0 + 0.9) / 32768.0),
		} ;

	static short out [MAX (ARRAY_LEN (fpos), ARRAY_LEN (fneg))] ;

	int k ;

	printf ("\tfloat_to_short_test ............................. ") ;

	src_float_to_short_array (fpos, out, ARRAY_LEN (fpos)) ;

	for (k = 0 ; k < ARRAY_LEN (fpos) ; k++)
		if (out [k] < 30000)
		{	printf ("\n\n\tLine %d : out [%d] == %d\n", __LINE__, k, out [k]) ;
			exit (1) ;
			} ;

	src_float_to_short_array (fneg, out, ARRAY_LEN (fneg)) ;

	for (k = 0 ; k < ARRAY_LEN (fneg) ; k++)
		if (out [k] > -30000)
		{	printf ("\n\n\tLine %d : out [%d] == %d\n", __LINE__, k, out [k]) ;
			exit (1) ;
			} ;

	puts ("ok") ;

	return ;
} /* float_to_short_test */

/*-------------------------------------------------------------------------------------
*/

static void
short_to_float_test (void)
{
	static short input	[BUFFER_LEN] ;
	static short output	[BUFFER_LEN] ;
	static float temp	[BUFFER_LEN] ;

	int k ;

	printf ("\tshort_to_float_test ............................. ") ;

	for (k = 0 ; k < ARRAY_LEN (input) ; k++)
		input [k] = (k * 0x8000) / ARRAY_LEN (input) ;

	src_short_to_float_array (input, temp, ARRAY_LEN (temp)) ;
	src_float_to_short_array (temp, output, ARRAY_LEN (output)) ;

	for (k = 0 ; k < ARRAY_LEN (input) ; k++)
		if (ABS (input [k] - output [k]) > 0)
		{	printf ("\n\n\tLine %d : index %d   %d -> %d\n", __LINE__, k, input [k], output [k]) ;
			exit (1) ;
			} ;

	puts ("ok") ;

	return ;
} /* short_to_float_test */

/*=====================================================================================
*/

static void
float_to_int_test (void)
{
	static float fpos [] =
	{	0.95f, 0.99f, 1.0f, 1.01f, 1.1f, 2.0f, 11.1f, 111.1f, 2222.2f, 33333.3f
		} ;
	static float fneg [] =
	{	-0.95f, -0.99f, -1.0f, -1.01f, -1.1f, -2.0f, -11.1f, -111.1f, -2222.2f, -33333.3f
		} ;

	static int out [MAX (ARRAY_LEN (fpos), ARRAY_LEN (fneg))] ;

	int k ;

	printf ("\tfloat_to_int_test ............................... ") ;

	src_float_to_int_array (fpos, out, ARRAY_LEN (fpos)) ;

	for (k = 0 ; k < ARRAY_LEN (fpos) ; k++)
		if (out [k] < 30000 * 0x10000)
		{	printf ("\n\n\tLine %d : out [%d] == %d\n", __LINE__, k, out [k]) ;
			exit (1) ;
			} ;

	src_float_to_int_array (fneg, out, ARRAY_LEN (fneg)) ;

	for (k = 0 ; k < ARRAY_LEN (fneg) ; k++)
		if (out [k] > -30000 * 0x1000)
		{	printf ("\n\n\tLine %d : out [%d] == %d\n", __LINE__, k, out [k]) ;
			exit (1) ;
			} ;

	puts ("ok") ;

	return ;
} /* float_to_int_test */

/*-------------------------------------------------------------------------------------
*/

static void
int_to_float_test (void)
{
	static int input	[BUFFER_LEN] ;
	static int output	[BUFFER_LEN] ;
	static float temp	[BUFFER_LEN] ;

	int k ;

	printf ("\tint_to_float_test ............................... ") ;

	for (k = 0 ; k < ARRAY_LEN (input) ; k++)
		input [k] = (k * 0x80000000) / ARRAY_LEN (input) ;

	src_int_to_float_array (input, temp, ARRAY_LEN (temp)) ;
	src_float_to_int_array (temp, output, ARRAY_LEN (output)) ;

	for (k = 0 ; k < ARRAY_LEN (input) ; k++)
		if (ABS (input [k] - output [k]) > 0)
		{	printf ("\n\n\tLine %d : index %d   %d -> %d\n", __LINE__, k, input [k], output [k]) ;
			exit (1) ;
			} ;

	puts ("ok") ;

	return ;
} /* int_to_float_test */

