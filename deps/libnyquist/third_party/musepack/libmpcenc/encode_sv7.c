/*
 * Musepack audio compression
 * Copyright (c) 2005-2009, The Musepack Development Team
 * Copyright (C) 1999-2004 Buschmann/Klemm/Piecha/Wolf
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

#include <stdlib.h>
#include <stdio.h>

#include "libmpcenc.h"
#include <mpc/minimax.h>

void Klemm ( void );
void Init_Skalenfaktoren ( void );

// huffsv7.c
extern Huffman_t const HuffBands [33];
extern Huffman_t const HuffRes [2][17];
extern Huffman_t const HuffSCFI_1 [4];         // contains tables for SV7-scalefactor select
extern Huffman_t const HuffSCFI_2 [16];         // contains tables for SV7-scalefactor select
extern Huffman_t const HuffDSCF_1 [64];         // contains tables for SV7-scalefactor coding
extern Huffman_t const HuffDSCF_2 [65];         // contains tables for SV7-scalefactor coding
extern Huffman_t const * const HuffQ [2][8];         // points to tables for SV7-sample coding
extern Huffman_t const HuffQ9up [256];

/*
 *  SV1:   DATE 13.12.1998
 *  SV2:   DATE 12.06.1999
 *  SV3:   DATE 19.10.1999
 *  SV4:   DATE 20.10.1999
 *  SV5:   DATE 18.06.2000
 *  SV6:   DATE 10.08.2000
 *  SV7:   DATE 23.08.2000
 *  SV7.f: DATE 20.07.2002
 */

// initialize SV8
void
mpc_encoder_init ( mpc_encoder_t * e,
				   mpc_uint64_t SamplesInWAVE,
				   unsigned int FramesBlockPwr,
				   unsigned int SeekDistance )
{
	Init_Skalenfaktoren ();
	Klemm    ();

	memset(e, 0, sizeof(*e));

	if (SeekDistance > 15)
		SeekDistance = 1;
	if (FramesBlockPwr > 14)
		FramesBlockPwr = 6;

	e->seek_pwr = SeekDistance;
	e->frames_per_block_pwr = FramesBlockPwr;

	if (SamplesInWAVE == 0)
		e->seek_table = malloc((1 << 16) * sizeof(mpc_uint32_t));
	else
		e->seek_table = malloc((size_t)(2 + SamplesInWAVE / (MPC_FRAME_LENGTH << (e->seek_pwr + e->frames_per_block_pwr))) * sizeof(mpc_uint32_t));

	e->buffer = malloc(MAX_FRAME_SIZE * (1 << e->frames_per_block_pwr) * sizeof(mpc_uint8_t));
}

void
mpc_encoder_exit ( mpc_encoder_t * e )
{
	free(e->seek_table);
	free(e->buffer);
}

// writes replay gain info
void writeGainInfo ( mpc_encoder_t * e,
					 unsigned short t_gain,
					 unsigned short t_peak,
					 unsigned short a_gain,
					 unsigned short a_peak)
{
	writeBits ( e, 1,  8 ); // version
	writeBits ( e, t_gain,  16 ); // Title gain
	writeBits ( e, t_peak,  16 ); // Title peak
	writeBits ( e, a_gain,  16 ); // Album gain
	writeBits ( e, a_peak,  16 ); // Album peak
}

