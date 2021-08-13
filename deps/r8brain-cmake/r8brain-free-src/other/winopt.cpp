// Window function parameters optimizer - finds parameters for the specified
// filter bandwidth for Kaiser window functions using the BiteOptDeep
// derivative-free optimization method.
//
// r8brain-free-src Copyright (c) 2013-2021 Aleksey Vaneev
// See the "LICENSE" file for license.

#include <stdio.h>
#include <math.h>
#include "/projects/biteopt/biteopt.h"
#include "../CDSPSincFilterGen.h"

const int Bandwidth = 2; // Filter's bandwidth (2 or 3).
const double LinFraction = 1.25 / Bandwidth; // Linear part of the spectrum.
const double StopFraction = 2.0 - 1.0 / Bandwidth; // Stop-band part of the spectrum, start.
const double StopFractionEnd = 4.0; // Stop-band part of the spectrum, end.
const int MinFilterLen = 8; // Minimal filter length (must be even).
const int MaxFilterLen = 30; // Maximal filter length (must be even).
int FilterLen = 24; // Filter length.
const int Oversample = 20; // Spectrum oversampling ratio.
const int OptDepth = 8; // BiteOptDeep optimizer depth.

class CWinOpt : public CBiteOptDeep
{
public:
	CWinOpt()
	{
		updateDims( 2, OptDepth );
	}

	virtual void getMinValues( double* const p ) const
	{
		p[ 0 ] = 1.0;
		p[ 1 ] = 1.0;
	}

	virtual void getMaxValues( double* const p ) const
	{
		p[ 0 ] = 50.0;
		p[ 1 ] = 3.0;
	}

	double opt_cost1;
	double opt_cost2;

	virtual double optcost( const double* const p )
	{
		r8b :: CDSPSincFilterGen gen;
		gen.Freq1 = 0.0;
		gen.Freq2 = M_PI / Oversample;
		gen.Len2 = FilterLen * 0.5 * Oversample;
		gen.initBand( r8b :: CDSPSincFilterGen :: wftKaiser, p, true );

		const int KernelLen = gen.KernelLen;
		double KernelBlock[ KernelLen ];
		gen.generateBand( &KernelBlock[ 0 ],
			&r8b :: CDSPSincFilterGen :: calcWindowKaiser );

		r8b :: normalizeFIRFilter( &KernelBlock[ 0 ], KernelLen, 1.0 );

		const double _10ln10 = 10.0 / log( 10.0 );
		const int Count1 = 500;
		double cost1 = 0.0;
		double re;
		double im;
		int i;

		for( i = 0; i <= Count1; i++ )
		{
			const double th = M_PI * LinFraction / Oversample * i / Count1;
			r8b :: calcFIRFilterResponse( KernelBlock, KernelLen, th, re, im );
			double p = fabs( _10ln10 * log( re * re + im * im ));

			cost1 = max( p, cost1 );
		}

		const int Count2 = 2000;
		double cost2 = -1000.0;
		const double th1 = M_PI * StopFraction / Oversample;
		const double th2 = M_PI * StopFractionEnd / Oversample;

		for( i = 0; i <= Count2; i++ )
		{
			const double th = th1 + ( th2 - th1 ) * i / Count2;
			r8b :: calcFIRFilterResponse( KernelBlock, KernelLen, th, re, im );
			double p = _10ln10 * log( re * re + im * im );

			cost2 = max( p, cost2 );
		}

		opt_cost1 = cost1;
		opt_cost2 = cost2;

		return( cost1 * 180.0 + cost2 );
	}
};

int main()
{
	CBiteRnd rnd;
	rnd.init( 1 );

	CWinOpt opt;

	printf( "\t\tstatic const int Coeffs%iBase = %i;\n", Bandwidth,
		MinFilterLen );

	printf( "\t\tstatic const int Coeffs%iCount = %i;\n", Bandwidth,
		( MaxFilterLen - MinFilterLen + 2 ) / 2 );

	printf( "\t\tstatic const double Coeffs%i[ Coeffs%iCount ][ 3 ] = {\n",
		Bandwidth, Bandwidth );

	for( FilterLen = MinFilterLen; FilterLen <= MaxFilterLen; FilterLen += 2 )
	{
		opt.init( rnd );

		const int sct = 2 * 128; // Plateau threshold.
		int i;

		for( i = 0; i < 1500000; i++ )
		{
			if( opt.optimize( rnd ) > sct )
			{
				break;
			}
		}

		opt.optcost( opt.getBestParams() );

		printf( "\t\t\t{ %.16f, %.16f, %.4f }, // %.4f\n",
			opt.getBestParams()[ 0 ], opt.getBestParams()[ 1 ],
			-opt.opt_cost2, opt.opt_cost1 );
	}

	printf( "\t\t};\n" );
}
