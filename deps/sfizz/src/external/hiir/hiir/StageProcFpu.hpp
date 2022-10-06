/*****************************************************************************

        StageProcFpu.hpp
        Author: Laurent de Soras, 2005

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#if defined (hiir_StageProc_CURRENT_CODEHEADER)
	#error Recursive inclusion of StageProcFpu code header.
#endif
#define hiir_StageProc_CURRENT_CODEHEADER

#if ! defined (hiir_StageProc_CODEHEADER_INCLUDED)
#define hiir_StageProc_CODEHEADER_INCLUDED



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#if defined (_MSC_VER)
	#pragma inline_depth (255)
#endif



namespace hiir
{



/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



template <>
hiir_FORCEINLINE void	StageProcFpu <1>::process_sample_pos (const int nbr_coefs, float &spl_0, float &/*spl_1*/, const float coef [], float x [], float y [])
{
	const int      last = nbr_coefs - 1;
	const float    temp = (spl_0 - y [last]) * coef [last] + x [last];
	x [last] = spl_0;
	y [last] = temp;
	spl_0    = temp;
}



template <>
hiir_FORCEINLINE void	StageProcFpu <0>::process_sample_pos (const int /*nbr_coefs*/, float &/*spl_0*/, float &/*spl_1*/, const float /*coef*/ [], float /*x*/ [], float /*y*/ [])
{
	// Nothing (stops recursion)
}



template <int REMAINING>
void	StageProcFpu <REMAINING>::process_sample_pos (const int nbr_coefs, float &spl_0, float &spl_1, const float coef [], float x [], float y [])
{
	const int      cnt    = nbr_coefs - REMAINING;

	const float    temp_0 =
		(spl_0 - y [cnt + 0]) * coef [cnt + 0] + x [cnt + 0];
	const float    temp_1 =
		(spl_1 - y [cnt + 1]) * coef [cnt + 1] + x [cnt + 1];

	x [cnt + 0] = spl_0;
	x [cnt + 1] = spl_1;

	y [cnt + 0] = temp_0;
	y [cnt + 1] = temp_1;

	spl_0       = temp_0;
	spl_1       = temp_1;

	StageProcFpu <REMAINING - 2>::process_sample_pos (
		nbr_coefs,
		spl_0,
		spl_1,
		&coef [0],
		&x [0],
		&y [0]
	);
}



template <>
hiir_FORCEINLINE void	StageProcFpu <1>::process_sample_neg (const int nbr_coefs, float &spl_0, float &/*spl_1*/, const float coef [], float x [], float y [])
{
	const int      last = nbr_coefs - 1;
	const float    temp = (spl_0 + y [last]) * coef [last] - x [last];
	x [last] = spl_0;
	y [last] = temp;
	spl_0    = temp;
}



template <>
hiir_FORCEINLINE void	StageProcFpu <0>::process_sample_neg (const int /*nbr_coefs*/, float &/*spl_0*/, float &/*spl_1*/, const float /*coef*/ [], float /*x*/ [], float /*y*/ [])
{
	// Nothing (stops recursion)
}



template <int REMAINING>
void	StageProcFpu <REMAINING>::process_sample_neg (const int nbr_coefs, float &spl_0, float &spl_1, const float coef [], float x [], float y [])
{
	const int      cnt    = nbr_coefs - REMAINING;

	const float    temp_0 =
		(spl_0 + y [cnt + 0]) * coef [cnt + 0] - x [cnt + 0];
	const float    temp_1 =
		(spl_1 + y [cnt + 1]) * coef [cnt + 1] - x [cnt + 1];

	x [cnt + 0] = spl_0;
	x [cnt + 1] = spl_1;

	y [cnt + 0] = temp_0;
	y [cnt + 1] = temp_1;

	spl_0       = temp_0;
	spl_1       = temp_1;

	StageProcFpu <REMAINING - 2>::process_sample_neg (
		nbr_coefs,
		spl_0,
		spl_1,
		&coef [0],
		&x [0],
		&y [0]
	);
}



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



}  // namespace hiir



#endif   // hiir_StageProc_CODEHEADER_INCLUDED

#undef hiir_StageProc_CURRENT_CODEHEADER



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
