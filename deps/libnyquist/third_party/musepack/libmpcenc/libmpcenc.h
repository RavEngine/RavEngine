/*
 * Musepack audio compression
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#pragma once

#include <mpc/mpc_types.h>
#include <stdio.h>

// FIXME : define this somewhere else
#ifndef NULL
#define NULL 0
#endif

#define MPC_FRAME_LENGTH (36 * 32)
#define MAX_FRAME_SIZE 4352

typedef struct {
	mpc_uint16_t	Code;        // >= 14 bit
	mpc_uint16_t	Length;      // >=  4 bit
} Huffman_t;

typedef struct {
	mpc_uint_t pos; // next free byte position in the buffer
	mpc_uint_t bitsCount; // number of used bits in bitsBuff
	mpc_uint64_t outputBits; // Counter for the number of written bits in the bitstream
	mpc_uint32_t bitsBuff; // bits buffer
	mpc_uint8_t * buffer; // Buffer for bitstream-file
	mpc_uint_t framesInBlock; // Number of frames in current block
	mpc_uint_t frames_per_block_pwr; // Number of frame in a block = 1 << frames_per_block_pwr

	// seeking
	mpc_uint32_t * seek_table;
	mpc_uint32_t seek_pos; /// current position in the seek table
	mpc_uint32_t seek_ref; /// reference position for the seek information
	mpc_uint32_t seek_ptr; /// position of the seek pointer block
	mpc_uint32_t seek_pwr; /// keep a seek table entry every 2^seek_pwr block
	mpc_uint32_t block_cnt; /// number of encoded blocks

	FILE * outputFile; // ouput file

	mpc_uint32_t MS_Channelmode;
	mpc_uint32_t Overflows; //       = 0;      // number of internal (filterbank) clippings
	mpc_uint32_t MaxBand; /// number of non zero bands in last frame

 	mpc_int32_t   SCF_Index_L [32] [3];
	mpc_int32_t   SCF_Index_R [32] [3];       // holds scalefactor-indices
	mpc_int32_t   SCF_Last_L  [32];
	mpc_int32_t   SCF_Last_R  [32];           // Last coded SCF value
	mpc_quantizer Q [32];                     // holds quantized samples
	mpc_int32_t   Res_L [32];
	mpc_int32_t   Res_R [32];                 // holds the chosen quantizer for each subband
	mpc_bool_t    DSCF_Flag_L [32];
	mpc_bool_t    DSCF_Flag_R [32];           // differential SCF used?
	mpc_bool_t    MS_Flag[32];                // MS used?
 } mpc_encoder_t;

 void mpc_encoder_init ( mpc_encoder_t * e,
						 mpc_uint64_t SamplesInWAVE,
						 unsigned int FramesBlockPwr,
						 unsigned int SeekDistance );
 void mpc_encoder_exit ( mpc_encoder_t * e );
 void writeStreamInfo ( mpc_encoder_t*e,
						const unsigned int  MaxBand,
						const unsigned int  MS_on,
						const unsigned int  SamplesCount,
						const unsigned int  SamplesSkip,
						const unsigned int  SampleFreq,
						const unsigned int  ChannelCount);
 void writeGainInfo ( mpc_encoder_t * e,
					  unsigned short t_gain,
					  unsigned short t_peak,
					  unsigned short a_gain,
					  unsigned short a_peak);
 void writeEncoderInfo ( mpc_encoder_t * e,
						 const float profile,
						 const int PNS_on,
						 const int version_major,
						 const int version_minor,
						 const int version_build );
 mpc_uint32_t writeBlock ( mpc_encoder_t *, const char *, const mpc_bool_t, mpc_uint32_t);
 void writeMagic (mpc_encoder_t * e);
 void emptyBits(mpc_encoder_t * e);

 /// maximum number of output bits is 31 !
 static mpc_inline void writeBits (mpc_encoder_t * e, mpc_uint32_t input, unsigned int bits )
 {
	 e->outputBits += bits;

	 if (e->bitsCount + bits > sizeof(e->bitsBuff) * 8) {
		 int tmp = (sizeof(e->bitsBuff) * 8 - e->bitsCount);
		 bits -= tmp;
		 e->bitsBuff = (e->bitsBuff << tmp) | (input >> bits);
		 e->bitsCount = sizeof(e->bitsBuff) * 8;
		 emptyBits(e);
		 input &= (1 << bits) - 1;
	 }
	 e->bitsBuff = (e->bitsBuff << bits) | input;
	 e->bitsCount += bits;
 }
 void writeSeekTable (mpc_encoder_t * e);
 void writeBitstream_SV8 ( mpc_encoder_t*, int);

 unsigned int encodeSize(mpc_uint64_t, char *, mpc_bool_t);
 void encodeEnum(mpc_encoder_t * e, const mpc_uint32_t bits, const mpc_uint_t N);
 void encodeLog(mpc_encoder_t * e, mpc_uint32_t value, mpc_uint32_t max);


