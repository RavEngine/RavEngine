/* 	$Id: g711.c,v 1.3 2009/09/11 16:51:07 toad32767 Exp $ */
/* 	Id: ulaw.c,v 1.10 2007/01/07 14:47:32 toad32767 Exp    */
/* 	Id: alaw.c,v 1.6 2007/01/07 14:47:32 toad32767 Exp     */

/*-
 * Copyright (c) 2006, 2007, 2009 Marco Trillo.
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

static  int8_t expt[128] = {0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4,
                            5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
                            6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
                            6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
                            7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                            7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                            7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                            7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7};
                            
/* 
 * G.711 mu-Law 
 */

static int16_t
ulawdec(uint8_t x)
{
	int sgn, exp, mant;
	int y;

	x = ~x;                         /* bits are sent reversed */
	sgn = x & 0x80;                 /* sign bit */
	mant = ((x & 0xF) << 1) | 0x21;	/* mantissa plus hidden bits */
	exp = (x & 0x70) >> 4;          /* exponent */
	mant = (mant << exp) - 0x21;    /* get mantissa, unraise */

	y = (sgn ? -mant : mant);
	return (y << 2);
}

static uint8_t
ulawenc(int16_t x)
{
        int     sgn, exp;
        uint8_t out;
        
        x >>= 2;
        sgn = x < 0;                               /* get sgn */
        if (0 != sgn)
                x = -x;                            /* get 13-bit magnitude */
        if (x > 0x1FDE)
                x = 0x1FDE;                        /* clip for raising... */
                
        x = (x + 0x21) >> 1;                       /* raise output -> 5 bits */
        exp = expt[x >> 5];                        /* get exponent */
        x >>= exp;                                 /* get mantissa */
        
        out = (sgn << 7) | (exp << 4) | (x & 0xF); /* pack */
        return (~out);                             /* reverse */       
}


/* 
 * G.711 A-Law 
 */

static int16_t
alawdec(uint8_t x)
{
	int             sgn, exp, mant;
	int             y;

	x = (~x & 0xD5) | (x & 0x2A);   /* even bits (ex. sign) are reversed */
	sgn = x & 0x80;
	mant = ((x & 0xF) << 1) | 0x21; /* mantissa plus hidden bits */
	exp = (x & 0x70) >> 4;          /* exponent */

	if (0 == exp)
		mant &= ~0x20;          /* denormalized */
	else
		mant <<= exp - 1;

	y = (sgn ? -mant : mant);
	return (y << 3);
}

static  uint8_t
alawenc(int16_t x)
{
        int sgn, exp;
        uint8_t out;
        
        x >>= 4;
        sgn = x < 0;                                /* get sgn */
        if (0 != sgn)
                x = -x;                             /* get 11-bit magnitude */
                
        exp = expt[x >> 4];                         /* get exponent */
        if (0 != exp)
                x >>= exp - 1;                      /* hidden bit */
        
        out = (sgn << 7) | (exp << 4) | (x & 0xF);  /* pack */
        return ((~out & 0xD5) | (out & 0x2A));      /* reverse */   
}

static int
g711_ulaw_create(AIFF_Ref r)
{
	int             i;
	int16_t        *p = malloc(256 * sizeof(int16_t));
	if (p != NULL) {
		for (i = 0; i < 256; ++i)
			p[i] = ulawdec(i);
		r->pdata = p;
		return 1;
	} else
		return -1;
}

static int
g711_alaw_create(AIFF_Ref r)
{
	int             i;
	int16_t        *p = malloc(256 * sizeof(int16_t));
	if (p != NULL) {
		for (i = 0; i < 256; ++i)
			p[i] = alawdec(i);
		r->pdata = p;
		return 1;
	} else
		return -1;
}

static void
g711_delete(AIFF_Ref r)
{
	free(r->pdata);
}

