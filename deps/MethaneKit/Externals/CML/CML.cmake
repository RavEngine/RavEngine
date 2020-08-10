# -*- cmake -*- -----------------------------------------------------------
# @@COPYRIGHT@@
#*-------------------------------------------------------------------------

if(${CMAKE_GENERATOR} MATCHES "Visual Studio")
  set(_header_group "Header Files\\")
else()
  set(_header_group)
endif()

set(main_HEADERS
  cml/cml.h
  cml/matrix.h
  cml/quaternion.h
  cml/types.h
  cml/util.h
  cml/vector.h
  cml/version.h
  )
source_group("${_header_group}" FILES ${main_HEADERS})

set(common_HEADERS
  cml/common/array_size_of.h
  cml/common/basis_tags.h
  cml/common/compiler.h
  cml/common/exception.h
  cml/common/layout_tags.h
  cml/common/memory_tags.h
  cml/common/promotion.h
  cml/common/size_tags.h
  cml/common/storage_tags.h
  cml/common/temporary.h
  cml/common/traits.h
  cml/common/type_util.h
  )
source_group("${_header_group}common" FILES ${common_HEADERS})

set(common_mpl_HEADERS
  cml/common/mpl/are_convertible.h
  cml/common/mpl/are_same.h
  cml/common/mpl/enable_if_arithmetic.h
  cml/common/mpl/enable_if_array.h
  cml/common/mpl/enable_if_convertible.h
  cml/common/mpl/enable_if_pointer.h
  cml/common/mpl/enable_if_reshapeable.h
  cml/common/mpl/enable_if_same.h
  cml/common/mpl/enable_if_t.h
  cml/common/mpl/if_t.h
  cml/common/mpl/int_c.h
  cml/common/mpl/is_reshapeable.h
  cml/common/mpl/is_same_pair.h
  cml/common/mpl/is_statically_polymorphic.h
  cml/common/mpl/item_at.h
  cml/common/mpl/plus_c.h
  cml/common/mpl/rebind.h
  cml/common/mpl/type_map.h
  cml/common/mpl/type_table.h
  )
source_group("${_header_group}common\\mpl" FILES ${common_mpl_HEADERS})

set(scalar_HEADERS
  cml/scalar/binary_ops.h
  cml/scalar/constants.h
  cml/scalar/functions.h
  cml/scalar/promotion.h
  cml/scalar/traits.h
  cml/scalar/unary_ops.h
  )
source_group("${_header_group}scalar" FILES ${scalar_HEADERS})

set(storage_HEADERS
  cml/storage/allocated_selector.h
  cml/storage/any_selector.h
  cml/storage/compiled_selector.h
  cml/storage/external_selector.h
  cml/storage/promotion.h
  cml/storage/resize.h
  cml/storage/selectors.h
  cml/storage/type_util.h
  )
source_group("${_header_group}storage" FILES ${storage_HEADERS})

set(vector_HEADERS
  cml/vector/binary_node.h
  cml/vector/binary_node.tpp
  cml/vector/binary_ops.h
  cml/vector/comparison.h
  cml/vector/comparison.tpp
  cml/vector/cross.h
  cml/vector/cross_node.h
  cml/vector/cross_node.tpp
  cml/vector/cross_ops.h
  cml/vector/dot.h
  cml/vector/dot.tpp
  cml/vector/dynamic.h
  cml/vector/dynamic_allocated.h
  cml/vector/dynamic_allocated.tpp
  cml/vector/dynamic_const_external.h
  cml/vector/dynamic_const_external.tpp
  cml/vector/dynamic_external.h
  cml/vector/dynamic_external.tpp
  cml/vector/external.h
  cml/vector/fixed.h
  cml/vector/fixed_compiled.h
  cml/vector/fixed_compiled.tpp
  cml/vector/fixed_const_external.h
  cml/vector/fixed_const_external.tpp
  cml/vector/fixed_external.h
  cml/vector/fixed_external.tpp
  cml/vector/functions.h
  cml/vector/functions.tpp
  cml/vector/fwd.h
  cml/vector/hadamard_product.h
  cml/vector/ops.h
  cml/vector/outer_product.h
  cml/vector/outer_product_node.h
  cml/vector/outer_product_node.tpp
  cml/vector/outer_product_ops.h
  cml/vector/perp_dot.h
  cml/vector/perp_dot.tpp
  cml/vector/products.h
  cml/vector/promotion.h
  cml/vector/readable_vector.h
  cml/vector/readable_vector.tpp
  cml/vector/scalar_node.h
  cml/vector/scalar_node.tpp
  cml/vector/scalar_ops.h
  cml/vector/size_checking.h
  cml/vector/size_checking.tpp
  cml/vector/subvector.h
  cml/vector/subvector_node.h
  cml/vector/subvector_node.tpp
  cml/vector/subvector_ops.h
  cml/vector/subvector_ops.tpp
  cml/vector/temporary.h
  cml/vector/traits.h
  cml/vector/triple_product.h
  cml/vector/triple_product.tpp
  cml/vector/types.h
  cml/vector/type_util.h
  cml/vector/unary_node.h
  cml/vector/unary_node.tpp
  cml/vector/unary_ops.h
  cml/vector/vector.h
  cml/vector/writable_vector.h
  cml/vector/writable_vector.tpp
  )
