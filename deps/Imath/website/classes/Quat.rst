..
  SPDX-License-Identifier: BSD-3-Clause
  Copyright Contributors to the OpenEXR Project.

Quat
####

.. code-block::

   #include <Imath/ImathQuat.h>
   
The ``Quat`` class template represents a quaterion
rotation/orientation, with predefined typedefs for ``float`` and
``double``.

Example:

.. literalinclude:: ../examples/Quat.cpp
   :language: c++
              
.. doxygentypedef:: Quatf

.. doxygenclass:: Imath::Quat
   :undoc-members:
   :members:

.. doxygenfunction:: operator<<(std::ostream& s, const Quat<T>& q)
      
