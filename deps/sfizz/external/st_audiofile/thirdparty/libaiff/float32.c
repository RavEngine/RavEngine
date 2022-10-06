/*	$Id: float32.c,v 1.22 2009/09/11 16:51:07 toad32767 Exp $ */

/*-
 * Copyright (c) 2005, 2006, 2007 Marco Trillo
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

#include "private.h"

#if !defined IEEE754_NATIVE
# define IEEE754_NATIVE 1
#endif

/*
 * IEEE-754 32-bit single-precision floating point
 *
 * This is a conversor to linear 32-bit integer PCM
 * using only integer arithmetic.
 * 
 * Some IEEE-754 documentation and references on the WWW:
 * -------------------------------------------------------------
 * http://stevehollasch.com/cgindex/coding/ieeefloat.html
 * http://www.answers.com/topic/ieee-floating-point-standard
 * http://www.cs.berkeley.edu/~wkahan/ieee754status/IEEE754.PDF
 * http://www.psc.edu/general/software/packages/ieee/ieee.html
 * http://www.duke.edu/~twf/cps104/floating.html
 * -------------------------------------------------------------
 */

static int32_t
float32dec(uint32_t in)
{
	int sgn, exp;
	uint32_t mantissa;

	if (in == 0 || in == 0x80000000)
		return (0);

	sgn = in >> 31;
	exp = (in >> 23) & 0xFF;
	mantissa = in & 0x7FFFFF;

	if (exp == 255) {
		/*
		 * Infinity and NaN.
		 * XXX: what we should do with these?
		 */
		if (mantissa == 0) /* infinity */
			return (int32_t) (0x7FFFFFFF);
		else /* NaN */
			return (0);
	} else if (exp == 0) {
		exp = -126; /* denormalized number */
	} else {
		/* Normalized numbers */
		mantissa |= 0x800000; /* hidden bit */
		exp -= 127; /* unbias exponent */
	}
	
	/*
	 * Quantization to linear PCM 31 bits.
	 * 
	 * Multiply the mantissa by 2^(-23+exp) to get the floating
	 * point value, and then multiply by 2^31 to quantize
	 * the floating point to 31 bits. So:
	 * 
	 * 2^(-23+exp) * 2^31 = 2^(8 + exp)
	 */
	exp += 8;
	if (exp < 0) {
		mantissa >>= -exp;
	} else {
		mantissa <<= exp;
	}
	
	if (sgn) {
		if (mantissa == 0x80000000)
			return (-2147483647 - 1); /* -(2^31) */
		else {
			return -((int32_t) mantissa);
		}
	} else {
		/*
		 * if mantissa is 0x80000000 (for a 1.0 float),
		 * we should return +2147483648 (2^31), but in
		 * two's complement arithmetic the max. positive value
		 * is +2147483647, so clip the result.
		 */
		if (mantissa == 0x80000000)
			return (2147483647);
		else
			return ((int32_t) mantissa);
	}
}

static void
float32_decode(void *dst, void *src, int n)
{
	uint32_t *srcSamples = (uint32_t *) src;
	int32_t *dstSamples = (int32_t *) dst;

	while (n-- > 0)
		dstSamples[n] = float32dec(srcSamples[n]);
}

static void
float32_swap_samples(void *buffer, int n)
{
	lpcm_swap32(buffer, buffer, n);
}

static size_t 
float32_read_lpcm(AIFF_Ref r, void *buffer, size_t len)
{
	int n;
	uint32_t clen;
	size_t slen;
	size_t bytes_in;
	size_t bytesToRead;
	void *buf;

	n = len >> 2;
	len &= ~3;
	
	slen = (size_t) (r->soundLen) - (size_t) (r->pos);
	bytesToRead = MIN(len, slen);

	if (0 == bytesToRead || 
	    NULL == (buf = AIFFBufAllocate(r, kAIFFBufConv, bytesToRead)))
		return 0;

	bytes_in = fread(buf, 1, bytesToRead, r->fd);
	if (bytes_in > 0)
		clen = (uint32_t) bytes_in;
	else
		clen = 0;
	r->pos += clen;
	
	if (r->flags & LPCM_NEED_SWAP)
		float32_swap_samples(buf, n);
	float32_decode(buffer, buf, n);

	return bytes_in;
}

static int 
float32_seek(AIFF_Ref r, uint64_t pos)
{
	long of;
	uint32_t b;

	b = (uint32_t) pos * r->nChannels * 4;
	if (b >= r->soundLen)
		return 0;
	of = (long) b;

	if (fseek(r->fd, of, SEEK_CUR) < 0) {
		return -1;
	}
	r->pos = b;
	return 1;
}

/* 
 * Infinite & NAN values
 * for non-IEEE systems
 */
#ifndef HUGE_VAL
# ifdef HUGE
#  define INFINITE_VALUE	HUGE
#  define NAN_VALUE		HUGE
# endif
#else
# define INFINITE_VALUE	HUGE_VAL
# define NAN_VALUE	HUGE_VAL
#endif

#if ! IEEE754_NATIVE

/*
 * Write IEEE Single Precision Numbers.
 */
