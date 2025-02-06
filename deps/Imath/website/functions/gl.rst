..
  SPDX-License-Identifier: BSD-3-Clause
  Copyright Contributors to the OpenEXR Project.

.. _gl-functions:

GL Functions
############

Functions that wrap OpenGL calls to accept Imath vectors and matrices.

.. code-block::

   #include <Imath/ImathGL.h>

Example:

.. literalinclude:: ../examples/gl.cpp
   :language: c++
              
.. doxygenfunction:: glVertex(const Imath::V2f &v)

.. doxygenfunction:: glVertex(const Imath::V3f &v)
                     
.. doxygenfunction:: glNormal(const Imath::V3f &v)
                     
.. doxygenfunction:: glColor(const Imath::V3f &v)
                     
.. doxygenfunction:: glTranslate                     

.. doxygenfunction:: glTexCoord

.. doxygenfunction:: throwBadMatrix
                     
.. doxygenfunction:: glMultMatrix( const Imath::M44f &m )
                     
.. doxygenfunction:: glMultMatrix( const Imath::M44f *m )
                     
.. doxygenfunction:: glLoadMatrix(const Imath::M44f &m)

.. doxygenfunction:: glLoadMatrix(const Imath::M44f *m)

.. doxygenclass:: Imath::GLPushMatrix
   :members:
   :undoc-members:
                     
.. doxygenclass:: Imath::GLPushAttrib
   :members:
   :undoc-members:
                     
.. doxygenclass:: Imath::GLBegin
   :members:
   :undoc-members:
