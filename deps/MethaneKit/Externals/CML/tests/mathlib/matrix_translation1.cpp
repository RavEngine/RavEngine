/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

// Make sure the main header compiles cleanly:
#include <cml/mathlib/matrix/translation.h>

#include <cml/vector.h>
#include <cml/matrix.h>

/* Testing headers: */
#include "catch_runner.h"


CATCH_TEST_CASE("set 2D, set1")
{
  cml::matrix33d M;
  cml::matrix_set_translation_2D(M.identity(), 2., -3.);
  CATCH_CHECK(M(0,2) == 2.);
  CATCH_CHECK(M(1,2) == -3.);
}

CATCH_TEST_CASE("set 2D, set2")
{
  cml::matrix33d_r M;
  cml::matrix_set_translation_2D(M.identity(), 2., -3.);
  CATCH_CHECK(M(2,0) == 2.);
  CATCH_CHECK(M(2,1) == -3.);
}

CATCH_TEST_CASE("set 2D, set3")
{
  cml::matrix33d M;
  cml::matrix_set_translation_2D(M.identity(), cml::vector2d(2.,-3.));
  CATCH_CHECK(M(0,2) == 2.);
  CATCH_CHECK(M(1,2) == -3.);
}

CATCH_TEST_CASE("set 2D, set4")
{
  cml::matrix33d_r M;
  cml::matrix_set_translation_2D(M.identity(), cml::vector2d(2., -3.));
  CATCH_CHECK(M(2,0) == 2.);
  CATCH_CHECK(M(2,1) == -3.);
}


CATCH_TEST_CASE("set 2D, make1")
{
  cml::matrix33d M;
  cml::matrix_translation_2D(M, 2., -3.);
  CATCH_CHECK(M(0,2) == 2.);
  CATCH_CHECK(M(1,2) == -3.);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(1,1) == 1.);
  CATCH_CHECK(M(2,2) == 1.);
}

CATCH_TEST_CASE("set 2D, make2")
{
  cml::matrix33d_r M;
  cml::matrix_translation_2D(M, 2., -3.);
  CATCH_CHECK(M(2,0) == 2.);
  CATCH_CHECK(M(2,1) == -3.);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(1,1) == 1.);
  CATCH_CHECK(M(2,2) == 1.);
}

CATCH_TEST_CASE("set 2D, make3")
{
  cml::matrix33d M;
  cml::matrix_translation_2D(M, cml::vector2d(2.,-3.));
  CATCH_CHECK(M(0,2) == 2.);
  CATCH_CHECK(M(1,2) == -3.);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(1,1) == 1.);
  CATCH_CHECK(M(2,2) == 1.);
}

CATCH_TEST_CASE("set 2D, make4")
{
  cml::matrix33d_r M;
  cml::matrix_translation_2D(M, cml::vector2d(2., -3.));
  CATCH_CHECK(M(2,0) == 2.);
  CATCH_CHECK(M(2,1) == -3.);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(1,1) == 1.);
  CATCH_CHECK(M(2,2) == 1.);
}




CATCH_TEST_CASE("get 2D, get1")
{
  cml::matrix33d M;
  cml::matrix_set_translation_2D(M.identity(), 2., -3.);

  double e0, e1;
  cml::matrix_get_translation_2D(M, e0, e1);
  CATCH_CHECK(e0 == 2.);
  CATCH_CHECK(e1 == -3.);
}

CATCH_TEST_CASE("get 2D, get2")
{
  cml::matrix33d_r M;
  cml::matrix_set_translation_2D(M.identity(), 2., -3.);

  double e0, e1;
  cml::matrix_get_translation_2D(M, e0, e1);
  CATCH_CHECK(e0 == 2.);
  CATCH_CHECK(e1 == -3.);
}

CATCH_TEST_CASE("get 2D, get3")
{
  cml::matrix33d M;
  cml::matrix_set_translation_2D(M.identity(), 2., -3.);
  auto T = cml::matrix_get_translation_2D(M);
  CATCH_CHECK(T[0] == 2.);
  CATCH_CHECK(T[1] == -3.);
}

CATCH_TEST_CASE("get 2D, get4")
{
  cml::matrix33d_r M;
  cml::matrix_set_translation_2D(M.identity(), 2., -3.);
  auto T = cml::matrix_get_translation_2D(M);
  CATCH_CHECK(T[0] == 2.);
  CATCH_CHECK(T[1] == -3.);
}





