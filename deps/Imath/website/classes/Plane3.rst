..
  SPDX-License-Identifier: BSD-3-Clause
  Copyright Contributors to the OpenEXR Project.

Plane3
######

.. code-block::

   #include <Imath/ImathPlane.h>
   
The ``Plane3`` class template represents a plane in 3D space, with
predefined typedefs for planes of type ``float`` and ``double``.

Example:

.. literalinclude:: ../examples/Plane3.cpp
   :language: c++
               
.. doxygentypedef:: Plane3f

.. doxygentypedef:: Plane3d

.. doxygenclass:: Imath::Plane3
   :undoc-members:
   :members:

.. doxygenfunction:: operator<<(std::ostream& s, const Plane3<T>& plane)
      
