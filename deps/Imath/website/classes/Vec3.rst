..
  SPDX-License-Identifier: BSD-3-Clause
  Copyright Contributors to the OpenEXR Project.

Vec3
####

.. code-block::

   #include <Imath/ImathVec.h>
   
The ``Vec3`` class template represents a 3D vector, with predefined
typedefs for vectors of type ``short``, ``int``, ``int64_t``,
``float``, and ``double``.

Note that the integer specializations of ``Vec3`` lack the
``length()`` and ``normalize()`` methods that are present in the
``float`` and ``double`` versions, because the results don't fit into
integer quantities.

There are also various utility functions that operate on vectors
defined in ``ImathVecAlgo.h`` and described in :ref:`Vector Functions
<vector-functions>`.

Individual components of a vector ``V`` may be referenced as either ``V[i]``
or ``V.x``, ``V.y``, ``V.z``. Obviously, the ``[]`` notation is more
suited to looping over components, or in cases where a variable determines
which coordinate is needed. However, when the coordinate is known, it can be
more efficient to directly address the components, such as ``V.y`` rather than
``V[1]``. While both appear to do the same thing (and indeed do generate the
same machine operations for ordinary scalar code), when used inside loops that
you hope to parallelize (either through compiler auto-vectorization or
explicit hints such as ``#pragma omp simd``), the function call and
pointer casting of ``operator[]`` can confuse the compiler just enough to
prevent vectorization of the loop.

Example:

.. literalinclude:: ../examples/Vec3.cpp
   :language: c++
              
.. doxygentypedef:: V3s

.. doxygentypedef:: V3i
                    
.. doxygentypedef:: V3i64
                    
.. doxygentypedef:: V3f
                    
.. doxygentypedef:: V3d
                    
.. doxygenclass:: Imath::Vec3
   :undoc-members:
   :members:

.. doxygenfunction:: operator<<(std::ostream& s, const Vec3<T>& v)

      
