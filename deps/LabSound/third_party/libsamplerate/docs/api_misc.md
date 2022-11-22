---
layout: default
---

# Miscellaneous API Documentation

## Error Reporting

Most of the API functions either return an integer error (ie **src_simple** and
**src_process**) or return an integer error value via an int pointer parameter
(**src_new**). These integer error values can be converted into human-readable
text strings by calling the function:

```c
const char* src_strerror (int error) ;
```

which will return an error string for valid error numbers, the string
\"No Error\" for an error value of zero or a NULL pointer if no error message
has been defined for that error value.

## Converters

Secret Rabbit Code has a number of different converters which can be selected
using the **converter_type** parameter when calling **src_simple** or
**src_new**. Currently, the five converters available are:

```c
enum
{
    SRC_SINC_BEST_QUALITY       = 0,
    SRC_SINC_MEDIUM_QUALITY     = 1,
    SRC_SINC_FASTEST            = 2,
    SRC_ZERO_ORDER_HOLD         = 3,
    SRC_LINEAR                  = 4
} ;
```

As new converters are added, they will be given a number corresponding to the next
integer.

The details of these converters are as follows:

- **SRC_SINC_BEST_QUALITY** - This is a bandlimited interpolator derived from
  the mathematical **sinc** function and this is the highest quality sinc based
  converter, providing a worst case Signal-to-Noise Ratio (SNR) of 97 decibels
  (dB) at a bandwidth of 97%. All three SRC_SINC_* converters are based on the
  techniques of [Julius O. Smith](http://ccrma.stanford.edu/~jos/resample/)
  although this code was developed independently.
- **SRC_SINC_MEDIUM_QUALITY** - This is another bandlimited interpolator much
  like the previous one. It has an SNR of 97dB and a bandwidth of 90%. The speed
  of the conversion is much faster than the previous one.
- **SRC_SINC_FASTEST** - This is the fastest bandlimited interpolator and has an
  SNR of 97dB and a bandwidth of 80%.
- **SRC_ZERO_ORDER_HOLD** - A Zero Order Hold converter (interpolated value is
  equal to the last value). The quality is poor but the conversion speed is
  blindlingly fast.
- **SRC_LINEAR** - A linear converter. Again the quality is poor, but the
  conversion speed is blindingly fast.

There are two functions that give either a (text string) name or description for
each converter:

```c
const char *src_get_name (int converter_type) ;
const char *src_get_description (int converter_type) ;
```

The name will typically be a short string for use in a dialog box, while the
description string is longer.

Both of these functions return a NULL pointer if there is no converter for the
given **converter_type** value. Since the converters have consecutive
**converter_type** values, the caller is easily able to figure out the number of
converters at run time. This enables a binary dynamically linked against an old
version of the library to know about converters from later versions of the
library as they become available.

## SRC_DATA

Both the simple and the full featured versions of the API use the **SRC_DATA**
struct to pass audio and control data into the sample rate converter. This
struct is defined as:

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

The **data_in** pointer is used to pass audio data into the converter while the
**data_out** pointer supplies the converter with an array to hold the
converter's output. For a converter which has been configured for multichannel
operation, these pointers need to point to a single array of interleaved data.

The **input_frames** and **output_frames** fields supply the converter with the
lengths of the arrays (in frames) pointed to by the **data_in** and **data_out**
pointers respectively. For monophonic data, these values would indicate the
length of the arrays while for multi channel data these values would be equal to
the length of the array divided by the number of channels.

The **end_of_input** field is only used when the sample rate converter is used
by calling the **src_process** function. In this case it should be set to zero
if more buffers are to be passed to the converter and 1 if the current buffer is
the last.

Finally, the **src_ratio** field specifies the conversion ratio defined as the
output sample rate divided by the input sample rate. For a connected set of
buffers, this value can vary on each call to **src_process** resulting in a
time varying sample rate conversion process. For time varying sample rate
conversions, the ratio will be linearly interpolated between the **src_ratio**
value of the previous call to **src_process** and the value for the current
call.

The **input_frames_used** and **output_frames_gen** fields are set by the
converter to inform the caller of the number of frames consumed from the
**data_in** array and the number of frames generated in the **data_out** array
respectively. These values are for the current call to **src_process** only.

## Auxillary Functions

There are four auxillary functions for converting arrays of float data to and
from short or int data. These functions are defined as:

```c
void src_short_to_float_array (const short *in, float *out, int len) ;
void src_float_to_short_array (const float *in, short *out, int len) ;
void src_int_to_float_array (const int *in, float *out, int len) ;
void src_float_to_int_array (const float *in, int *out, int len) ;
```

The float data is assumed to be in the range [-1.0, 1.0] and it is automatically
scaled on the conversion to and from float. On the float to short/int conversion
path, any data values which would overflow the range of short/int data are
clipped.
