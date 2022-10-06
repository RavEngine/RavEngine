/*	$Id: libaiff.c,v 1.50 2009/09/11 16:51:07 toad32767 Exp $	*/

/*-
 * Copyright (c) 2005, 2006, 2007, 2008 Marco Trillo
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

static struct codec* codecs[] = {
	&lpcm,
	&ulaw,
	&alaw,
	&float32,
	NULL
};

static AIFF_Ref AIFF_ReadOpen (FILE *, int);
static AIFF_Ref AIFF_WriteOpen (FILE *, int);
static void AIFF_ReadClose (AIFF_Ref);
static int AIFF_WriteClose (AIFF_Ref);
static int DoWriteSamples (AIFF_Ref, void *, size_t, int);
static int Prepare (AIFF_Ref);
static void Unprepare (AIFF_Ref);
static struct codec* FindCodec (IFFType);

AIFF_Ref
AIFF_OpenFile(const char *file, int flags)
{
	AIFF_Ref ref = NULL;
	FILE *fd = NULL;

	if (flags & F_RDONLY) {
		fd = fopen(file, "rb");
		if (fd)
			ref = AIFF_ReadOpen(fd, flags);
	} else if (flags & F_WRONLY) {
		fd = fopen(file, "wb");
		if (fd)
			ref = AIFF_WriteOpen(fd, flags);
	}

	if (fd && !ref)
		fclose(fd);

	return ref;
}

#if defined(_WIN32)
AIFF_Ref
AIFF_OpenFileW(const wchar_t *file, int flags)
{
	AIFF_Ref ref = NULL;
	FILE *fd = NULL;

	if (flags & F_RDONLY) {
		fd = _wfopen(file, L"rb");
		if (fd)
			ref = AIFF_ReadOpen(fd, flags);
	} else if (flags & F_WRONLY) {
		fd = _wfopen(file, L"wb");
		if (fd)
			ref = AIFF_WriteOpen(fd, flags);
	}

	if (fd && !ref)
		fclose(fd);

	return ref;
}
#endif

int
AIFF_CloseFile(AIFF_Ref ref)
{
	int r;
	
	if (!ref)
		return -1;
	if (ref->flags & F_RDONLY) {
		AIFF_ReadClose(ref);
		r = 1;
	} else if (ref->flags & F_WRONLY) {
		r = AIFF_WriteClose(ref);
	} else {
		r = -1;
	}
	
	return r;
}

static AIFF_Ref 
AIFF_ReadOpen(FILE *fd, int flags)
{
	AIFF_Ref r;
	IFFHeader hdr;

	r = malloc(kAIFFRecSize);
	if (!r) {
		return NULL;
	}
	r->fd = fd;
	r->flags = F_RDONLY | flags;
	if (fread(&hdr, 1, sizeof(hdr), fd) != sizeof(hdr)) {
		free(r);
		return NULL;
	}
	switch (hdr.hid) {
	case AIFF_TYPE_IFF:
		if (hdr.len == 0) {
			free(r);
			return NULL;
		}
		/*
 		 * Check the format type (AIFF or AIFC)
 		 */
		r->format = hdr.fid;
		switch (r->format) {
		case AIFF_TYPE_AIFF:
		case AIFF_TYPE_AIFC:
			break;
		default:
			free(r);
			return NULL;
		}

		if (init_aifx(r) < 1) {
			free(r);
			return NULL;
		}
		break;
	default:
		free(r);
		return NULL;
	}

	r->stat = 0;

	memset(r->buf, 0, sizeof(r->buf));

	return r;
}

char *
AIFF_GetAttribute(AIFF_Ref r, IFFType attrib)
{
	if (!r || !(r->flags & F_RDONLY))
		return NULL;
	Unprepare(r);
	
	switch (r->format) {
	case AIFF_TYPE_AIFF:
	case AIFF_TYPE_AIFC:
		return get_iff_attribute(r, attrib);
	default:
		return NULL;
	}

	/* NOTREACHED */
	return NULL;
}

