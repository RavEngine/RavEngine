---
layout: default
---

# SRC Quality

**This document not yet complete.**

When measuring the performance of a Sample Rate Converter, there are three
factors to consider:

- **Signal-to-Noise Ratio** - a measure of how much noise the sample rate
  conversion process adds to the signal. This is measured in decibels (dB) and
  the higher this value the better. For most sample rate converters, the SNR
  will vary depending on the input signal and the ratio between input and output
  sample rates. The only valid comparison of SNR is between the worst case for
  each converter.
- **Bandwidth** - most sample rate converters attenuate high frequencies as part
  of their operation. Bandwidth can be measured by finding the frequency where
  the attenuation is 3dB and expressing that as a percentage of the full
  bandwidth at that sampling rate.
- **Speed** - the faster the better **:-)**.

There are a number of sample rate converters available for downloading but I
will limit the comparison ot Secret Rabbit Code to the following:

- [sndfile-resample](http://libsndfile.github.io/libsamplerate/download.html)
  which is a program (which uses libsamplerate) from the **examples/** directory
  of the Secret Rabbit Code source code distribution.
- [Resample](https://ccrma.stanford.edu/~jos/resample/) by Julius O Smiths which
  seems to have been the first high quality converter available as source code.
- [ResampAudio](http://www.tsp.ece.mcgill.ca/MMSP/Documents/Software/AFsp/ResampAudio.html)
  which is part of [Audio File Programs and Routines](http://www.tsp.ece.mcgill.ca/MMSP/Documents/Software/AFsp/AFsp.html)
  by Peter Kabal.
- [SoX](http://home.sprynet.com/~cbagwell/sox.html) which is maintained by Chris
  Bagwell. SoX is also able to perform some low quality sample rate conversions
  but these will not be investigated.
- [Shibatch](http://shibatch.sourceforge.net/) which seems to be a frequency
  domain sample rate converter. Unfortunately, this converter does not handle
  arbitrary conversion ratios and hence could not be properly compared to the
  other converters.
- [sr-convert](http://sr-convert.sourceforge.net/) is another converter which
  does not handle arbitrary conversion ratios.

It should be noted that the first three converters above are based on the
algorithm by [Julius O. Smith](http://www-ccrma.stanford.edu/~jos/resample/)
which emulates the conversion of the digital signal to an analogue one and then
sampling the analogue signal at the new sample rate.

## Methodology

Measuring the SNR of a converter is relatively straight forward. Generate an
input signal consisting of a windowed sine wave, sample rate convert it and
measure the signal-to-noise ratio of the output signal. A typical length for the
original file is 30000 samples.

The bandwidth of a sample rate converter is a little more difficult to measure.
Currently this is done by generating two short files containing a windowed sine
wave. The frequencies of the sine waves are 0.35 and 0.495 of the sample rate.
These file are then upsampled by a factor of 2 using the converter under test.
If the attenutaion of the lower frquency is less than 3dB and higher frequency
is more than 3dB, it is then possible to iteratively increase the lower
frequency and decrease the upper frequency keeping the -3dB point bracketed.
When the distance between the upper and lower frequency is sufficiently small,
it is possible to obtain a very accurate estimate of the -3dB frequency.

The speed of a sample rate converter is easy to measure; simply perform a
conversion on a large file or a number of smaller files and time the conversion
process.

The above measurement techniques are built into a test program which is
delivered with the Secret Rabbit Code source code distibution. This program is
able to test the first four of the above converters.

## SoX

SoX provides three methods of resampling; a linear interpolator, a polyphase
resampler and the Julius O. Smith simulated analogue filter method.

## Shibatch

Shibach

**More Coming Soon.**
