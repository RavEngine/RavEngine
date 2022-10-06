/*****************************************************************************

        PolyphaseIir2Designer.cpp
        Author: Laurent de Soras, 2005

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#if defined (_MSC_VER)
	#pragma warning (1 : 4130) // "'operator' : logical operation on address of string constant"
	#pragma warning (1 : 4223) // "nonstandard extension used : non-lvalue array converted to pointer"
	#pragma warning (1 : 4705) // "statement has no effect"
	#pragma warning (1 : 4706) // "assignment within conditional expression"
	#pragma warning (4 : 4786) // "identifier was truncated to '255' characters in the debug information"
	#pragma warning (4 : 4800) // "forcing value to bool 'true' or 'false' (performance warning)"
	#pragma warning (4 : 4355) // "'this' : used in base member initializer list"
#endif



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include "hiir/def.h"
#include "hiir/fnc.h"
#include "hiir/PolyphaseIir2Designer.h"

#include <cassert>
#include <cmath>



namespace hiir
{



/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*
==============================================================================
Name: compute_nbr_coefs_from_proto
Description:
	Finds the minimum number of coefficients for a given filter specification
Input parameters:
	- attenuation: stopband attenuation, dB. > 0.
	- transition: normalized transition bandwith. Range ]0 ; 1/2[
Returns: Number of coefficients, > 0
Throws: Nothing
==============================================================================
*/

int	PolyphaseIir2Designer::compute_nbr_coefs_from_proto (double attenuation, double transition)
{
	assert (attenuation > 0);
	assert (transition > 0);
	assert (transition < 0.5);

	double         k;
	double         q;
	compute_transition_param (k, q, transition);
	const int      order     = compute_order (attenuation, q);
	const int      nbr_coefs = (order - 1) / 2;

	return nbr_coefs;
}



/*
==============================================================================
Name: compute_atten_from_order_tbw
Description:
	Compute the attenuation correspounding to a given number of coefficients
	and the transition bandwith.
Input parameters:
	- nbr_coefs: Number of desired coefficients. > 0.
	- transition: normalized transition bandwith. Range ]0 ; 1/2[
Returns: stopband attenuation, dB. > 0.
Throws: Nothing
==============================================================================
*/

double	PolyphaseIir2Designer::compute_atten_from_order_tbw (int nbr_coefs, double transition)
{
	assert (nbr_coefs > 0);
	assert (transition > 0);
	assert (transition < 0.5);

	double         k;
	double         q;
	compute_transition_param (k, q, transition);
	const int      order       = nbr_coefs * 2 + 1;
	const double   attenuation = compute_atten (q, order);

	return attenuation;
}



/*
==============================================================================
Name: compute_coefs
Description:
	Computes coefficients for a half-band polyphase IIR filter, function of a
	given stopband gain / transition bandwidth specification.
	Order is automatically calculated.
Input parameters:
	- attenuation: stopband attenuation, dB. > 0.
	- transition: normalized transition bandwith. Range ]0 ; 1/2[
Output parameters:
	- coef_arr: Coefficient list, must be large enough to store all the
		coefficients. Filter order = nbr_coefs * 2 + 1
Returns: number of coefficients
Throws: Nothing
==============================================================================
*/

int	PolyphaseIir2Designer::compute_coefs (double coef_arr [], double attenuation, double transition)
{
	assert (attenuation > 0);
	assert (transition > 0);
	assert (transition < 0.5);

	double         k;
	double         q;
	compute_transition_param (k, q, transition);

	// Computes number of required coefficients
	const int      order     = compute_order (attenuation, q);
	const int      nbr_coefs = (order - 1) / 2;

	// Coefficient calculation
	for (int index = 0; index < nbr_coefs; ++index)
	{
		coef_arr [index] = compute_coef (index, k, q, order);
	}

	return nbr_coefs;
}



/*
==============================================================================
Name: compute_coefs_spec_order_tbw
Description:
	Computes coefficients for a half-band polyphase IIR filter, function of a
	given transition bandwidth and desired filter order. Bandstop attenuation
	is set to the maximum value for these constraints.
Input parameters:
	- nbr_coefs: Number of desired coefficients. > 0.
	- transition: normalized transition bandwith. Range ]0 ; 1/2[
Output parameters:
	- coef_arr: Coefficient list, must be large enough to store all the
		coefficients.
Throws: Nothing
==============================================================================
*/

void	PolyphaseIir2Designer::compute_coefs_spec_order_tbw (double coef_arr [], int nbr_coefs, double transition)
{
	assert (nbr_coefs > 0);
	assert (transition > 0);
	assert (transition < 0.5);

	double         k;
	double         q;
	compute_transition_param (k, q, transition);
	const int      order = nbr_coefs * 2 + 1;

	// Coefficient calculation
	for (int index = 0; index < nbr_coefs; ++index)
	{
		coef_arr [index] = compute_coef (index, k, q, order);
	}
}



/*
==============================================================================
Name: compute_phase_delay
Description:
	Computes the phase delay introduced by a single filtering unit at a
	specified frequency.
	The delay is given for a constant sampling rate between input and output.
Input parameters:
	- a: coefficient for the cell, [0 ; 1]
	- f_fs: frequency relative to the sampling rate, [0 ; 0.5].
Returns:
	The phase delay in samples, >= 0.
Throws: Nothing
==============================================================================
*/

double	PolyphaseIir2Designer::compute_phase_delay (double a, double f_fs)
{
	assert (a >= 0);
	assert (a <= 1);
	assert (f_fs >= 0);
	assert (f_fs < 0.5);

	const double   w  = 2 * hiir::PI * f_fs;
	const double   c  = cos (w);
	const double   s  = sin (w);
	const double   x  = a + c + a * (c * (a + c) + s * s);
	const double   y  = a * a * s - s;
	double         ph = atan2 (y, x);
	if (ph < 0)
	{
		ph += 2 * hiir::PI;
	}
	const double   dly = ph / w;

	return dly;
}



