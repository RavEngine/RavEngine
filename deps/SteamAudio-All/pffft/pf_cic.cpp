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

/* gcc requires this for M_PI !? */
#undef __STRICT_ANSI__

/* include own header first, to see missing includes */
#include "pf_cic.h"
#include "fmv.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>


/*
   ____ ___ ____   ____  ____   ____
  / ___|_ _/ ___| |  _ \|  _ \ / ___|
 | |    | | |     | | | | | | | |
 | |___ | | |___  | |_| | |_| | |___
  \____|___\____| |____/|____/ \____|
*/

#define SINESHIFT 12
#define SINESIZE (1<<SINESHIFT)
typedef int64_t cic_dt; // data type used for integrators and combs
typedef struct {
    int factor;
    uint64_t phase;
    float gain;
    cic_dt ig0a, ig0b, ig1a, ig1b;
    cic_dt comb0a, comb0b, comb1a, comb1b;
    int16_t *sinetable;
} cicddc_t;

void *cicddc_init(int factor) {
    int i;
    int sinesize2 = SINESIZE * 5/4; // 25% extra to get cosine from the same table
    cicddc_t *s;
    s = (cicddc_t *)malloc(sizeof(cicddc_t));
    memset(s, 0, sizeof(cicddc_t));

    float sineamp = 32767.0f;
    s->factor = factor;
    s->gain = 1.0f / SHRT_MAX / sineamp / factor / factor / factor; // compensate for gain of 3 integrators

    s->sinetable = (int16_t *)malloc(sinesize2 * sizeof(*s->sinetable));
    double f = 2.0 * M_PI / (double)SINESIZE;
    for(i = 0; i < sinesize2; i++) {
        s->sinetable[i] = sineamp * cos(f * i);
    }
    return s;
}

void cicddc_free(void *state) {
    cicddc_t *s = (cicddc_t *)state;
    free(s->sinetable);
    free(s);
}


PF_TARGET_CLONES
void cicddc_s16_c(void *state, int16_t *input, complexf *output, int outsize, float rate) {
    cicddc_t *s = (cicddc_t *)state;
    int k;
    int factor = s->factor;
    cic_dt ig0a = s->ig0a, ig0b = s->ig0b, ig1a = s->ig1a, ig1b = s->ig1b;
    cic_dt comb0a = s->comb0a, comb0b = s->comb0b, comb1a = s->comb1a, comb1b = s->comb1b;
    uint64_t phase = s->phase, freq;
    int16_t *sinetable = s->sinetable;
    float gain = s->gain;

    freq = rate * ((float)(1ULL << 63) * 2);

    int16_t *inp = input;
    for(k = 0; k < outsize; k++) {
        int i;
        cic_dt out0a, out0b, out1a, out1b;
        cic_dt ig2a = 0, ig2b = 0; // last integrator and first comb replaced simply by sum
        for(i = 0; i < factor; i++) {
            cic_dt in_a, in_b;
            int sinep = phase >> (64-SINESHIFT);
            in_a = (int32_t)inp[i] * (int32_t)sinetable[sinep + (1<<(SINESHIFT-2))];
            in_b = (int32_t)inp[i] * (int32_t)sinetable[sinep];
            phase += freq;
            /* integrators:
            The calculations are ordered so that each integrator
            takes a result from previous loop iteration
            to make the code more "pipeline-friendly". */
            ig2a += ig1a; ig2b += ig1b;
            ig1a += ig0a; ig1b += ig0b;
            ig0a += in_a; ig0b += in_b;
        }
        inp += factor;
        // comb filters:
        out0a  = ig2a - comb0a;  out0b  = ig2b - comb0b;
        comb0a = ig2a;           comb0b = ig2b;
        out1a  = out0a - comb1a; out1b  = out0b - comb1b;
        comb1a = out0a;          comb1b = out0b;

        output[k].i = (float)out1a * gain;
        output[k].q = (float)out1b * gain;
    }

    s->ig0a = ig0a; s->ig0b = ig0b;
    s->ig1a = ig1a; s->ig1b = ig1b;
    s->comb0a = comb0a; s->comb0b = comb0b;
    s->comb1a = comb1a; s->comb1b = comb1b;
    s->phase = phase;
}

