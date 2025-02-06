..
  SPDX-License-Identifier: BSD-3-Clause
  Copyright Contributors to the OpenEXR Project.

Color3
######

.. code-block::

   #include <Imath/ImathColor.h>

The ``Color3`` class template represents a 3-component color, with
pre-defined typedefs of ``unsigned char``, ``half``, and ``float``.

The ``Color3`` class inherits from ``Vec3`` and thus has
fields named ``x``, ``y``, and ``z``. The class itself implies no
specific interpretation of the values.

There are also various utility functions that operate on colors
defined in ``ImathColorAlgo.h`` and described in :ref:`Color Functions
<color-functions>`.

Example:

.. literalinclude:: ../examples/Color3.cpp
   :language: c++

.. doxygentypedef:: Color3c

.. doxygentypedef:: Color3h

.. doxygentypedef:: Color3f

.. doxygentypedef:: C3c

.. doxygentypedef:: C3h

.. doxygentypedef:: C3f
                    
.. doxygenclass:: Imath::Color3
   :undoc-members:
   :members:
