/**
 * @file example.cpp
 *
 * @brief An example C++ file that demonstrates resampler's usage.
 *
 * This is an example file which you won't be able to compile as it includes
 * undisclosed WAVE audio file input-output code. Please consider this example
 * as a pseudo-code demonstrating the use of the library. Here you can find an
 * example implementation of the simplest sample rate converter utility.
 *
 * r8brain-free-src Copyright (c) 2013-2021 Aleksey Vaneev
 * See the "LICENSE" file for license.
 */

int main()
{
	const double OutSampleRate = 96000.0;

	CWaveFile inf;
	inf.loadFile( "Audio.wav" );

	CWaveFile outf;
	outf.inheritFormat( inf );
	outf.SampleRate = OutSampleRate;
	outf.saveFile( "AudioOut.wav" );

	const int InBufCapacity = 1024;
	CFixedBuffer< double > InBufs[ inf.ChannelCount ];
	CPtrKeeper< CDSPResampler24* > Resamps[ inf.ChannelCount ];

	int i;

	for( i = 0; i < inf.ChannelCount; i++ )
	{
		InBufs[ i ].alloc( InBufCapacity );

		Resamps[ i ] = new CDSPResampler24( inf.SampleRate, OutSampleRate,
			InBufCapacity );
	}

	long long int ol = inf.SampleCount * OutSampleRate / inf.SampleRate;

	while( ol > 0 )
	{
		int ReadCount;
		inf.readData( InBufs, InBufCapacity, ReadCount );

		if( ReadCount == -1 )
		{
			ReadCount = InBufCapacity;

			for( i = 0; i < inf.ChannelCount; i++ )
			{
				memset( &InBufs[ i ][ 0 ], 0, ReadCount * sizeof( double ));
			}
		}

		double* opp[ inf.ChannelCount ];
		int WriteCount; // At initial steps this variable can be equal to 0
			// after resampler. Same number for all channels.

		for( i = 0; i < inf.ChannelCount; i++ )
		{
			WriteCount = Resamps[ i ] -> process(
				InBufs[ i ], ReadCount, opp[ i ]);
		}

		if( WriteCount > ol )
		{
			WriteCount = (int) ol;
		}

		outf.writeData( opp, WriteCount );
		ol -= WriteCount;
	}

	outf.finalize();
}
