/* $Id: private.h,v 1.29 2009/09/11 16:51:07 toad32767 Exp $ */

#pragma once
#if !defined(LIBAIFF_PRIVATE_H_INCL)
#define LIBAIFF_PRIVATE_H_INCL 1

#define LIBAIFF 1
#define _LARGEFILE_SOURCE 1
#include <libaiff/libaiff.h>
#include <libaiff/endian.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#if !defined(_WIN32)
#include <sys/types.h>
#endif

#if defined(_WIN32)
#define FTELLO _ftelli64
#define FSEEKO _fseeki64
#define OFF_T int64_t
#define RESTRICT __restrict
#else
#define FTELLO ftello
#define FSEEKO fseeko
#define OFF_T off_t
#define RESTRICT restrict
#endif

#ifdef LIBAIFF_UNITY_BUILD
# define LIBAIFF_INTERNAL static
#else
# define LIBAIFF_INTERNAL
#endif

struct codec {
	IFFType         fmt;
	int             (*construct) (AIFF_Ref);
        size_t		(*read_lpcm) (AIFF_Ref, void *, size_t);
	int             (*read_float32) (AIFF_Ref, float *, int);
        int             (*write_lpcm) (AIFF_Ref, void *, size_t, int);
	int             (*seek) (AIFF_Ref, uint64_t);
	void            (*destroy) (AIFF_Ref);
};

typedef struct s_AIFF_Buf {
	void		*ptr;
	unsigned int 	 len;
} AIFF_Buf;

enum {
	kAIFFBufConv,
	kAIFFBufExt,
	kAIFFNBufs
};

struct s_AIFF_Rec {
	FILE* fd;
	int flags;
	int stat; /* status */
	int segmentSize;
	int bitsPerSample;
	int nMarkers;
	int nChannels;
	int markerPos;
	double samplingRate;
	uint64_t nSamples;
	uint64_t len;
	uint64_t soundLen;
	uint64_t pos;
	uint64_t sampleBytes;
	uint64_t commonOffset;
	uint64_t soundOffset;
	uint64_t markerOffset;
	IFFType format;
	IFFType audioFormat;
	struct codec *codec;
	void* pdata;
	AIFF_Buf buf[kAIFFNBufs];
} ;
#define kAIFFRecSize	sizeof(struct s_AIFF_Rec)

#ifdef MIN
#undef MIN
#endif
#define MIN(x, y) ((x) < (y) ? (x) : (y))

/* private flags */
#define F_IEEE754_CHECKED	(1 << 27)
#define F_IEEE754_NATIVE	(1 << 28)
#define SSND_REACHED		(1 << 29)


/* == Supported formats == */

/* File formats */
#define AIFF_TYPE_IFF  ARRANGE_BE32(0x464F524D)
#define AIFF_TYPE_AIFF ARRANGE_BE32(0x41494646)
#define AIFF_TYPE_AIFC ARRANGE_BE32(0x41494643)

/* Audio encoding formats */
#define AUDIO_FORMAT_LPCM  ARRANGE_BE32(0x4E4F4E45)
#define AUDIO_FORMAT_lpcm  ARRANGE_BE32(0x6C70636D)
#define AUDIO_FORMAT_twos  ARRANGE_BE32(0x74776F73)
#define AUDIO_FORMAT_sowt  ARRANGE_LE32(0x74776F73)
#define AUDIO_FORMAT_ULAW  ARRANGE_BE32(0x554C4157)
#define AUDIO_FORMAT_ulaw  ARRANGE_BE32(0x756C6177)
#define AUDIO_FORMAT_ALAW  ARRANGE_BE32(0x414C4157)
#define AUDIO_FORMAT_alaw  ARRANGE_BE32(0x616C6177)
#define AUDIO_FORMAT_FL32  ARRANGE_BE32(0x464c3332)
#define AUDIO_FORMAT_fl32  ARRANGE_BE32(0x666c3332)
#define AUDIO_FORMAT_UNKNOWN 0xFFFFFFFF

/* OSTypes */
#define AIFF_FORM 0x464f524d
#define AIFF_AIFF 0x41494646
#define AIFF_AIFC 0x41494643
#define AIFF_FVER 0x46564552
#define AIFF_COMM 0x434f4d4d
#define AIFF_SSND 0x53534e44
#define AIFF_MARK 0x4d41524b
#define AIFF_INST 0x494e5354
#define AIFF_COMT 0x434f4d54

