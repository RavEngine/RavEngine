/*****************************************************************************

        Upsampler2xNeon.h
        Author: Laurent de Soras, 2016

Upsamples by a factor 2 the input signal, using NEON instruction set.

This object must be aligned on a 16-byte boundary!

If the number of coefficients is 2 or 3 modulo 4, the output is delayed from
1 sample, compared to the theoretical formula (or FPU implementation).

Template parameters:
	- NC: number of coefficients, > 0

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#pragma once
#if ! defined (hiir_Upsampler2xNeon_HEADER_INCLUDED)
#define hiir_Upsampler2xNeon_HEADER_INCLUDED

#if defined (_MSC_VER)
	#pragma warning (4 : 4250)
#endif



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include "hiir/StageDataNeon.h"

#include <array>



namespace hiir
{



template <int NC>
class Upsampler2xNeon
{

	static_assert ((NC > 0), "Number of coefficient must be positive.");

/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

public:

	enum {         NBR_COEFS = NC };

	               Upsampler2xNeon ();
	               Upsampler2xNeon (const Upsampler2xNeon &other)   = default;
	Upsampler2xNeon &
	               operator = (const Upsampler2xNeon &other)        = default;

	void           set_coefs (const double coef_arr [NBR_COEFS]);
	inline void    process_sample (float &out_0, float &out_1, float input);
	void           process_block (float out_ptr [], const float in_ptr [], long nbr_spl);
	void           clear_buffers ();



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

protected:



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	enum {         STAGE_WIDTH = 4 };
	enum {         NBR_STAGES  = (NBR_COEFS + STAGE_WIDTH - 1) / STAGE_WIDTH };

	typedef	std::array <StageDataNeon, NBR_STAGES + 1>	Filter;	// Stage 0 contains only input memory

	Filter         _filter;    // Should be the first member (thus easier to align)



/*\\\ FORBIDDEN MEMBER FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	bool           operator == (const Upsampler2xNeon <NC> &other) const = delete;
	bool           operator != (const Upsampler2xNeon <NC> &other) const = delete;

}; // class Upsampler2xNeon



}  // namespace hiir



#include "hiir/Upsampler2xNeon.hpp"



#endif   // hiir_Upsampler2xNeon_HEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
