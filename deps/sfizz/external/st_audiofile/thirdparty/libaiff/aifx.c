/*	$Id: aifx.c,v 1.26 2008/08/23 15:48:18 toad32767 Exp $ */

/*-
 * Copyright (c) 2005, 2006 Marco Trillo
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


int 
init_aifx(AIFF_Ref r)
{
	int bps, wSegmentSize;
	double sRate;
	uint8_t buffer[10];
	uint32_t len;
	CommonChunk p;
	IFFType aFmt;

	if (!find_iff_chunk(AIFF_COMM, r, &len))
		return (-1);

	if (len < 18)
		return (-1);

	if (fread(&(p.numChannels), 1, 2, r->fd) < 2 ||
	    fread(&(p.numSampleFrames), 1, 4, r->fd) < 4 ||
	    fread(&(p.sampleSize), 1, 2, r->fd) < 2 ||
	    fread(buffer, 1, 10, r->fd) < 10)
		return (-1);

	p.numChannels = ARRANGE_BE16(p.numChannels);
	p.numSampleFrames = ARRANGE_BE32(p.numSampleFrames);
	p.sampleSize = ARRANGE_BE16(p.sampleSize);
	sRate = ieee754_read_extended(buffer);

	r->nSamples = p.numSampleFrames;
	r->nChannels = p.numChannels;
	r->samplingRate = sRate;
	r->bitsPerSample = p.sampleSize;

	bps = p.sampleSize;
	wSegmentSize = (bps + 7) >> 3;
	r->segmentSize = wSegmentSize;

	if (len >= 22 && r->format == AIFF_TYPE_AIFC) {
		if (fread(&aFmt, 1, 4, r->fd) < 4)
			return (-1);
		switch (aFmt) {
		case AUDIO_FORMAT_LPCM:	/* 'NONE' */
		case AUDIO_FORMAT_lpcm:	/* 'lpcm' (not standard) */
		case AUDIO_FORMAT_twos:	/* 'twos' */
			r->audioFormat = AUDIO_FORMAT_LPCM;
			r->flags |= LPCM_BIG_ENDIAN;
			break;
			
		case AUDIO_FORMAT_ULAW: /* 'ULAW' */
		case AUDIO_FORMAT_ulaw: /* 'ulaw' */
			r->audioFormat = AUDIO_FORMAT_ULAW;
			r->segmentSize = 2;
			r->bitsPerSample = 14;
			break;
		
		case AUDIO_FORMAT_ALAW: /* 'ALAW' */
		case AUDIO_FORMAT_alaw: /* 'alaw' */
			r->audioFormat = AUDIO_FORMAT_ALAW;
			r->segmentSize = 2;
			r->bitsPerSample = 13;
			break;
			
		case AUDIO_FORMAT_sowt:	/* 'sowt' */
			r->audioFormat = AUDIO_FORMAT_LPCM;
			r->flags |= LPCM_LTE_ENDIAN;
			break;

		case AUDIO_FORMAT_FL32: /* 'FL32' */
		case AUDIO_FORMAT_fl32: /* 'fl32' */
			r->audioFormat = AUDIO_FORMAT_FL32;
			r->segmentSize = 4;
			r->bitsPerSample = 32;
			r->flags |= LPCM_BIG_ENDIAN;
			break;
				
		default:
			r->audioFormat = AUDIO_FORMAT_UNKNOWN;
		}

		/*
		 * Read the description string if 
		 * the F_NOTSEEKABLE flag is set
		 */
		if (len > 22 && (r->flags & F_NOTSEEKABLE)) {
			int count = PASCALInGetLength(r->fd);
			while (count-- > 0) {
				if (getc(r->fd) < 0)
					return (-1);
			}
		}
	} else {
		r->audioFormat = AUDIO_FORMAT_LPCM;
		r->flags |= LPCM_BIG_ENDIAN;
	}

	return (1);
}

int 
read_aifx_marker(AIFF_Ref r, int *id, uint64_t * position, char **name)
{
	uint16_t nMarkers;
	uint32_t cklen;
	int n;
	Marker m;

	if (r->stat != 2) {
		if (!find_iff_chunk(AIFF_MARK, r, &cklen))
			return (0);
		if (cklen < 2)
			return (-1);
		if (fread(&nMarkers, 1, 2, r->fd) < 2)
			return (-1);
		nMarkers = ARRANGE_BE16(nMarkers);
		r->nMarkers = nMarkers;
		r->markerPos = 0;
		r->stat = 2;
	}
	n = r->nMarkers;
	if (r->markerPos >= n) {
		r->stat = 0;
		return (0);
	}
	if (fread(&(m.id), 1, 2, r->fd) < 2 || 
	    fread(&(m.position), 1, 4, r->fd) < 4)
		return (-1);
	m.id = ARRANGE_BE16(m.id);
	m.position = ARRANGE_BE32(m.position);
	
	if (name != NULL) {
		int l;
		*name = PASCALInRead(r->fd, &l);
	} else {
		int l = PASCALInGetLength(r->fd);
		
		if (!(r->flags & F_NOTSEEKABLE)) {
			if (fseek(r->fd, (long) l, SEEK_CUR) < 0)
				return (-1);
		} else {
			while (l-- > 0) {
				if (getc(r->fd) < 0)
					return (-1);
			}
		}
	}

	*id = m.id;
	*position = m.position;
	r->markerPos++;

	return (1);
}