int 
AIFF_ReadMarker(AIFF_Ref r, int *id, uint64_t * pos, char **name)
{
	if (!r || !(r->flags & F_RDONLY))
		return -1;
	
	switch (r->format) {
	case AIFF_TYPE_AIFF:
	case AIFF_TYPE_AIFC:
		return read_aifx_marker(r, id, pos, name);
	default:
		return 0;
	}

	/* NOTREACHED */
	return 0;
}

int 
AIFF_GetInstrumentData(AIFF_Ref r, Instrument * i)
{
	if (!r || !(r->flags & F_RDONLY))
		return (-1);
	Unprepare(r);
	
	switch (r->format) {
	case AIFF_TYPE_AIFF:
	case AIFF_TYPE_AIFC:
		return get_aifx_instrument(r, i);
	default:
		return (0);
	}
}

int 
AIFF_GetAudioFormat(AIFF_Ref r, uint64_t * nSamples, int *channels,
    double *samplingRate, int *bitsPerSample, int *segmentSize)
{
	if (NULL == r 
	    || 0 == (r->flags & (F_WRONLY | F_RDONLY))
	    || (F_WRONLY == (r->flags & F_WRONLY) && r->stat < 1))
		return -1;

	if (nSamples)
		*nSamples = r->nSamples;
	if (channels)
		*channels = r->nChannels;
	if (samplingRate)
		*samplingRate = r->samplingRate;
	if (bitsPerSample)
		*bitsPerSample = r->bitsPerSample;
	if (segmentSize)
		*segmentSize = r->segmentSize;

	return 1;
}

static int
Prepare (AIFF_Ref r)
{
	int res;
	struct codec *dec;
	
	if (r->stat != 1) {
		switch (r->format) {
		case AIFF_TYPE_AIFF:
		case AIFF_TYPE_AIFC:
			res = do_aifx_prepare(r);
			break;
		default:
			res = -1;
		}
		if (res < 1)
			return res;
		if ((dec = FindCodec(r->audioFormat)) == NULL)
			return -1;
		if (dec->construct) {
			if ((res = dec->construct(r)) < 1)
				return res;
		}

		r->codec = dec;
		r->stat = 1;
	}

	return 1;
}

static void
Unprepare (AIFF_Ref r)
{
	struct codec *dec;
	
	if (r->stat == 1) {
		dec = r->codec;
		if (dec->destroy)
			dec->destroy(r);
	}
	r->stat = 0;
}

static struct codec*
FindCodec (IFFType fmt)
{
	struct codec **dd, *d;
	
	for (dd = codecs; (d = *dd) != NULL; ++dd)
		if (d->fmt == fmt)
			return d;
	
	return NULL;
}

size_t 
AIFF_ReadSamples(AIFF_Ref r, void *buffer, size_t len)
{
	struct codec *dec;
	
	if (!r || !(r->flags & F_RDONLY) || Prepare(r) < 1)
		return 0;
	dec = r->codec;
	
	return dec->read_lpcm(r, buffer, len);
}

int
AIFF_ReadSamplesFloat(AIFF_Ref r, float *buffer, int nSamplePoints)
{
	int res;
	struct codec *dec;
	
	if (!r || !(r->flags & F_RDONLY))
		return -1;
	if (nSamplePoints % (r->nChannels) != 0)
		return 0;
	if ((res = Prepare(r)) < 1)
		return res;
	dec = r->codec;
	
	return dec->read_float32(r, buffer, nSamplePoints);
}

int 
AIFF_Seek(AIFF_Ref r, uint64_t framePos)
{
	int res = 0;
	struct codec *dec;

	if (!r || !(r->flags & F_RDONLY))
		return -1;
	if (r->flags & F_NOTSEEKABLE)
		return -1;
	Unprepare(r);
	if ((res = Prepare(r)) < 1)
		return res;
	dec = r->codec;

	return dec->seek(r, framePos);
}

