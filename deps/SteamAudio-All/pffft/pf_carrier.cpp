/*
This software is part of pffft/pfdsp, a set of simple DSP routines.

Copyright (c) 2014, Andras Retzler <randras@sdr.hu>
Copyright (c) 2020  Hayati Ayguen <h_ayguen@web.de>

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL ANDRAS RETZLER BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* include own header first, to see missing includes */
#include "pf_carrier.h"
#include "fmv.h"

#include <limits.h>
#include <assert.h>


PF_TARGET_CLONES
void generate_dc_f(float* output, int size)
{
    for(int i=0;i<2*size;)
    {
        /* exp(i*0) = 1+i*0 */
        output[i++]=(127.0F / 128.0F);
        output[i++]=0.0F;
    }
}

PF_TARGET_CLONES
void generate_dc_s16(short* output, int size)
{
    for(int i=0;i<2*size;)
    {
        /* exp(i*0) = 1+i*0 */
        output[i++]=SHRT_MAX;
        output[i++]=0;
    }
}

PF_TARGET_CLONES
void generate_pos_fs4_f(float* output, int size)
{
    /* size must be multiple of 4 */
    assert(!(size&3));
    for(int i=0;i<2*size;)
    {
        /* exp(i*0) = 1+i*0 */
        output[i++]=(127.0F / 128.0F);
        output[i++]=0.0F;
        /* exp(i* +pi/2) = 0+i*1 */
        output[i++]=0.0F;
        output[i++]=(127.0F / 128.0F);
        /* exp(i* +pi) = -1+i*0 */
        output[i++]=(-127.0F / 128.0F);
        output[i++]=0.0F;
        /* exp(i* -pi/2) = 0+i*-1 */
        output[i++]=0.0F;
        output[i++]=(-127.0F / 128.0F);
    }
}

PF_TARGET_CLONES
void generate_pos_fs4_s16(short* output, int size)
{
    /* size must be multiple of 4 */
    assert(!(size&3));
    for(int i=0;i<2*size;)
    {
        /* exp(i*0) = 1+i*0 */
        output[i++]=SHRT_MAX;
        output[i++]=0;
        /* exp(i* +pi/2) = 0+i*1 */
        output[i++]=0;
        output[i++]=SHRT_MAX;
        /* exp(i* +pi) = -1+i*0 */
        output[i++]=-SHRT_MAX;
        output[i++]=0;
        /* exp(i* -pi/2) = 0+i*-1 */
        output[i++]=0;
        output[i++]=-SHRT_MAX;
    }
}

PF_TARGET_CLONES
void generate_neg_fs4_f(float* output, int size)
{
    /* size must be multiple of 4 */
    assert(!(size&3));
    for(int i=0;i<2*size;)
    {
        /* exp(i*0) = 1+i*0 */
        output[i++]=(127.0F / 128.0F);
        output[i++]=0.0F;
        /* exp(i* -pi/2) = 0+i*-1 */
        output[i++]=0.0F;
        output[i++]=(-127.0F / 128.0F);
        /* exp(i* +pi) = -1+i*0 */
        output[i++]=(-127.0F / 128.0F);
        output[i++]=0.0F;
        /* exp(i* +pi/2) = 0+i*1 */
        output[i++]=0.0F;
        output[i++]=(127.0F / 128.0F);
    }
}

PF_TARGET_CLONES
void generate_neg_fs4_s16(short* output, int size)
{
    /* size must be multiple of 4 */
    assert(!(size&3));
    for(int i=0;i<2*size;)
    {
        /* exp(i*0) = 1+i*0 */
        output[i++]=SHRT_MAX;
        output[i++]=0;
        /* exp(i* -pi/2) = 0+i*-1 */
        output[i++]=0;
        output[i++]=-SHRT_MAX;
        /* exp(i* +pi) = -1+i*0 */
        output[i++]=-SHRT_MAX;
        output[i++]=0;
        /* exp(i* +pi/2) = 0+i*1 */
        output[i++]=0;
        output[i++]=SHRT_MAX;
    }
}

/****************************************************/

PF_TARGET_CLONES
void generate_dc_pos_fs4_s16(short* output, int size)
{
    const int m = SHRT_MAX / 2;
    /* size must be multiple of 4 */
    assert(!(size&3));
    for(int i=0;i<2*size;)
    {
        /* exp(i*0) = 1+1+i*0 */
        output[i++]=m+m;
        output[i++]=0;
        /* exp(i* +pi/2) = 1+0+i*1 */
        output[i++]=m+0;
        output[i++]=m;
        /* exp(i* +pi) = 1-1+i*0 */
        output[i++]=m-m;
        output[i++]=0;
        /* exp(i* -pi/2) = 1+0+i*-1 */
        output[i++]=m;
        output[i++]=-m;
    }
}