static uint32_t
ieee754_write_single(float in)
{
	uint32_t        sgn, mantissa;
	int             exp;
	double          fraction;
	
	if (in == 0.0)
		return 0;
	
	if (in < 0.0) {
		in = (float) fabs(in);
		sgn = 1;
	} else {
		sgn = 0;
	}

	/*
	 * Grab the exponent and mantissa. This function will return 24-bit
	 * mantissas of type 0.1F, having F 23 bits of precision.
	 */
	fraction = ldexp(frexp(in, &exp), 24);
	mantissa = (uint32_t) floor(fraction);

	if (exp > 128) {
		mantissa = 0;	/* infinity */
		exp = 255;
		goto done;
	}

	/*
	 * If the exponent is -126 or lssser, we use 'denormalised numbers'
	 * with 0.F mantissas, having F 23 bits of precision.
	 */
	if (exp <= -126) {
		mantissa >>= 1 - exp - 126;
		exp = 0;
		goto done;
	}

	/*
	 * Any other number is stored as a 'normalised number', with a 
	 * mantissa of type 1.F with 23 bits of precision for F.
	 */

	mantissa &= 0x7FFFFF;	/* hidden bit */
	exp += 127 - 1;		/* biased exponent */

done:
	return ((sgn << 31) | ((exp & 0xff) << 23) | mantissa);
}

# define IEEE754_TEST_VALUE	-1.12877

/*
 * ieee754_native(): check if host supports single-precision IEEE-754 natively.
 */
static int
ieee754_native(void)
{
	union {
		uint32_t w;
		float f;
	} u;
	uint32_t t;

	if (sizeof(u) != sizeof(uint32_t) ||
	    sizeof(u) != sizeof(float))
		return (0);
	u.f = IEEE754_TEST_VALUE;
	t = ieee754_write_single(IEEE754_TEST_VALUE);

	return (u.w == t);
}

/*
 * Read IEEE Single Precision Numbers.
 */
static float
ieee754_read_single(uint32_t in)
{
	uint32_t        mantissa;
	int             sgn, exp;
	double          fraction;
	float           out;

	if (in == 0)
		return 0.0;
	else if (in == 0x80000000)
		return -0.0;

	sgn = in >> 31;
	exp = (in >> 23) & 0xFF;
	mantissa = in & 0x7FFFFF;

	switch (exp) {
	case 255:
		if (mantissa == 0) {
			return (sgn ? -INFINITE_VALUE : INFINITE_VALUE);
		} else {	
			return (sgn ? -NAN_VALUE : NAN_VALUE);
		}
	case 0:
		/* Denormalised numbers */
		exp = -126;
		break;
	default:
		/* Normalised numbers */
		mantissa |= 0x800000;	/* hidden bit */
		exp -= 127;	/* unbias exponent */
	}

	fraction = (double) mantissa;
	out = (float) ldexp(fraction, -23 + exp);

	return (sgn ? -out : out);
}

#endif /* ! IEEE754_NATIVE */

static int
float32_read_float32(AIFF_Ref r, float *buffer, int n)
{
	int nSamplesRead;
	uint32_t clen;
	size_t len, slen;
	size_t bytes_in;
	size_t bytesToRead;
	
	len = n << 2;
	slen = r->soundLen - r->pos;
	bytesToRead = MIN(len, slen);
	if (bytesToRead == 0)
		return 0;
	
#if ! IEEE754_NATIVE
	/*
	 * Check if this host supports 32-bit IEEE floats
	 * natively and take note about it to avoid doing
	 * the test each time a buffer is to be read...
	 */
	if (!(r->flags & F_IEEE754_CHECKED)) {
		r->flags |= F_IEEE754_CHECKED;
		if (ieee754_native())
			r->flags |= F_IEEE754_NATIVE;
	}
	
	if (r->flags & F_IEEE754_NATIVE) {
#endif
		bytes_in = fread((void *) buffer, 1, bytesToRead, r->fd);
		
		if (bytes_in > 0) {
			nSamplesRead = (int) bytes_in >> 2;
			if (r->flags & LPCM_NEED_SWAP)
				float32_swap_samples((void *) buffer, nSamplesRead);
		} else {
			nSamplesRead = 0;
		}
		
#if ! IEEE754_NATIVE
	} else {
		void	*buf = AIFFBufAllocate(r, kAIFFBufConv, bytesToRead);

		if (buf == NULL)
			return 0;
		
		bytes_in = fread(buf, 1, bytesToRead, r->fd);
		if (bytes_in > 0) {
			uint32_t *dwords = buf;
			
			nSamplesRead = (int) bytes_in >> 2;
			if (r->flags & LPCM_NEED_SWAP)
				float32_swap_samples(dwords, nSamplesRead);
			
			while (nSamplesRead-- > 0)
				buffer[nSamplesRead] = ieee754_read_single(dwords[nSamplesRead]);
		} else {
			nSamplesRead = 0;
		}
	}
#endif
	
	if (bytes_in > 0)
		clen = (uint32_t) bytes_in;
	else
		clen = 0;
	r->pos += clen;
	
	return nSamplesRead;
}


LIBAIFF_INTERNAL
struct codec float32 = {
	AUDIO_FORMAT_FL32,
	NULL,
	float32_read_lpcm,
	float32_read_float32,
        NULL,
	float32_seek,
	NULL
};

