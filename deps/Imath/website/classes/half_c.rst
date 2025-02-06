..
  SPDX-License-Identifier: BSD-3-Clause
  Copyright Contributors to the OpenEXR Project.

C-language half-float Conversion
################################

The ``half.h`` header can be included in pure C code:

.. literalinclude:: ../examples/half.c
   :language: c

The only C-language operations supported for the ``half`` type are
conversion to and from ``float``. No arithmetic operations are
currently implemented in the C interface.

.. doxygenfunction:: imath_half_to_float

.. doxygenfunction:: imath_float_to_half


