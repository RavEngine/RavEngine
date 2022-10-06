/*****************************************************************************

        Downsampler2xNeon.hpp
        Author: Laurent de Soras, 2016

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#if ! defined (hiir_Downsampler2xNeon_CODEHEADER_INCLUDED)
#define hiir_Downsampler2xNeon_CODEHEADER_INCLUDED



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include "hiir/StageProcNeon.h"

#include <arm_neon.h>

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
Downsampler2xNeon <NC>::Downsampler2xNeon ()
:	_filter ()
{
	for (int i = 0; i < NBR_STAGES + 1; ++i)
	{
		_filter [i]._mem4 = vdupq_n_f32 (0);
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
void	Downsampler2xNeon <NC>::set_coefs (const double coef_arr [])
{
	assert (coef_arr != 0);

	for (int i = 0; i < NBR_COEFS; ++i)
	{
		const int      stage = (i / STAGE_WIDTH) + 1;
		const int      pos   = (i ^ 1) & (STAGE_WIDTH - 1);
		_filter [stage]._coef [pos] = float (coef_arr [i]);
	}
}



/*
==============================================================================
Name: process_sample
Description:
   Downsamples (x2) one pair of samples, to generate one output sample.
Input parameters:
	- in_ptr: pointer on the two samples to decimate
Returns: Samplerate-reduced sample.
Throws: Nothing
==============================================================================
*/

template <int NC>
float	Downsampler2xNeon <NC>::process_sample (const float in_ptr [2])
{
	assert (in_ptr != 0);

	// Combines two input samples and two mid-processing data
	const float32x2_t spl_in  = vreinterpret_f32_u8 (
		vld1_u8 (reinterpret_cast <const uint8_t *> (in_ptr))
	);
	const float32x2_t spl_mid = vget_low_f32 (_filter [NBR_STAGES]._mem4);
	float32x4_t       y       = vcombine_f32 (spl_in, spl_mid);
	float32x4_t       mem     = _filter [0]._mem4;

	// Processes each stage
	StageProcNeon <NBR_STAGES>::process_sample_pos (&_filter [0], y, mem);
	_filter [NBR_STAGES]._mem4 = y;

	// Averages both paths and outputs the result
	const float       out_0  = vgetq_lane_f32 (y, 3);
	const float       out_1  = vgetq_lane_f32 (y, 2);
	const float       out    = (out_0 + out_1) * 0.5f;

	return out;
}



/*
==============================================================================
Name: process_block
Description:
   Downsamples (x2) a block of samples.
	Input and output blocks may overlap, see assert() for details.
Input parameters:
	- in_ptr: Input array, containing nbr_spl * 2 samples.
	- nbr_spl: Number of samples to output, > 0
Output parameters:
	- out_ptr: Array for the output samples, capacity: nbr_spl samples.
Throws: Nothing
==============================================================================
*/

template <int NC>
void	Downsampler2xNeon <NC>::process_block (float out_ptr [], const float in_ptr [], long nbr_spl)
{
	assert (in_ptr != 0);
	assert (out_ptr != 0);
	assert (out_ptr <= in_ptr || out_ptr >= in_ptr + nbr_spl * 2);
	assert (nbr_spl > 0);

	long           pos = 0;
	do
	{
		out_ptr [pos] = process_sample (in_ptr + pos * 2);
		++ pos;
	}
	while (pos < nbr_spl);
}



/*
==============================================================================
Name: process_sample_split
Description:
   Split (spectrum-wise) in half a pair of samples. The lower part of the
   spectrum is a classic downsampling, equivalent to the output of
   process_sample().
   The higher part is the complementary signal: original filter response
   is flipped from left to right, becoming a high-pass filter with the same
   cutoff frequency. This signal is then critically sampled (decimation by 2),
   flipping the spectrum: Fs/4...Fs/2 becomes Fs/4...0.
Input parameters:
	- in_ptr: pointer on the pair of input samples
Output parameters:
	- low: output sample, lower part of the spectrum (downsampling)
	- high: output sample, higher part of the spectrum.
Throws: Nothing
==============================================================================
*/

template <int NC>
void	Downsampler2xNeon <NC>::process_sample_split (float &low, float &high, const float in_ptr [2])
{
	assert (in_ptr != 0);

	// Combines two input samples and two mid-processing data
	const float32x2_t spl_in  = vreinterpret_f32_u8 (
		vld1_u8 (reinterpret_cast <const uint8_t *> (in_ptr))
	);
	const float32x2_t spl_mid = vget_low_f32 (_filter [NBR_STAGES]._mem4);
	float32x4_t       y       = vcombine_f32 (spl_in, spl_mid);
	float32x4_t       mem     = _filter [0]._mem4;

	// Processes each stage
	StageProcNeon <NBR_STAGES>::process_sample_pos (&_filter [0], y, mem);
	_filter [NBR_STAGES]._mem4 = y;

	// Outputs the result
	const float       out_0  = vgetq_lane_f32 (y, 3);
	const float       out_1  = vgetq_lane_f32 (y, 2);
	low  = (out_0 + out_1) * 0.5f;
	high =  out_0 - low;
}



/*
==============================================================================
Name: process_block_split
Description:
   Split (spectrum-wise) in half a block of samples. The lower part of the
   spectrum is a classic downsampling, equivalent to the output of
   process_block().
   The higher part is the complementary signal: original filter response
   is flipped from left to right, becoming a high-pass filter with the same
   cutoff frequency. This signal is then critically sampled (decimation by 2),
   flipping the spectrum: Fs/4...Fs/2 becomes Fs/4...0.
	Input and output blocks may overlap, see assert() for details.
Input parameters:
	- in_ptr: Input array, containing nbr_spl * 2 samples.
	- nbr_spl: Number of samples for each output, > 0
Output parameters:
	- out_l_ptr: Array for the output samples, lower part of the spectrum
      (downsampling). Capacity: nbr_spl samples.
	- out_h_ptr: Array for the output samples, higher part of the spectrum.
      Capacity: nbr_spl samples.
Throws: Nothing
==============================================================================
*/

template <int NC>
void	Downsampler2xNeon <NC>::process_block_split (float out_l_ptr [], float out_h_ptr [], const float in_ptr [], long nbr_spl)
{
	assert (in_ptr != 0);
	assert (out_l_ptr != 0);
	assert (out_l_ptr <= in_ptr || out_l_ptr >= in_ptr + nbr_spl * 2);
	assert (out_h_ptr != 0);
	assert (out_h_ptr <= in_ptr || out_h_ptr >= in_ptr + nbr_spl * 2);
	assert (out_h_ptr != out_l_ptr);
	assert (nbr_spl > 0);

	long           pos = 0;
	do
	{
		process_sample_split (out_l_ptr [pos], out_h_ptr [pos], in_ptr + pos * 2);
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
void	Downsampler2xNeon <NC>::clear_buffers ()
{
	for (int i = 0; i < NBR_STAGES + 1; ++i)
	{
		_filter [i]._mem4 = vdupq_n_f32 (0);
	}
}



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



}  // namespace hiir



#endif   // hiir_Downsampler2xNeon_CODEHEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
