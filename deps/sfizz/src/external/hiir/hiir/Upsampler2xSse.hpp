/*****************************************************************************

        Upsampler2xSse.hpp
        Author: Laurent de Soras, 2005

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#if defined (hiir_Upsampler2xSse_CURRENT_CODEHEADER)
	#error Recursive inclusion of Upsampler2xSse code header.
#endif
#define hiir_Upsampler2xSse_CURRENT_CODEHEADER

#if ! defined (hiir_Upsampler2xSse_CODEHEADER_INCLUDED)
#define hiir_Upsampler2xSse_CODEHEADER_INCLUDED



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include "hiir/StageProcSse.h"

#include <xmmintrin.h>

#include <cassert>



namespace hiir
{



/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*
==============================================================================
Name: ctor
Throws: Nothing
==============================================================================
*/

template <int NC>
Upsampler2xSse <NC>::Upsampler2xSse ()
:	_filter ()
{
	for (int i = 0; i < NBR_STAGES + 1; ++i)
	{
		_filter [i]._coef [0] = 0;
		_filter [i]._coef [1] = 0;
		_filter [i]._coef [2] = 0;
		_filter [i]._coef [3] = 0;
	}
	if ((NBR_COEFS & 1) != 0)
	{
		const int      pos = (NBR_COEFS ^ 1) & (STAGE_WIDTH - 1);
		_filter [NBR_STAGES]._coef [pos] = 1;
	}

	clear_buffers ();
}



/*
==============================================================================
Name: set_coefs
Description:
   Sets filter coefficients. Generate them with the PolyphaseIir2Designer
   class.
   Call this function before doing any processing.
Input parameters:
	- coef_arr: Array of coefficients. There should be as many coefficients as
      mentioned in the class template parameter.
Throws: Nothing
==============================================================================
*/

template <int NC>
void	Upsampler2xSse <NC>::set_coefs (const double coef_arr [NBR_COEFS])
{
	assert (coef_arr != 0);

	for (int i = 0; i < NBR_COEFS; ++i)
	{
		const int      stage = (i / STAGE_WIDTH) + 1;
		const int      pos = (i ^ 1) & (STAGE_WIDTH - 1);
		_filter [stage]._coef [pos] = float (coef_arr [i]);
	}
}



/*
==============================================================================
Name: process_sample
Description:
	Upsamples (x2) the input sample, generating two output samples.
Input parameters:
	- input: The input sample.
Output parameters:
	- out_0: First output sample.
	- out_1: Second output sample.
Throws: Nothing
==============================================================================
*/

template <int NC>
void	Upsampler2xSse <NC>::process_sample (float &out_0, float &out_1, float input)
{
	const __m128   spl_in  = _mm_set_ss (input);
	const __m128   spl_mid = _mm_load_ps (_filter [NBR_STAGES]._mem);
	__m128         y       = _mm_shuffle_ps (spl_in, spl_mid, 0x40);

	__m128         mem     = _mm_load_ps (_filter [0]._mem);

	StageProcSse <NBR_STAGES>::process_sample_pos (&_filter [0], y, mem);

	_mm_store_ps (_filter [NBR_STAGES]._mem, y);

	// The latest shufps/movss instruction pairs can be freely inverted
	y = _mm_shuffle_ps (y, y, 0xE3);
	_mm_store_ss (&out_0, y);
	y = _mm_shuffle_ps (y, y, 0xE2);
	_mm_store_ss (&out_1, y);
}



/*
==============================================================================
Name: process_block
Description:
	Upsamples (x2) the input sample block.
	Input and output blocks may overlap, see assert() for details.
Input parameters:
	- in_ptr: Input array, containing nbr_spl samples.
	- nbr_spl: Number of input samples to process, > 0
Output parameters:
	- out_0_ptr: Output sample array, capacity: nbr_spl * 2 samples.
Throws: Nothing
==============================================================================
*/

template <int NC>
void	Upsampler2xSse <NC>::process_block (float out_ptr [], const float in_ptr [], long nbr_spl)
{
	assert (out_ptr != 0);
	assert (in_ptr != 0);
	assert (out_ptr >= in_ptr + nbr_spl || in_ptr >= out_ptr + nbr_spl);
	assert (nbr_spl > 0);

	long           pos = 0;
	do
	{
		process_sample (out_ptr [pos * 2], out_ptr [pos * 2 + 1], in_ptr [pos]);
		++ pos;
	}
	while (pos < nbr_spl);
}



/*
==============================================================================
Name: clear_buffers
Description:
	Clears filter memory, as if it processed silence since an infinite amount
	of time.
Throws: Nothing
==============================================================================
*/

template <int NC>
void	Upsampler2xSse <NC>::clear_buffers ()
{
	for (int i = 0; i < NBR_STAGES + 1; ++i)
	{
		_filter [i]._mem [0] = 0;
		_filter [i]._mem [1] = 0;
		_filter [i]._mem [2] = 0;
		_filter [i]._mem [3] = 0;
	}
}



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



} // namespace hiir



#endif // hiir_Upsampler2xSse_CODEHEADER_INCLUDED

#undef hiir_Upsampler2xSse_CURRENT_CODEHEADER



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