CATCH_TEST_CASE("set 3D, set1")
{
  cml::matrix44d M;
  cml::matrix_set_translation(M.identity(), 2., -3., 1.);
  CATCH_CHECK(M(0,3) == 2.);
  CATCH_CHECK(M(1,3) == -3.);
  CATCH_CHECK(M(2,3) == 1.);
}

CATCH_TEST_CASE("set 3D, set2")
{
  cml::matrix44d_r M;
  cml::matrix_set_translation(M.identity(), 2., -3., 1.);
  CATCH_CHECK(M(3,0) == 2.);
  CATCH_CHECK(M(3,1) == -3.);
  CATCH_CHECK(M(3,2) == 1.);
}

CATCH_TEST_CASE("set 3D, set3")
{
  cml::matrix44d M;
  cml::matrix_set_translation(M.identity(), cml::vector3d(2.,-3., 1.));
  CATCH_CHECK(M(0,3) == 2.);
  CATCH_CHECK(M(1,3) == -3.);
  CATCH_CHECK(M(2,3) == 1.);
}

CATCH_TEST_CASE("set 3D, set4")
{
  cml::matrix44d_r M;
  cml::matrix_set_translation(M.identity(), cml::vector3d(2., -3., 1.));
  CATCH_CHECK(M(3,0) == 2.);
  CATCH_CHECK(M(3,1) == -3.);
  CATCH_CHECK(M(3,2) == 1.);
}


CATCH_TEST_CASE("set 3D, set2D_1")
{
  cml::matrix44d M;
  cml::matrix_set_translation(M.identity(), 2., -3.);
  CATCH_CHECK(M(0,3) == 2.);
  CATCH_CHECK(M(1,3) == -3.);
  CATCH_CHECK(M(2,3) == 0.);
}

CATCH_TEST_CASE("set 3D, set2D_2")
{
  cml::matrix44d_r M;
  cml::matrix_set_translation(M.identity(), 2., -3.);
  CATCH_CHECK(M(3,0) == 2.);
  CATCH_CHECK(M(3,1) == -3.);
  CATCH_CHECK(M(3,2) == 0.);
}

CATCH_TEST_CASE("set 3D, set2D_3")
{
  cml::matrix44d M;
  cml::matrix_set_translation(M.identity(), cml::vector2d(2.,-3.));
  CATCH_CHECK(M(0,3) == 2.);
  CATCH_CHECK(M(1,3) == -3.);
  CATCH_CHECK(M(2,3) == 0.);
}

CATCH_TEST_CASE("set 3D, set2D_4")
{
  cml::matrix44d_r M;
  cml::matrix_set_translation(M.identity(), cml::vector2d(2., -3.));
  CATCH_CHECK(M(3,0) == 2.);
  CATCH_CHECK(M(3,1) == -3.);
  CATCH_CHECK(M(3,2) == 0.);
}


CATCH_TEST_CASE("set 3D, make1")
{
  cml::matrix44d M;
  cml::matrix_translation(M, 2., -3., 1.);
  CATCH_CHECK(M(0,3) == 2.);
  CATCH_CHECK(M(1,3) == -3.);
  CATCH_CHECK(M(2,3) == 1.);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(1,1) == 1.);
  CATCH_CHECK(M(2,2) == 1.);
  CATCH_CHECK(M(3,3) == 1.);
}

CATCH_TEST_CASE("set 3D, make2")
{
  cml::matrix44d_r M;
  cml::matrix_translation(M, 2., -3., 1.);
  CATCH_CHECK(M(3,0) == 2.);
  CATCH_CHECK(M(3,1) == -3.);
  CATCH_CHECK(M(3,2) == 1.);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(1,1) == 1.);
  CATCH_CHECK(M(2,2) == 1.);
  CATCH_CHECK(M(3,3) == 1.);
}

CATCH_TEST_CASE("set 3D, make3")
{
  cml::matrix44d M;
  cml::matrix_translation(M, cml::vector3d(2.,-3., 1.));
  CATCH_CHECK(M(0,3) == 2.);
  CATCH_CHECK(M(1,3) == -3.);
  CATCH_CHECK(M(2,3) == 1.);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(1,1) == 1.);
  CATCH_CHECK(M(2,2) == 1.);
  CATCH_CHECK(M(3,3) == 1.);
}