int 
get_aifx_instrument(AIFF_Ref r, Instrument * inpi)
{
	int i;
	uint32_t cklen;
	int8_t buffer[6];
	int16_t gain;
	AIFFLoop sustainLoop, releaseLoop;
	int ids[4];
	int id;
	uint32_t pos;
	uint32_t positions[4];
	char *name;

	if (!find_iff_chunk(AIFF_INST, r, &cklen))
		return (0);
	if (cklen != 20)
		return (0);
	if (fread(buffer, 1, 6, r->fd) < 6)
		return (0);
	if (fread(&gain, 1, 2, r->fd) < 2)
		return (0);
	if (fread(&sustainLoop, 1, 6, r->fd) < 6)
		return (0);
	if (fread(&releaseLoop, 1, 6, r->fd) < 6)
		return (0);

	inpi->baseNote = buffer[0];
	inpi->detune = buffer[1];
	inpi->lowNote = buffer[2];
	inpi->highNote = buffer[3];
	inpi->lowVelocity = buffer[4];
	inpi->highVelocity = buffer[5];
	inpi->gain = ARRANGE_BE16(gain);
	inpi->sustainLoop.playMode = ARRANGE_BE16(sustainLoop.playMode);
	inpi->releaseLoop.playMode = ARRANGE_BE16(releaseLoop.playMode);

	/* Read the MarkerId`s for the positions */
	ids[0] = ARRANGE_BE16(sustainLoop.beginLoop);
	ids[1] = ARRANGE_BE16(sustainLoop.endLoop);
	ids[2] = ARRANGE_BE16(releaseLoop.beginLoop);
	ids[3] = ARRANGE_BE16(releaseLoop.endLoop);

	/* Read the positions */
	memset(positions, 0, 16 /* 4*4 */ );	/* by default set them to 0 */
	for (;;) {
		uint64_t p;
		if (read_aifx_marker(r, &id, &p, &name) < 1) {
			break;
		}
		pos = p;
		if (name)
			free(name);
		for (i = 0; i < 4; ++i) {
			if (id == ids[i])
				positions[i] = pos;
		}
	}

	inpi->sustainLoop.beginLoop = positions[0];
	inpi->sustainLoop.endLoop = positions[1];
	inpi->releaseLoop.beginLoop = positions[2];
	inpi->releaseLoop.endLoop = positions[3];

	return (1);
}

int 
do_aifx_prepare(AIFF_Ref r)
{
	uint32_t clen;
	SoundChunk s;
	long of;
	assert(sizeof(SoundChunk) == 8);

	if (!find_iff_chunk(AIFF_SSND, r, &clen))
		return (-1);
	if (clen < 8)
		return (-1);
	clen -= 8;
	r->soundLen = clen;
	r->pos = 0;
	if (fread(&s, 1, 8, r->fd) < 8) {
		return (-1);
	}
	s.offset = ARRANGE_BE32(s.offset);
	if (s.offset)
		r->soundLen -= s.offset;
	of = s.offset;

	/*
	 * FIXME: What is s.blockSize?
	 */
	if (of > 0) {
		if (!(r->flags & F_NOTSEEKABLE)) {
			if (fseek(r->fd, of, SEEK_CUR) < 0)
				return (-1);
		} else {
			while (of-- > 0) {
				if (getc(r->fd) < 0)
					return (-1);
			}
		}
	}

	return (1);
}

struct s_enc_name {
	IFFType enc;
	const char *name;
};
#define kNumEncs	6
static struct s_enc_name encNames[kNumEncs] = {
	{AUDIO_FORMAT_LPCM, "Signed integer (big-endian) linear PCM"},
	{AUDIO_FORMAT_twos, "Signed integer (big-endian) linear PCM"},
	{AUDIO_FORMAT_sowt, "Signed integer (little-endian) linear PCM"},
	{AUDIO_FORMAT_FL32, "Signed IEEE-754 single precision (big-endian) "
	                    "floating point PCM"},
	{AUDIO_FORMAT_ULAW, "Signed 8-bit mu-Law floating point PCM"},
	{AUDIO_FORMAT_ALAW, "Signed 8-bit A-Law floating point PCM"}
};

const char *
get_aifx_enc_name(IFFType enc)
{
	int i;
	struct s_enc_name* e = encNames;

	for (i = 0; i < kNumEncs; ++i) {
		if (e[i].enc == enc)
			return (e[i].name);
	}

	return (NULL);
}

