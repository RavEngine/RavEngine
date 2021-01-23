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

#include "libmpcenc.h"
#include <mpc/minimax.h>
#include <mpc/mpcmath.h>

/* V A R I A B L E S */
float  __SCF    [128 + 6];   // tabulated scalefactors
#define SCF             ( __SCF + 6 )
float  __invSCF [128 + 6];   // inverted scalefactors
#define invSCF  (__invSCF + 6)


// Quantization-coefficients: step/65536 bzw. (2*D[Res]+1)/65536
static const float  __A [1 + 18] = {
    0.0000762939453125f,
    0.0000000000000000f, 0.0000457763671875f, 0.0000762939453125f, 0.0001068115234375f,
    0.0001373291015625f, 0.0002288818359375f, 0.0004730224609375f, 0.0009613037109375f,
    0.0019378662109375f, 0.0038909912109375f, 0.0077972412109375f, 0.0156097412109375f,
    0.0312347412109375f, 0.0624847412109375f, 0.1249847412109375f, 0.2499847412109375f,
    0.4999847412109375f
};


// Requantization-coefficients: 65536/step bzw. 1/A[Res]
static const float  __C [1 + 18] = {
    13107.200000000001f,
    65535.000000000000f, 21845.333333333332f, 13107.200000000001f, 9362.285714285713f,
     7281.777777777777f,  4369.066666666666f,  2114.064516129032f, 1040.253968253968f,
      516.031496062992f,   257.003921568627f,   128.250489236790f,   64.062561094819f,
       32.015632633121f,    16.003907203907f,     8.000976681723f,    4.000244155527f,
        2.000061037018f,     1.000015259022f
};


// Requantization-Offset: 2*D+1 = steps of quantizer
static const int  __D [1 + 18] = {
    2,
    0,     1,     2,     3,     4,     7,    15,    31,    63,
  127,   255,   511,  1023,  2047,  4095,  8191, 16383, 32767
};

#define A   (__A + 1)
#define C   (__C + 1)
#define D   (__D + 1)

// Generation of the scalefactors and their inverses
void
Init_Skalenfaktoren ( void )
{
    int  n;

    for ( n = -6; n < 128; n++ ) {
        SCF[n]    = (float) ( pow(10.,-0.1*(n-1)/1.26) );
        invSCF[n] = (float) ( pow(10., 0.1*(n-1)/1.26) );
    }
}

#ifdef WIN32
#pragma warning ( disable : 4305 )
#endif

static float  NoiseInjectionCompensation1D [18] = {
#if 1
    1.f,
    0.884621,
    0.935711,
    0.970829,
    0.987941,
    0.994315,
    0.997826,
    0.999744,
    1., 1., 1., 1., 1., 1., 1., 1., 1., 1.
#else
    1.,
    0.907073,   //  -1...+1
    0.946334,   //  -2...+2
    0.974793,   //  -3...+3
    0.987647,   //  -4...+4
    0.994330,   //  -7...+7
    0.997846,   // -15...+15
    1.,         // -31...+31
    1.,
    1.,
    1.,
    1.,
    1.,
    1.,
    1.,
    1.,
    1.,
    1.,
#endif
} ;

