/*	$Id: iff.c,v 1.18 2008/09/15 19:58:49 toad32767 Exp $ */

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

/*
 * Find an IFF chunk. Return 1 (found) or 0 (not found / error).
 * If found, update 'length' to be the chunk length.
 */
int 
find_iff_chunk(IFFType chunk, AIFF_Ref r, uint32_t * length)
{
	union cio {
		uint8_t buf[8];
		IFFChunk chk;
	} d;
	
	assert(sizeof(IFFChunk) == 8);
	chunk = ARRANGE_BE32(chunk);

	/*
	 * If possible, start the search at the first chunk
	 */
	if (!(r->flags & F_NOTSEEKABLE)) {
		if (fseek(r->fd, 12, SEEK_SET) < 0)
			return (0);
	} else {
		/*
		 * If we can't seek, don't search beyond
		 * the SSND chunk
		 */
		if (r->flags & SSND_REACHED) {
			if (chunk == ARRANGE_BE32(AIFF_SSND)) {
				r->flags &= ~SSND_REACHED;
				*length = (uint32_t) (r->soundLen);
				return (1);
			} else {
				return (0);
			}
		}
	}

	/*
	 * Navigate through the file to find the chunk
	 */
	for (;;) {
		if (fread(d.buf, 1, 8, r->fd) < 8)
			return (0);
		
		d.chk.len = ARRANGE_BE32(d.chk.len);
		if (d.chk.id == chunk) {
			*length = d.chk.len;
			return (1);
		} else {
			uint32_t l = d.chk.len;

			/*
			 * In IFF files chunk start offsets must be even.
			 */
			l += l & 1;

			/* skip this chunk */
			if (!(r->flags & F_NOTSEEKABLE)) {
				if (fseek(r->fd, (long) l, SEEK_CUR) < 0) {
					return (0);
				}
			} else {
				int count;

				if (d.chk.id == ARRANGE_BE32(AIFF_SSND)) {
					r->flags |= SSND_REACHED;
					r->soundLen = (uint64_t) (d.chk.len);
					return (0);
				}
				count = (int) l;
				while (count-- > 0) {
					if (getc(r->fd) < 0)
						return (0);
				}
			}
		}

	}

	/* NOTREACHED */
	return (0);
}

char *
get_iff_attribute(AIFF_Ref r, IFFType attrib)
{
	char *str;
	uint32_t len;
	
	if (!find_iff_chunk(attrib, r, &len))
		return NULL;

	if (!len)
		return NULL;

	str = malloc(len + 1);
	if (!str)
		return NULL;

	if (fread(str, 1, len, r->fd) < len) {
		free(str);
		return NULL;
	}
	str[len] = '\0';

	return str;
}

int 
set_iff_attribute(AIFF_Ref w, IFFType attrib, char *str)
{
	IFFChunk chk;
	uint32_t len = strlen(str);
	
	assert(sizeof(IFFChunk) == 8);
	chk.id = ARRANGE_BE32(attrib);
	chk.len = ARRANGE_BE32(len);

	if (fwrite(&chk, 1, 8, w->fd) < 8 ||
	    fwrite(str, 1, len, w->fd) < len) {
		return -1;
	}
	/*
	 * Write a pad byte if chunk length is odd,
	 * as required by the IFF specification.
	 */
	if (len & 1) {
		putc(0, w->fd);
		w->len++;
	}
	w->len += 8 + len;

	return 1;
}

#define kIFFNumAttributes 4

int
clone_iff_attributes(AIFF_Ref w, AIFF_Ref r)
{
	IFFType attrs[kIFFNumAttributes] = {AIFF_NAME, AIFF_AUTH, AIFF_COPY, AIFF_ANNO};
	int i;
	int ret, rval = 1;
	char *p;
	
	for (i = 0; i < kIFFNumAttributes; ++i) {
		if ((p = get_iff_attribute(r, attrs[i])) != NULL) {
			ret = set_iff_attribute(w, attrs[i], p);
			free(p);
			rval = (rval > 0 ? ret : rval); /* preserve previous errors */
		}
	}
	
	return rval;
}

