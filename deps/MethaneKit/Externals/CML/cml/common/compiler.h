/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef cml_common_compiler_h
#define cml_common_compiler_h

#if defined(__GNUC__) && (__GNUC__ > 4 \
  || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 9)))

/* N2439 move semantics for *this, used, for example, to efficiently return
 * an expression node from a class method. Without this, a temporary is
 * returned instead:
 */
# define CML_HAS_RVALUE_REFERENCE_FROM_THIS

/* Trailing method type for ref from *this (to disambiguate overloads using
 * rvalue reference from this):
 */
# define __CML_REF		&

/* Defaulted move constructor supported: */
# define CML_HAS_DEFAULTED_MOVE_CONSTRUCTOR

/* Constexpr support: */
# define CML_CONSTEXPR constexpr

/* This dummy qualifier is not needed for gcc: */
#  define CML_DUMMY_TYPENAME

#elif defined(__clang__)
/* Note: checking for Clang BEFORE checking for Visual Studio, since clang-cl
 * defines _MSC_VER.
 */

# ifdef __cpp_ref_qualifiers
/* N2439 move semantics for *this, used, for example, to efficiently return
 * an expression node from a class method. Without this, a temporary is
 * returned instead:
 */
#  define CML_HAS_RVALUE_REFERENCE_FROM_THIS

/* Trailing method type for ref from *this (to disambiguate overloads using
 * rvalue reference from this):
 */
#  define __CML_REF		&
# else
#  define __CML_REF
# endif


/* Constexpr support: */
# ifdef __cpp_constexpr
#  define CML_CONSTEXPR constexpr
# else
#  define CML_CONSTEXPR inline
# endif


/* Structured bindings: */
# ifdef __cpp_structured_bindings
#  define CML_HAS_STRUCTURED_BINDINGS
# endif


/* Defaulted move constructor: */
# if ((__clang_major__ >= 4) \
  || ((__clang_major__ >= 3) && (__clang_minor__ >= 5))) \
  || (__apple_build_version__ >= 7000000)

#  define CML_HAS_DEFAULTED_MOVE_CONSTRUCTOR
# endif


/* This dummy qualifier is not needed for Clang: */
#  define CML_DUMMY_TYPENAME

#elif defined(_MSC_VER)

# if _MSC_VER < 1900

/* VC++ (at least VS12) does not support move from *this, so no need to
 * disambiguate:
 */
#  define __CML_REF

/* VC++ (at least VS12) has brain-dead operator= overload resolution: */
#  define CML_HAS_MSVC_BRAIN_DEAD_ASSIGNMENT_OVERLOADS

/* VC++ 2013 cannot figure out default arguments returned as a template
 * method without this qualifier:
 */
#  define CML_DUMMY_TYPENAME typename

/* No constexpr support: */
#  define CML_CONSTEXPR inline

# else

# if(_MSC_VER < 1914)
/* Apparently, VS2015 does not allow a parameter pack expansion to target non-
 * packed parameters.
 */
#  define CML_HAS_MSVC_WONKY_PARAMETER_PACK
# endif

/* N2439 move semantics for *this, used, for example, to efficiently return
 * an expression node from a class method. Without this, a temporary is
 * returned instead:
 */
#  define CML_HAS_RVALUE_REFERENCE_FROM_THIS

/* Trailing method type for ref from *this (to disambiguate overloads using
 * rvalue reference from this):
 */
#  define __CML_REF		&

/* Defaulted move constructor supported: */
#  define CML_HAS_DEFAULTED_MOVE_CONSTRUCTOR

/* Constexpr support: */
#  define CML_CONSTEXPR constexpr

/* Not needed for other versions of VC++: */
#  define CML_DUMMY_TYPENAME

# endif

# if (_MSC_VER >= 1911) && (_MSVC_LANG >= 201703L)

/* Enable structured bindings: */
#  define CML_HAS_STRUCTURED_BINDINGS

# endif

#else
#warning "Unrecognized compiler; using safe defaults"

/* Assume no default move constructors: */
// # define CML_HAS_DEFAULTED_MOVE_CONSTRUCTOR

/* Assume no rvalue reference from this: */
// # define CML_HAS_RVALUE_REFERENCE_FROM_THIS

/* Assume no structured bindings: */
// # define CML_HAS_STRUCTURED_BINDINGS

/* Assume no disambiguation needed: */
# define __CML_REF

/* Assume no constexpr support: */
# define CML_CONSTEXPR inline

/* Assume this dummy qualifier is not needed: */
#  define CML_DUMMY_TYPENAME

#endif

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