#if 0
static float  NoiseInjectionCompensation2D [18] [32] = {
    { 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000,  },
    { 0.931595, 0.891390, 0.852494, 0.872420, 0.904053, 0.933716, 0.958976, 0.977719, 0.993979, 1.009011, 1.020961, 1.029564, 1.026582, 1.026753, 1.035573, 1.053251, 1.073429, 1.096344, 1.096344, 1.096344, 1.096344, 1.096344, 1.096344, 1.096344, 1.096344, 1.096344, 1.096344, 1.096344, 1.096344, 1.096344, 1.096344, 1.096344,  },
    { 0.878264, 0.882351, 0.904261, 0.930843, 0.949243, 0.966741, 0.980500, 0.988182, 0.993361, 0.997112, 0.998918, 0.999501, 1.003179, 1.007445, 1.008678, 0.995890, 0.991015, 0.988019, 0.985479, 0.987646, 1.003605, 1.029301, 1.040511, 1.061531, 1.083302, 1.083302, 1.083302, 1.083302, 1.083302, 1.083302, 1.083302, 1.083302,  },
    { 0.866977, 0.943500, 0.941561, 0.953049, 0.967274, 0.980476, 0.988678, 0.993240, 0.996376, 0.998513, 0.999545, 0.999775, 1.000898, 1.003954, 1.006308, 1.004932, 1.002867, 1.002922, 1.003624, 1.005487, 1.003919, 1.008022, 0.987693, 1.000358, 1.017461, 1.039166, 1.056053, 1.068191, 1.068191, 1.068191, 1.068191, 1.068191,  },
    { 0.880390, 0.976713, 0.976180, 0.976596, 0.982011, 0.988786, 0.993619, 0.996641, 0.998824, 1.000297, 1.001195, 1.001718, 1.002395, 1.003503, 1.005617, 1.005072, 1.002409, 1.003703, 1.003412, 1.003318, 1.005290, 1.007112, 1.014370, 1.010040, 1.000780, 1.005700, 1.020505, 1.030123, 1.030123, 1.030123, 1.030123, 1.030123,  },
    { 0.916894, 0.987164, 0.988734, 0.992318, 0.995268, 0.996932, 0.998141, 0.999072, 0.999674, 1.000104, 1.000292, 1.000386, 1.000399, 1.000222, 1.000671, 1.002127, 1.000137, 1.000046, 0.999644, 0.999156, 1.000568, 1.000098, 0.993764, 0.993954, 0.998971, 1.002835, 1.002972, 0.995376, 1.001643, 1.001643, 1.001643, 1.001643,  },
    { 0.982771, 0.995034, 0.997118, 0.998294, 0.998652, 0.999016, 0.999382, 0.999598, 0.999746, 0.999851, 0.999837, 0.999881, 0.999847, 1.000154, 0.999885, 1.000222, 0.999963, 1.000934, 0.999804, 0.999927, 1.000379, 0.997574, 0.997943, 0.998748, 0.998151, 0.997458, 1.000319, 1.001091, 0.998461, 0.996151, 1.005969, 1.005969,  },
    { 0.997150, 0.999903, 0.999424, 0.999537, 0.999661, 0.999753, 0.999851, 0.999903, 0.999928, 0.999963, 0.999969, 0.999941, 0.999974, 0.999967, 0.999996, 0.999975, 0.999966, 0.999704, 0.999946, 0.999894, 0.999905, 1.000840, 1.000716, 1.000799, 1.000406, 0.999912, 1.000153, 0.999789, 1.000495, 1.000495, 1.001167, 1.001347,  },
    { 0.995524, 0.999983, 1.000044, 0.999965, 0.999970, 0.999974, 0.999986, 0.999995, 0.999996, 1.000011, 0.999997, 1.000010, 1.000010, 1.000026, 1.000006, 1.000148, 1.000048, 0.999999, 1.000161, 1.000193, 0.999797, 1.000145, 0.999974, 1.000039, 0.999731, 0.999985, 1.000563, 1.000256, 1.000637, 1.000050, 1.002013, 1.001053,  },
    { 0.994796, 0.999833, 1.000003, 1.000012, 0.999986, 0.999991, 0.999991, 1.000000, 1.000004, 0.999999, 1.000005, 1.000004, 1.000008, 0.999996, 1.000027, 1.000097, 0.999951, 0.999938, 0.999989, 1.000001, 1.000048, 0.999935, 1.000068, 1.000134, 0.999961, 1.000198, 0.999956, 0.999957, 0.999844, 1.000087, 0.999708, 1.000198,  },
    { 0.996046, 0.999902, 1.000019, 1.000017, 0.999983, 0.999997, 1.000002, 0.999993, 0.999999, 1.000003, 1.000001, 1.000015, 1.000004, 1.000006, 0.999987, 0.999993, 0.999992, 1.000029, 1.000064, 0.999997, 1.000044, 1.000044, 0.999919, 0.999875, 1.000011, 0.999897, 0.999905, 0.999996, 0.999934, 0.999968, 1.000008, 0.999902,  },
    { 0.998703, 0.999963, 1.000021, 1.000006, 1.000008, 1.000000, 1.000003, 0.999994, 0.999990, 0.999990, 1.000003, 1.000009, 1.000001, 0.999999, 1.000001, 1.000009, 0.999999, 0.999988, 1.000003, 0.999971, 1.000005, 1.000042, 0.999924, 0.999995, 0.999998, 0.999988, 0.999961, 0.999942, 1.000046, 1.000061, 1.000112, 1.000052,  },
    { 0.999872, 1.000001, 1.000004, 0.999998, 0.999999, 0.999998, 0.999992, 0.999990, 0.999991, 1.000000, 1.000000, 1.000000, 1.000002, 0.999996, 1.000004, 1.000011, 0.999963, 1.000016, 1.000050, 0.999996, 0.999998, 1.000006, 0.999990, 0.999948, 0.999974, 1.000060, 1.000014, 0.999987, 0.999986, 0.999917, 0.999973, 1.000035,  },
    { 1.000366, 1.000006, 0.999996, 0.999995, 0.999998, 0.999996, 0.999991, 1.000001, 0.999990, 0.999996, 1.000010, 0.999999, 1.000002, 1.000000, 0.999996, 0.999990, 1.000014, 0.999978, 1.000011, 0.999983, 0.999988, 0.999971, 0.999997, 0.999989, 0.999986, 0.999958, 1.000005, 0.999992, 0.999975, 0.999975, 0.999975, 0.999975,  },
    { 0.999736, 0.999995, 1.000002, 1.000004, 0.999999, 1.000000, 1.000003, 1.000000, 1.000007, 0.999992, 0.999997, 0.999998, 0.999998, 0.999997, 1.000007, 1.000012, 1.000004, 0.999995, 0.999996, 1.000009, 1.000003, 1.000008, 1.000001, 1.000003, 1.000011, 1.000019, 0.999991, 0.999970, 0.999970, 0.999970, 0.999970, 0.999965,  },
    { 0.999970, 1.000000, 1.000000, 1.000000, 1.000001, 1.000000, 1.000000, 0.999999, 1.000001, 1.000000, 0.999999, 0.999999, 0.999999, 1.000007, 1.000005, 1.000002, 0.999999, 0.999999, 1.000000, 0.999997, 0.999999, 1.000001, 1.000001, 0.999988, 0.999988, 0.999984, 0.999995, 0.999986, 0.999986, 0.999986, 0.999986, 0.999986,  },
    { 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000,  },
    { 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000,  },
};
#endif

