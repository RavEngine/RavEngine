/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_vector_types_h
#define	cml_vector_types_h

#include <cml/storage/selectors.h>
#include <cml/vector/vector.h>

namespace cml {

/** @defgroup vector_types Predefined Vector Types */
/*@{*/

typedef vector<int,          fixed<2>>     vector2i;
typedef vector<int,          fixed<3>>     vector3i;
typedef vector<int,          fixed<4>>     vector4i;
typedef vector<int,          dynamic<>>    vectori;
typedef vector<int,          external<2>>  external2i;
typedef vector<int,          external<3>>  external3i;
typedef vector<int,          external<4>>  external4i;
typedef vector<int,          external<>>   externalni;
typedef vector<const int,    external<2>>  external2ci;
typedef vector<const int,    external<3>>  external3ci;
typedef vector<const int,    external<4>>  external4ci;
typedef vector<const int,    external<>>   externalnci;

typedef vector<float,        fixed<2>>     vector2f;
typedef vector<float,        fixed<3>>     vector3f;
typedef vector<float,        fixed<4>>     vector4f;
typedef vector<float,        dynamic<>>    vectorf;
typedef vector<float,        external<2>>  external2f;
typedef vector<float,        external<3>>  external3f;
typedef vector<float,        external<4>>  external4f;
typedef vector<float,        external<>>   externalnf;
typedef vector<const float,  external<2>>  external2cf;
typedef vector<const float,  external<3>>  external3cf;
typedef vector<const float,  external<4>>  external4cf;
typedef vector<const float,  external<>>   externalncf;

typedef vector<double,       fixed<2>>     vector2d;
typedef vector<double,       fixed<3>>     vector3d;
typedef vector<double,       fixed<4>>     vector4d;
typedef vector<double,       dynamic<>>    vectord;
typedef vector<double,       external<2>>  external2d;
typedef vector<double,       external<3>>  external3d;
typedef vector<double,       external<4>>  external4d;
typedef vector<double,       external<>>   externalnd;
typedef vector<const double, external<2>>  external2cd;
typedef vector<const double, external<3>>  external3cd;
typedef vector<const double, external<4>>  external4cd;
typedef vector<const double, external<>>   externalncd;

/*@}*/

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
