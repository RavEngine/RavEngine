..
  SPDX-License-Identifier: BSD-3-Clause
  Copyright Contributors to the OpenEXR Project.

Matrix22
########

.. code-block::

   #include <Imath/ImathMatrix.h>
   
The ``Matrix22`` class template represents a 2x2 matrix, with
predefined typedefs for ``float`` and ``double``.

There are also various utility functions that operate on matrices
defined in ``ImathMatrixAlgo.h`` and described in :ref:`Matrix
Functions <matrix-functions>`.

Individual components of a matrix ``M`` may be referenced as either
``M[j][i]`` or ``M.x[j][i]``. While the latter is a little awkward, it has an
advantage when used in loops that may be auto-vectorized or explicitly
vectorized by ``#pragma omp simd`` or other such hints, because the function
call and pointer casting of ``operator[]`` can confuse the compiler just
enough to prevent vectorization of the loop, whereas directly addressing the
real underlying array (``M.x[j][i]``) does not.

Example:

.. literalinclude:: ../examples/Matrix22.cpp
   :language: c++                 

.. doxygentypedef:: M22f

.. doxygentypedef:: M22d

.. doxygenclass:: Imath::Matrix22
   :undoc-members:
   :members:

.. doxygenfunction:: operator<<(std::ostream& s, const Matrix22<T>& m)

      
