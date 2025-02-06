..
  SPDX-License-Identifier: BSD-3-Clause
  Copyright Contributors to the OpenEXR Project.

Box
###

.. code-block::

   #include <Imath/ImathBox.h>
   
The ``Box`` class template represents 2D and 3D axis-aligned bounding
boxes, with predefined typedefs for boxes of type ``short``, ``int``,
``int64_t``, ``float``, and ``double``.

The box is defined by minimum and maximum values along each axis,
represented by ``Vec2<T>`` for the ``Box2`` types and by ``Vec3<T>``
for ``Box3`` types.

There are also various utility functions that operate on bounding
boxes defined in ``ImathBoxAlgo.h`` and described in :ref:`Box
Functions <box-functions>`.

Example:

.. literalinclude:: ../examples/Box.cpp
   :language: c++

.. doxygentypedef:: Box2s

.. doxygentypedef:: Box2i

.. doxygentypedef:: Box2i64

.. doxygentypedef:: Box2f

.. doxygentypedef:: Box2d
      
.. doxygentypedef:: Box3s

.. doxygentypedef:: Box3i

.. doxygentypedef:: Box3i64

.. doxygentypedef:: Box3f

.. doxygentypedef:: Box3d

.. doxygenclass:: Imath::Box
   :undoc-members:
   :members:

.. doxygenclass:: Imath::Box< Vec2< T > >
   :undoc-members:
   :members:
      
.. doxygenclass:: Imath::Box< Vec3< T > >
   :undoc-members:
   :members:
