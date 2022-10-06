/*****************************************************************************

        StageProcNeon.hpp
        Author: Laurent de Soras, 2016

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#if ! defined (hiir_StageProcNeon_CODEHEADER_INCLUDED)
#define hiir_StageProcNeon_CODEHEADER_INCLUDED



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include "hiir/StageDataNeon.h"



namespace hiir
{



/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



template <int CUR>
void	StageProcNeon <CUR>::process_sample_pos (StageDataNeon *stage_ptr, float32x4_t &y, float32x4_t &mem)
{
	StageProcNeon <CUR - 1>::process_sample_pos (stage_ptr, y, mem);

	const float32x4_t x    = mem;
	stage_ptr [PREV]._mem4 = y;

	mem = stage_ptr [CUR]._mem4;
	y   = vmlaq_f32 (x, y - mem, stage_ptr [CUR]._coef4);
}

template <>
inline void	StageProcNeon <0>::process_sample_pos (StageDataNeon * /* stage_ptr */, float32x4_t & /* y */, float32x4_t & /* mem */)
{
	// Nothing, stops the recursion
}



template <int CUR>
void	StageProcNeon <CUR>::process_sample_neg (StageDataNeon *stage_ptr, float32x4_t &y, float32x4_t &mem)
{
	StageProcNeon <CUR - 1>::process_sample_neg (stage_ptr, y, mem);

	const float32x4_t x = mem;
	stage_ptr [PREV]._mem4 = y;

	mem = stage_ptr [CUR]._mem4;
	y  += mem;
	y  *= stage_ptr [CUR]._coef4;
	y  -= x;
}

template <>
inline void	StageProcNeon <0>::process_sample_neg (StageDataNeon * /* stage_ptr */, float32x4_t & /* y */, float32x4_t & /* mem */)
{
	// Nothing, stops the recursion
}



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



}  // namespace hiir



#endif   // hiir_StageProcNeon_CODEHEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
