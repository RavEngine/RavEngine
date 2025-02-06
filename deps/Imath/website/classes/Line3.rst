..
  SPDX-License-Identifier: BSD-3-Clause
  Copyright Contributors to the OpenEXR Project.

Line3
#####

.. code-block::

   #include <Imath/ImathLine.h>
   
The ``Line3`` class template represents a line in 3D space, with
predefined typedefs for lines of type ``float`` and ``double``.

There are also various utility functions that operate on ``Line3``
objects defined in ``ImathLineAlgo.h`` and described in :ref:`Line
Functions <line-functions>`.

Example:

.. literalinclude:: ../examples/Line3.cpp
   :language: c++                 

.. doxygentypedef:: Line3f

.. doxygentypedef:: Line3d
   
.. doxygenclass:: Imath::Line3
   :undoc-members:
   :members:

.. doxygenfunction:: operator<<(std::ostream& s, const Line3<T>& line)
      
