# -*- cmake -*- -----------------------------------------------------------
# @@COPYRIGHT@@
#*-------------------------------------------------------------------------

project(CML_Testing_Vector)
set(CML_CURRENT_TEST_GROUP "Vector")

CML_ADD_TEST(fixed_compiled_vector1)
CML_ADD_TEST(fixed_external_vector1)
CML_ADD_TEST(dynamic_external_vector1)
CML_ADD_TEST(dynamic_allocated_vector1)
CML_ADD_TEST(vector_temporary1)
CML_ADD_TEST(vector_copy1)
CML_ADD_TEST(vector_unary_node1)
CML_ADD_TEST(vector_binary_node1)
CML_ADD_TEST(vector_scalar_node1)
CML_ADD_TEST(vector_comparison1)
CML_ADD_TEST(vector_functions1)
CML_ADD_TEST(vector_dot1)
CML_ADD_TEST(perp_dot1)
CML_ADD_TEST(cross1)
CML_ADD_TEST(triple_product1)
CML_ADD_TEST(subvector1)
CML_ADD_TEST(outer_product1)
CML_ADD_TEST(vector_hadamard_product1)

# --------------------------------------------------------------------------
# vim:ft=cmake
