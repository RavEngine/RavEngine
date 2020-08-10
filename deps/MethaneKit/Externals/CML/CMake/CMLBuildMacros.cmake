# -*- cmake -*- -----------------------------------------------------------
# @@COPYRIGHT@@
#*-------------------------------------------------------------------------

# Find and parse a version string from a file.  The version should have the
# format:
#
# #define <version_macro_name> MMmmmpp
#   or
# #define <version_macro_name> Mmmmpp
#
# The major, minor, and patch versions are stored to <version_major>,
# <version_minor>, and <version_patch>.
macro(CML_VERSION_FROM_FILE
    version_file
    version_macro_name
    version_major
    version_minor
    version_patch
    version_string
    )

  # Parse ${version_file} to find #define ${version_macro_name} (this is from
  # the root Boost.CMake CMakeLists.txt file).  The recognized version
  # pattern is
  #
  #     #define [space] <version_macro> Mmp[U]
  #
  # e.g.
  #
  #     #define APP_VERSION_STRING 010103U
  #     #define APP_VERSION_STRING 10103U
  #     #define APP_VERSION_STRING 010103
  file(STRINGS ${version_file} _version_string REGEX
    "#define[ \t]+${version_macro_name}[ \t]+[0-9]+U?$")

  # Make sure it's a real version number:
  string(REGEX MATCH "[ \t]+[0-9]+" _version_string ${_version_string})

  # Calculate the version parts:
  if(_version_string)
    math(EXPR ${version_major} "${_version_string} / 100000")
    math(EXPR ${version_minor} "${_version_string} / 100 % 1000")
    math(EXPR ${version_patch} "${_version_string} % 100")
    set(${version_string}
      "${${version_major}}.${${version_minor}}.${${version_patch}}")
  else()
    message(FATAL_ERROR 
      "Unable to parse the version from ${version_file}")
  endif()
endmacro()

# --------------------------------------------------------------------------
# vim:ft=cmake