#ifdef WIN32
#pragma warning ( default : 4305 )
#endif

void
NoiseInjectionComp ( void )
{
    int  i;

    for ( i = 0; i < sizeof(NoiseInjectionCompensation1D)/sizeof(*NoiseInjectionCompensation1D); i++ )
        NoiseInjectionCompensation1D [i] = 1.f;
#if 0
    for ( i = 0; i < sizeof(NoiseInjectionCompensation2D)/sizeof(**NoiseInjectionCompensation2D); i++ )
        NoiseInjectionCompensation2D [0][i] = 1.f;
#endif
}


// Quantizes a subband and calculates iSNR
float
ISNR_Schaetzer ( const float* input, const float SNRcomp, const int res )
{
	const float  fac    = A [res] * NoiseInjectionCompensation1D [res];
	const float  invfac = C [res] / NoiseInjectionCompensation1D [res];
    float  signal = 1.e-30f;
    float  error = 1.e-30f;
	const float * in_end = input + 36;

    // Summation of the absolute power and the quadratic error
	do {
		float  err;
		err = mpc_nearbyintf(input[0] * fac) * invfac - input[0];
        error += err * err;
		signal += input[0] * input[0];

		err = mpc_nearbyintf(input[1] * fac) * invfac - input[1];
		error += err * err;
		signal += input[1] * input[1];

		err = mpc_nearbyintf(input[2] * fac) * invfac - input[2];
		error += err * err;
		signal += input[2] * input[2];

		err = mpc_nearbyintf(input[3] * fac) * invfac - input[3];
		error += err * err;
		signal += input[3] * input[3];

		input += 4;

	} while (input < in_end);

	error *= NoiseInjectionCompensation1D [res] * NoiseInjectionCompensation1D [res];
	signal *= NoiseInjectionCompensation1D [res] * NoiseInjectionCompensation1D [res];

    // Utilization of SNRcomp only if SNR > 1 !!!
    return signal > error  ?  error / (SNRcomp * signal)  :  error / signal;
}


float
ISNR_Schaetzer_Trans ( const float* input, const float SNRcomp, const int res )
{
    int    k;
    float  fac    = A [res];
    float  invfac = C [res];
	float  signal, error, ret, err, sig;

    // Summation of the absolute power and the quadratic error
	k = 0;
	signal = error = 1.e-30f;
    for ( ; k < 12; k++ ) {
        sig = input[k] * NoiseInjectionCompensation1D [res];
		err = mpc_nearbyintf(sig * fac) * invfac - sig;

        error += err * err;
        signal += sig * sig;
    }
    err = signal > error  ?  error / (SNRcomp * signal)  :  error / signal;
    ret = err;
    signal = error = 1.e-30f;
    for ( ; k < 24; k++ ) {
        sig = input[k] * NoiseInjectionCompensation1D [res];
		err = mpc_nearbyintf(sig * fac) * invfac - sig;

        error += err * err;
        signal += sig * sig;
    }
    err = signal > error  ?  error / (SNRcomp * signal)  :  error / signal;
	ret = maxf(ret, err);

    signal = error = 1.e-30f;
    for ( ; k < 36; k++ ) {
        sig = input[k] * NoiseInjectionCompensation1D [res];
		err = mpc_nearbyintf(sig * fac) * invfac - sig;

        error += err * err;
        signal += sig * sig;
    }
    err = signal > error  ?  error / (SNRcomp * signal)  :  error / signal;
	ret = maxf(ret, err);

    return ret;
}


