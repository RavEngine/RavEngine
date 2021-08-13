/**
 * @file calcErrorTable.cpp
 *
 * @brief Utility program that calculates attenuation correction table.
 *
 * This utility program calculates attenuation correction coefficients used
 * to increase filter's attenuation precision.
 *
 * r8brain-free-src Copyright (c) 2013-2021 Aleksey Vaneev
 * See the "LICENSE" file for license.
 */

#include <stdio.h>
#include "../CDSPFIRFilter.h"
using namespace r8b;

template< class T >
inline T round( const T d )
{
	return( d < 0.0 ? -floor( (T) 0.5 - d ) : floor( d + (T) 0.5 ));
}

int main()
{
	const double g = 10.0 / log( 10.0 );
	const double MinAtten = CDSPFIRFilter :: getLPMinAtten();
	const double MaxAtten = CDSPFIRFilter :: getLPMaxAtten();
	const double AttenDiff = MaxAtten - MinAtten;

	const double AttenScale = 196.0;
	const int AttenTableSize = (int) round( AttenDiff * 1.5 );
	double AttensAvg[ AttenTableSize + 1 ];
	double AttensMin[ AttenTableSize + 1 ];
	double AttensMax[ AttenTableSize + 1 ];
	double AttenCorrs[ AttenTableSize + 1 ];
	int AttenCounts[ AttenTableSize + 1 ];
	int i;

	for( i = 0; i <= AttenTableSize; i++ )
	{
		AttenCorrs[ i ] = 0.0;
	}

	printf( "\t\t\tstatic const int AttenCorrCount = %i;\n", AttenTableSize );
	printf( "\t\t\tstatic const double AttenCorrMin = %.1f;\n", MinAtten );
	printf( "\t\t\tstatic const double AttenCorrDiff = %.2f;\n", AttenDiff );
	printf( "\t\t\tstatic const double AttenCorrScale = %.1f;\n", AttenScale );
	printf( "\t\t\tconst int AttenCorr = (int) floor(( -atten - "
		"AttenCorrMin ) *\n\t\t\t\tAttenCorrCount / AttenCorrDiff + "
		"0.5 );\n\n" );

	const int IterCount = 5;
	int j;

	for( j = 0; j < IterCount; j++ )
	{
		int i;

		for( i = 0; i <= AttenTableSize; i++ )
		{
			AttensAvg[ i ] = 0.0;
			AttensMin[ i ] = 1e20;
			AttensMax[ i ] = -1e20;
			AttenCounts[ i ] = 0;
		}

		const int AttenSteps = 10000;
//		const int AttenSteps = 2000;

		for( i = 0; i <= AttenSteps; i++ )
		{
			const double ReqAtten = MinAtten + AttenDiff * i / AttenSteps;
			double tb;

//			for( tb = 0.5; tb < 10.0; tb *= 1.005 )
//			for( tb = 10.0; tb < 25.0; tb *= 1.00153 )
			for( tb = 25.0; tb <= 45.0; tb *= 1.001 )
			{
				CDSPFIRFilter& f = CDSPFIRFilterCache :: getLPFilter( 0.5, tb,
					ReqAtten, fprLinearPhase, AttenCorrs );

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

				const int z = (int) floor(( ReqAtten - MinAtten ) *
					AttenTableSize / AttenDiff + 0.5 );

				if( z >= 0 && z <= AttenTableSize )
				{
					const double v = g05 + ReqAtten;

					if( v > AttensMax[ z ])
					{
						AttensMax[ z ] = v;
					}
					else
					if( v < AttensMin[ z ])
					{
						AttensMin[ z ] = v;
					}

					AttensAvg[ z ] += v;
					AttenCounts[ z ]++;
				}

				f.unref();
			}
		}

		for( i = 0; i <= AttenTableSize; i++ )
		{
			AttenCorrs[ i ] += ( AttensMax[ i ] + AttensMin[ i ]) * 0.5;
		}
	}

	printf( "\t\t\tstatic const signed char AttenCorrs[] = {\n\t\t\t\t" );
	int ClipCount = 0;
	int LinePos = 4 * 4;
	char StrBuf[ 32 ];

	for( i = 0; i <= AttenTableSize; i++ )
	{
		int a = (int) round( AttenCorrs[ i ] * AttenScale );

		if( a > 127 )
		{
			a = 127;
			ClipCount++;
		}
		else
		if( a < -128 )
		{
			a = -128;
			ClipCount++;
		}

		sprintf( StrBuf, "%i,", a );
		const int l = (int) strlen( StrBuf );

		if( LinePos + l + 1 > 78 )
		{
			printf( "\n\t\t\t\t%s", StrBuf );
			LinePos = 4 * 4 + l;
		}
		else
		{
			printf( " %s", StrBuf );
			LinePos += l + 1;
		}
	}

	printf( "};\n\n\t\t\tif( ExtAttenCorrs != NULL )\n\t\t\t{\n" );
	printf( "\t\t\t\tatten -= ExtAttenCorrs[\n\t\t\t\t\t"
		"min( AttenCorrCount, max( 0, AttenCorr ))];\n\t\t\t}\n"
		"\t\t\telse\n\t\t\t{\n" );

	printf( "\t\t\t\tatten -= AttenCorrs[ "
		"min( AttenCorrCount,\n\t\t\t\t\tmax( 0, AttenCorr ))] / "
		"AttenCorrScale;\n\t\t\t}\n" );

	printf( "\n// Clipping count: %i\n", ClipCount );

	return( 0 );
}
