..
  SPDX-License-Identifier: BSD-3-Clause
  Copyright Contributors to the OpenEXR Project.

Shear6
######

.. code-block::

   #include <Imath/ImathShear.h>   

The ``Shear6`` class template represent a 3D shear transformation,
with predefined typedefs for ``float`` and ``double``.

Example:

.. literalinclude:: ../examples/Shear6.cpp
   :language: c++
              
.. doxygentypedef:: Shear6f

.. doxygenclass:: Imath::Shear6
   :undoc-members:
   :members:

.. doxygenfunction:: operator<<(std::ostream& s, const Shear6<T>& h)
      