// Linear quantizer for a subband
void
QuantizeSubband ( mpc_int16_t* qu_output, const float* input, const int res, float* errors, const int maxNsOrder )
{
	int    n, quant;
    int    offset  = D [res];
    float  mult    = A [res] * NoiseInjectionCompensation1D [res];
    float  invmult = C [res];
    float  signal;

	for ( n = 0; n < 36 - maxNsOrder; n++) {
		quant = (unsigned int)(mpc_lrintf(input[n] * mult) + offset);

        // limitation to 0...2D
        if ((unsigned int)quant > (unsigned int)offset * 2 ) {
            quant = mini ( quant, offset * 2 );
            quant = maxi ( quant, 0 );
        }
        qu_output[n] = quant;
    }

    for ( ; n < 36; n++) {
        signal = input[n] * mult;
		quant = (unsigned int)(mpc_lrintf(signal) + offset);

        // calculate the current error and save it for error refeeding
        errors [n + 6] = invmult * (quant - offset) - signal * NoiseInjectionCompensation1D [res];

        // limitation to 0...2D
        if ((unsigned int)quant > (unsigned int)offset * 2 ) {
            quant = mini ( quant, offset * 2 );
            quant = maxi ( quant, 0 );
        }
        qu_output[n] = quant;
    }
}


// NoiseShaper for a subband
void
QuantizeSubbandWithNoiseShaping ( mpc_int16_t* qu_output, const float* input, const int res, float* errors, const float* FIR )
{
    float  signal;
    float  mult    = A [res];
    float  invmult = C [res];
    int    offset  = D [res];
    int    n, quant;

	memset(errors, 0, 6 * sizeof *errors);       // arghh, it produces pops on each frame boundary!

    for ( n = 0; n < 36; n++) {
        signal = input[n] * NoiseInjectionCompensation1D [res] - (FIR[5]*errors[n+0] + FIR[4]*errors[n+1] + FIR[3]*errors[n+2] + FIR[2]*errors[n+3] + FIR[1]*errors[n+4] + FIR[0]*errors[n+5]);
		quant = mpc_lrintf(signal * mult);

        // calculate the current error and save it for error refeeding
        errors [n + 6] = invmult * quant - signal * NoiseInjectionCompensation1D [res];

        // limitation to +/-D
        quant = minf ( quant, +offset );
        quant = maxf ( quant, -offset );

        qu_output[n] = (unsigned int)(quant + offset);
    }
}

/* end of quant.c */

// pfk@schnecke.offl.uni-jena.de@EMAIL, Andree.Buschmann@web.de@EMAIL, BuschmannA@becker.de@EMAIL, miyaguch@eskimo.com@EMAIL, r3mix@irc.openprojects.net@EMAIL, dibrom@users.sourceforge.net@EMAIL, m.p.bakker-10@student.utwente.nl@EMAIL, djmrob@essex.ac.uk@EMAIL, dim@psytel-research.co.yu@EMAIL, lerch@zplane.de@EMAIL, takehiro@users.sourceforge.net@EMAIL, aleidinger@users.sourceforge.net@EMAIL, Robert.Hegemann@gmx.de@EMAIL, bouvigne@mp3-tech.org@EMAIL, monty@xiph.org@EMAIL, Pumpkinz99@aol.com@EMAIL, spase@outerspase.net@EMAIL, mt@wildpuppy.com@EMAIL, juha.laaksonheimo@tut.fi@EMAIL, speek@myrealbox.com@EMAIL, w.speek@12move.nl@EMAIL, martin@spueler.de@EMAIL, nicolaus.berglmeir@t-online.de@EMAIL, thomas.a.juerges@ruhr-uni-bochum.de@EMAIL, HelH@mpex.net@EMAIL, garf@roadum.demon.co.uk@EMAIL, gcp@sjeng.org@EMAIL, mike@naivesoftware.com@EMAIL, case@mobiili.net@EMAIL, steve.lhomme@free.fr@EMAIL, walter@binity.com@EMAIL
