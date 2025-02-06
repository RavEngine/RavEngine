..
  SPDX-License-Identifier: BSD-3-Clause
  Copyright Contributors to the OpenEXR Project.

.. _half:

half
####

.. code-block::

   #include <Imath/half.h>

``half`` is a 16-bit floating point number. See :doc:`float` for an
explanation of the representation.

See :doc:`half_c` for C-language functions for conversion
between ``half`` and ``float``. Also, see :doc:`half_conversion`
for information about building Imath with support for the F16C SSE
instruction set.

Example:

.. literalinclude:: ../examples/half.cpp
   :language: c++
              
.. toctree::
   :caption: half
   :maxdepth: 1

   half_class
   half_limits
   half_c
   half_conversion
   float
              
