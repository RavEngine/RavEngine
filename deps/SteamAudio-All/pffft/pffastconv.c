/*
  Copyright (c) 2019  Hayati Ayguen ( h_ayguen@web.de )
 */

#include "pffastconv.h"
#include "pffft.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <string.h>

#define FASTCONV_DBG_OUT  0


/* detect compiler flavour */
#if defined(_MSC_VER)
#  define RESTRICT __restrict
#pragma warning( disable : 4244 4305 4204 4456 )
#elif defined(__GNUC__)
#  define RESTRICT __restrict
#endif


void *pffastconv_malloc(size_t nb_bytes)
{
  return pffft_aligned_malloc(nb_bytes);
}

void pffastconv_free(void *p)
{
  pffft_aligned_free(p);
}

int pffastconv_simd_size()
{
  return pffft_simd_size();
}



struct PFFASTCONV_Setup
{
  float * Xt;      /* input == x in time domain - copy for alignment */
  float * Xf;      /* input == X in freq domain */
  float * Hf;      /* filterCoeffs == H in freq domain */
  float * Mf;      /* input * filterCoeffs in freq domain */
  PFFFT_Setup *st;
  int filterLen;   /* convolution length */
  int Nfft;        /* FFT/block length */
  int flags;
  float scale;
};


PFFASTCONV_Setup * pffastconv_new_setup( const float * filterCoeffs, int filterLen, int * blockLen, int flags )
{
  PFFASTCONV_Setup * s = NULL;
  const int cplxFactor = ( (flags & PFFASTCONV_CPLX_INP_OUT) && (flags & PFFASTCONV_CPLX_SINGLE_FFT) ) ? 2 : 1;
  const int minFftLen = 2*pffft_simd_size()*pffft_simd_size();
  int i, Nfft = 2 * pffft_next_power_of_two(filterLen -1);
#if FASTCONV_DBG_OUT
  const int iOldBlkLen = *blockLen;
#endif

  if ( Nfft < minFftLen )
    Nfft = minFftLen;

  if ( flags & PFFASTCONV_CPLX_FILTER )
    return NULL;

  s = pffastconv_malloc( sizeof(struct PFFASTCONV_Setup) );

  if ( *blockLen > Nfft ) {
    Nfft = *blockLen;
    Nfft = pffft_next_power_of_two(Nfft);
  }
  *blockLen = Nfft;  /* this is in (complex) samples */

  Nfft *= cplxFactor;

  if ( (flags & PFFASTCONV_DIRECT_INP) && !(flags & PFFASTCONV_CPLX_INP_OUT) )
    s->Xt = NULL;
  else
    s->Xt = pffastconv_malloc((unsigned)Nfft * sizeof(float));
  s->Xf = pffastconv_malloc((unsigned)Nfft * sizeof(float));
  s->Hf = pffastconv_malloc((unsigned)Nfft * sizeof(float));
  s->Mf = pffastconv_malloc((unsigned)Nfft * sizeof(float));
  s->st = pffft_new_setup(Nfft, PFFFT_REAL);  /* with complex: we do 2 x fft() */
  s->filterLen = filterLen;        /* filterLen == convolution length == length of impulse response */
  if ( cplxFactor == 2 )
    s->filterLen = 2 * filterLen - 1;
  s->Nfft = Nfft;  /* FFT/block length */
  s->flags = flags;
  s->scale = (float)( 1.0 / Nfft );

  memset( s->Xt, 0, (unsigned)Nfft * sizeof(float) );
  if ( flags & PFFASTCONV_CORRELATION ) {
    for ( i = 0; i < filterLen; ++i )
      s->Xt[ ( Nfft - cplxFactor * i ) & (Nfft -1) ] = filterCoeffs[ i ];
  } else {
    for ( i = 0; i < filterLen; ++i )
      s->Xt[ ( Nfft - cplxFactor * i ) & (Nfft -1) ] = filterCoeffs[ filterLen - 1 - i ];
  }

  pffft_transform(s->st, s->Xt, s->Hf, /* tmp = */ s->Mf, PFFFT_FORWARD);

#if FASTCONV_DBG_OUT
  printf("\n  fastConvSetup(filterLen = %d, blockLen %d) --> blockLen %d, OutLen = %d\n"
    , filterLen, iOldBlkLen, *blockLen, Nfft - filterLen +1 );
#endif

  return s;
}


void pffastconv_destroy_setup( PFFASTCONV_Setup * s )
{
  if (!s)
    return;
  pffft_destroy_setup(s->st);
  pffastconv_free(s->Mf);
  pffastconv_free(s->Hf);
  pffastconv_free(s->Xf);
  if ( s->Xt )
    pffastconv_free(s->Xt);
  pffastconv_free(s);
}