source_group("${_header_group}vector" FILES ${vector_HEADERS})

set(vector_detail_HEADERS
  cml/vector/detail/check_or_resize.h
  cml/vector/detail/combined_size_of.h
  cml/vector/detail/resize.h
  )
source_group("${_header_group}vector\\detail" FILES ${vector_detail_HEADERS})

set(matrix_HEADERS
  cml/matrix/array_size_of.h
  cml/matrix/basis.h
  cml/matrix/basis_node.h
  cml/matrix/basis_node.tpp
  cml/matrix/basis_ops.h
  cml/matrix/binary_node.h
  cml/matrix/binary_node.tpp
  cml/matrix/binary_ops.h
  cml/matrix/col_node.h
  cml/matrix/col_node.tpp
  cml/matrix/col_ops.h
  cml/matrix/comparison.h
  cml/matrix/comparison.tpp
  cml/matrix/determinant.h
  cml/matrix/determinant.tpp
  cml/matrix/dynamic.h
  cml/matrix/dynamic_allocated.h
  cml/matrix/dynamic_allocated.tpp
  cml/matrix/dynamic_external.h
  cml/matrix/dynamic_external.tpp
  cml/matrix/external.h
  cml/matrix/fixed.h
  cml/matrix/fixed_compiled.h
  cml/matrix/fixed_compiled.tpp
  cml/matrix/fixed_external.h
  cml/matrix/fixed_external.tpp
  cml/matrix/functions.h
  cml/matrix/fwd.h
  cml/matrix/hadamard_product.h
  cml/matrix/inverse.h
  cml/matrix/lu.h
  cml/matrix/lu.tpp
  cml/matrix/matrix.h
  cml/matrix/matrix_product.h
  cml/matrix/matrix_product.tpp
  cml/matrix/ops.h
  cml/matrix/promotion.h
  cml/matrix/readable_matrix.h
  cml/matrix/readable_matrix.tpp
  cml/matrix/row_col.h
  cml/matrix/row_node.h
  cml/matrix/row_node.tpp
  cml/matrix/row_ops.h
  cml/matrix/scalar_node.h
  cml/matrix/scalar_node.tpp
  cml/matrix/scalar_ops.h
  cml/matrix/size_checking.h
  cml/matrix/size_checking.tpp
  cml/matrix/temporary.h
  cml/matrix/trace.h
  cml/matrix/trace.tpp
  cml/matrix/traits.h
  cml/matrix/transpose.h
  cml/matrix/transpose_node.h
  cml/matrix/transpose_node.tpp
  cml/matrix/transpose_ops.h
  cml/matrix/types.h
  cml/matrix/type_util.h
  cml/matrix/unary_node.h
  cml/matrix/unary_node.tpp
  cml/matrix/unary_ops.h
  cml/matrix/vector_product.h
  cml/matrix/vector_product.tpp
  cml/matrix/writable_matrix.h
  cml/matrix/writable_matrix.tpp
  )
source_group("${_header_group}matrix" FILES ${matrix_HEADERS})

set(matrix_detail_HEADERS
  cml/matrix/detail/apply.h
  cml/matrix/detail/check_or_resize.h
  cml/matrix/detail/copy.h
  cml/matrix/detail/determinant.h
  cml/matrix/detail/determinant.tpp
  cml/matrix/detail/generate.h
  cml/matrix/detail/get.h
  cml/matrix/detail/inverse.h
  cml/matrix/detail/lu.h
  cml/matrix/detail/lu.tpp
  cml/matrix/detail/resize.h
  cml/matrix/detail/transpose.h
  )
source_group("${_header_group}matrix\\detail" FILES ${matrix_detail_HEADERS})

set(quaternion_HEADERS
  cml/quaternion/binary_node.h
  cml/quaternion/binary_node.tpp
  cml/quaternion/binary_ops.h
  cml/quaternion/comparison.h
  cml/quaternion/comparison.tpp
  cml/quaternion/conjugate.h
  cml/quaternion/conjugate_node.h
  cml/quaternion/conjugate_node.tpp
  cml/quaternion/conjugate_ops.h
  cml/quaternion/conjugate_ops.tpp
  cml/quaternion/cross_tags.h
  cml/quaternion/dot.h
  cml/quaternion/dot.tpp
  cml/quaternion/fixed.h
  cml/quaternion/fixed_compiled.h
  cml/quaternion/fixed_compiled.tpp
  cml/quaternion/functions.h
  cml/quaternion/functions.tpp
  cml/quaternion/fwd.h
  cml/quaternion/imaginary.h
  cml/quaternion/imaginary_node.h
  cml/quaternion/imaginary_node.tpp
  cml/quaternion/imaginary_ops.h
  cml/quaternion/imaginary_ops.tpp
  cml/quaternion/inverse.h
  cml/quaternion/inverse_node.h
  cml/quaternion/inverse_node.tpp
  cml/quaternion/inverse_ops.h
  cml/quaternion/inverse_ops.tpp
  cml/quaternion/ops.h
  cml/quaternion/order_tags.h
  cml/quaternion/product.h
  cml/quaternion/product.tpp
  cml/quaternion/promotion.h
  cml/quaternion/quaternion.h
  cml/quaternion/readable_quaternion.h
  cml/quaternion/readable_quaternion.tpp
  cml/quaternion/scalar_node.h
  cml/quaternion/scalar_node.tpp
  cml/quaternion/scalar_ops.h
  cml/quaternion/size_checking.h
  cml/quaternion/temporary.h
  cml/quaternion/traits.h
  cml/quaternion/types.h
  cml/quaternion/type_util.h
  cml/quaternion/unary_node.h
  cml/quaternion/unary_node.tpp
  cml/quaternion/unary_ops.h
  cml/quaternion/writable_quaternion.h
  cml/quaternion/writable_quaternion.tpp
  )