/* Standards & specifications */
#define AIFC_STD_DRAFT_082691	2726318400U

struct s_IFFHeader {
	IFFType         hid;
	uint32_t        len;
	IFFType         fid;
};
typedef struct s_IFFHeader IFFHeader;

struct s_IFFChunk {
	IFFType         id;
	uint32_t        len;
};
typedef struct s_IFFChunk IFFChunk;

/*
 * WARNING: these structures do NOT match on-file record layout!
 */
struct s_AIFFCommon {
	uint32_t        numSampleFrames;
	uint16_t        numChannels;
	uint16_t        sampleSize;
};
typedef struct s_AIFFCommon CommonChunk;

struct s_AIFFSound {
	uint32_t        offset;
	uint32_t        blockSize;
};
typedef struct s_AIFFSound SoundChunk;

struct s_Marker {
	uint32_t        position;
	MarkerId        id;
	uint16_t	garbage;
};
typedef struct s_Marker Marker;

struct s_AIFFMarker {
	uint16_t        numMarkers;
};
typedef struct s_AIFFMarker MarkerChunk;

struct s_AIFFLoop {
	int16_t         playMode;
	MarkerId        beginLoop;
	MarkerId        endLoop;
	uint16_t        garbage;/* not read (size=6 bytes) */
};
typedef struct s_AIFFLoop AIFFLoop;


struct s_Comment {
	uint32_t        timeStamp;
	MarkerId        marker;
	uint16_t        count;
};
typedef struct s_Comment Comment;

struct s_AIFFComment {
	uint16_t        numComments;
};
typedef struct s_AIFFComment CommentChunk;


/* iff.c */
LIBAIFF_INTERNAL int 
find_iff_chunk(IFFType, AIFF_Ref, uint32_t *);
LIBAIFF_INTERNAL char           *
get_iff_attribute(AIFF_Ref r, IFFType attrib);
LIBAIFF_INTERNAL int 
set_iff_attribute(AIFF_Ref w, IFFType attrib, char *str);
LIBAIFF_INTERNAL int 
clone_iff_attributes(AIFF_Ref w, AIFF_Ref r);

/* aifx.c */
LIBAIFF_INTERNAL int 
init_aifx(AIFF_Ref);
LIBAIFF_INTERNAL int 
read_aifx_marker(AIFF_Ref r, int *id, uint64_t * position, char **name);
LIBAIFF_INTERNAL int 
get_aifx_instrument(AIFF_Ref r, Instrument * inpi);
LIBAIFF_INTERNAL int 
do_aifx_prepare(AIFF_Ref r);
LIBAIFF_INTERNAL const char           *
get_aifx_enc_name(IFFType);

/* lpcm.c */
LIBAIFF_INTERNAL void            lpcm_swap16(int16_t *, const int16_t *, int);
LIBAIFF_INTERNAL void            lpcm_swap32(int32_t *, const int32_t *, int);
LIBAIFF_INTERNAL void            lpcm_swap_samples(int, int, const void *, void *, int);
LIBAIFF_INTERNAL void            lpcm_dequant(int segmentSize, void *buffer, float *outFrames, int nFrames);
#ifndef LIBAIFF_UNITY_BUILD
extern struct codec lpcm;
#endif

/* g711.c */
#ifndef LIBAIFF_UNITY_BUILD
extern struct codec ulaw;
extern struct codec alaw;
#endif

/* float32.c */
#ifndef LIBAIFF_UNITY_BUILD
extern struct codec float32;
#endif

/* extended.c */
LIBAIFF_INTERNAL void            ieee754_write_extended(double, uint8_t *);
LIBAIFF_INTERNAL double          ieee754_read_extended(const uint8_t *);

/* pascal.c */
LIBAIFF_INTERNAL int             PASCALInGetLength(FILE *);
LIBAIFF_INTERNAL char           *PASCALInRead(FILE *, int *);
LIBAIFF_INTERNAL int             PASCALOutGetLength(const char *);
LIBAIFF_INTERNAL int             PASCALOutWrite(FILE *, const char *);

/* libaiff.c */
LIBAIFF_INTERNAL void		 AIFFBufDelete(AIFF_Ref, int);
LIBAIFF_INTERNAL void		*AIFFBufAllocate(AIFF_Ref, int, unsigned int);

#endif // !defined(LIBAIFF_PRIVATE_H_INCL)
