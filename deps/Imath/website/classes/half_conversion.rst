..
  SPDX-License-Identifier: BSD-3-Clause
  Copyright Contributors to the OpenEXR Project.

.. _half-float-conversion-configuration-options:

Build-time half-float Conversion Configuration Options
######################################################

The Imath library supports three options for conversion between 16-bit
half and 32-bit float:

1. Conversion from half to float via a 16-bit lookup table. Prior to
   Imath v3.1, this was the only method supported.

2. F16C SSE instructions: single-instruction conversion for machine
   architectures that support it. When available, this is the fastest
   option, by far.

3. Bit-shift conversion algorithm.

To use the F16C SSE instruction set on an architecture that supports
it, simply provide the appropriate compiler flags when building an
application that includes ``half.h``. For g++ and clang,
for example:
::

    $ cmake -DCMAKE_CXX_FLAGS="-m16fc" <source directory> 
    
When code including ``half.h`` is compiled with F16C enabled, it will
automatically perform conversions using the instruction set. F16C
compiler flags take precedence over other lookup-table-related Imath
CMake settings.

On architectures that do not support F16C, you may choose at
compile-time between the bit-shift conversion and lookup table
conversion via the ``IMATH_HALF_USE_LOOKUP_TABLE`` CMake option:
::

    $ cmake -DIMATH_HALF_USE_LOOKUP_TABLE=OFF <source directory>

Note that when building and installing the Imath library itself, the
65,536-entry lookup table symbol will be compiled into the library
even if the ``IMATH_HALF_USE_LOOKUP_TABLE`` setting is false. This
allows applications using that installed Imath library downstream to
choose at compile time which conversion method to use.

Applications with memory limitations that cannot accomodate the
conversion lookup table can eliminate it from the library by building
Imath with the C preprocessor define ``IMATH_HALF_NO_LOOKUP_TABLE``
defined. Note that this is a compile-time option, not a CMake setting
(making it possible for application code to choose the desired
behavior). Simply add:
::

    #define IMATH_HALF_NO_LOOKUP_TABLE

before including ``half.h``, or define the symbol on the compile
command line.

Furthermore, an implementation wishing to receive ``FE_OVERFLOW`` and
``FE_UNDERFLOW`` floating point exceptions when converting float to
half by the bit-shift algorithm can define the preprocessor symbol
``IMATH_HALF_ENABLE_FP_EXCEPTIONS`` prior to including ``half.h``:
::
   
    #define IMATH_HALF_ENABLE_FP_EXCEPTIONS

By default, no exceptions are raised on overflow and underflow.






