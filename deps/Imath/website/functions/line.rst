..
  SPDX-License-Identifier: BSD-3-Clause
  Copyright Contributors to the OpenEXR Project.

.. _line-functions:

Line3 Functions
###############

.. code-block::

   #include <Imath/ImathLineAlgo.h>   

Functions that operate on the ``Line3`` object.

.. doxygenfunction:: closestPoints
                     
.. doxygenfunction:: intersect(const Line3<T>& line, const Vec3<T>&v0, const Vec3<T>& v1, const Vec3<T>& v2, Vec3<T>& pt, Vec3<T>& barycentric, bool& front) noexcept
                     
.. doxygenfunction:: closestVertex(const Vec3<T>& v0, const Vec3<T>& v1, const Vec3<T>& v2, const Line3<T>& l) noexcept

.. doxygenfunction:: rotatePoint
