/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_matrix_types_h
#define	cml_matrix_types_h

#include <cml/storage/selectors.h>
#include <cml/matrix/matrix.h>

namespace cml {

/** @defgroup matrix_types Predefined Matrix Types */
/*@{*/

// Column-basis, row-major:
typedef matrix<int,    fixed<2,2>>    matrix22i;
typedef matrix<int,    fixed<3,3>>    matrix33i;
typedef matrix<int,    fixed<4,4>>    matrix44i;
typedef matrix<int,    fixed<2,3>>    matrix23i;
typedef matrix<int,    fixed<3,4>>    matrix34i;
typedef matrix<int,    dynamic<>>     matrixi;
typedef matrix<int,    external<2,2>> external22i;
typedef matrix<int,    external<3,3>> external33i;
typedef matrix<int,    external<4,4>> external44i;
typedef matrix<int,    external<2,3>> external23i;
typedef matrix<int,    external<3,4>> external34i;
typedef matrix<int,    external<>>    externalmni;

typedef matrix<float,  fixed<2,2>>    matrix22f;
typedef matrix<float,  fixed<3,3>>    matrix33f;
typedef matrix<float,  fixed<4,4>>    matrix44f;
typedef matrix<float,  fixed<2,3>>    matrix32f;
typedef matrix<float,  fixed<3,4>>    matrix34f;
typedef matrix<float,  dynamic<>>     matrixf;
typedef matrix<float,  external<2,2>> external22f;
typedef matrix<float,  external<3,3>> external33f;
typedef matrix<float,  external<4,4>> external44f;
typedef matrix<float,  external<2,3>> external23f;
typedef matrix<float,  external<3,4>> external34f;
typedef matrix<float,  external<>>    externalmnf;

typedef matrix<double, fixed<2,2>>    matrix22d;
typedef matrix<double, fixed<3,3>>    matrix33d;
typedef matrix<double, fixed<4,4>>    matrix44d;
typedef matrix<double, fixed<2,3>>    matrix23d;
typedef matrix<double, fixed<3,4>>    matrix34d;
typedef matrix<double, dynamic<>>     matrixd;
typedef matrix<double, external<2,2>> external22d;
typedef matrix<double, external<3,3>> external33d;
typedef matrix<double, external<4,4>> external44d;
typedef matrix<double, external<2,3>> external23d;
typedef matrix<double, external<3,4>> external34d;
typedef matrix<double, external<>>    externalmnd;


// Row-basis, row-major:
typedef matrix<int,    fixed<2,2>, row_basis, row_major>    matrix22i_r;
typedef matrix<int,    fixed<3,3>, row_basis, row_major>    matrix33i_r;
typedef matrix<int,    fixed<4,4>, row_basis, row_major>    matrix44i_r;
typedef matrix<int,    fixed<3,2>, row_basis, row_major>    matrix32i_r;
typedef matrix<int,    fixed<4,3>, row_basis, row_major>    matrix43i_r;
typedef matrix<int,    dynamic<>,  row_basis, row_major>    matrixi_r;
typedef matrix<int,    external<2,2>, row_basis, row_major> external22i_r;
typedef matrix<int,    external<3,3>, row_basis, row_major> external33i_r;
typedef matrix<int,    external<4,4>, row_basis, row_major> external44i_r;
typedef matrix<int,    external<3,2>, row_basis, row_major> external32i_r;
typedef matrix<int,    external<4,3>, row_basis, row_major> external43i_r;
typedef matrix<int,    external<>,  row_basis, row_major>   externalmni_r;


typedef matrix<float,  fixed<2,2>, row_basis, row_major>    matrix22f_r;
typedef matrix<float,  fixed<3,3>, row_basis, row_major>    matrix33f_r;
typedef matrix<float,  fixed<4,4>, row_basis, row_major>    matrix44f_r;
typedef matrix<float,  fixed<3,2>, row_basis, row_major>    matrix32f_r;
typedef matrix<float,  fixed<4,3>, row_basis, row_major>    matrix43f_r;
typedef matrix<float,  dynamic<>,  row_basis, row_major>    matrixf_r;
typedef matrix<float,  external<2,2>, row_basis, row_major> external22f_r;
typedef matrix<float,  external<3,3>, row_basis, row_major> external33f_r;
typedef matrix<float,  external<4,4>, row_basis, row_major> external44f_r;
typedef matrix<float,  external<3,2>, row_basis, row_major> external32f_r;
typedef matrix<float,  external<4,3>, row_basis, row_major> external43f_r;
typedef matrix<float,  external<>,  row_basis, row_major>   externalmnf_r;

typedef matrix<double, fixed<2,2>, row_basis, row_major>    matrix22d_r;
typedef matrix<double, fixed<3,3>, row_basis, row_major>    matrix33d_r;
typedef matrix<double, fixed<4,4>, row_basis, row_major>    matrix44d_r;
typedef matrix<double, fixed<3,2>, row_basis, row_major>    matrix32d_r;
typedef matrix<double, fixed<4,3>, row_basis, row_major>    matrix43d_r;
typedef matrix<double, dynamic<>,  row_basis, row_major>    matrixd_r;
typedef matrix<double, external<2,2>, row_basis, row_major> external22d_r;
typedef matrix<double, external<3,3>, row_basis, row_major> external33d_r;
typedef matrix<double, external<4,4>, row_basis, row_major> external44d_r;
typedef matrix<double, external<3,2>, row_basis, row_major> external32d_r;
typedef matrix<double, external<4,3>, row_basis, row_major> external43d_r;
typedef matrix<double, external<>,  row_basis, row_major>   externalmnd_r;


// Column-basis, column-major:
typedef matrix<int,    fixed<2,2>, col_basis, col_major>    matrix22i_c;
typedef matrix<int,    fixed<3,3>, col_basis, col_major>    matrix33i_c;
typedef matrix<int,    fixed<4,4>, col_basis, col_major>    matrix44i_c;
typedef matrix<int,    fixed<2,3>, col_basis, col_major>    matrix23i_c;
typedef matrix<int,    fixed<3,4>, col_basis, col_major>    matrix34i_c;
typedef matrix<int,    dynamic<>,  col_basis, col_major>    matrixi_c;
typedef matrix<int,    external<2,2>, col_basis, col_major> external22i_c;
typedef matrix<int,    external<3,3>, col_basis, col_major> external33i_c;
typedef matrix<int,    external<4,4>, col_basis, col_major> external44i_c;
typedef matrix<int,    external<2,3>, col_basis, col_major> external23i_c;
typedef matrix<int,    external<3,4>, col_basis, col_major> external34i_c;
typedef matrix<int,    external<>,  col_basis, col_major>   externalmni_c;

typedef matrix<float,  fixed<2,2>, col_basis, col_major>    matrix22f_c;
typedef matrix<float,  fixed<3,3>, col_basis, col_major>    matrix33f_c;
typedef matrix<float,  fixed<4,4>, col_basis, col_major>    matrix44f_c;
typedef matrix<float,  fixed<2,3>, col_basis, col_major>    matrix23f_c;
typedef matrix<float,  fixed<3,4>, col_basis, col_major>    matrix34f_c;
typedef matrix<float,  dynamic<>,  col_basis, col_major>    matrixf_c;
typedef matrix<float,  external<2,2>, col_basis, col_major> external22f_c;
typedef matrix<float,  external<3,3>, col_basis, col_major> external33f_c;
typedef matrix<float,  external<4,4>, col_basis, col_major> external44f_c;
typedef matrix<float,  external<2,3>, col_basis, col_major> external23f_c;
typedef matrix<float,  external<3,4>, col_basis, col_major> external34f_c;
typedef matrix<float,  external<>,  col_basis, col_major>   externalmnf_c;

typedef matrix<double, fixed<2,2>, col_basis, col_major>    matrix22d_c;
typedef matrix<double, fixed<3,3>, col_basis, col_major>    matrix33d_c;
typedef matrix<double, fixed<4,4>, col_basis, col_major>    matrix44d_c;
typedef matrix<double, fixed<2,3>, col_basis, col_major>    matrix23d_c;
typedef matrix<double, fixed<3,4>, col_basis, col_major>    matrix34d_c;
typedef matrix<double, dynamic<>,  col_basis, col_major>    matrixd_c;
typedef matrix<double, external<2,2>, col_basis, col_major> external22d_c;
typedef matrix<double, external<3,3>, col_basis, col_major> external33d_c;
typedef matrix<double, external<4,4>, col_basis, col_major> external44d_c;
typedef matrix<double, external<2,3>, col_basis, col_major> external23d_c;
typedef matrix<double, external<3,4>, col_basis, col_major> external34d_c;
typedef matrix<double, external<>,  col_basis, col_major>   externalmnd_c;

/*@}*/

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
