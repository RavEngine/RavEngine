..
  SPDX-License-Identifier: BSD-3-Clause
  Copyright Contributors to the OpenEXR Project.

.. _Concepts:

Concepts
########

.. toctree::
   :caption: Concepts
   :maxdepth: 0
              
The Imath library emphasizes simplicity, ease of use, correctness and
verifiability, performance, and breadth of adoption. Imath is not
intended to be a comprehensive linear algebra or numerical analysis
package.

Imath is not a substitute for `Eigen <https://eigen.tuxfamily.org>`_!
It's not a full-featured linear algebra package, and it doesn't
represent vectors and matrices of arbitrary dimension. Its greatest
utility is as a geometric data representation, primarily for 2D images
and 3D scenes and coordinate transformations, along with an
accompanying set of utility methods and functions.

Example
=======

A basic program:

.. literalinclude:: examples/intro.cpp

Matrices Are Row-Major
======================

Imath stores matrices in row-major layout, originally inspired by
compatibility with OpenGL matrices. 

A matrix described as:

.. math::
  \begin{bmatrix}
  m_{00} & m_{01} & m_{02} & m_{03} \\
  m_{10} & m_{11} & m_{12} & m_{13} \\
  m_{20} & m_{21} & m_{22} & m_{23} \\
  m_{30} & m_{31} & m_{32} & m_{33} \\
  \end{bmatrix}

is laid out in memory as:

.. list-table::
   :widths: 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10
   :header-rows: 1

   * - 0
     - 1
     - 2
     - 3
     - 4
     - 5
     - 6
     - 7
     - 8
     - 9
     - 10
     - 11
     - 12
     - 13
     - 14
     - 15
   * - :math:`m_{00}`
     - :math:`m_{01}`
     - :math:`m_{02}`
     - :math:`m_{03}`
     - :math:`m_{10}`
     - :math:`m_{11}`
     - :math:`m_{12}`
     - :math:`m_{13}`
     - :math:`m_{20}`
     - :math:`m_{21}`
     - :math:`m_{22}`
     - :math:`m_{23}`
     - :math:`m_{30}`
     - :math:`m_{31}`
     - :math:`m_{32}`
     - :math:`m_{33}`

A matrix representing a homogeneous transform has a right-hand column
of :math:`\begin{bmatrix} 0 & 0 & 0 & 1\end{bmatrix}` and the
translation component across the bottom row.

As a result, it is best to think of Imath vectors as row-vectors, and
vector-matrix multiplication with the vector on the left and matrix on
the right:

.. math::
  \begin{bmatrix} v_{0}' & v_{1}' & v_{2}' & 1' \end{bmatrix}
  =
  \begin{bmatrix} v_{0} & v_{1} & v_{2} & 1 \end{bmatrix}
  \begin{bmatrix} m_{00} & m_{01} & m_{02} & 0 \\
  m_{10} & m_{11} & m_{12} & 0 \\
  m_{20} & m_{21} & m_{22} & 0 \\
  m_{30} & m_{31} & m_{32} & 1
  \end{bmatrix}

This further implies that you should interpret local transformations
as pre-multiplication:

.. code-block::

   M44f M;
   M.translate (tx, ty, tz);
   m.rotate (r, 0, 0);
   m.scale (s);

.. math::
  \begin{bmatrix}
  m_{00} & m_{01} & m_{02} & m_{03} \\
  m_{10} & m_{11} & m_{12} & m_{13} \\
  m_{20} & m_{21} & m_{22} & m_{23} \\
  m_{30} & m_{31} & m_{32} & m_{33} \\
  \end{bmatrix}
  = 
  \begin{bmatrix}
  s & 0 & 0 & 0 \\
  0 & s & 0 & 0 \\
  0 & 0 & s & 0 \\
  0 & 0 & 0 & 1 \\
  \end{bmatrix}
  \begin{bmatrix}
  1 & 0 & 0 & 0 \\
  0 & \cos(r) & \sin(r) & 0 \\
  0 & -\sin(r) & \cos(r) & 0 \\
  0 & 0 & 0 & 1 \\
  \end{bmatrix}
  \begin{bmatrix}
  1 & 0 & 0 & 0 \\
  0 & 1 & 0 & 0 \\
  0 & 0 & 1 & 0 \\
  tx & ty & tz & 1 \\
  \end{bmatrix}


