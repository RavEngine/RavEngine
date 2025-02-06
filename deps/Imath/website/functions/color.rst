..
  SPDX-License-Identifier: BSD-3-Clause
  Copyright Contributors to the OpenEXR Project.

.. _color-functions:

Color Functions
###############

Functions that operate on colors.

.. code-block::

   #include <Imath/ImathColorAlgo.h>

.. doxygenfunction:: hsv2rgb(const Vec3<T>& hsv) noexcept

.. doxygenfunction:: hsv2rgb(const Color4<T>& hsv) noexcept
                     
.. doxygenfunction:: rgb2hsv(const Color4<T> &rgb) noexcept
                     
.. doxygenfunction:: rgb2hsv(const Vec3<T> &rgb) noexcept

.. doxygenfunction:: rgb2packed(const Color4<T> &c) noexcept

.. doxygenfunction:: rgb2packed(const Vec3<T> &c) noexcept

.. doxygenfunction:: packed2rgb(PackedColor packed, Color4<T> &out) noexcept

.. doxygenfunction:: packed2rgb(PackedColor packed, Vec3<T> &out) noexcept

                     
