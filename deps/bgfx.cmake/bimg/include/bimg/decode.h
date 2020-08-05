/*
 * Copyright 2011-2020 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bimg#license-bsd-2-clause
 */

#ifndef BIMG_DECODE_H_HEADER_GUARD
#define BIMG_DECODE_H_HEADER_GUARD

#include "bimg.h"

namespace bimg
{
	///
	ImageContainer* imageParse(
		  bx::AllocatorI* _allocator
		, const void* _data
		, uint32_t _size
		, TextureFormat::Enum _dstFormat = TextureFormat::Count
		, bx::Error* _err = NULL
		);

} // namespace bimg

#endif // BIMG_DECODE_H_HEADER_GUARD