// writes SV8-header
void
writeStreamInfo ( mpc_encoder_t*e,
				  const unsigned int  MaxBand,
                  const unsigned int  MS_on,
                  const unsigned int  SamplesCount,
				  const unsigned int  SamplesSkip,
                  const unsigned int  SampleFreq,
				  const unsigned int  ChannelCount)
{
	unsigned char tmp[10];
	int i, len;

    writeBits ( e, 8,  8 );    // StreamVersion

	len = encodeSize(SamplesCount, (char *)tmp, MPC_FALSE);
	for( i = 0; i < len; i++) // nb of samples
		writeBits ( e, tmp[i], 8 );
	len = encodeSize(SamplesSkip, (char *)tmp, MPC_FALSE);
	for( i = 0; i < len; i++) // nb of samples to skip at beginning
		writeBits ( e, tmp[i], 8 );

	switch ( SampleFreq ) {
		case 44100: writeBits ( e, 0, 3 ); break;
		case 48000: writeBits ( e, 1, 3 ); break;
		case 37800: writeBits ( e, 2, 3 ); break;
		case 32000: writeBits ( e, 3, 3 ); break;
		default   : fprintf(stderr, "Internal error\n");// FIXME : stderr_printf ( "Internal error\n");
		exit (1);
	}

	writeBits ( e, MaxBand - 1  ,  5 );    // Bandwidth
	writeBits ( e, ChannelCount - 1  ,  4 );    // Channels
	writeBits ( e, MS_on        ,  1 );    // MS-Coding Flag
	writeBits ( e, e->frames_per_block_pwr >> 1,  3 );    // frames per block (log4 unit)
}

// writes encoder signature
void writeEncoderInfo ( mpc_encoder_t * e,
						const float profile,
						const int PNS_on,
						const int version_major,
						const int version_minor,
						const int version_build )
{
	writeBits ( e, (mpc_uint32_t)(profile * 8 + .5),  7 );
	writeBits ( e, PNS_on,  1 );
	writeBits ( e, version_major,  8 );
	writeBits ( e, version_minor,  8 );
	writeBits ( e, version_build,  8 );
}

