/**
 * @file calcErrorTable.cpp
 *
 * @brief Utility program that calculates parameter error table.
 *
 * This utility program calculates tabbed parameter error table for the whole
 * range of transition bands and stop-band attenuations. The error is
 * introduced by approximation functions used in the CDSPFIRFilter class's
 * buildLPFilter() function. If the error is small the "atten_err" should be
 * close to 0 and "gtb" should be close to -3.01. Results will be sent to
 * the "stdout".
 *
 * r8brain-free-src Copyright (c) 2013-2021 Aleksey Vaneev
 * See the "LICENSE" file for license.
 */

#include <stdio.h>
#include "../CDSPFIRFilter.h"
using namespace r8b;

int main()
{
	const double MinAtten = CDSPFIRFilter :: getLPMinAtten();
	const double MaxAtten = CDSPFIRFilter :: getLPMaxAtten();
	const int AttenSteps = 300;
//	const int AttenSteps = 10000;
	const double g = 10.0 / log( 10.0 );
	double ErrorMax = -1e20;
	double ErrorMin = 1e20;
	double ErrorMax60 = -1e20;
	double ErrorMin60 = 1e20;
	double ErrorMax117 = -1e20;
	double ErrorMin117 = 1e20;
	int i;

	printf( "tb\tg05\tgtb\tatten\tatten_err\n" );

	for( i = 0; i < AttenSteps; i++ )
	{
		const double atten = MinAtten + ( MaxAtten - MinAtten ) * i /
			( AttenSteps - 1 );

		double tb;

//		for( tb = 0.5; tb < 10.0; tb *= 1.01 )
//		for( tb = 10.0; tb < 25.0; tb *= 1.003 )
		for( tb = 25.0; tb <= 45.0; tb *= 1.003 )

//		for( tb = 0.5; tb < 10.0; tb *= 1.001 )
//		for( tb = 10.0; tb < 25.0; tb *= 1.001 )
//		for( tb = 25.0; tb <= 45.0; tb *= 1.001 )
		{
			CDSPFIRFilter& f =
				CDSPFIRFilterCache :: getLPFilter( 0.5, tb, atten,
				fprLinearPhase, 1.0 );

			// Perform inverse FFT to obtain time-domain FIR filter.

			CDSPRealFFTKeeper ffto( f.getBlockLenBits() + 1 );
			CFixedBuffer< double > Kernel( 2 << f.getBlockLenBits() );
			memcpy( &Kernel[ 0 ], f.getKernelBlock(),
				( 2 * sizeof( double )) << f.getBlockLenBits() );

			ffto -> inverse( Kernel );

			// Calculate spectrum power at points of interest.

			const int l = f.getKernelLen();
			double re;
			double im;

			calcFIRFilterResponse( &Kernel[ 0 ], l, M_PI * 0.5, re, im );
			const double g05 = g * log( re * re + im * im );

			calcFIRFilterResponse( &Kernel[ 0 ], l,
				M_PI * ( 1.0 - tb * 0.01 ) * 0.5, re, im );

			const double gtb = g * log( re * re + im * im );
			const double err = g05 + atten;

			if( atten >= 117.0 )
			{
				if( err > ErrorMax117 )
				{
					ErrorMax117 = err;
				}
				else
				if( err < ErrorMin117 )
				{
					ErrorMin117 = err;
				}
			}
			else
			if( atten >= 60.0 )
			{
				if( err > ErrorMax60 )
				{
					ErrorMax60 = err;
				}
				else
				if( err < ErrorMin60 )
				{
					ErrorMin60 = err;
				}
			}
			else
			{
				if( err > ErrorMax )
				{
					ErrorMax = err;
				}
				else
				if( err < ErrorMin )
				{
					ErrorMin = err;
				}
			}

			// Output the error data.

			printf( "%f\t%f\t%f\t%f\t%f\n", tb, g05, gtb, -atten, err );

			f.unref();
		}
	}

	printf( "error min %f\n", ErrorMin );
	printf( "error max %f\n", ErrorMax );
	printf( "error min >=60 %f\n", ErrorMin60 );
	printf( "error max >=60 %f\n", ErrorMax60 );
	printf( "error min >=117 %f\n", ErrorMin117 );
	printf( "error max >=117 %f\n", ErrorMax117 );

	return( 0 );
}
