# -*- cmake -*- -----------------------------------------------------------
# @@COPYRIGHT@@
#*-------------------------------------------------------------------------

project(CML_Testing_Mathlib)
set(CML_CURRENT_TEST_GROUP "Mathlib")

CML_ADD_TEST(vector_angle1)
CML_ADD_TEST(vector_transform1)
CML_ADD_TEST(vector_rotation1)
CML_ADD_TEST(vector_misc1)
    
CML_ADD_TEST(matrix_generators1)
CML_ADD_TEST(matrix_basis1)
CML_ADD_TEST(matrix_translation1)
CML_ADD_TEST(matrix_scale1)
CML_ADD_TEST(matrix_invert1)
CML_ADD_TEST(matrix_rotation1)
CML_ADD_TEST(matrix_projection1)
CML_ADD_TEST(matrix_transform1)
    
CML_ADD_TEST(quaternion_basis1)
CML_ADD_TEST(quaternion_rotation1)
    
CML_ADD_TEST(coordinate_conversion1)
CML_ADD_TEST(random_unit1)
CML_ADD_TEST(frustum1)

# --------------------------------------------------------------------------
# vim:ft=cmake
