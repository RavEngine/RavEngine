/*	$Id: endian.h,v 1.5 2006/12/27 17:10:40 toad32767 Exp $ */

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


/* === Endian-related stuff === */

#if defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__)
# if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#  define LIBAIFF_BIGENDIAN
# endif
#elif defined(_WIN32)
// always little-endian
#else
# error Unknown endianness for this platform
#endif

#define ARRANGE_ENDIAN_16(dat) ( (((dat) & 0xff00 ) >> 8 ) | (((dat) & 0x00ff ) << 8 ) )
#define ARRANGE_ENDIAN_32(dat) ( (((dat) & 0xff000000 ) >> 24 ) | (((dat) & 0x00ff0000 ) >> 8 ) | (((dat) & 0x0000ff00 ) << 8 ) | (((dat) & 0x000000ff ) << 24 ) )

#ifdef LIBAIFF_BIGENDIAN
# define ARRANGE_BE16(dat) (dat)
# define ARRANGE_BE32(dat) (dat)
# define ARRANGE_LE16(dat) ARRANGE_ENDIAN_16(dat)
# define ARRANGE_LE32(dat) ARRANGE_ENDIAN_32(dat)
#else
# define ARRANGE_BE16(dat) ARRANGE_ENDIAN_16(dat)
# define ARRANGE_BE32(dat) ARRANGE_ENDIAN_32(dat)
# define ARRANGE_LE16(dat) (dat)
# define ARRANGE_LE32(dat) (dat)
#endif /* LIBAIFF_BIGENDIAN */



