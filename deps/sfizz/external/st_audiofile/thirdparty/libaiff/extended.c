/*	$Id: extended.c,v 1.13 2008/08/18 15:40:21 toad32767 Exp $ */

/*-
 * Copyright (c) 2005, 2006, 2008 Marco Trillo
 *
 * Permission is hereby granted, free of charge, to any
 * person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the
 * Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the
 * Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice
 * shall be included in all copies or substantial portions of
 * the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
 * OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * 	These routines read/write 80-bit extended-precision floating point
 * 	numbers in the format used by the Motorola 68881, Motorola 68882,
 * 	Motorola 68040 and Intel 80x87 FPUs.
 */

#include "private.h"

#if defined(__GNUC__) && __LDBL_MANT_DIG__ == 64 && \
    __LDBL_MIN_EXP__ == -16381 && __LDBL_MAX_EXP__ == 16384
#define HAVE_INTEL_80x87
#endif

#ifdef HAVE_INTEL_80x87

static void
byteswap(const uint8_t *buffer, uint8_t *data)
{
        uint16_t p;

        /* XXX - big endian */
        p = *((const uint16_t *) &buffer[8]);
        *((uint16_t *) &data[0]) = ARRANGE_ENDIAN_16(p);
        p = *((const uint16_t *) &buffer[6]);
        *((uint16_t *) &data[2]) = ARRANGE_ENDIAN_16(p);
        p = *((const uint16_t *) &buffer[4]);
        *((uint16_t *) &data[4]) = ARRANGE_ENDIAN_16(p);
        p = *((const uint16_t *) &buffer[2]);
        *((uint16_t *) &data[6]) = ARRANGE_ENDIAN_16(p);
        p = *((const uint16_t *) &buffer[0]);
        *((uint16_t *) &data[8]) = ARRANGE_ENDIAN_16(p);
}

double
ieee754_read_extended(const uint8_t *buffer)
{
	long double x = 0;
	byteswap(buffer, (uint8_t *)&x);
	return (double)x;
}

void
ieee754_write_extended(double d, uint8_t *out)
{
	long double x = (long double)d;
	byteswap((const uint8_t*)&x, out);
}

#else /* HAVE_INTEL_80x87 */

#include <math.h>
#include <string.h>


/* XXX infinite and NaN values */
#ifndef HUGE_VAL
#ifdef HUGE
#define INFINITE_VALUE	HUGE
#define NAN_VALUE	HUGE
#endif
#else
#define INFINITE_VALUE	HUGE_VAL
#define NAN_VALUE	HUGE_VAL
#endif


/*
 * IEEE 754 Extended Precision
 * 
 * Implementation here is the 80-bit extended precision
 * format of Motorola 68881, Motorola 68882 and Motorola
 * 68040 FPUs, as well as Intel 80x87 FPUs.
 * 
 * See:
 *    http://www.freescale.com/files/32bit/doc/fact_sheet/BR509.pdf
 */
/*
 * Exponent range: [-16383,16383]
 * Precision for mantissa: 64 bits with no hidden bit
 * Bias: 16383
 */

/*
 * Write IEEE Extended Precision Numbers.
 */
void
ieee754_write_extended(double in, uint8_t* out)
{
	int sgn, exp, shift;
	uint32_t high, low;
	double fraction, t;

	if (in == 0.0) {
		memset(out, 0, 10);
		return;
	}
	if (in < 0.0) {
		in = fabs(in);
		sgn = 1;
	} else
		sgn = 0;

	fraction = frexp(in, &exp);

	if (exp > 16384) {
		high = low = 0; /* infinity */
		exp = 32767;
		goto done;
	}
	fraction = ldexp(fraction, 32);
	t = floor(fraction);
	high = (uint32_t) t;
	fraction -= t;
	t = floor(ldexp(fraction, 32));
	low = (uint32_t) t;

	if (exp < -16382) {
		shift = 0 - exp - 16382;
		low >>= shift;
		low |= (high << (32 - shift));
		high >>= shift;
		exp = -16382;
	}
	exp += 16383 - 1;	/* bias */

done:
	/* big endian */
	out[0] = (sgn << 7) | (exp >> 8);
	out[1] = exp & 0xff;
	out[2] = high >> 24;
	out[3] = (high >> 16) & 0xff;
	out[4] = (high >> 8) & 0xff;
	out[5] = high & 0xff;
	out[6] = low >> 24;
	out[7] = (low >> 16) & 0xff;
	out[8] = (low >> 8) & 0xff;
	out[9] = low & 0xff;
}


/*
 * Read IEEE Extended Precision Numbers.
 */
double
ieee754_read_extended(const uint8_t* in)
{
	int sgn, exp;
	uint32_t high, low;
	double out;

	/* big endian */
	sgn = in[0] >> 7;
	exp = ((in[0] & 0x7f) << 8) | in[1];
	high = (in[2] << 24)
	     | (in[3] << 16)
	     | (in[4] << 8)
	     | in[5];
	low = (in[6] << 24)
	    | (in[7] << 16)
	    | (in[8] << 8)
	    | in[9];

	if (exp == 0 && high == 0 && low == 0)
		return (sgn ? -0.0 : 0.0);

	if (exp == 32767) {
		if (high == 0 && low == 0)
			return (sgn ? -INFINITE_VALUE : INFINITE_VALUE);
		else
			return (sgn ? -NAN_VALUE : NAN_VALUE);
	} else {
		exp -= 16383;	/* unbias exponent */
	}

	out = ldexp((double) high, -31 + exp);
	out += ldexp((double) low, -63 + exp);

	return (sgn ? -out : out);
}

#endif /* ! HAVE_INTEL_80x87 */