source_group("${_header_group}quaternion" FILES ${quaternion_HEADERS})

set(mathlib_HEADERS
  cml/mathlib/axis_order.h
  cml/mathlib/constants.h
  cml/mathlib/coordinate_conversion.h
  cml/mathlib/coordinate_conversion.tpp
  cml/mathlib/euler_order.h
  cml/mathlib/frustum.h
  cml/mathlib/frustum.tpp
  cml/mathlib/mathlib.h
  cml/mathlib/random_unit.h
  cml/mathlib/random_unit.tpp
  )
source_group("${_header_group}mathlib" FILES ${mathlib_HEADERS})

set(mathlib_vector_HEADERS
  cml/mathlib/vector/angle.h
  cml/mathlib/vector/angle.tpp
  cml/mathlib/vector/generators.h
  cml/mathlib/vector/misc.h
  cml/mathlib/vector/misc.tpp
  cml/mathlib/vector/orthonormal.h
  cml/mathlib/vector/orthonormal.tpp
  cml/mathlib/vector/products.h
  cml/mathlib/vector/rotation.h
  cml/mathlib/vector/rotation.tpp
  cml/mathlib/vector/transform.h
  cml/mathlib/vector/transform.tpp
  )
source_group("${_header_group}mathlib\\vector" FILES ${mathlib_vector_HEADERS})

set(mathlib_matrix_HEADERS
  cml/mathlib/matrix/basis.h
  cml/mathlib/matrix/basis.tpp
  cml/mathlib/matrix/concat.h
  cml/mathlib/matrix/concat.tpp
  cml/mathlib/matrix/generators.h
  cml/mathlib/matrix/invert.h
  cml/mathlib/matrix/invert.tpp
  cml/mathlib/matrix/misc.h
  cml/mathlib/matrix/misc.tpp
  cml/mathlib/matrix/projection.h
  cml/mathlib/matrix/projection.tpp
  cml/mathlib/matrix/rotation.h
  cml/mathlib/matrix/rotation.tpp
  cml/mathlib/matrix/scale.h
  cml/mathlib/matrix/scale.tpp
  cml/mathlib/matrix/size_checking.h
  cml/mathlib/matrix/size_checking.tpp
  cml/mathlib/matrix/temporary.h
  cml/mathlib/matrix/transform.h
  cml/mathlib/matrix/transform.tpp
  cml/mathlib/matrix/translation.h
  cml/mathlib/matrix/translation.tpp
  )
source_group("${_header_group}mathlib\\matrix" FILES ${mathlib_matrix_HEADERS})

set(mathlib_quaternion_HEADERS
  cml/mathlib/quaternion/basis.h
  cml/mathlib/quaternion/basis.tpp
  cml/mathlib/quaternion/rotation.h
  cml/mathlib/quaternion/rotation.tpp
  )
source_group("${_header_group}mathlib\\quaternion"
  FILES ${mathlib_quaternion_HEADERS})

set(util_HEADERS
  cml/util/matrix_print.h
  cml/util/matrix_print.tpp
  cml/util/quaternion_print.h
  cml/util/quaternion_print.tpp
  cml/util/vector_hash.h
  cml/util/vector_print.h
  cml/util/vector_print.tpp
  )
source_group("${_header_group}\\util" FILES ${util_HEADERS})

set(all_headers
  ${main_HEADERS}
  ${common_HEADERS}
  ${common_mpl_HEADERS}
  ${scalar_HEADERS}
  ${storage_HEADERS}
  ${matrix_HEADERS}
  ${matrix_detail_HEADERS}
  ${quaternion_HEADERS}
  ${util_HEADERS}
  ${vector_HEADERS}
  ${vector_detail_HEADERS}
  ${mathlib_HEADERS}
  ${mathlib_vector_HEADERS}
  ${mathlib_matrix_HEADERS}
  ${mathlib_quaternion_HEADERS}
  )

add_library(cml STATIC ${all_headers} ignore-me.cpp)
target_include_directories(cml
  PUBLIC $<BUILD_INTERFACE:${CML_ROOT}>
  )

source_group("_" FILES ignore-me.cpp)
set_target_properties(cml PROPERTIES EXCLUDE_FROM_ALL TRUE)

# --------------------------------------------------------------------------
# vim:ft=cmake