PF_TARGET_CLONES
void generate_dc_neg_fs4_s16(short* output, int size)
{
    const int m = SHRT_MAX / 2;
    /* size must be multiple of 4 */
    assert(!(size&3));
    for(int i=0;i<2*size;)
    {
        /* exp(i*0) = 1+1+i*0 */
        output[i++]=m+m;
        output[i++]=0;
        /* exp(i* -pi/2) = 1+0+i*-1 */
        output[i++]=m+0;
        output[i++]=-m;
        /* exp(i* +pi) = 1-1+i*0 */
        output[i++]=m-m;
        output[i++]=0;
        /* exp(i* +pi/2) = 1+0+i*1 */
        output[i++]=m+0;
        output[i++]=m;
    }
}

PF_TARGET_CLONES
void generate_pos_neg_fs4_s16(short* output, int size)
{
    const int m = SHRT_MAX / 2;
    /* size must be multiple of 4 */
    assert(!(size&3));
    for(int i=0;i<2*size;)
    {
        /* pos(0) + neg(0) = exp(i*  0   ) + exp(i*  0   ) =  1 +i*  0  +  1 +i*  0 */
        output[i++]=m;
        output[i++]=-m;

        /* pos(1) + neg(1) = exp(i* +pi/2) + exp(i* -pi/2) =  0 +i*  1  +  0 +i* -1 */
        output[i++]=-m;
        output[i++]=m;

        /* pos(2) + neg(2) = exp(i* +pi  ) + exp(i* +pi  ) = -1 +i*  0  + -1 +i*  0 */
        output[i++]=-m;
        output[i++]=m;

        /* pos(3) + neg(3) = exp(i* -pi/2) + exp(i* +pi/2) =  0 +i* -1  +  0 +i*  1 */
        output[i++]=m;
        output[i++]=-m;
    }
}

PF_TARGET_CLONES
void generate_dc_pos_neg_fs4_s16(short* output, int size)
{
    const int m = SHRT_MAX / 2;
    /* size must be multiple of 4 */
    assert(!(size&3));
    for(int i=0;i<2*size;)
    {
        /* dc + pos(0) + neg(0) = dc + exp(i*  0   ) + exp(i*  0   ) =  1 +i*  0  +  1 +i*  0 */
        output[i++]=m+m;
        output[i++]=-m;

        /* dc + pos(1) + neg(1) = dc + exp(i* +pi/2) + exp(i* -pi/2) =  0 +i*  1  +  0 +i* -1 */
        output[i++]=0;
        output[i++]=m;

        /* dc + pos(2) + neg(2) = dc + exp(i* +pi  ) + exp(i* +pi  ) = -1 +i*  0  + -1 +i*  0 */
        output[i++]=0;
        output[i++]=m;

        /* dc + pos(3) + neg(3) = dc + exp(i* -pi/2) + exp(i* +pi/2) =  0 +i* -1  +  0 +i*  1 */
        output[i++]=m+m;
        output[i++]=-m;
    }
}


PF_TARGET_CLONES
void generate_pos_neg_fs2_s16(short* output, int size)
{
    const int m = SHRT_MAX / 2;
    /* size must be multiple of 4 */
    assert(!(size&3));
    for(int i=0;i<2*size;)
    {
        /* dc + exp(i* 0 ) = +1 */
        output[i++]=m;
        output[i++]=0;
        /* dc + exp(i* pi) = -1 */
        output[i++]=-m;
        output[i++]=0;
        /* dc + exp(i* 0 ) = +1 */
        output[i++]=m;
        output[i++]=0;
        /* dc + exp(i* pi) = -1 */
        output[i++]=-m;
        output[i++]=0;
    }
}

PF_TARGET_CLONES
void generate_dc_pos_neg_fs2_s16(short* output, int size)
{
    const int m = SHRT_MAX / 2;
    /* size must be multiple of 4 */
    assert(!(size&3));
    for(int i=0;i<2*size;)
    {
        /* with dc = i*1 */
        /* dc + exp(i* 0 ) = i*1 +1 */
        output[i++]=m;
        output[i++]=m;
        /* dc + exp(i* pi) = i*1 -1 */
        output[i++]=-m;
        output[i++]=m;
        /* dc + exp(i* 0 ) = i*1 +1 */
        output[i++]=m;
        output[i++]=m;
        /* dc + exp(i* pi) = i*1 -1 */
        output[i++]=-m;
        output[i++]=m;
    }
}