int
AIFF_ReadSamples16Bit(AIFF_Ref r, int16_t * samples, unsigned int n)
{
	unsigned int 	 len, i;
	int 		 h;
	void		*buf;

	if (NULL == r || 0 == (r->flags & F_RDONLY))
		return -1;
	if (0 == n || 0 != (n % r->nChannels))
		return 0;
	len = n * r->segmentSize;
	
	if (r->segmentSize == sizeof(int16_t)) {
		return AIFF_ReadSamples(r, samples, len) / sizeof(int16_t);
	}

	buf = AIFFBufAllocate(r, kAIFFBufExt, len);
	if (NULL == buf)
		return -1;

	h = AIFF_ReadSamples(r, buf, len);
	if (-1 == h || 0 != (h % r->segmentSize))
		return -1;
	n = h / r->segmentSize;

	switch (r->segmentSize) {
	case sizeof(int8_t): {
		int8_t *p = buf;

		for (i = 0; i < n; ++i)
			samples[i] = p[i] << 8;

		break;
	}

	case sizeof(int32_t): {
		int32_t *p = buf;

		for (i = 0; i < n; ++i)
			samples[i] = p[i] >> 16;

		break;
	}

	case 3: { /* XXX -- this is gross. */
		uint8_t *rp = buf;
		uint8_t *wp = (uint8_t *) samples;

		i = n;
		while (i > 0) {
#ifdef LIBAIFF_BIGENDIAN
			*wp++ = *rp++;
			*wp++ = *rp++;
			rp++;
#else
			rp++;
			*wp++ = *rp++;
			*wp++ = *rp++;
#endif
			--i;
		}

		break;
	}

	default:
		return 0;
	}

	return n;
}

int 
AIFF_ReadSamples32Bit(AIFF_Ref r, int32_t * samples, unsigned int n)
{
	unsigned int 	 len, i;
	int 		 h;
	void		*buf;	

	if (NULL == r || 0 == (r->flags & F_RDONLY))
		return -1;
	if (0 == n || 0 != (n % r->nChannels))
		return 0;
	len = n * r->segmentSize;

	if (r->segmentSize == sizeof(int32_t)) {
		return AIFF_ReadSamples(r, samples, len) / sizeof(int32_t);
	}

	buf = AIFFBufAllocate(r, kAIFFBufExt, len);
	if (NULL == buf)
		return -1;

	h = AIFF_ReadSamples(r, buf, len);
	if (-1 == h || 0 != (h % r->segmentSize))
		return -1;
	n = h / r->segmentSize;

	switch (r->segmentSize) {
	case 3: { /* XXX -- this is gross. */
		uint8_t *rp = (uint8_t *) buf;
		uint8_t *wp = (uint8_t *) samples;

		i = n;
		while (i > 0) {
#ifdef LIBAIFF_BIGENDIAN
			*wp++ = *rp++;
			*wp++ = *rp++;
			*wp++ = *rp++;
			*wp++ = 0;
#else
			*wp++ = 0;
			*wp++ = *rp++;
			*wp++ = *rp++;
			*wp++ = *rp++;
#endif
			--i;
		}

		break;
	}

	case sizeof(int16_t): {
		int16_t *p = (int16_t *) buf;
		
		for (i = 0; i < n; ++i)
			samples[i] = p[i] << 16;
		break;
	}

	case sizeof(int8_t): {
		int8_t *p = (int8_t *) buf;
		
		for (i = 0; i < n; ++i)
			samples[i] = p[i] << 24;
		break;
	}

	default:
		return 0;
	}

	return n;
}


static void 
AIFF_ReadClose(AIFF_Ref r)
{
	int 	i;
	
	for (i = 0; i < kAIFFNBufs; ++i)
		AIFFBufDelete(r, i);

	Unprepare(r);
	fclose(r->fd);
	free(r);
}

