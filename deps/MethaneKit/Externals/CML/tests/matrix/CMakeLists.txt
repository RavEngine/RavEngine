# -*- cmake -*- -----------------------------------------------------------
# @@COPYRIGHT@@
#*-------------------------------------------------------------------------

project(CML_Testing_Matrix)
set(CML_CURRENT_TEST_GROUP "Matrix")

CML_ADD_TEST(matrix_promotion1)
CML_ADD_TEST(fixed_compiled_matrix1)
CML_ADD_TEST(fixed_external_matrix1)
CML_ADD_TEST(dynamic_external_matrix1)
CML_ADD_TEST(dynamic_allocated_matrix1)
CML_ADD_TEST(matrix_scalar_node1)
CML_ADD_TEST(matrix_unary_node1)
CML_ADD_TEST(matrix_binary_node1)
CML_ADD_TEST(matrix_functions1)
CML_ADD_TEST(matrix_matrix_product1)
CML_ADD_TEST(matrix_vector_product1)
CML_ADD_TEST(matrix_transpose1)
CML_ADD_TEST(matrix_inverse1)
CML_ADD_TEST(basis1)
CML_ADD_TEST(rowcol1)
CML_ADD_TEST(lu1)
CML_ADD_TEST(determinant1)
CML_ADD_TEST(matrix_hadamard_product1)
CML_ADD_TEST(matrix_comparison1)

# --------------------------------------------------------------------------
# vim:ft=cmake
