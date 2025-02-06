..
  SPDX-License-Identifier: BSD-3-Clause
  Copyright Contributors to the OpenEXR Project.

Color4
######

.. code-block::

   #include <Imath/ImathColor.h>
   
The ``Color4`` class template represents a 4-component color (red,
green, blue, and alpha), with pre-defined typedefs of ``unsigned
char``, ``half``, and ``float``.

The ``Color4`` class is *not* derived from ``Vec4``. Its fields are
named ``r``, ``g``, ``b``, and ``a``. The class itself implies no
specific interpretation of the values.

There are also various utility functions that operate on colors
defined in ``ImathColorAlgo.h`` and described in :ref:`Color Functions
<color-functions>`.

Example:

.. literalinclude:: ../examples/Color4.cpp
   :language: c++

.. doxygentypedef:: Color4c

.. doxygentypedef:: Color4h

.. doxygentypedef:: Color4f

.. doxygentypedef:: C4c

.. doxygentypedef:: C4h

.. doxygentypedef:: C4f
                    
.. doxygenclass:: Imath::Color4
   :undoc-members:
   :members:

.. doxygenfunction:: operator<<(std::ostream& s, const Color4<T>& v)
      
