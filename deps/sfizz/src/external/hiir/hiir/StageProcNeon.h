/*****************************************************************************

        StageProcNeon.h
        Author: Laurent de Soras, 2016

Template parameters:
	- CUR: index of the coefficient coefficient to process, >= 0

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#pragma once
#if ! defined (hiir_StageProcNeon_HEADER_INCLUDED)
#define hiir_StageProcNeon_HEADER_INCLUDED

#if defined (_MSC_VER)
	#pragma warning (4 : 4250)
#endif



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include "hiir/def.h"



namespace hiir
{



class StageDataNeon;

template <int CUR>
class StageProcNeon
{

	static_assert ((CUR >= 0), "CUR must be >= 0");

/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

public:

	static hiir_FORCEINLINE void
	               process_sample_pos (StageDataNeon *stage_ptr, float32x4_t &y, float32x4_t &mem);
	static hiir_FORCEINLINE void
	               process_sample_neg (StageDataNeon *stage_ptr, float32x4_t &y, float32x4_t &mem);



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

protected:



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	enum {         PREV = CUR - 1 };



/*\\\ FORBIDDEN MEMBER FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	               StageProcNeon ()                                     = delete;
	               StageProcNeon (const StageProcNeon <CUR> &other)     = delete;
	               ~StageProcNeon ()                                    = delete;
	StageProcNeon &
	               operator = (const StageProcNeon <CUR> &other)        = delete;
	bool           operator == (const StageProcNeon <CUR> &other) const = delete;
	bool           operator != (const StageProcNeon <CUR> &other) const = delete;

}; // class StageProcNeon



}  // namespace hiir



#include "hiir/StageProcNeon.hpp"



#endif   // hiir_StageProcNeon_HEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
