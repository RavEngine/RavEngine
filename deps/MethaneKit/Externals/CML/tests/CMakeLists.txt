# -*- cmake -*- -----------------------------------------------------------
# @@COPYRIGHT@@
#*-------------------------------------------------------------------------

# Need the test macros:
include(CMLTestMacros)

# Catch-based test runner:
add_subdirectory(main)

# Common functionality tests:
add_subdirectory(common)

# Storage tests:
add_subdirectory(storage)

# cml::vector<> tests:
add_subdirectory(vector)

# cml::matrix<> tests:
add_subdirectory(matrix)

# cml::quaternion<> tests:
add_subdirectory(quaternion)

# mathlib tests:
add_subdirectory(mathlib)

# util tests:
add_subdirectory(util)

# --------------------------------------------------------------------------
# vim:ft=cmake