static AIFF_Ref 
AIFF_WriteOpen(FILE *fd, int flags)
{
	AIFF_Ref w;
	IFFHeader hdr;
	assert(sizeof(IFFHeader) == 12);
	
	w = malloc(kAIFFRecSize);
	if (!w) {
		return NULL;
	}

	w->fd = fd;
	hdr.hid = ARRANGE_BE32(AIFF_FORM);
	w->len = 4;
	hdr.len = ARRANGE_BE32(4);
	if (flags & F_AIFC)
		hdr.fid = ARRANGE_BE32(AIFF_AIFC);
	else
		hdr.fid = ARRANGE_BE32(AIFF_AIFF);

	if (fwrite(&hdr, 1, sizeof(hdr), fd) != sizeof(hdr)) {
		free(w);
		return NULL;
	}
	w->stat = 0;
	w->segmentSize = 0;
	
	memset(w->buf, 0, sizeof(w->buf));

	/*
	 * If writing AIFF-C, write the required FVER chunk
	 */
	if (flags & F_AIFC) {
		IFFChunk chk;
		uint32_t vers;
		assert(sizeof(IFFChunk) == 8);

		chk.id = ARRANGE_BE32(AIFF_FVER);
		chk.len = ARRANGE_BE32(4);
		vers = ARRANGE_BE32(AIFC_STD_DRAFT_082691);

		if (fwrite(&chk, sizeof(chk), 1, fd) != 1 ||
		    fwrite(&vers, sizeof(vers), 1, fd) != 1) {
			free(w);
			return NULL;
		}

		w->len += 12;

		/*
		 * If no endianness specified for AIFF-C,
		 * default to big endian
		 */
		if (!(flags & (LPCM_LTE_ENDIAN | LPCM_BIG_ENDIAN))) {
			flags |= LPCM_BIG_ENDIAN;
		}
	} else {
		/*
		 * If writing regular AIFF, make sure we
		 * write big-endian data
		 */
		flags &= ~(LPCM_LTE_ENDIAN | LPCM_BIG_ENDIAN);
		flags |= LPCM_BIG_ENDIAN;
	}

        w->audioFormat = AUDIO_FORMAT_LPCM; /* default. */
	w->flags = F_WRONLY | flags;

	return w;
}

int 
AIFF_SetAttribute(AIFF_Ref w, IFFType attr, char *value)
{
	if (!w || !(w->flags & F_WRONLY))
		return -1;
	return set_iff_attribute(w, attr, value);
}

int
AIFF_CloneAttributes(AIFF_Ref w, AIFF_Ref r, int cloneMarkers)
{
	int rval, ret;
	int doneReadingMarkers;

	if (!w || !(w->flags & F_WRONLY))
		return -1;
	
	/*
	 * first of all, clone the IFF attributes
	 */
	rval = clone_iff_attributes(w, r);
	
	doneReadingMarkers = !cloneMarkers;
	if (!doneReadingMarkers) {
		int mId;
		uint64_t mPos;
		char *mName;
		
		if ((ret = AIFF_StartWritingMarkers(w)) < 1)
			return ret;
		
		do {
			if (AIFF_ReadMarker(r, &mId, &mPos, &mName) < 1)
				doneReadingMarkers = 1;
			else {
				ret = AIFF_WriteMarker(w, mPos, mName);
				/* Preserve previous errors. */
				rval = (rval > 0 ? ret : rval);
			}
		} while (!doneReadingMarkers);
		
		if ((ret = AIFF_EndWritingMarkers(w)) < 1)
			return ret;
	}
	
	return rval;
}

int
AIFF_SetAudioEncoding(AIFF_Ref w, IFFType fmt)
{
        if (0 != w->stat)
                return (0);
        if (NULL == FindCodec(fmt))
                return (-1);
        
        w->audioFormat = fmt;
        return (1);
}

