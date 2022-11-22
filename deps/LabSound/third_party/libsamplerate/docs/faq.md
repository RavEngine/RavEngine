---
layout: default
---

# Frequently Asked Questions

1. [Q1 : Is it normal for the output of libsamplerate to be louder than its input?](#Q001)
2. [Q2 : On Unix/Linux/MacOSX, what is the best way of detecting the presence and location of libsamplerate and its header file using autoconf?](#Q002)
3. [Q3 : If I upsample and downsample to the original rate, for example 44.1-\>96-\>44.1, do I get an identical signal as the one before the up/down resampling?](#Q003)
4. [Q4 : If I ran src\_simple (libsamplerate) on small chunks (160 frames) would that sound bad?](#Q004)
5. [Q5 : I\'m using libsamplerate but the high quality settings sound worse than the SRC\_LINEAR converter. Why?](#Q005)
6. [Q6 : I\'m use the SRC\_SINC\_\* converters and up-sampling by a ratio of 2. I reset the converter and put in 1000 samples and I expect to get 2000 samples out, but I\'m getting less than that. Why?](#Q006)
7. [Q7 : I have input and output sample rates that are integer values, but the API wants me to divide one by the other and put the result in a floating point number. Won\'t this case problems for long running conversions?](#Q007)

## Q1 : Is it normal for the output of libsamplerate to be louder than its input? {#Q001}

The output of libsamplerate will be roughly the same volume as the input.
However, even if the input is strictly in the range (-1.0, 1.0), it is still
possible for the output to contain peak values outside this range.

Consider four consecutive samples of [0.5 0.999 0.999 0.5]. If we are up
sampling by a factor of two we need to insert samples between each of the
existing samples. Its pretty obvious then, that the sample between the two 0.999
values should and will be bigger than 0.999.

This means that anyone using libsamplerate should normalize its output before
doing things like saving the audio to a 16 bit WAV file.

## Q2 : On Unix/Linux/MacOSX, what is the best way of detecting the presence and location of libsamplerate and its header file using autoconf? {#Q002}

libsamplerate uses the pkg-config (man pkg-config) method of registering itself
with the host system. The best way of detecting its presence is using something
like this in configure.ac (or configure.in):

    PKG_CHECK_MODULES(SAMPLERATE, samplerate >= 0.1.3,
            ac_cv_samplerate=1, ac_cv_samplerate=0)

    AC_DEFINE_UNQUOTED([HAVE_SAMPLERATE],${ac_cv_samplerate},
            [Set to 1 if you have libsamplerate.])

    AC_SUBST(SAMPLERATE_CFLAGS)
    AC_SUBST(SAMPLERATE_LIBS)

This will automatically set the **SAMPLERATE_CFLAGS** and **SAMPLERATE_LIBS**
variables which can be used in Makefile.am or Makefile.in like this:

    SAMPLERATE_CFLAGS = @SAMPLERATE_CFLAGS@
    SAMPLERATE_LIBS = @SAMPLERATE_LIBS@

If you install libsamplerate from source, you will probably need to set the
**PKG_CONFIG_PATH** environment variable's suggested at the end of the
libsamplerate configure process. For instance on my system I get this:

    -=-=-=-=-=-=-=-=-=-= Configuration Complete =-=-=-=-=-=-=-=-=-=-=-

        Configuration summary :

        Version : ..................... 0.1.3
        Enable debugging : ............ no

        Tools :

        Compiler is GCC : ............. yes
        GCC major version : ........... 3

        Extra tools required for testing and examples :

        Have FFTW : ................... yes
        Have libsndfile : ............. yes
        Have libefence : .............. no

        Installation directories :

        Library directory : ........... /usr/local/lib
        Program directory : ........... /usr/local/bin
        Pkgconfig directory : ......... /usr/local/lib/pkgconfig

## Q3 : If I upsample and downsample to the original rate, for example 44.1->96->44.1, do I get an identical signal as the one before the up/down resampling? {#Q003}

The short answer is that for the general case, no, you don't. The long answer is
that for some signals, with some converters, you will get very, very close.

In order to resample correctly (ie using the **SRC_SINC_*** converters),
filtering needs to be applied, regardless of whether its upsampling or
downsampling. This filter needs to attenuate all frequencies above 0.5 times the
minimum of the source and destination sample rate (call this fshmin). Since the
filter needed to achieve full attenuation at this point, it has to start rolling
off a some frequency below this point. It is this rolloff of the very highest
frequencies which causes some of the loss.

The other factor is that the filter itself can introduce transient artifacts
which causes the output to be different to the input.

## Q4 : If I ran src_simple on small chunks (say 160 frames) would that sound bad? {#Q004}

Well if you are after odd sound effects, it might sound OK. If you are after
high quality sample rate conversion you will be disappointed.

The src_simple() was designed to provide a simple to use interface for people
who wanted to do sample rate conversion on say, a whole file all at once.

## Q5 : I'm using libsamplerate but the high quality settings sound worse than the SRC_LINEAR converter. Why? {#Q005}

There are two possible problems. Firstly, if you are using the src_simple()
function on successive blocks of a stream of samples, you will get bad results.
The src_simple() function is designed for use on a whole sound file, all at
once, not on contiguous segments of the same sound file. To fix the problem, you
need to move to the src_process() API or the callback based API.

If you are already using the src_process() API or the callback based API and the
high quality settings sound worse than SRC_LINEAR, then you have other problems.
Read on for more debugging hints.

All of the higher quality converters need to keep state while doing conversions
on segments of a large chunk of audio. This state information is kept inside the
private data pointed to by the SRC_STATE pointer returned by the src_new()
function. This means, that when you want to start doing sample rate conversion
on a stream of data, you should call src_new() to get a new SRC_STATE pointer
(or alternatively, call src_reset() on an existing SRC_STATE pointer). You
should then pass this SRC_STATE pointer to the src_process() function with each
new block of audio data. When you have completed the conversion, you can then
call src_delete() on the SRC_STATE pointer.

If you are doing all of the above correctly, you need to examine your usage of
the values passed to src\_process() in the [SRC_DATA](api_misc.md#src_data)
struct. Specifically:

- Check that input_frames and output_frames fields are being set in terms of
  frames (number of sample values times channels) instead of just the number of
  samples.
- Check that you are using the return values input_frames_used and
  output_frames_gen to update your source and destination pointers correctly.
- Check that you are updating the data_in and data_out pointers correctly for
  each successive call.

While doing the above, it is probably useful to compare what you are doing to
what is done in the example programs in the examples/ directory of the source
code tarball.

If you have done all of the above and are still having problems then its
probably time to email the author with the smallest chunk of code that
adequately demonstrates your problem. This chunk should not need to be any more
than 100 lines of code.

## Q6 : I'm use the SRC_SINC_* converters and up-sampling by a ratio of 2. I reset the converter and put in 1000 samples and I expect to get 2000 samples out, but I'm getting less than that. Why? {#Q006}

The short answer is that there is a transport delay inside the converter itself.
Long answer follows.

By way of example, the first time you call src_process() you might only get 1900
samples out. However, after that first call all subsequent calls will probably
get you about 2000 samples out for every 1000 samples you put in.

The main problems people have with this transport delay is that they need to
read out an exact number of samples and the transport delay scews this up. The
best way to overcome this problem is to always supply more samples on the input
than is actually needed to create the required number of output samples. With
reference to the example above, if you always supply 1500 samples at the input,
you will always get 2000 samples at the output. You will always need to keep
track of the number of input frames used on each call to src_process() and deal
with these values appropriately.

## Q7 : I have input and output sample rates that are integer values, but the API wants me to divide one by the other and put the result in a floating point number. Won't this case problems for long running conversions? {#Q007}

The short answer is no, the precision of the ratio is many orders of magnitude
more than is really needed.

For the long answer, lets do come calculations. Firstly, the `src_ratio` field
is double precision floating point number which has [53 bits of precision](http://en.wikipedia.org/wiki/Double_precision).

That means that the maximum error in your ratio converted to a double is one bit
in 2^53 which means the the double float value would be wrong by one sample
after 9007199254740992 samples have passed or wrong by more than half a sample
wrong after half that many (4503599627370496 samples) have passed.

Now if for example our output sample rate is 96kHz then

    4503599627370496 samples at 96kHz is 46912496118 seconds
    46912496118 seconds is 781874935 minutes
    781874935 minutes is 13031248 hours
    13031248 hours is 542968 days
    542968 days is 1486 years

So, after 1486 years, the input will be wrong by more than half of one sampling
period.

All this assumes that the crystal oscillators uses to sample the audio stream is
perfect. This is not the case. According to [this web site](http://www.ieee-uffc.org/freqcontrol/quartz/vig/vigcomp.htm),
the accuracy of standard crystal oscillators (XO, TCXO, OCXO) is at best 1 in
100 million. The `src_ratio` is therefore 45035996 times more accurate than the
crystal clock source used to sample the original audio signal and any potential
problem with the `src_ratio` being a floating point number will be completely
swamped by sampling inaccuracies.
