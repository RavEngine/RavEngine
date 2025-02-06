..
  SPDX-License-Identifier: BSD-3-Clause
  Copyright Contributors to the OpenEXR Project.

.. _matrix-functions:

Matrix Functions
################

.. code-block::

   #include <Imath/ImathMatrixAlgo.h>   

Functions that operate on matrices

.. doxygenfunction:: extractScaling(const Matrix44<T>& mat, Vec3<T>& scl, bool exc)
                     
.. doxygenfunction:: sansScaling(const Matrix44<T>& mat, bool exc)

.. doxygenfunction:: removeScaling(Matrix44<T>& mat, bool exc)

.. doxygenfunction:: extractScalingAndShear(const Matrix44<T>& mat, Vec3<T>& scl, Vec3<T>& shr, bool exc)

.. doxygenfunction:: sansScalingAndShear(const Matrix44<T>& mat, bool exc)

.. doxygenfunction:: sansScalingAndShear(Matrix44<T>& result, const Matrix44<T>& mat, bool exc)

.. doxygenfunction:: removeScalingAndShear(Matrix44<T>& mat, bool exc)

.. doxygenfunction:: extractAndRemoveScalingAndShear(Matrix44<T>& mat, Vec3<T>& scl, Vec3<T>& shr, bool exc)

.. doxygenfunction:: extractEulerXYZ(const Matrix44<T>& mat, Vec3<T>& rot)

.. doxygenfunction:: extractEulerZYX(const Matrix44<T>& mat, Vec3<T>& rot)

.. doxygenfunction:: extractQuat(const Matrix44<T>& mat)

.. doxygenfunction:: extractSHRT(const Matrix44<T>& mat, Vec3<T>& s, Vec3<T>& h, Vec3<T>& r, Vec3<T>& t, bool exc, typename Euler<T>::Order rOrder)

.. doxygenfunction:: extractSHRT(const Matrix44<T>& mat, Vec3<T>& s, Vec3<T>& h, Vec3<T>& r, Vec3<T>& t, bool exc)

.. doxygenfunction:: extractSHRT(const Matrix44<T>& mat, Vec3<T>& s, Vec3<T>& h, Euler<T>& r, Vec3<T>& t, bool exc)

.. doxygenfunction:: checkForZeroScaleInRow(const T& scl, const Vec3<T>& row, bool exc)

.. doxygenfunction:: outerProduct(const Vec4<T>& a, const Vec4<T>& b)

.. doxygenfunction:: rotationMatrix(const Vec3<T>& fromDirection, const Vec3<T>& toDirection)                     

.. doxygenfunction:: rotationMatrixWithUpDir(const Vec3<T>& fromDir, const Vec3<T>& toDir, const Vec3<T>& upDir)

.. doxygenfunction:: alignZAxisWithTargetDir(Matrix44<T>& result, Vec3<T> targetDir, Vec3<T> upDir)

.. doxygenfunction:: computeLocalFrame(const Vec3<T>& p, const Vec3<T>& xDir, const Vec3<T>& normal)

.. doxygenfunction:: addOffset(const Matrix44<T>& inMat, const Vec3<T>& tOffset, const Vec3<T>& rOffset, const Vec3<T>& sOffset, const Vec3<T>& ref)

.. doxygenfunction:: computeRSMatrix(bool keepRotateA, bool keepScaleA, const Matrix44<T>& A, const Matrix44<T>& B)

.. doxygenfunction:: extractScaling(const Matrix33<T>& mat, Vec2<T>& scl, bool exc)

.. doxygenfunction:: sansScaling(const Matrix33<T>& mat, bool exc)

.. doxygenfunction:: removeScaling(Matrix33<T>& mat, bool exc)

.. doxygenfunction:: extractScalingAndShear(const Matrix33<T>& mat, Vec2<T>& scl, T& shr, bool exc)

.. doxygenfunction:: sansScalingAndShear(const Matrix33<T>& mat, bool exc)

.. doxygenfunction:: removeScalingAndShear(Matrix33<T>& mat, bool exc)

.. doxygenfunction:: extractAndRemoveScalingAndShear(Matrix33<T>& mat, Vec2<T>& scl, T& shr, bool exc)

.. doxygenfunction:: extractEuler(const Matrix22<T>& mat, T& rot)

.. doxygenfunction:: extractEuler(const Matrix33<T>& mat, T& rot)

.. doxygenfunction:: extractSHRT(const Matrix33<T>& mat, Vec2<T>& s, T& h, T& r, Vec2<T>& t, bool exc)

.. doxygenfunction:: checkForZeroScaleInRow(const T& scl, const Vec2<T>& row, bool exc)

.. doxygenfunction:: outerProduct(const Vec3<T>& a, const Vec3<T>& b)

.. doxygenfunction:: procrustesRotationAndTranslation(const Vec3<T>* A, const Vec3<T>* B, const T* weights, const size_t numPoints, const bool doScaling)

.. doxygenfunction:: procrustesRotationAndTranslation(const Vec3<T>* A, const Vec3<T>* B, const size_t numPoints, const bool doScaling)

.. doxygenfunction:: jacobiSVD(const Matrix33<T>& A, Matrix33<T>& U, Vec3<T>& S, Matrix33<T>& V, const T tol, const bool forcePositiveDeterminant)

.. doxygenfunction:: jacobiSVD(const Matrix44<T>& A, Matrix44<T>& U, Vec4<T>& S, Matrix44<T>& V, const T tol, const bool forcePositiveDeterminant)

.. doxygenfunction:: jacobiEigenSolver(Matrix33<T>& A, Vec3<T>& S, Matrix33<T>& V, const T tol)

.. doxygenfunction:: jacobiEigenSolver(Matrix33<T>& A, Vec3<T>& S, Matrix33<T>& V)

.. doxygenfunction:: jacobiEigenSolver(Matrix44<T>& A, Vec4<T>& S, Matrix44<T>& V, const T tol)

.. doxygenfunction:: jacobiEigenSolver(Matrix44<T>& A, Vec4<T>& S, Matrix44<T>& V)

.. doxygenfunction:: maxEigenVector(TM& A, TV& S)

.. doxygenfunction:: minEigenVector(TM& A, TV& S)