int 
AIFF_SetAudioFormat(AIFF_Ref w, int channels, double sRate, int bitsPerSample)
{
	CommonChunk c;
	IFFChunk chk;
	IFFType enc;
	uint32_t ckLen = 18;
	const char* encName = NULL;
	uint8_t buffer[10];
	assert(sizeof(chk) == 8);
	assert(sizeof(enc) == 4);

	if (!w || !(w->flags & F_WRONLY))
		return -1;
	if (w->stat != 0)
		return (0);

	if (w->flags & F_AIFC) {
        
                switch (w->audioFormat) {
                case AUDIO_FORMAT_LPCM:
                        if (w->flags & LPCM_LTE_ENDIAN)
                                enc = AUDIO_FORMAT_sowt;
                        else
                                enc = AUDIO_FORMAT_LPCM;
                        break;
                
                case AUDIO_FORMAT_ALAW:
                case AUDIO_FORMAT_ULAW:
                        enc = w->audioFormat;
                        break;
                
                default:
                        return (-1);
                }
           
		ckLen += sizeof(enc);
		encName = get_aifx_enc_name(enc);
		ckLen += PASCALOutGetLength(encName);
	} else {
                if (w->audioFormat != AUDIO_FORMAT_LPCM)
                        return (-1);
                        
		enc = AUDIO_FORMAT_LPCM;
	}
	
	chk.id = ARRANGE_BE32(AIFF_COMM);
	chk.len = ARRANGE_BE32(ckLen);

	if (fwrite(&chk, sizeof(chk), 1, w->fd) != 1) {
		return -1;
	}
	/* Fill in the chunk */
	c.numChannels = channels;
	c.numChannels = ARRANGE_BE16(c.numChannels);
	c.numSampleFrames = 0;
	c.sampleSize = bitsPerSample;
	c.sampleSize = ARRANGE_BE16(c.sampleSize);
	ieee754_write_extended(sRate, buffer);

	/*
	 * Write out the data. Write each field independently to avoid
	 * alignment problems within the structure.
	 */
	if (fwrite(&c.numChannels, 2, 1, w->fd) != 1  
	    || fwrite(&c.numSampleFrames, 4, 1, w->fd) != 1  
	    || fwrite(&c.sampleSize, 2, 1, w->fd) != 1  
	    || fwrite(buffer, 1, 10, w->fd) != 10) {
		return -1;
	}

	/*
	 * On AIFF-C, write the encoding + encstring
	 * (encstring is a PASCAL string)
	 */
	if (w->flags & F_AIFC) {
		if (fwrite(&enc, sizeof(enc), 1, w->fd) != 1)
			return -1;
		if (PASCALOutWrite(w->fd, encName) < 2)
			return -1;
	}
		    
	/*
	 * We need to return here later to update the 'numSampleFrames' member.
	 */
	w->commonOffset = w->len + 8;
	w->len += sizeof(chk) + ckLen;
	w->bitsPerSample = bitsPerSample;
	w->segmentSize = (bitsPerSample + 7) >> 3;
	w->nChannels = channels;
	w->samplingRate = sRate;
        w->codec = FindCodec(w->audioFormat);
	w->stat = 1;

	return 1;
}

int 
AIFF_StartWritingSamples(AIFF_Ref w)
{
	IFFChunk chk;
	SoundChunk s;
	assert(sizeof(chk) == 8);
	assert(sizeof(s) == 8);

	if (!w || !(w->flags & F_WRONLY))
		return -1;
	if (w->stat != 1)
		return 0;

	chk.id = ARRANGE_BE32(AIFF_SSND);
	chk.len = ARRANGE_BE32(sizeof(s));
	if (fwrite(&chk, sizeof(chk), 1, w->fd) != 1) {
		return -1;
	}
	/* We don't use these values. */
	s.offset = 0;
	s.blockSize = 0;
	if (fwrite(&s, 1, sizeof(s), w->fd) != sizeof(s)) {
		return -1;
	}

	w->soundOffset = w->len + 8;
	w->len += sizeof(chk) + sizeof(s);
        w->nSamples = 0;
	w->sampleBytes = 0;
	w->stat = 2;

	return 1;
}

static int
DoWriteSamples(AIFF_Ref w, void *samples, size_t len, int readOnlyBuf)
{
        struct codec    *c = w->codec;
        
        if (NULL == w || !(w->flags & F_WRONLY))
		return -1;
	if (w->stat != 2)
		return 0;
        
        assert(NULL != c);

        return ((*c->write_lpcm)(w, samples, len, readOnlyBuf));
}

int
AIFF_WriteSamplesRaw(AIFF_Ref w, void *samples, size_t len)
{
	if (!w || 0 == (w->flags & F_WRONLY))
		return (-1);
	if (w->stat != 2)
		return (0);

	if (fwrite(samples, 1, len, w->fd) != len) {
		return (-1);
	}

	w->nSamples += len / w->segmentSize;
	w->sampleBytes += len;
	w->len += len;

	return (1);
}

int
AIFF_WriteSamples(AIFF_Ref w, void *samples, size_t len)
{
	return DoWriteSamples(w, samples, len, 1);
}


