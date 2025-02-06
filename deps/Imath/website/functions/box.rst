..
  SPDX-License-Identifier: BSD-3-Clause
  Copyright Contributors to the OpenEXR Project.

.. _box-functions:

Box Functions
#############

Functions that operate on bounding boxes.

.. code-block::

   #include <Imath/ImathBoxAlgo.h>

.. doxygenfunction:: clip
                     
.. doxygenfunction:: closestPointInBox
                     
.. doxygenfunction:: transform(const Box<Vec3<S>>& box, const Matrix44<T>& m) noexcept

.. doxygenfunction:: affineTransform(const Box<Vec3<S>>& box, const Matrix44<T>& m) noexcept

.. doxygenfunction:: findEntryAndExitPoints

.. doxygenfunction:: intersects(const Box<Vec3<T>>& b, const Line3<T>& r, Vec3<T>& ip) noexcept
