---
layout: default
---

# Callback API

The callback API consists of the following functions :

```c
typedef long (*src_callback_t) (void *cb_data, float **data) ;

SRC_STATE* src_callback_new (src_callback_t func,
                    int converter_type, int channels,
                    int *error, void* cb_data) ;

SRC_STATE* src_delete (SRC_STATE *state) ;

long src_callback_read (SRC_STATE *state, double src_ratio,
                    long frames, float *data) ;

int src_reset (SRC_STATE *state) ;
int src_set_ratio (SRC_STATE *state, double new_ratio) ;
```

Like the [simple API](api_simple.md) and the [full API](api_full.md), the
callback based API is able to operate on interleaved multi channel data.

An example of the use of the callback based API can be found in the
**varispeed-play** program in the **examples/** directory of the source code
tarball.

## Initialisation

```c
SRC_STATE* src_callback_new (src_callback_t func,
                    int converter_type, int channels,
                    int *error, void* cb_data) ;
```

The **src_callback_new** function returns an anonymous pointer to a sample rate
converter callback object, src_state. This is the same type of object as that
returned by [src\_new](api_full.md#initialisation), but with different internal
state. Although these are the same object type, they cannot be used
interchangeably. If an error occurs the function returns a NULL pointer and
fills in the error value pointed to by the **error** pointer supplied by the
caller.

The caller then passes the SRC_STATE object to the **src_callback_read**
function to pull data out of the converter. When the caller is finished with the
converter they should pass it to the clean up function [src_delete](api_full.md#cleanup).

The **func** parameter is a user supplied function which must match the
**src_callback_t** type signature while **cb_data** is a pointer to data which
be passed as the first parameter to the user supplied callback function. This
function is called by the converter whenever it needs input data as a result of
being calls to **src_callback_read**.

If the converter was initialised to work with more than one channel, the
callback function must work with mutiple channels of interleaved data. The
callback function should return the number of frames it supplying to the
converter. For multi channel files, this return value should be the number of
floats divided by the number of channels.

The converter must be one of the supplied converter types documented [here](api_misc.md#converters).

The caller then passes the SRC_STATE pointer to the **src_callback_read**
function to pull data out of the converter.

## Callback Read

```c
long src_callback_read (SRC_STATE *state, double src_ratio,
                    long frames, float *data) ;
```

The **src_callback_read** function is passed the [**SRC_STATE**](api_misc.md#src_data)
pointer returned by **src_callback_new**, the coversion ratio
(output_sample_rate / input_sample_rate), the maximum number of output frames to
generate and a pointer to a buffer in which to place the output data. For multi
channel files, the data int the output buffer is stored in interleaved format.

The **src_callback_read** function returns the number of frames generated or
zero if an error occurs or it runs out of input (ie the user supplied callback
function returns zero and there is no more data buffered internally). If an
error has occurred, the function [src_error](api_misc.md#error-reporting)
will return non-zero.

See also : [**src_set_ratio**](api_full.md#set-ratio)
