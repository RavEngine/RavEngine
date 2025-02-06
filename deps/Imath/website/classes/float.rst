..
  SPDX-License-Identifier: BSD-3-Clause
  Copyright Contributors to the OpenEXR Project.

Floating Point Representation
#############################

The half type is a 16-bit floating number, compatible with the
IEEE 754-2008 binary16 type.

Representation of a 32-bit float
--------------------------------

We assume that a float, ``f``, is an IEEE 754 single-precision
floating point number, whose bits are arranged as follows:

.. code-block::

    31 (msb)
    |
    | 30     23
    | |      |
    | |      | 22                    0 (lsb)
    | |      | |                     |
    X XXXXXXXX XXXXXXXXXXXXXXXXXXXXXXX

    s e        m

``s`` is the sign-bit, ``e`` is the exponent and ``m`` is the significand.

If ``e`` is between 1 and 254, ``f`` is a normalized number:

.. math::    {\tt f} = (-1)^{\tt s} \times 2^{\tt e-127} \times {\tt 1.m}

If ``e`` is 0, and ``m`` is not zero, ``f`` is a denormalized number:

.. math::    {\tt f} = (-1)^{\tt s} \times 2^{\tt e-126} \times {\tt 0.m}
    
If ``e`` and ``m`` are both zero, ``f`` is zero:

.. math::    {\tt f} = 0.0

If ``e`` is 255, ``f`` is an "infinity" or "not a number" (NAN),
depending on whether ``m`` is zero or not.

Examples:

.. code-block::

    0 00000000 00000000000000000000000 = 0.0
    0 01111110 00000000000000000000000 = 0.5
    0 01111111 00000000000000000000000 = 1.0
    0 10000000 00000000000000000000000 = 2.0
    0 10000000 10000000000000000000000 = 3.0
    1 10000101 11110000010000000000000 = -124.0625
    0 11111111 00000000000000000000000 = +infinity
    1 11111111 00000000000000000000000 = -infinity
    0 11111111 10000000000000000000000 = NAN
    1 11111111 11111111111111111111111 = NAN

Representation of a 16-bit half
-------------------------------

Here is the bit-layout for a half number, ``h``:

.. code-block::

    15 (msb)
    |
    | 14  10
    | |   |
    | |   | 9        0 (lsb)
    | |   | |        |
    X XXXXX XXXXXXXXXX

    s e     m

``s`` is the sign-bit, ``e`` is the exponent and ``m`` is the significand.

If ``e`` is between 1 and 30, ``h`` is a normalized number:

.. math::    {\tt h} = (-1)^{\tt s} \times 2^{\tt e-15} \times {\tt 1.m}
    
If ``e`` is 0, and ``m`` is not zero, ``h`` is a denormalized number:

.. math::    {\tt h} = (-1)^{\tt s} \times 2^{\tt -14} \times {\tt 0.m}

If ``e`` and ``m`` are both zero, ``h`` is zero:

.. math::    {\tt h} = 0.0

If ``e`` is 31, ``h`` is an "infinity" or "not a number" (NAN),
depending on whether ``m`` is zero or not.

Examples:

.. code-block::

    0 00000 0000000000 = 0.0
    0 01110 0000000000 = 0.5
    0 01111 0000000000 = 1.0
    0 10000 0000000000 = 2.0
    0 10000 1000000000 = 3.0
    1 10101 1111000001 = -124.0625
    0 11111 0000000000 = +infinity
    1 11111 0000000000 = -infinity
    0 11111 1000000000 = NAN
    1 11111 1111111111 = NAN

Conversion via Lookup Table
---------------------------

Converting from half to float is performed by default using a
lookup table. There are only 65,536 different half numbers; each
of these numbers has been converted and stored in a table pointed
to by the ``imath_half_to_float_table`` pointer.

Prior to Imath v3.1, conversion from float to half was
accomplished with the help of an exponent look table, but this is
now replaced with explicit bit shifting.

Conversion via Hardware
-----------------------

For Imath v3.1, the conversion routines have been extended to use
F16C SSE instructions whenever present and enabled by compiler
flags.

Conversion via Bit-Shifting
---------------------------

If F16C SSE instructions are not available, conversion can be
accomplished by a bit-shifting algorithm. For half-to-float
conversion, this is generally slower than the lookup table, but it
may be preferable when memory limits preclude storing of the
65,536-entry lookup table.

The lookup table symbol is included in the compilation even if
``IMATH_HALF_USE_LOOKUP_TABLE`` is false, because application code
using the exported ``half.h`` may choose to enable the use of the table.

An implementation can eliminate the table from compilation by
defining the ``IMATH_HALF_NO_LOOKUP_TABLE`` preprocessor symbol.
Simply add:

.. code-block::

    #define IMATH_HALF_NO_LOOKUP_TABLE

before including ``half.h``, or define the symbol on the compile
command line.

Furthermore, an implementation wishing to receive ``FE_OVERFLOW``
and ``FE_UNDERFLOW`` floating point exceptions when converting
float to half by the bit-shift algorithm can define the
preprocessor symbol ``IMATH_HALF_ENABLE_FP_EXCEPTIONS`` prior to
including ``half.h``:

.. code-block::

    #define IMATH_HALF_ENABLE_FP_EXCEPTIONS

Conversion Performance Comparison
---------------------------------

Testing on a Core i9, the timings are approximately:

- half to float:

  * table: 0.71 ns / call
  * no table: 1.06 ns / call
  * f16c: 0.45 ns / call

- float-to-half:

  * original: 5.2 ns / call
  * no exp table + opt: 1.27 ns / call
  * f16c: 0.45 ns / call

**Note:** the timing above depends on the distribution of the
floats in question.


