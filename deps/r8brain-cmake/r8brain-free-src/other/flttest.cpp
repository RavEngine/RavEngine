// Fractional-delay interpolation test. Tests FilterFracs and interpolation
// at different target SNRs.
//
// r8brain-free-src Copyright (c) 2013-2021 Aleksey Vaneev
// See the "LICENSE" file for license.

#include "/libvox/Sources/Core/CWorkerThreadPool.h"
using namespace vox;

#include <stdio.h>
#include "../CDSPResampler.h"

const double MinTargetSNR = -r8b :: CDSPFIRFilter :: getLPMinAtten();
const double MaxTargetSNR = -r8b :: CDSPFIRFilter :: getLPMaxAtten();
const int TargetCount = 20;

const double InSampleRate = 44100.0;
const double SineFreq = 16000.0;
const int InBufSize = 70000;
r8b :: CFixedBuffer< double > InBuf( InBufSize );

class CStats
{
public:
	CSyncObject StateSync;

	double avgd;
	double maxd;
	double peakd;
	int avgc;

	CStats()
	{
		reset();
	}

	void reset()
	{
		avgd = 0.0;
		maxd = 0.0;
		peakd = 0.0;
		avgc = 0;
	}
};

class CResamp : public r8b :: CDSPResampler
{
public:
	CResamp( const double SrcSampleRate, const double DstSampleRate,
		const int aMaxInLen, const double ReqTransBand = 2.0 )
		: CDSPResampler( SrcSampleRate, DstSampleRate, aMaxInLen,
			ReqTransBand, 220.0, r8b :: fprLinearPhase )
	{
	}
};

inline double calcSineRMS( const double* const p1, const int l, const int o,
	const double SrcSampleRate, const double DstSampleRate, double& peakd )
{
	double s = 0.0;
	int i;

	for( i = 0; i < l - o; i++ )
	{
		const double v = sin( i * 2.0 * M_PI * SineFreq * SrcSampleRate /
			( InSampleRate * DstSampleRate ));

		if( i >= o )
		{
			const double d = fabs( p1[ i ] - v );
			s += d * d;
			peakd = max( peakd, d );
		}
	}

	return( sqrt( s / ( l - o * 2 )));
}

class CResampThread : public CWorkerThread
{
public:
	CStats* Stats; ///< Pointer to summary statistics object.
		///<
	double SrcSampleRate; ///< Source sample rate.
		///<
	double DstSampleRate; ///< Destination sample rate.
		///<

protected:
	virtual ecode performWork()
	{
		const int MaxInLen = 521;
		const double tb = 5.0;

		const int ol1 = (int) ( InBufSize * DstSampleRate / SrcSampleRate );

		r8b :: CFixedBuffer< double > OutBuf1( ol1 );

		r8b :: CPtrKeeper< CResamp* > Resamp1;
		Resamp1 = new CResamp( SrcSampleRate, DstSampleRate, MaxInLen, tb );

		Resamp1 -> oneshot( &InBuf[ 0 ], InBufSize, &OutBuf1[ 0 ], ol1 );

		const int o = (int) ( ol1 * 0.2 );
		double peakd = 0.0;
		const double r = calcSineRMS( OutBuf1, ol1, o, SrcSampleRate,
			DstSampleRate, peakd );

		VOXSYNC( Stats -> StateSync );

		Stats -> avgd += r * r;
		Stats -> maxd = max( Stats -> maxd, r );
		Stats -> peakd = max( Stats -> peakd, peakd );
		Stats -> avgc++;

		VOXRET;
	}
};

int main()
{
	// Create reference sinewave signal.

	int i;

	if( true )
	{
		for( i = 0; i < InBufSize; i++ )
		{
			InBuf[ i ] = sin( i * 2.0 * M_PI * SineFreq / InSampleRate );
		}
	}

	CWorkerThreadPool Threads;

	for( i = 0; i < CSystem :: getProcessorCount(); i++ )
	{
		Threads.add( new CResampThread() );
	}

	for( i = 0; i < TargetCount; i++ )
	{
		const double TargetSNR = MinTargetSNR +
			( MaxTargetSNR - MinTargetSNR ) * i / ( TargetCount - 1 );

		const int Fracs = (int) ceil( pow( 6.4, -TargetSNR / 50.0 ));

		CStats Stats;
		const double SrcSampleRate = 20.0;
		double DstSampleRate = SrcSampleRate;
		int ctr = 0;

		while( DstSampleRate < SrcSampleRate * 30.0 )
		{
			r8b :: InterpFilterFracs = Fracs + ctr;
			ctr = ( ctr + 1 ) & 7;

			CResampThread* rs;
			VOXERRSKIP( Threads.getIdleThread( rs ));

			rs -> Stats = &Stats;
			rs -> SrcSampleRate = SrcSampleRate;
			DstSampleRate *= 1.035;
			rs -> DstSampleRate = DstSampleRate;

			rs -> start();
		}

		VOXERRSKIP( Threads.waitAllForFinish() );

		Stats.avgd = 10.0 * log( Stats.avgd / Stats.avgc ) / log( 10.0 );
		Stats.maxd = 20.0 * log( Stats.maxd ) / log( 10.0 );
		Stats.peakd = 20.0 * log( Stats.peakd ) / log( 10.0 );

		printf( "%.1f %i > ", TargetSNR, Fracs );
		printf( "Avg diff %.2f ", Stats.avgd );
		printf( "Max diff %.2f ", Stats.maxd );
		printf( "Peak diff %.2f\n", Stats.peakd );
	}
}
