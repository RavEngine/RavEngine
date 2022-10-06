/*	$Id: pascal.c,v 1.3 2008/08/23 15:48:18 toad32767 Exp $ */

/*-
 * Copyright (c) 2006 Marco Trillo
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
 * PASCAL-style strings I/O
 */

/*
 * return num. of bytes to skip the PASCAL string
 */
int
PASCALInGetLength (FILE * fd)
{
	int count;

	if ((count = fgetc(fd)) < 0)
		return (-1);
	return (count + !(count & 1));
}

/*
 * read and return the PASCAL string (allocate the needed
 * memory). Update 'length' to be the total num. of bytes read
 */
char *
PASCALInRead (FILE * fd, int * length)
{
	int c, l, r, fr;
	char *str;

	if ((c = fgetc(fd)) < 0) {
		*length = -1;
		return (NULL);
	}
	r = 1;
	l = c + !(c & 1); /* pad */
	if ((str = malloc(c + 1)) == NULL) {
		goto ret;
	}
	if ((fr = fread(str, 1, l, fd)) != l) {
		free(str);
		str = NULL;
		r += fr;
		goto ret;
	}
	r += l;
	str[c] = 0; /* NUL terminator */
	
ret:
	*length = r;
	return (str);
}

/*
 * return the num. of bytes needed to write
 * 'str' as a PASCAL string, but don't write
 * anything
 */
int
PASCALOutGetLength (const char * str)
{
	int l = strlen(str);
	l = MIN(l, 0xFF);
	return (1 + l + !(l & 1));
}

/*
 * write 'str' to file 'fd' as a PASCAL string 
 * and return the num. of bytes written
 */
int
PASCALOutWrite (FILE * fd, const char * str)
{
	int l = strlen(str), w;
	
	l = MIN(l, 0xFF);
	if (fputc(l, fd) < 0) {
		return (-1);
	}
	w = 1;
	l += !(l & 1); /* pad */
	/*
	 * The C-string NUL terminator will serve
	 * as a pad byte if necessary
	 */
	w += fwrite(str, 1, l, fd);
	
	return (w);
}