static          size_t
g711_read_lpcm(AIFF_Ref r, void *buffer, size_t len)
{
	size_t          n, i, rem, bytesToRead, bytesRead;
	uint8_t        *bytes;
	int16_t        *samples, *table = r->pdata;
	void           *buf;

	/* length must be even */
	len &= ~1;

	n = len >> 1;
	rem = r->soundLen - r->pos;
	bytesToRead = MIN(n, rem);
	if (bytesToRead == 0)
		return 0;

	buf = AIFFBufAllocate(r, kAIFFBufConv, bytesToRead);
	if (NULL == buf)
		return 0;
	
	bytesRead = fread(buf, 1, bytesToRead, r->fd);
	if (bytesRead > 0) {
		r->pos += bytesRead;
	} else {
		return 0;
	}

	bytes = buf;
	samples = buffer;
	for (i = 0; i < bytesRead; ++i) {
		samples[i] = table[bytes[i]];
	}

	return (bytesRead << 1);
}

static int
g711_seek(AIFF_Ref r, uint64_t pos)
{
	uint64_t        b;
	OFF_T           of;

	b = pos * r->nChannels;
	if (b >= r->soundLen)
		return 0;
                
	of = b;
	if (FSEEKO(r->fd, of, SEEK_CUR) < 0) {
		return -1;
	}
        
	r->pos = b;
	return 1;
}

static int
g711_read_float32(AIFF_Ref r, float *buffer, int nFrames)
{
	size_t          n = nFrames, i, rem, bytesToRead, bytesRead;
	uint8_t        *bytes;
	int16_t        *table = r->pdata;
	void           *buf;

	rem = r->soundLen - r->pos;
	bytesToRead = MIN(n, rem);
	if (bytesToRead == 0)
		return 0;

	buf = AIFFBufAllocate(r, kAIFFBufConv, bytesToRead);
	if (NULL == buf)
		return 0;
	
	bytesRead = fread(buf, 1, bytesToRead, r->fd);
	if (bytesRead > 0) {
		r->pos += bytesRead;
	} else {
		return 0;
	}

	bytes = buf;
	for (i = 0; i < bytesRead; ++i) {
		buffer[i] = table[bytes[i]] * 0.000030517578125f;
	}

	return bytesRead;       /* = framesRead */
}

static int 
g711_write_lpcm(AIFF_Ref w, void *inptr, size_t ilen, int readOnlyBuf)
{
        uint8_t  *outb;
        int16_t  *inb;
        uint8_t (*f)(int16_t);
        int       i, n;
        
        if (2 != w->segmentSize)
                return (-1);
        
        n = ilen >> 1;
        
	if (readOnlyBuf) {
		if ((outb = AIFFBufAllocate(w, kAIFFBufExt, n)) == NULL)
			return -1;
	} else {
		outb = inptr;
	}
        inb = inptr;
        
        switch (w->audioFormat) {
        case AUDIO_FORMAT_ULAW: f = ulawenc; break;
        case AUDIO_FORMAT_ALAW: f = alawenc; break;
        default:
                assert(0);
        }
        
        for (i = 0; i < n; i++) {
                outb[i] = (*f)(inb[i]);
        }
        if (fwrite(outb, 1, n, w->fd) != (size_t)n) {
                return (-1);
        }
        
        w->nSamples += n;
        w->sampleBytes += n;
        w->len += n;
        
        return (1);
}

LIBAIFF_INTERNAL
struct codec  ulaw = {
	AUDIO_FORMAT_ULAW,
	g711_ulaw_create,
	g711_read_lpcm,
	g711_read_float32,
        g711_write_lpcm,
	g711_seek,
	g711_delete
};

LIBAIFF_INTERNAL
struct codec  alaw = {
	AUDIO_FORMAT_ALAW,
	g711_alaw_create,
	g711_read_lpcm,
	g711_read_float32,
        g711_write_lpcm,
	g711_seek,
	g711_delete
};