int 
AIFF_WriteSamples32Bit(AIFF_Ref w, int32_t * samples, int n)
{
	int i, j;
	unsigned int len;
	void *buffer;

	if (!w || !(w->flags & F_WRONLY))
		return -1;
	if (w->stat != 2 || w->segmentSize == 0 || n < 1)
		return -1;
	len = n * w->segmentSize;

	if (w->segmentSize == 4)
		return DoWriteSamples(w, samples, len, 1) >> 2;
	
	if (NULL == (buffer = AIFFBufAllocate(w, kAIFFBufExt, len)))
		return -1;

	switch (w->segmentSize) {
	case 3:
	{
		/* XXX -- this is gross. */
		uint8_t *inbytes = (uint8_t *) samples;
		uint8_t *outbytes = (uint8_t *) buffer;
		
		for (i = 0, j = 0; i < n; ++i, j += 3) {
#ifdef LIBAIFF_BIGENDIAN
			outbytes[j + 0] = inbytes[(i << 2) + 0];
			outbytes[j + 1] = inbytes[(i << 2) + 1];
			outbytes[j + 2] = inbytes[(i << 2) + 2];
#else
			outbytes[j + 0] = inbytes[(i << 2) + 1];
			outbytes[j + 1] = inbytes[(i << 2) + 2];
			outbytes[j + 2] = inbytes[(i << 2) + 3];
#endif
		}
		break;
	}

	case 2:
	{
		int16_t *words = (int16_t *) buffer;

		for (i = 0; i < n; ++i) {
			words[i] = samples[i] >> 16;
		}
		break;
	}

	case 1:
	{
		int8_t *sbytes = (int8_t *) buffer;
		
		for (i = 0; i < n; ++i) {
			sbytes[i] = samples[i] >> 24;
		}
		break;
	}
	}

	return DoWriteSamples(w, buffer, len, 0);
}

int 
AIFF_EndWritingSamples(AIFF_Ref w)
{
	IFFChunk chk;
	long of;
	uint32_t curpos, numSampleFrames;

	if (!w || !(w->flags & F_WRONLY))
		return -1;
	if (w->stat != 2)
		return 0;

	AIFFBufDelete(w, kAIFFBufExt);
	if (w->sampleBytes & 1) {
		fputc(0, w->fd);
		w->sampleBytes++;
		w->len++;
	}
	
	curpos = w->len + 8;
	of = w->soundOffset;
	
	chk.id = ARRANGE_BE32(AIFF_SSND);
	chk.len = w->sampleBytes + sizeof(SoundChunk);
	chk.len = ARRANGE_BE32(chk.len);

	if (fseek(w->fd, of, SEEK_SET) < 0 || 
	    fwrite(&chk, sizeof(chk), 1, w->fd) != 1) {
		return -1;
	}

	numSampleFrames = w->nSamples / w->nChannels;
	numSampleFrames = ARRANGE_BE32(numSampleFrames);

	/* Write out */
	of = w->commonOffset + 10;
	if (fseek(w->fd, of, SEEK_SET) < 0) {
		return -1;
	}
	if (1 != fwrite(&numSampleFrames, sizeof(numSampleFrames), 1, w->fd)) { 
		return -1;
	}
	/* Return back to current position in the file. */
	of = curpos;
	if (fseek(w->fd, of, SEEK_SET) < 0) {
		return -1;
	}
	w->stat = 3;

	return 1;
}

int 
AIFF_StartWritingMarkers(AIFF_Ref w)
{
	IFFChunk chk;
	uint16_t nMarkers = 0;

	if (!w || !(w->flags & F_WRONLY))
		return -1;
	if (w->stat != 3)
		return -1;

	chk.id = ARRANGE_BE32(AIFF_MARK);
	chk.len = ARRANGE_BE16(2);

	if (fwrite(&chk, sizeof(chk), 1, w->fd) != 1)
		return -1;
	w->len += 8;
	w->markerOffset = w->len;
	if (fwrite(&nMarkers, sizeof(nMarkers), 1, w->fd) != 1)
		return -1;
	w->len += 2;

	w->markerPos = 0;
	w->stat = 4;

	return 1;
}

