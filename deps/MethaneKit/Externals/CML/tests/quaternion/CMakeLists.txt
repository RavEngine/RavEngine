# -*- cmake -*- -----------------------------------------------------------
# @@COPYRIGHT@@
#*-------------------------------------------------------------------------

project(CML_Testing_Quaternion)
set(CML_CURRENT_TEST_GROUP "Quaternion")

CML_ADD_TEST(fixed_compiled_quaternion1)
#ADD_CML_TEST(quaternion_copy1)  ## Not implemented yet.
CML_ADD_TEST(quaternion_unary_node1)
CML_ADD_TEST(quaternion_binary_node1)
CML_ADD_TEST(quaternion_scalar_node1)
CML_ADD_TEST(quaternion_comparison1)
CML_ADD_TEST(quaternion_functions1)
CML_ADD_TEST(quaternion_product1)
CML_ADD_TEST(quaternion_dot1)
CML_ADD_TEST(imaginary1)
CML_ADD_TEST(conjugate1)
CML_ADD_TEST(inverse1)

# --------------------------------------------------------------------------
# vim:ft=cmake