/*
==============================================================================
Name: compute_group_delay
Description:
	Computes the group delay introduced by a single filtering unit at a
	specified frequency.
	The delay is given for a constant sampling rate between input and output.
	To compute the group delay of a complete filter, add the group delays
	of all the units in A0 (z).
Input parameters:
	- a: coefficient for the cell, [0 ; 1]
	- f_fs: frequency relative to the sampling rate, [0 ; 0.5].
	- ph_flag: set if filtering unit is used in pi/2-phaser mode, in the form
		(a - z^-2) / (1 - az^-2)
Returns:
	The group delay in samples, >= 0.
Throws: Nothing
==============================================================================
*/

double	PolyphaseIir2Designer::compute_group_delay (double a, double f_fs, bool ph_flag)
{
	assert (a >= 0);
	assert (a <= 1);
	assert (f_fs >= 0);
	assert (f_fs < 0.5);

	const double   w   = 2 * hiir::PI * f_fs;
	const double   a2  = a * a;
	const double   sig = (ph_flag) ? -2 : 2;
	const double   dly = 2 * (1 - a2) / (a2 + sig * a * cos (2 * w) + 1);

	return dly;
}



/*
==============================================================================
Name: compute_group_delay
Description:
	Computes the group delay introduced by a complete filter at a specified
	frequency.
	The delay is given for a constant sampling rate between input and output.
Input parameters:
	- coef_arr: filter coefficient, as given by the designing functions
	- nbr_coefs: Number of filter coefficients. > 0.
	- f_fs: frequency relative to the sampling rate, [0 ; 0.5].
	- ph_flag: set if filter is used in pi/2-phaser mode, in the form
		(a - z^-2) / (1 - az^-2)
Returns:
	The group delay in samples, >= 0.
Throws: Nothing
==============================================================================
*/

double	PolyphaseIir2Designer::compute_group_delay (const double coef_arr [], int nbr_coefs, double f_fs, bool ph_flag)
{
	assert (nbr_coefs > 0);
	assert (f_fs >= 0);
	assert (f_fs < 0.5);

	double         dly_total = 0;
	for (int k = 0; k < nbr_coefs; ++k)
	{
		const double   dly = compute_group_delay (coef_arr [k], f_fs, ph_flag);
		dly_total += dly;
	}

	return dly_total;
}



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



void	PolyphaseIir2Designer::compute_transition_param (double &k, double &q, double transition)
{
	assert (transition > 0);
	assert (transition < 0.5);

	k  = tan ((1 - transition * 2) * hiir::PI / 4);
	k *= k;
	assert (k < 1);
	assert (k > 0);
	double         kksqrt = pow (1 - k * k, 0.25);
	const double   e = 0.5 * (1 - kksqrt) / (1 + kksqrt);
	const double   e2 = e * e;
	const double   e4 = e2 * e2;
	q = e * (1 + e4 * (2 + e4 * (15 + 150 * e4)));
	assert (q > 0);
}



int	PolyphaseIir2Designer::compute_order (double attenuation, double q)
{
	assert (attenuation > 0);
	assert (q > 0);

	const double   attn_p2 = pow (10.0, -attenuation / 10);
	const double   a       = attn_p2 / (1 - attn_p2);
	int            order   = hiir::ceil_int (log (a * a / 16) / log (q));
	if ((order & 1) == 0)
	{
		++ order;
	}
	if (order == 1)
	{
		order = 3;
	}

	return order;
}



double	PolyphaseIir2Designer::compute_atten (double q, int order)
{
	assert (q > 0);
	assert (order > 0);
	assert ((order & 1) == 1);

	const double   a           = 4 * exp (order * 0.5 * log (q));
	assert (a != -1.0);
	const double   attn_p2     = a / (1 + a);
	const double   attenuation = -10 * log10 (attn_p2);
	assert (attenuation > 0);

	return attenuation;
}



double	PolyphaseIir2Designer::compute_coef (int index, double k, double q, int order)
{
	assert (index >= 0);
	assert (index * 2 < order);

	const int      c    = index + 1;
	const double   num  = compute_acc_num (q, order, c) * pow (q, 0.25);
	const double   den  = compute_acc_den (q, order, c) + 0.5;
	const double   ww   = num / den;
	const double   wwsq = ww * ww;

	const double   x    = sqrt ((1 - wwsq * k) * (1 - wwsq / k)) / (1 + wwsq);
	const double   coef = (1 - x) / (1 + x);

	return coef;
}



double	PolyphaseIir2Designer::compute_acc_num (double q, int order, int c)
{
	assert (c >= 1);
	assert (c < order * 2);

	int            i   = 0;
	int            j   = 1;
	double         acc = 0;
	double         q_ii1;
	do
	{
		q_ii1  = hiir::ipowp (q, i * (i + 1));
		q_ii1 *= sin ((i * 2 + 1) * c * hiir::PI / order) * j;
		acc   += q_ii1;

		j = -j;
		++i;
	}
	while (fabs (q_ii1) > 1e-100);

	return acc;
}



double	PolyphaseIir2Designer::compute_acc_den (double q, int order, int c)
{
	assert (c >= 1);
	assert (c < order * 2);

	int            i = 1;
	int            j = -1;
	double         acc = 0;
	double         q_i2;
	do
	{
		q_i2  = hiir::ipowp (q, i * i);
		q_i2 *= cos (i * 2 * c * hiir::PI / order) * j;
		acc  += q_i2;

		j = -j;
		++i;
	}
	while (fabs (q_i2) > 1e-100);

	return acc;
}



}	// namespace hiir



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