// formatting and writing SV8-bitstream for one frame
void
writeBitstream_SV8 ( mpc_encoder_t* e, int MaxBand)
{
	int n;
	const Huffman_t * Table, * Tables[2];
	mpc_int32_t * Res_L = e->Res_L;
	mpc_int32_t * Res_R = e->Res_R;
	mpc_bool_t * DSCF_Flag_L = e->DSCF_Flag_L;
	mpc_bool_t * DSCF_Flag_R = e->DSCF_Flag_R;
	mpc_int32_t * SCF_Last_L = e->SCF_Last_L;
	mpc_int32_t * SCF_Last_R = e->SCF_Last_R;

	for( n = MaxBand; n >= 0; n--)
		if (Res_L[n] != 0 || Res_R[n] != 0) break;

	n++;
	if (e->framesInBlock == 0) {
		encodeLog(e, n, MaxBand + 1);
		MaxBand = e->MaxBand = n;
	} else {
		n = n - e->MaxBand;
		MaxBand = e->MaxBand = n + e->MaxBand;
		if (n < 0) n += 33;
		writeBits(e, HuffBands[n].Code, HuffBands[n].Length);
	}

	/************************************ Resolution *********************************/

	if (MaxBand) {
		{
			int tmp = Res_L[MaxBand - 1];
			if (tmp < 0) tmp += 17;
			writeBits(e, HuffRes[0][tmp].Code, HuffRes[0][tmp].Length);
			tmp = Res_R[MaxBand - 1];
			if (tmp < 0) tmp += 17;
			writeBits(e, HuffRes[0][tmp].Code, HuffRes[0][tmp].Length);
		}
		for ( n = MaxBand - 2; n >= 0; n--) {
			int tmp = Res_L[n] - Res_L[n + 1];
			if (tmp < 0) tmp += 17;
			writeBits(e, HuffRes[Res_L[n + 1] > 2][tmp].Code, HuffRes[Res_L[n + 1] > 2][tmp].Length);

			tmp = Res_R[n] - Res_R[n + 1];
			if (tmp < 0) tmp += 17;
			writeBits(e, HuffRes[Res_R[n + 1] > 2][tmp].Code, HuffRes[Res_R[n + 1] > 2][tmp].Length);
		}

		if (e->MS_Channelmode > 0) {
			mpc_uint32_t tmp = 0;
			int cnt = 0, tot = 0;
			mpc_bool_t * MS_Flag = e->MS_Flag;
			for( n = 0; n < MaxBand; n++) {
				if ( Res_L[n] != 0 || Res_R[n] != 0 ) {
					tmp = (tmp << 1) | MS_Flag[n];
					cnt += MS_Flag[n];
					tot++;
				}
			}
			encodeLog(e, cnt, tot);
			if (cnt * 2 > tot) tmp = ~tmp;
			encodeEnum(e, tmp, tot);
		}
	}

	/************************************ SCF encoding type ***********************************/

	if (e->framesInBlock == 0){
		for( n = 0; n < 32; n++)
			DSCF_Flag_L[n] = DSCF_Flag_R[n] = 1; // new block -> force key frame
	}

	Tables[0] = HuffSCFI_1;
	Tables[1] = HuffSCFI_2;
	for ( n = 0; n < MaxBand; n++ ) {
		int tmp = 0, cnt = -1;
		if (Res_L[n]) {
			tmp = (e->SCF_Index_L[n][1] == e->SCF_Index_L[n][0]) * 2 + (e->SCF_Index_L[n][2] == e->SCF_Index_L[n][1]);
			cnt++;
		}
		if (Res_R[n]) {
			tmp = (tmp << 2) | ((e->SCF_Index_R[n][1] == e->SCF_Index_R[n][0]) * 2 + (e->SCF_Index_R[n][2] == e->SCF_Index_R[n][1]));
			cnt++;
		}
		if (cnt >= 0)
			writeBits(e, Tables[cnt][tmp].Code, Tables[cnt][tmp].Length);
	}

	/************************************* SCF **********************************/

	for ( n = 0; n < MaxBand; n++ ) {
		if ( Res_L[n] ) {
			int m;
			mpc_int32_t * SCFI_L_n = e->SCF_Index_L[n];
			if (DSCF_Flag_L[n] == 1) {
				writeBits(e, SCFI_L_n[0] + 6, 7);
				DSCF_Flag_L[n] = 0;
			} else {
				unsigned int tmp = (SCFI_L_n[0] - SCF_Last_L[n] + 31) & 127;
				if (tmp < 64)
					writeBits(e, HuffDSCF_2[tmp].Code, HuffDSCF_2[tmp].Length);
				else {
					writeBits(e, HuffDSCF_2[64].Code, HuffDSCF_2[64].Length);
					writeBits(e, tmp - 64, 6);
				}
			}
			for( m = 0; m < 2; m++){
				if (SCFI_L_n[m+1] != SCFI_L_n[m]) {
					unsigned int tmp = (SCFI_L_n[m+1] - SCFI_L_n[m] + 31) & 127;
					if (tmp < 64)
						writeBits(e, HuffDSCF_1[tmp].Code, HuffDSCF_1[tmp].Length);
					else {
						writeBits(e, HuffDSCF_1[31].Code, HuffDSCF_1[31].Length);
						writeBits(e, tmp - 64, 6);
					}
				}
			}
			SCF_Last_L[n] = SCFI_L_n[2];
		}
		if ( Res_R[n] ) {
			int m;
			mpc_int32_t * SCFI_R_n = e->SCF_Index_R[n];
			if (DSCF_Flag_R[n] == 1) {
				writeBits(e, SCFI_R_n[0] + 6, 7);
				DSCF_Flag_R[n] = 0;
			} else {
				unsigned int tmp = (SCFI_R_n[0] - SCF_Last_R[n] + 31) & 127;
				if (tmp < 64)
					writeBits(e, HuffDSCF_2[tmp].Code, HuffDSCF_2[tmp].Length);
				else {
					writeBits(e, HuffDSCF_2[64].Code, HuffDSCF_2[64].Length);
					writeBits(e, tmp - 64, 6);
				}
			}
			for( m = 0; m < 2; m++){
				if (SCFI_R_n[m+1] != SCFI_R_n[m]) {
					unsigned int tmp = (SCFI_R_n[m+1] - SCFI_R_n[m] + 31) & 127;
					if (tmp < 64)
						writeBits(e, HuffDSCF_1[tmp].Code, HuffDSCF_1[tmp].Length);
					else {
						writeBits(e, HuffDSCF_1[31].Code, HuffDSCF_1[31].Length);
						writeBits(e, tmp - 64, 6);
					}
				}
			}
			SCF_Last_R[n] = SCFI_R_n[2];
		}
	}

	/*********************************** Samples *********************************/
	for ( n = 0; n < MaxBand; n++ ) {
		int Res = Res_L[n];
		const mpc_int16_t * q = e->Q[n].L;
		static const unsigned int thres[] = {0, 0, 3, 7, 9, 1, 3, 4, 8};
		static const int HuffQ2_var[5*5*5] =
		{6, 5, 4, 5, 6, 5, 4, 3, 4, 5, 4, 3, 2, 3, 4, 5, 4, 3, 4, 5, 6, 5, 4, 5, 6, 5, 4, 3, 4, 5, 4, 3, 2, 3, 4, 3, 2, 1, 2, 3, 4, 3, 2, 3, 4, 5, 4, 3, 4, 5, 4, 3, 2, 3, 4, 3, 2, 1, 2, 3, 2, 1, 0, 1, 2, 3, 2, 1, 2, 3, 4, 3, 2, 3, 4, 5, 4, 3, 4, 5, 4, 3, 2, 3, 4, 3, 2, 1, 2, 3, 4, 3, 2, 3, 4, 5, 4, 3, 4, 5, 6, 5, 4, 5, 6, 5, 4, 3, 4, 5, 4, 3, 2, 3, 4, 5, 4, 3, 4, 5, 6, 5, 4, 5, 6};

		do {
			int k = 0, idx = 1, cnt = 0, sng;
			switch ( Res ) {
				case -1:
				case  0:
					break;
				case  1:
					Table = HuffQ [0][0];
					for( ; k < 36; ){
						int kmax = k + 18;
						cnt = 0, sng = 0;
						for ( ; k < kmax; k++) {
							idx <<= 1;
							if (q[k] != 1) {
								cnt++;
								idx |= 1;
								sng = (sng << 1) | (q[k] >> 1);
							}
						}
						writeBits(e, Table[cnt].Code, Table[cnt].Length);
						if (cnt > 0) {
							if (cnt > 9) idx = ~idx;
							encodeEnum(e, idx, 18);
							writeBits(e, sng, cnt);
						}
					}
					break;
				case  2:
					Tables[0] = HuffQ [0][1];
					Tables[1] = HuffQ [1][1];
					idx = 2 * thres[Res];
					for ( ; k < 36; k += 3) {
						int tmp = q[k] + 5*q[k+1] + 25*q[k+2];
						writeBits ( e, Tables[idx > thres[Res]][tmp].Code,
									Tables[idx > thres[Res]][tmp].Length );
						idx = (idx >> 1) + HuffQ2_var[tmp];
					}
					break;
				case  3:
				case  4:
					Table = HuffQ [0][Res - 1];
					for ( ; k < 36; k += 2 ) {
						int tmp = q[k] + thres[Res]*q[k+1];
						writeBits ( e, Table[tmp].Code, Table[tmp].Length );
					}
					break;
				case  5:
				case  6:
				case  7:
				case  8:
					Tables[0] = HuffQ [0][Res - 1];
					Tables[1] = HuffQ [1][Res - 1];
					idx = 2 * thres[Res];
					for ( ; k < 36; k++ ) {
						int tmp = q[k] - (1 << (Res - 2)) + 1;
						writeBits ( e, Tables[idx > thres[Res]][q[k]].Code,
									Tables[idx > thres[Res]][q[k]].Length );
						if (tmp < 0) tmp = -tmp;
						idx = (idx >> 1) + tmp;
					}
					break;
				default:
					for ( ; k < 36; k++ ) {
						writeBits ( e, HuffQ9up[q[k] >> (Res - 9)].Code,
									HuffQ9up[q[k] >> (Res - 9)].Length );
						if (Res != 9)
							writeBits ( e, q[k] & ((1 << (Res - 9)) - 1), Res - 9);
					}
					break;
			}

			Res = Res_R[n];
		} while (q == e->Q[n].L && (q = e->Q[n].R));
	}

	e->framesInBlock++;
	if (e->framesInBlock == (1 << e->frames_per_block_pwr)) {
		if ((e->block_cnt & ((1 << e->seek_pwr) - 1)) == 0) {
			e->seek_table[e->seek_pos] = ftell(e->outputFile);
			e->seek_pos++;
		}
		e->block_cnt++;
		writeBlock(e, "AP", MPC_FALSE, 0);
	}
}