CATCH_TEST_CASE("set 3D, make4")
{
  cml::matrix44d_r M;
  cml::matrix_translation(M, cml::vector3d(2., -3., 1.));
  CATCH_CHECK(M(3,0) == 2.);
  CATCH_CHECK(M(3,1) == -3.);
  CATCH_CHECK(M(3,2) == 1.);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(1,1) == 1.);
  CATCH_CHECK(M(2,2) == 1.);
  CATCH_CHECK(M(3,3) == 1.);
}


CATCH_TEST_CASE("set 3D, make2D_1")
{
  cml::matrix44d M;
  cml::matrix_translation(M, 2., -3.);
  CATCH_CHECK(M(0,3) == 2.);
  CATCH_CHECK(M(1,3) == -3.);
  CATCH_CHECK(M(2,3) == 0.);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(1,1) == 1.);
  CATCH_CHECK(M(2,2) == 1.);
  CATCH_CHECK(M(3,3) == 1.);
}

CATCH_TEST_CASE("set 3D, make2D_2")
{
  cml::matrix44d_r M;
  cml::matrix_translation(M, 2., -3.);
  CATCH_CHECK(M(3,0) == 2.);
  CATCH_CHECK(M(3,1) == -3.);
  CATCH_CHECK(M(3,2) == 0.);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(1,1) == 1.);
  CATCH_CHECK(M(2,2) == 1.);
  CATCH_CHECK(M(3,3) == 1.);
}

CATCH_TEST_CASE("set 3D, make2D_3")
{
  cml::matrix44d M;
  cml::matrix_translation(M, cml::vector2d(2.,-3.));
  CATCH_CHECK(M(0,3) == 2.);
  CATCH_CHECK(M(1,3) == -3.);
  CATCH_CHECK(M(2,3) == 0.);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(1,1) == 1.);
  CATCH_CHECK(M(2,2) == 1.);
  CATCH_CHECK(M(3,3) == 1.);
}

CATCH_TEST_CASE("set 3D, make2D_4")
{
  cml::matrix44d_r M;
  cml::matrix_translation(M, cml::vector2d(2., -3.));
  CATCH_CHECK(M(3,0) == 2.);
  CATCH_CHECK(M(3,1) == -3.);
  CATCH_CHECK(M(3,2) == 0.);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(1,1) == 1.);
  CATCH_CHECK(M(2,2) == 1.);
  CATCH_CHECK(M(3,3) == 1.);
}




CATCH_TEST_CASE("get 3D, get1")
{
  cml::matrix44d M;
  cml::matrix_set_translation(M.identity(), 2., -3., 1.);

  double e0, e1, e2;
  cml::matrix_get_translation(M, e0, e1, e2);
  CATCH_CHECK(e0 == 2.);
  CATCH_CHECK(e1 == -3.);
  CATCH_CHECK(e2 == 1.);
}

CATCH_TEST_CASE("get 3D, get2")
{
  cml::matrix44d_r M;
  cml::matrix_set_translation(M.identity(), 2., -3., 1.);

  double e0, e1, e2;
  cml::matrix_get_translation(M, e0, e1, e2);
  CATCH_CHECK(e0 == 2.);
  CATCH_CHECK(e1 == -3.);
  CATCH_CHECK(e2 == 1.);
}

CATCH_TEST_CASE("get 3D, get3")
{
  cml::matrix44d M;
  cml::matrix_set_translation(M.identity(), 2., -3., 1.);
  auto T = cml::matrix_get_translation(M);
  CATCH_CHECK(T[0] == 2.);
  CATCH_CHECK(T[1] == -3.);
  CATCH_CHECK(T[2] == 1.);
}

CATCH_TEST_CASE("get 3D, get4")
{
  cml::matrix44d_r M;
  cml::matrix_set_translation(M.identity(), 2., -3., 1.);
  auto T = cml::matrix_get_translation(M);
  CATCH_CHECK(T[0] == 2.);
  CATCH_CHECK(T[1] == -3.);
  CATCH_CHECK(T[2] == 1.);
}


// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
