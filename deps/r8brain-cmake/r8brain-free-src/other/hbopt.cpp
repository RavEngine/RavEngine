// Half-band filter coefficient optimizer. Finds coefficients for the list of
// fractions using the BiteOptDeep derivative-free optimization method.
//
// r8brain-free-src Copyright (c) 2013-2021 Aleksey Vaneev
// See the "LICENSE" file for license.

#include <stdio.h>
#include <math.h>
#include "/projects/biteopt/biteopt.h"
#include "../CDSPSincFilterGen.h"

const double Fractions[] = { 4.0, 8.0, 16.0, 32.0, 64.0, 128.0, 256.0, -1.0 };
//const double Fractions[] = { 6.0, 12.0, 24.0, 48.0, 96.0, 192.0, 384.0, -1.0 };

double Fraction = 4.0; // 1/Fraction of frequency response that should
	// be preserved (in low freqs) and attenuated (in high freqs).
const double TapMult = 2.0; // Tap gain multiplier.
const int OptDepth = 8; // BiteOptDeep optimizer depth.
const char LettBase = 'A'; // Base letter for array naming.

class CHBOpt : public CBiteOptDeep
{
public:
	virtual void getMinValues( double* const p ) const
	{
		int i;

		for( i = 0; i < TapCount; i++ )
		{
			p[ i ] = 0.0;
		}
	}

	virtual void getMaxValues( double* const p ) const
	{
		int i;

		for( i = 0; i < TapCount; i++ )
		{
			p[ i ] = 1.0;
		}
	}

	double opt_cost2;

	virtual double optcost( const double* const p )
	{
		double* kc = KernelBlock + KernelLenD2;
		*kc = 0.5;
		int i;

		for( i = 0; i < TapCount; i++ )
		{
			kc[ -i * 2 - 1 ] = Sinc[ i ] * p[ i ];
			kc[ i * 2 + 1 ] = Sinc[ i ] * p[ i ];
		}

		const double _10ln10 = 10.0 / log( 10.0 );
		const int Count1 = 250;
		double cost1 = -1000.0;
		double re;
		double im;
		const double fr = 1.25 / Fraction;
		const double fre = 1.0 / Fraction;

		for( i = 0; i <= Count1; i++ )
		{
			const double th = M_PI * fr * i / Count1;
			r8b :: calcFIRFilterResponse( KernelBlock, KernelLen, th, re, im );
			double p = fabs( _10ln10 * log( re * re + im * im ));

			cost1 = max( p, cost1 );
		}

		const int Count2 = 500;
		double cost2 = -1000.0;

		for( i = 0; i <= Count2; i++ )
		{
			const double th = M_PI * ( 1.0 - fre * i / Count2 );
			r8b :: calcFIRFilterResponse( KernelBlock, KernelLen, th, re, im );
			double p = _10ln10 * log( re * re + im * im );

			cost2 = max( p, cost2 );
		}

		opt_cost2 = cost2;

		return( cost1 * 3600.0 + cost2 );
	}

	void setTapCount( const int NewTapCount, const int NewSeed = 1 )
	{
		memset( KernelBlock, 0, sizeof( KernelBlock ));

		TapCount = NewTapCount;
		KernelLen = TapCount * 4 - 1;
		KernelLenD2 = KernelLen >> 1;
		sct = TapCount * 128;

		int i;

		for( i = 0; i < TapCount; i++ )
		{
			const double xpi = ( i * 2 + 1 ) * M_PI;
			Sinc[ i ] = sin( xpi * 0.5 ) / xpi;
		}

		updateDims( TapCount, OptDepth );
		ornd.init( NewSeed );
	}

	void performOpt()
	{
		init( ornd );

		int i;

		for( i = 0; i < 1500000; i++ )
		{
			if( optimize( ornd ) > sct )
			{
				break;
			}
		}
	}

	void printOpt( const char Lett )
	{
		printf( "\t\tstatic const double HBKernel_%i%c[ %i ] = { "
			"// %.4f dB, %.0f", TapCount, Lett, TapCount,
			opt_cost2, Fraction );

		int i;

		for( i = 0; i < TapCount; i++ )
		{
			printf(( i & 1 ) == 0 ? "\n\t\t\t" : " " );
			printf( "%.16e,", Sinc[ i ] * getBestParams()[ i ] * TapMult );
		}

		printf( "};\n" );
	}

protected:
	int TapCount; // The number of non-zero taps in the filter.
	int KernelLen;
	int KernelLenD2;
	double Sinc[ 32 ]; // Sinc function.
	double KernelBlock[ 32 * 4 ];
	int sct; // Optimizer plateau threshold.
	CBiteRnd ornd; // PRNG object.
};

int main()
{
	CHBOpt opt;
	int CurFraction = 0;

	while( true )
	{
		if( Fractions[ CurFraction ] <= 0.0 )
		{
			break;
		}

		Fraction = Fractions[ CurFraction ];
		double att[ 16 ];
		int fltt[ 16 ];
		int attc = 0;
		bool WasMinObtained = false;
		char Lett = LettBase + CurFraction;
		int k;

		for( k = 1; k < 16; k++ )
		{
			opt.setTapCount( k );
			opt.performOpt();

			opt.optcost( opt.getBestParams() );

			if( opt.opt_cost2 < -49.0 )
			{
				if( opt.opt_cost2 < -220.0 )
				{
					if( WasMinObtained )
					{
						break;
					}

					WasMinObtained = true;
				}

				opt.printOpt( Lett );

				att[ attc ] = opt.opt_cost2;
				fltt[ attc ] = k;
				attc++;
			}
		}

		printf( "\t\tstatic const int FltCount%c = %i;\n", Lett,
			attc );

		printf( "\t\tstatic const int FlttBase%c = %i;\n", Lett,
			fltt[ 0 ]);

		printf( "\t\tstatic const double FltAttens%c[ FltCount%c ] = "
			"{\n\t\t\t", Lett, Lett );

		for( k = 0; k < attc; k++ )
		{
			printf( "%.4f, ", -att[ k ]);
		}

		printf( "};\n" );

		printf( "\t\tstatic const double* const FltPtrs%c[ FltCount%c ] = "
			"{\n\t\t\t", Lett, Lett );

		for( k = 0; k < attc; k++ )
		{
			printf( "HBKernel_%i%c, ", fltt[ k ], Lett );
		}

		printf( "};\n" );

		CurFraction++;
	}
}