#if 0

typedef struct {
	int Symbol;
	unsigned int Count;
	unsigned int Code;
	unsigned int Bits;
} huff_sym_t;

void _Huffman_MakeTree( huff_sym_t *sym, unsigned int num_symbols);
void _Huffman_PrintCodes(huff_sym_t * sym, unsigned int num_symbols, int print_type, int offset);

void print_histo(void)
{
	int i, j;
	huff_sym_t sym[HISTO_NB][HISTO_LEN];
	unsigned int dist[HISTO_NB];
	unsigned int size[HISTO_NB];
	unsigned int cnt[HISTO_NB];
	unsigned int total_cnt, total_size, full_count = 0, full_size = 0;
	double optim_size, full_optim = 0;

	return;

	memset(dist, 1, sizeof dist);
	memset(sym, 0, sizeof(huff_sym_t) * HISTO_LEN * HISTO_NB);

	for(j = 0 ; j < HISTO_NB ; j++) {
		for(i = 0 ; i < HISTO_LEN; i++) {
			sym[j][i].Symbol = i;
			sym[j][i].Count = histo[j][i];
			if (sym[j][i].Count == 0)
				sym[j][i].Count = 1;
		}
		_Huffman_MakeTree(sym[j], HISTO_LEN);
		_Huffman_PrintCodes(sym[j], HISTO_LEN, 3, 0);
		_Huffman_PrintCodes(sym[j], HISTO_LEN, 0, 0);
		_Huffman_PrintCodes(sym[j], HISTO_LEN, 1, 0);
		total_cnt = 0;
		total_size = 0;
		optim_size = 0;
		for( i = 0; i < HISTO_LEN; i++) {
			total_cnt += sym[j][i].Count;
			total_size += sym[j][i].Count * sym[j][i].Bits;
			if (sym[j][i].Count != 0)
				optim_size += sym[j][i].Count * __builtin_log2(sym[j][i].Count);
		}
		full_count += total_cnt;
		full_size += total_size;
		optim_size = total_cnt * __builtin_log2(total_cnt) - optim_size;
		full_optim += optim_size;
		size[j] = total_size;
		cnt[j] = total_cnt;
		printf("%u count : %u huff : %f bps ", j, total_cnt, (float)total_size / total_cnt);
		printf("opt : %f bps ", (float)optim_size / total_cnt);
		printf("loss : %f bps (%f %%)\n", (float)(total_size - optim_size) / total_cnt, (float)(total_size - optim_size) * 100 / optim_size);
		for( i = 0; i < HISTO_LEN; i++){
			printf("%u ", sym[j][i].Bits);
		}

		printf("\n\n");
	}
	printf("cnt : %u size %f optim %f\n", full_count, (float)full_size / full_count, (float)full_optim / full_count);
	printf("loss : %f bps (%f %%)\n", (float)(full_size - full_optim) / full_count, (float)(full_size - full_optim) * 100 / full_optim);


	printf("\n");
}

void
Dump ( const unsigned int* q, const int Res )
{
    switch ( Res ) {
    case  1:
        for ( k = 0; k < 36; k++, q++ )
            printf ("%2d%c", *q-1, k==35?'\n':' ');
        break;
    case  2:
        for ( k = 0; k < 36; k++, q++ )
            printf ("%2d%c", *q-2, k==35?'\n':' ');
        break;
    case  3: case  4: case  5: case  6: case  7:
        if ( Res == 5 )
            for ( k = 0; k < 36; k++, q++ )
                printf ("%2d%c", *q-7, k==35?'\n':' ');
        break;
    case  8: case  9: case 10: case 11: case 12: case 13: case 14: case 15: case 16: case 17:
        printf ("%2u: ", Res-1 );
        for ( k = 0; k < 36; k++, q++ ) {
            printf ("%6d", *q - (1 << (Res-2)) );
        }
        printf ("\n");
        break;
    }
}
#endif

/* end of encode_sv7.c */
