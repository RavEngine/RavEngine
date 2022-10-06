/*	$Id: lpcm.c,v 1.26 2009/09/11 16:51:07 toad32767 Exp $ */

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

void
lpcm_swap16(int16_t * RESTRICT dstSamples, const int16_t * RESTRICT srcSamples, int nSamples)
{
	int i;

	for (i = 0; i < nSamples; ++i) {
		dstSamples[i] = ARRANGE_ENDIAN_16(srcSamples[i]);
	}
}

void
lpcm_swap32(int32_t * RESTRICT dstSamples, const int32_t * RESTRICT srcSamples, int nSamples)
{
	int i;

	for (i = 0; i < nSamples; ++i) {
		dstSamples[i] = ARRANGE_ENDIAN_32(srcSamples[i]);
	}
}



void 
lpcm_swap_samples(int segmentSize, int flags, const void *from, void *to, int nsamples)
{
	/*
	 * Do we really need to do anything?
	 */
	if (from == to && !(flags & LPCM_NEED_SWAP))
		return;

	switch (segmentSize) {
	case 1:
		memcpy(to, from, nsamples);
		break;
	case 2:
		if (flags & LPCM_NEED_SWAP) {
			lpcm_swap16(to, from, nsamples);
		} else {
			memcpy(to, from, nsamples * sizeof(int16_t));
		}
		break;
	case 3:
		if (flags & LPCM_NEED_SWAP) {
			int i, n;
			const uint8_t *fubytes = from;
			uint8_t x, y, z, *ubytes = to;

			/*
			 * XXX -- this is gross.
			 */
			n = nsamples * 3;
			for (i = 0; i < n; i += 3) {
				x = fubytes[i];
				y = fubytes[i + 1];
				z = fubytes[i + 2];

				ubytes[i] = z;
				ubytes[i + 1] = y;
				ubytes[i + 2] = x;
			}
		} else {
			memcpy(to, from, nsamples * 3);
		}
		break;
	case 4:
		if (flags & LPCM_NEED_SWAP) {
			lpcm_swap32(to, from, nsamples);
		} else {
			memcpy(to, from, nsamples * sizeof(int32_t));
		}
		break;
	}

	return;
}

static size_t 
lpcm_read_lpcm(AIFF_Ref r, void *buffer, size_t len)
{
	int n;
	uint32_t clen;
	size_t slen;
	size_t bytes_in;
	size_t bytesToRead;

	n = len;
	len -= n % r->segmentSize;
	n /= r->segmentSize;

	slen = r->soundLen - r->pos;
	bytesToRead = MIN(len, slen);

	if (bytesToRead == 0)
		return 0;

	bytes_in = fread(buffer, 1, bytesToRead, r->fd);
	if (bytes_in > 0)
		clen = (uint32_t) bytes_in;
	else
		clen = 0;
	r->pos += clen;

	lpcm_swap_samples(r->segmentSize, r->flags, buffer, buffer, n);

	return bytes_in;
}

static int 
lpcm_seek(AIFF_Ref r, uint64_t pos)
{
        OFF_T           of;
        uint64_t        b;

	b = pos * r->nChannels * r->segmentSize;
	if (b >= r->soundLen)
		return 0;
                
	of = b;
	if (FSEEKO(r->fd, of, SEEK_CUR) < 0) {
		return -1;
	}
	r->pos = b;
	return 1;
}

/*
 * Dequantize LPCM (buffer) to floating point PCM (samples)
 */
void
lpcm_dequant(int segmentSize, void *buffer, float *outSamples, int nSamples)
{
	int i;

	switch (segmentSize) {
		case 4:
		  {
			  int32_t *integers = (int32_t *) buffer;

			  for (i = 0; i < nSamples; ++i)
				{
				  outSamples[i] = integers[i] / 2147483648.0;
				}
			  break;
		  }
		case 3:
		  {
			  uint8_t *f = (uint8_t *) buffer;
			  float *t = outSamples;
			  union {
				int32_t i;
				uint8_t b[4];
			  } u;

			  for (i = 0; i < nSamples; ++i)
				{
#ifdef LIBAIFF_BIGENDIAN
				u.b[0] = (f[0] & 0x80 ? 0xff : 0);
				u.b[1] = f[0];
				u.b[2] = f[1];
				u.b[3] = f[2];  
#else
				u.b[3] = (f[2] & 0x80 ? 0xff : 0);
				u.b[2] = f[2];
				u.b[1] = f[1];
				u.b[0] = f[0];				
#endif /* LIBAIFF_BIGENDIAN */

				*t++ = u.i * (1.0 / (1 << 23));
				f += 3;
				}
			  break;
		  }
		case 2:
		  {
			  int16_t *integers = (int16_t *) buffer;

			  for (i = 0; i < nSamples; ++i)
				{
				  outSamples[i] = integers[i] * 0.000030517578125f;
			  }
			  break;
		  }
		case 1:
		  {
			  int8_t *integers = (int8_t *) buffer;

			  for (i = 0; i < nSamples; ++i)
				{
				  outSamples[i] = integers[i] * 0.0078125f;
				}
			  break;
		  }
	}
}
			  
static int
lpcm_read_float32(AIFF_Ref r, float *buffer, int nSamples)
{
	size_t len, slen, bytesToRead, bytes_in;
	uint32_t clen;
	int nSamplesRead;
	void *buf;
	
	len = (size_t) nSamples * r->segmentSize;
	slen = (size_t) (r->soundLen) - (size_t) (r->pos);
	bytesToRead = MIN(len, slen);
	if (bytesToRead == 0)
		return 0;

	buf = AIFFBufAllocate(r, kAIFFBufConv, bytesToRead);
	if (NULL == buf)
		return 0;
	
	bytes_in = fread(buf, 1, bytesToRead, r->fd);
	if (bytes_in > 0)
		clen = (uint32_t) bytes_in;
	else
		clen = 0;
	r->pos += clen;
	nSamplesRead = (int) clen / (r->segmentSize);
	
	lpcm_swap_samples(r->segmentSize, r->flags, buf, buf, nSamplesRead);
	lpcm_dequant(r->segmentSize, buf, buffer, nSamplesRead);
	
	return nSamplesRead;
}

static int 
lpcm_write_lpcm(AIFF_Ref w, void *samples, size_t len, int readOnlyBuf)
{
	size_t   n, sampleBytes;
	void 	*buffer;
	
	n = len;
	if ((n % w->segmentSize) != 0)
		return 0;
	n /= w->segmentSize;

	if (0 != (w->flags & LPCM_NEED_SWAP) && readOnlyBuf) {
		if ((buffer = AIFFBufAllocate(w, kAIFFBufExt, len)) == NULL)
			return -1;
	} else {
		buffer = samples;
	}

	lpcm_swap_samples(w->segmentSize, w->flags, samples, buffer, n);

	if (fwrite(buffer, w->segmentSize, n, w->fd) != n) {
		return -1;
	}
        
	sampleBytes = n * w->segmentSize;
        
        w->nSamples += n;
	w->sampleBytes += sampleBytes;
	w->len += sampleBytes;

	return 1;
}

LIBAIFF_INTERNAL
struct codec lpcm = {
	AUDIO_FORMAT_LPCM,
	NULL,
	lpcm_read_lpcm,
	lpcm_read_float32,
        lpcm_write_lpcm,
	lpcm_seek,
	NULL
};