PF_TARGET_CLONES
void cicddc_cs16_c(void *state, int16_t *input, complexf *output, int outsize, float rate) {
    cicddc_t *s = (cicddc_t *)state;
    int k;
    int factor = s->factor;
    cic_dt ig0a = s->ig0a, ig0b = s->ig0b, ig1a = s->ig1a, ig1b = s->ig1b;
    cic_dt comb0a = s->comb0a, comb0b = s->comb0b, comb1a = s->comb1a, comb1b = s->comb1b;
    uint64_t phase = s->phase, freq;
    int16_t *sinetable = s->sinetable;
    float gain = s->gain;

    freq = rate * ((float)(1ULL << 63) * 2);

    int16_t *inp = input;
    for(k = 0; k < outsize; k++) {
        int i;
        cic_dt out0a, out0b, out1a, out1b;
        cic_dt ig2a = 0, ig2b = 0; // last integrator and first comb replaced simply by sum
        for(i = 0; i < factor; i++) {
            cic_dt in_a, in_b;
            int32_t m_a, m_b, m_c, m_d;
            int sinep = phase >> (64-SINESHIFT);
            m_a = inp[2*i];
            m_b = inp[2*i+1];
            m_c = (int32_t)sinetable[sinep + (1<<(SINESHIFT-2))];
            m_d = (int32_t)sinetable[sinep];
            // complex multiplication:
            in_a = m_a*m_c - m_b*m_d;
            in_b = m_a*m_d + m_b*m_c;
            phase += freq;
            /* integrators:
            The calculations are ordered so that each integrator
            takes a result from previous loop iteration
            to make the code more "pipeline-friendly". */
            ig2a += ig1a; ig2b += ig1b;
            ig1a += ig0a; ig1b += ig0b;
            ig0a += in_a; ig0b += in_b;
        }
        inp += 2*factor;
        // comb filters:
        out0a  = ig2a - comb0a;  out0b  = ig2b - comb0b;
        comb0a = ig2a;           comb0b = ig2b;
        out1a  = out0a - comb1a; out1b  = out0b - comb1b;
        comb1a = out0a;          comb1b = out0b;

        output[k].i = (float)out1a * gain;
        output[k].q = (float)out1b * gain;
    }

    s->ig0a = ig0a; s->ig0b = ig0b;
    s->ig1a = ig1a; s->ig1b = ig1b;
    s->comb0a = comb0a; s->comb0b = comb0b;
    s->comb1a = comb1a; s->comb1b = comb1b;
    s->phase = phase;
}


/* This is almost copy paste from cicddc_cs16_c.
   I'm afraid this is going to be annoying to maintain... */
PF_TARGET_CLONES
void cicddc_cu8_c(void *state, uint8_t *input, complexf *output, int outsize, float rate) {
    cicddc_t *s = (cicddc_t *)state;
    int k;
    int factor = s->factor;
    cic_dt ig0a = s->ig0a, ig0b = s->ig0b, ig1a = s->ig1a, ig1b = s->ig1b;
    cic_dt comb0a = s->comb0a, comb0b = s->comb0b, comb1a = s->comb1a, comb1b = s->comb1b;
    uint64_t phase = s->phase, freq;
    int16_t *sinetable = s->sinetable;
    float gain = s->gain;

    freq = rate * ((float)(1ULL << 63) * 2);

    uint8_t *inp = input;
    for(k = 0; k < outsize; k++) {
        int i;
        cic_dt out0a, out0b, out1a, out1b;
        cic_dt ig2a = 0, ig2b = 0; // last integrator and first comb replaced simply by sum
        for(i = 0; i < factor; i++) {
            cic_dt in_a, in_b;
            int32_t m_a, m_b, m_c, m_d;
            int sinep = phase >> (64-SINESHIFT);
            // subtract 127.4 (good for rtl-sdr)
            m_a = (((int32_t)inp[2*i])   << 8) - 32614;
            m_b = (((int32_t)inp[2*i+1]) << 8) - 32614;
            m_c = (int32_t)sinetable[sinep + (1<<(SINESHIFT-2))];
            m_d = (int32_t)sinetable[sinep];
            // complex multiplication:
            in_a = m_a*m_c - m_b*m_d;
            in_b = m_a*m_d + m_b*m_c;
            phase += freq;
            /* integrators:
            The calculations are ordered so that each integrator
            takes a result from previous loop iteration
            to make the code more "pipeline-friendly". */
            ig2a += ig1a; ig2b += ig1b;
            ig1a += ig0a; ig1b += ig0b;
            ig0a += in_a; ig0b += in_b;
        }
        inp += 2*factor;
        // comb filters:
        out0a  = ig2a - comb0a;  out0b  = ig2b - comb0b;
        comb0a = ig2a;           comb0b = ig2b;
        out1a  = out0a - comb1a; out1b  = out0b - comb1b;
        comb1a = out0a;          comb1b = out0b;

        output[k].i = (float)out1a * gain;
        output[k].q = (float)out1b * gain;
    }

    s->ig0a = ig0a; s->ig0b = ig0b;
    s->ig1a = ig1a; s->ig1b = ig1b;
    s->comb0a = comb0a; s->comb0b = comb0b;
    s->comb1a = comb1a; s->comb1b = comb1b;
    s->phase = phase;
}

