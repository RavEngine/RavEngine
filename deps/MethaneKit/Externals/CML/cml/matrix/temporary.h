/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_matrix_temporary_h
#define	cml_matrix_temporary_h

#include <cml/common/temporary.h>
#include <cml/storage/resize.h>
#include <cml/storage/promotion.h>
#include <cml/matrix/traits.h>
#include <cml/matrix/matrix.h>

namespace cml {

/** Deduce a temporary for a matrix expression. */
template<class Matrix>
struct temporary_of< Matrix, cml::enable_if_matrix_t<Matrix> >
{
  typedef cml::unqualified_type_t<Matrix>		matrix_type;

  /* Propagate the element type of the original matrix: */
  typedef matrix_traits<matrix_type>			traits_type;
  typedef typename traits_type::value_type		value_type;
  typedef typename traits_type::storage_type		storage_type;
  typedef typename traits_type::basis_tag		basis_tag;
  typedef typename traits_type::layout_tag		layout_tag;

  /* Need the proxy for the storage type: */
  typedef typename storage_type::proxy_type		proxy_type;

  /* Build the temporary: */
  typedef matrix<value_type,
	  proxy_type, basis_tag, layout_tag>		type;
};

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
