---
layout: default
---

# Simple API

**Important Note:** The simple API is not designed to work on small chunks of a
larger piece of audio. If you attempt to use it this way you are doing it wrong
and will not get the results you want. For processing audio data in chunks you
**must** use the [full api](api_full.md) or the [callback based api](api_callback.md).

The simple API consists of a single function:

```c
int src_simple (SRC_DATA *data, int converter_type, int channels) ;
```

The use of this function rather than the more fully featured API requires the
caller to know the total length of the input data before hand and that all input
and output data can be held in the system's memory at once. It also assumes that
there is a single constant ratio between input and output sample rates.

Dealing with the easy stuff first, the **converter_type** parameter should be
one of the values defined in **samplerate.h** and documented [here](api_misc.md#converters)
while the **channels** parameter specifies the number of interleaved channels
that the sample rate converter is being asked to process (number of input
channels and output channels is always equal). There is no hard upper limit on
the number of channels; it is limited purely by the amount of memory available.

The first parameter to **src_simple** is a pointer to an **SRC_DATA** struct
(more info [here](api_misc.md#src_data)) defined as follows:

```c
typedef struct
{   const float  *data_in;
    float *data_out ;

    long   input_frames, output_frames ;
    long   input_frames_used, output_frames_gen ;

    int    end_of_input ;

    double src_ratio ;
} SRC_DATA ;
```

The fields of this struct which must be filled in by the caller are:

data_in
: A pointer to the input data samples.

input_frames
: The number of frames of data pointed to by data_in.

data_out
: A pointer to the output data samples.

output_frames
: Maximum number of frames pointer to by data_out.

src_ratio
: Equal to output_sample_rate / input_sample_rate.

When the **src_simple** function returns **output_frames_gen** will be set to
the number of output frames generated and **input_frames_used** will be set to
the number of input frames used to generate the provided number of output
frames.

The **src_simple** function returns a non-zero value when an error occurs. See
[here](api_misc.md#error-reporting) for how to convert the error value into a
text string.