int 
AIFF_WriteMarker(AIFF_Ref w, uint64_t position, char *name)
{
	Marker m;

	if (!w || !(w->flags & F_WRONLY))
		return -1;
	if (w->stat != 4)
		return -1;

	/* max. number of markers --> 0xFFFF */
	if (w->markerPos == 0xFFFF)
		return 0;

	m.id = (MarkerId) (w->markerPos + 1);
	m.id = ARRANGE_BE16(m.id);
	m.position = (uint32_t) position; /* XXX: AIFF is a 32-bit format */
	m.position = ARRANGE_BE32(m.position);

	if (fwrite(&m.id, sizeof(m.id), 1, w->fd) != 1 || 
	    fwrite(&m.position, sizeof(m.position), 1, w->fd) != 1)
		return -1;
	w->len += sizeof(m.id) + sizeof(m.position);

	if (name) {
		int l;

		if ((l = PASCALOutWrite(w->fd, name)) < 2)
			return -1;
		w->len += l;
	} else {
		if (fwrite("\0", 1, 2, w->fd) != 2)
			return -1;
		w->len += 2;
	}

	++(w->markerPos);
	return 1;
}

int 
AIFF_EndWritingMarkers(AIFF_Ref w)
{
	uint32_t cklen, curpos;
	long offset;
	uint16_t nMarkers;

	if (!w || !(w->flags & F_WRONLY))
		return -1;
	if (w->stat != 4)
		return -1;

	curpos = w->len + 8;
	cklen = w->len - w->markerOffset;
	cklen = ARRANGE_BE32(cklen);

	offset = w->markerOffset;
	
	/*
	 * Correct the chunk length
	 * and the nMarkers field
	 */
	nMarkers = w->markerPos;
	nMarkers = ARRANGE_BE16(nMarkers);

	if (fseek(w->fd, offset + 4, SEEK_SET) < 0) {
		return -1;
	}
	if (fwrite(&cklen, sizeof(cklen), 1, w->fd) != 1 ||  
	    fwrite(&nMarkers, sizeof(nMarkers), 1, w->fd) != 1) {
		return -1;
	}
	/* Return back to current writing position */
	offset = curpos;
	if (fseek(w->fd, offset, SEEK_SET) < 0) {
		return -1;
	}
	w->stat = 3;
	return 1;
}

static int 
AIFF_WriteClose(AIFF_Ref w)
{
	int i, ret = 1;
	IFFHeader hdr;

	if (w->stat != 3)
		ret = 2;
	
	hdr.hid = ARRANGE_BE32(AIFF_FORM);
	hdr.len = w->len;
	hdr.len = ARRANGE_BE32(hdr.len);
	if (w->flags & F_AIFC)
		hdr.fid = ARRANGE_BE32(AIFF_AIFC);
	else
		hdr.fid = ARRANGE_BE32(AIFF_AIFF);

	if (fseek(w->fd, 0, SEEK_SET) < 0) {
		fclose(w->fd);
		free(w);
		return -1;
	}
	if (fwrite(&hdr, 1, sizeof(hdr), w->fd) != sizeof(hdr)) {
		fclose(w->fd);
		free(w);
		return -1;
	}
	/* Now fclose, free & return */
	fclose(w->fd);

	for (i = 0; i < kAIFFNBufs; ++i)
		AIFFBufDelete(w, i);

	free(w);
	return ret;
}

/*
 *	Buffer manipulation.
 */

void
AIFFBufDelete (AIFF_Ref a, int nbuf)
{
	AIFF_Buf	*b;

	assert(0 <= nbuf && nbuf < kAIFFNBufs);

	b = &a->buf[nbuf];
	if (b->len > 0) {
		assert(NULL != b->ptr);
		free(b->ptr);
		b->len = 0;
	}
}

void *
AIFFBufAllocate (AIFF_Ref a, int nbuf, unsigned int len)
{
	AIFF_Buf	*b;

	assert(0 <= nbuf && nbuf < kAIFFNBufs);
	
	b = &a->buf[nbuf];
	if (b->len < len) {
		if (b->ptr)
			free(b->ptr);
		b->ptr = malloc(len);
		if (NULL == b->ptr)
			b->len = 0;
		else
			b->len = len;
	}

	return b->ptr;
}