int pffastconv_apply(PFFASTCONV_Setup * s, const float *input_, int cplxInputLen, float *output_, int applyFlush)
{
  const float * RESTRICT X = input_;
  float * RESTRICT Y = output_;
  const int Nfft = s->Nfft;
  const int filterLen = s->filterLen;
  const int flags = s->flags;
  const int cplxFactor = ( (flags & PFFASTCONV_CPLX_INP_OUT) && (flags & PFFASTCONV_CPLX_SINGLE_FFT) ) ? 2 : 1;
  const int inputLen = cplxFactor * cplxInputLen;
  int inpOff, procLen, numOut = 0, j, part, cplxOff;

  /* applyFlush != 0:
   *     inputLen - inpOff -filterLen + 1 > 0
   * <=> inputLen -filterLen + 1 > inpOff
   * <=> inpOff < inputLen -filterLen + 1
   * 
   * applyFlush == 0:
   *     inputLen - inpOff >= Nfft
   * <=> inputLen - Nfft >= inpOff
   * <=> inpOff <= inputLen - Nfft
   * <=> inpOff < inputLen - Nfft + 1
   */

  if ( cplxFactor == 2 )
  {
    const int maxOff = applyFlush ? (inputLen -filterLen + 1) : (inputLen - Nfft + 1);
#if 0
    printf( "*** inputLen %d, filterLen %d, Nfft %d => maxOff %d\n", inputLen, filterLen, Nfft, maxOff);
#endif
    for ( inpOff = 0; inpOff < maxOff; inpOff += numOut )
    {
      procLen = ( (inputLen - inpOff) >= Nfft ) ? Nfft : (inputLen - inpOff);
      numOut = ( procLen - filterLen + 1 ) & ( ~1 );
      if (!numOut)
        break;
#if 0
      if (!inpOff)
        printf("*** inpOff = %d, numOut = %d\n", inpOff, numOut);
      if (inpOff + filterLen + 2 >= maxOff )
        printf("*** inpOff = %d, inpOff + numOut = %d\n", inpOff, inpOff + numOut);
#endif

      if ( flags & PFFASTCONV_DIRECT_INP )
      {
        pffft_transform(s->st, X + inpOff, s->Xf, /* tmp = */ s->Mf, PFFFT_FORWARD);
      }
      else
      {
        memcpy( s->Xt, X + inpOff, (unsigned)procLen * sizeof(float) );
        if ( procLen < Nfft )
          memset( s->Xt + procLen, 0, (unsigned)(Nfft - procLen) * sizeof(float) );
    
        pffft_transform(s->st, s->Xt, s->Xf, /* tmp = */ s->Mf, PFFFT_FORWARD);
      }

      pffft_zconvolve_no_accu(s->st, s->Xf, s->Hf, /* tmp = */ s->Mf, s->scale);

      if ( flags & PFFASTCONV_DIRECT_OUT )
      {
        pffft_transform(s->st, s->Mf, Y + inpOff, s->Xf, PFFFT_BACKWARD);
      }
      else
      {
        pffft_transform(s->st, s->Mf, s->Xf, /* tmp = */ s->Xt, PFFFT_BACKWARD);
        memcpy( Y + inpOff, s->Xf, (unsigned)numOut * sizeof(float) );
      }
    }
    return inpOff / cplxFactor;
  }
  else
  {
    const int maxOff = applyFlush ? (inputLen -filterLen + 1) : (inputLen - Nfft + 1);
    const int numParts = (flags & PFFASTCONV_CPLX_INP_OUT) ? 2 : 1;

    for ( inpOff = 0; inpOff < maxOff; inpOff += numOut )
    {
      procLen = ( (inputLen - inpOff) >= Nfft ) ? Nfft : (inputLen - inpOff);
      numOut = procLen - filterLen + 1;

      for ( part = 0; part < numParts; ++part )  /* iterate per real/imag component */
      {

        if ( flags & PFFASTCONV_CPLX_INP_OUT )
        {
          cplxOff = 2 * inpOff + part;
          for ( j = 0; j < procLen; ++j )
            s->Xt[j] = X[cplxOff + 2 * j];
          if ( procLen < Nfft )
            memset( s->Xt + procLen, 0, (unsigned)(Nfft - procLen) * sizeof(float) );

          pffft_transform(s->st, s->Xt, s->Xf, /* tmp = */ s->Mf, PFFFT_FORWARD);
        }
        else if ( flags & PFFASTCONV_DIRECT_INP )
        {
          pffft_transform(s->st, X + inpOff, s->Xf, /* tmp = */ s->Mf, PFFFT_FORWARD);
        }
        else
        {
          memcpy( s->Xt, X + inpOff, (unsigned)procLen * sizeof(float) );
          if ( procLen < Nfft )
            memset( s->Xt + procLen, 0, (unsigned)(Nfft - procLen) * sizeof(float) );
    
          pffft_transform(s->st, s->Xt, s->Xf, /* tmp = */ s->Mf, PFFFT_FORWARD);
        }

        pffft_zconvolve_no_accu(s->st, s->Xf, s->Hf, /* tmp = */ s->Mf, s->scale);

        if ( flags & PFFASTCONV_CPLX_INP_OUT )
        {
          pffft_transform(s->st, s->Mf, s->Xf, /* tmp = */ s->Xt, PFFFT_BACKWARD);
    
          cplxOff = 2 * inpOff + part;
          for ( j = 0; j < numOut; ++j )
            Y[ cplxOff + 2 * j ] = s->Xf[j];
        }
        else if ( flags & PFFASTCONV_DIRECT_OUT )
        {
          pffft_transform(s->st, s->Mf, Y + inpOff, s->Xf, PFFFT_BACKWARD);
        }
        else
        {
          pffft_transform(s->st, s->Mf, s->Xf, /* tmp = */ s->Xt, PFFFT_BACKWARD);
          memcpy( Y + inpOff, s->Xf, (unsigned)numOut * sizeof(float) );
        }

      }
    }

    return inpOff;
  }
}

