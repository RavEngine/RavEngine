/* Copyright (c) 2020  Dario Mambro ( dario.mambro@gmail.com )
   Copyright (c) 2020  Hayati Ayguen ( h_ayguen@web.de )

   Redistribution and use of the Software in source and binary forms,
   with or without modification, is permitted provided that the
   following conditions are met:

   - Neither the names of PFFFT, nor the names of its
   sponsors or contributors may be used to endorse or promote products
   derived from this Software without specific prior written permission.

   - Redistributions of source code must retain the above copyright
   notices, this list of conditions, and the disclaimer below.

   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions, and the disclaimer below in the
   documentation and/or other materials provided with the
   distribution.

   THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE CONTRIBUTORS OR COPYRIGHT
   HOLDERS BE LIABLE FOR ANY CLAIM, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE
   SOFTWARE.
*/

#pragma once

#include <complex>
#include <vector>
#include <limits>
#include <cassert>

namespace pffft {
namespace detail {
#if defined(PFFFT_ENABLE_FLOAT) || ( !defined(PFFFT_ENABLE_FLOAT) && !defined(PFFFT_ENABLE_DOUBLE) )
#include "pffft.h"
#endif
#if defined(PFFFT_ENABLE_DOUBLE)
#include "pffft_double.h"
#endif
}
}

namespace pffft {

// enum { PFFFT_REAL, PFFFT_COMPLEX }
typedef detail::pffft_transform_t TransformType;

// define 'Scalar' and 'Complex' (in namespace pffft) with template Types<>
// and other type specific helper functions
template<typename T> struct Types {};
#if defined(PFFFT_ENABLE_FLOAT) || ( !defined(PFFFT_ENABLE_FLOAT) && !defined(PFFFT_ENABLE_DOUBLE) )
template<> struct Types<float>  {
  typedef float  Scalar;
  typedef std::complex<Scalar> Complex;
  static int simd_size() { return detail::pffft_simd_size(); }
  static const char * simd_arch() { return detail::pffft_simd_arch(); }
  static int minFFtsize() { return pffft_min_fft_size(detail::PFFFT_REAL); }
  static bool isValidSize(int N) { return pffft_is_valid_size(N, detail::PFFFT_REAL); }
  static int nearestTransformSize(int N, bool higher) { return pffft_nearest_transform_size(N, detail::PFFFT_REAL, higher ? 1 : 0); }
};
template<> struct Types< std::complex<float> >  {
  typedef float  Scalar;
  typedef std::complex<float>  Complex;
  static int simd_size() { return detail::pffft_simd_size(); }
  static const char * simd_arch() { return detail::pffft_simd_arch(); }
  static int minFFtsize() { return pffft_min_fft_size(detail::PFFFT_COMPLEX); }
  static bool isValidSize(int N) { return pffft_is_valid_size(N, detail::PFFFT_COMPLEX); }
  static int nearestTransformSize(int N, bool higher) { return pffft_nearest_transform_size(N, detail::PFFFT_COMPLEX, higher ? 1 : 0); }
};
#endif
#if defined(PFFFT_ENABLE_DOUBLE)
template<> struct Types<double> {
  typedef double Scalar;
  typedef std::complex<Scalar> Complex;
  static int simd_size() { return detail::pffftd_simd_size(); }
  static const char * simd_arch() { return detail::pffftd_simd_arch(); }
  static int minFFtsize() { return pffftd_min_fft_size(detail::PFFFT_REAL); }
  static bool isValidSize(int N) { return pffftd_is_valid_size(N, detail::PFFFT_REAL); }
  static int nearestTransformSize(int N, bool higher) { return pffftd_nearest_transform_size(N, detail::PFFFT_REAL, higher ? 1 : 0); }
};
template<> struct Types< std::complex<double> > {
  typedef double Scalar;
  typedef std::complex<double> Complex;
  static int simd_size() { return detail::pffftd_simd_size(); }
  static const char * simd_arch() { return detail::pffftd_simd_arch(); }
  static int minFFtsize() { return pffftd_min_fft_size(detail::PFFFT_COMPLEX); }
  static bool isValidSize(int N) { return pffftd_is_valid_size(N, detail::PFFFT_COMPLEX); }
  static int nearestTransformSize(int N, bool higher) { return pffftd_nearest_transform_size(N, detail::PFFFT_COMPLEX, higher ? 1 : 0); }
};
#endif

// Allocator
template<typename T> class PFAlloc;

namespace detail {
  template<typename T> class Setup;
}

#if (__cplusplus >= 201103L || (defined(_MSC_VER) && _MSC_VER >= 1900))

// define AlignedVector<T> utilizing 'using' in C++11
template<typename T>
using AlignedVector = typename std::vector< T, PFAlloc<T> >;

#else

// define AlignedVector<T> having to derive std::vector<>
template <typename T>
struct AlignedVector : public std::vector< T, PFAlloc<T> > {
  AlignedVector() : std::vector< T, PFAlloc<T> >() { }
  AlignedVector(int N) : std::vector< T, PFAlloc<T> >(N) { }
};

#endif


// T can be float, double, std::complex<float> or std::complex<double>
//   define PFFFT_ENABLE_DOUBLE before include this file for double and std::complex<double>
template<typename T>
class Fft
{
public:

  // define types value_type, Scalar and Complex
  typedef T value_type;
  typedef typename Types<T>::Scalar  Scalar;
  typedef typename Types<T>::Complex Complex;

  // static retrospection functions
  static bool isComplexTransform()  { return sizeof(T) == sizeof(Complex); }
  static bool isFloatScalar()  { return sizeof(Scalar) == sizeof(float); }
  static bool isDoubleScalar() { return sizeof(Scalar) == sizeof(double); }

  // simple helper to determine next power of 2 - without inexact/rounding floating point operations
  static int nextPowerOfTwo(int N) { return detail::pffft_next_power_of_two(N); }
  static bool isPowerOfTwo(int N) { return detail::pffft_is_power_of_two(N) ? true : false; }


  static int simd_size() { return Types<T>::simd_size(); }
  static const char * simd_arch() { return Types<T>::simd_arch(); }

  // simple helper to get minimum possible fft length
  static int minFFtsize() { return Types<T>::minFFtsize(); }

  // helper to determine nearest transform size - factorizable to minFFtsize() with factors 2, 3, 5
  static bool isValidSize(int N) { return Types<T>::isValidSize(N); }
  static int nearestTransformSize(int N, bool higher=true) { return Types<T>::nearestTransformSize(N, higher); }


  //////////////////

  /*
   * Contructor, with transformation length, preparing transforms.
   *
   * For length <= stackThresholdLen, the stack is used for the internal
   * work memory. for bigger length', the heap is used.
   *
   * Using the stack is probably the best strategy for small
   * FFTs, say for N <= 4096). Threads usually have a small stack, that
   * there's no sufficient amount of memory, usually leading to a crash!
   */
  Fft( int length, int stackThresholdLen = 4096 );


  /*
   * constructor or prepareLength() produced a valid FFT instance?
   * delivers false for invalid FFT sizes
   */
  bool isValid() const;


  ~Fft();

  /*
   * prepare for transformation length 'newLength'.
   * length is identical to forward()'s input vector's size,
   * and also equals inverse()'s output vector size.
   * this function is no simple setter. it pre-calculates twiddle factors.
   * returns true if newLength is >= minFFtsize, false otherwise
   */
  bool prepareLength(int newLength);

  /*
   * retrieve the transformation length.
   */
  int getLength() const { return length; }

  /*
   * retrieve size of complex spectrum vector,
   * the output of forward()
   */
  int getSpectrumSize() const { return isComplexTransform() ? length : ( length / 2 ); }

  /*
   * retrieve size of spectrum vector - in internal layout;
   * the output of forwardToInternalLayout()
   */
  int getInternalLayoutSize() const { return isComplexTransform() ? ( 2 * length ) : length; }


  ////////////////////////////////////////////
  ////
  //// API 1, with std::vector<> based containers,
  ////   which free the allocated memory themselves (RAII).
  ////
  //// uses an Allocator for the alignment of SIMD data.
  ////
  ////////////////////////////////////////////

  // create suitably preallocated aligned vector for one FFT
  AlignedVector<T>       valueVector() const;
  AlignedVector<Complex> spectrumVector() const;
  AlignedVector<Scalar>  internalLayoutVector() const;

  ////////////////////////////////////////////
  // although using Vectors for output ..
  // they need to have resize() applied before!

  // core API, having the spectrum in canonical order

  /*
   * Perform the forward Fourier transform.
   *
   * Transforms are not scaled: inverse(forward(x)) = N*x.
   * Typically you will want to scale the backward transform by 1/N.
   *
   * The output 'spectrum' is canonically ordered - as expected.
   *
   * a) for complex input isComplexTransform() == true,
   *    and transformation length N  the output array is complex:
   *   index k in 0 .. N/2 -1  corresponds to frequency k * Samplerate / N
   *   index k in N/2 .. N -1  corresponds to frequency (k -N) * Samplerate / N,
   *     resulting in negative frequencies
   *
   * b) for real input isComplexTransform() == false,
   *    and transformation length N  the output array is 'mostly' complex:
   *   index k in 1 .. N/2 -1  corresponds to frequency k * Samplerate / N
   *   index k == 0 is a special case:
   *     the real() part contains the result for the DC frequency 0,
   *     the imag() part contains the result for the Nyquist frequency Samplerate/2
   *   both 0-frequency and half frequency components, which are real,
   *   are assembled in the first entry as  F(0)+i*F(N/2).
   *   with the output size N/2 complex values, it is obvious, that the
   *   result for negative frequencies are not output, cause of symmetry.
   *
   * input and output may alias - if you do nasty type conversion.
   * return is just the given output parameter 'spectrum'.
   */
  AlignedVector<Complex> & forward(const AlignedVector<T> & input, AlignedVector<Complex> & spectrum);

  /*
   * Perform the inverse Fourier transform, see forward().
   * return is just the given output parameter 'output'.
   */
  AlignedVector<T> & inverse(const AlignedVector<Complex> & spectrum, AlignedVector<T> & output);


  // provide additional functions with spectrum in some internal Layout.
  // these are faster, cause the implementation omits the reordering.
  // these are useful in special applications, like fast convolution,
  // where inverse() is following anyway ..

  /*
   * Perform the forward Fourier transform - similar to forward(), BUT:
   *
   * The z-domain data is stored in the most efficient order
   * for transforming it back, or using it for convolution.
   * If you need to have its content sorted in the "usual" canonical order,
   * either use forward(), or call reorderSpectrum() after calling
   * forwardToInternalLayout(), and before the backward fft
   *
   * return is just the given output parameter 'spectrum_internal_layout'.
   */
  AlignedVector<Scalar> & forwardToInternalLayout(
          const AlignedVector<T> & input,
          AlignedVector<Scalar> & spectrum_internal_layout );

  /*
   * Perform the inverse Fourier transform, see forwardToInternalLayout()
   *
   * return is just the given output parameter 'output'.
   */
  AlignedVector<T> & inverseFromInternalLayout(
          const AlignedVector<Scalar> & spectrum_internal_layout,
          AlignedVector<T> & output );

  /*
   * Reorder the spectrum from internal layout to have the
   * frequency components in the correct "canonical" order.
   * see forward() for a description of the canonical order.
   *
   * input and output should not alias.
   */
  void reorderSpectrum(
          const AlignedVector<Scalar> & input,
          AlignedVector<Complex> & output );

  /*
   * Perform a multiplication of the frequency components of
   * spectrum_internal_a and spectrum_internal_b
   * into spectrum_internal_ab.
   * The arrays should have been obtained with forwardToInternalLayout)
   * and should *not* have been reordered with reorderSpectrum().
   *
   * the operation performed is:
   *  spectrum_internal_ab = (spectrum_internal_a * spectrum_internal_b)*scaling
   *
   * The spectrum_internal_[a][b], pointers may alias.
   * return is just the given output parameter 'spectrum_internal_ab'.
   */
  AlignedVector<Scalar> & convolve(
          const AlignedVector<Scalar> & spectrum_internal_a,
          const AlignedVector<Scalar> & spectrum_internal_b,
          AlignedVector<Scalar> & spectrum_internal_ab,
          const Scalar scaling );

  /*
   * Perform a multiplication and accumulation of the frequency components
   * - similar to convolve().
   *
   * the operation performed is:
   *  spectrum_internal_ab += (spectrum_internal_a * spectrum_internal_b)*scaling
   *
   * The spectrum_internal_[a][b], pointers may alias.
   * return is just the given output parameter 'spectrum_internal_ab'.
   */
  AlignedVector<Scalar> & convolveAccumulate(
          const AlignedVector<Scalar> & spectrum_internal_a,
          const AlignedVector<Scalar> & spectrum_internal_b,
          AlignedVector<Scalar> & spectrum_internal_ab,
          const Scalar scaling );


  ////////////////////////////////////////////
  ////
  //// API 2, dealing with raw pointers,
  //// which need to be deallocated using alignedFree()
  ////
  //// the special allocation is required cause SIMD
  //// implementations require aligned memory
  ////
  //// Method descriptions are equal to the methods above,
  //// having  AlignedVector<T> parameters - instead of raw pointers.
  //// That is why following methods have no documentation.
  ////
  ////////////////////////////////////////////

  static void alignedFree(void* ptr);

  static T * alignedAllocType(int length);
  static Scalar* alignedAllocScalar(int length);
  static Complex* alignedAllocComplex(int length);

  // core API, having the spectrum in canonical order

  Complex* forward(const T* input, Complex* spectrum);

  T* inverse(const Complex* spectrum, T* output);


  // provide additional functions with spectrum in some internal Layout.
  // these are faster, cause the implementation omits the reordering.
  // these are useful in special applications, like fast convolution,
  // where inverse() is following anyway ..

  Scalar* forwardToInternalLayout(const T* input,
                                Scalar* spectrum_internal_layout);

  T* inverseFromInternalLayout(const Scalar* spectrum_internal_layout, T* output);

  void reorderSpectrum(const Scalar* input, Complex* output );

  Scalar* convolve(const Scalar* spectrum_internal_a,
                   const Scalar* spectrum_internal_b,
                   Scalar* spectrum_internal_ab,
                   const Scalar scaling);

  Scalar* convolveAccumulate(const Scalar* spectrum_internal_a,
                             const Scalar* spectrum_internal_b,
                             Scalar* spectrum_internal_ab,
                             const Scalar scaling);

private:
  detail::Setup<T> setup;
  Scalar* work;
  int length;
  int stackThresholdLen;
};


template<typename T>
inline T* alignedAlloc(int length) {
  return (T*)detail::pffft_aligned_malloc( length * sizeof(T) );
}

inline void alignedFree(void *ptr) {
    detail::pffft_aligned_free(ptr);
}


// simple helper to determine next power of 2 - without inexact/rounding floating point operations
inline int nextPowerOfTwo(int N) {
  return detail::pffft_next_power_of_two(N);
}

inline bool isPowerOfTwo(int N) {
  return detail::pffft_is_power_of_two(N) ? true : false;
}



////////////////////////////////////////////////////////////////////

// implementation

namespace detail {

template<typename T>
class Setup
{};

#if defined(PFFFT_ENABLE_FLOAT) || ( !defined(PFFFT_ENABLE_FLOAT) && !defined(PFFFT_ENABLE_DOUBLE) )

template<>
class Setup<float>
{
  PFFFT_Setup* self;

public:
  typedef float value_type;
  typedef Types< value_type >::Scalar Scalar;

  Setup()
    : self(NULL)
  {}

  ~Setup() { pffft_destroy_setup(self); }

  void prepareLength(int length)
  {
    if (self) {
      pffft_destroy_setup(self);
    }
    self = pffft_new_setup(length, PFFFT_REAL);
  }

  bool isValid() const { return (self); }

  void transform_ordered(const Scalar* input,
                         Scalar* output,
                         Scalar* work,
                         pffft_direction_t direction)
  {
    pffft_transform_ordered(self, input, output, work, direction);
  }

  void transform(const Scalar* input,
                 Scalar* output,
                 Scalar* work,
                 pffft_direction_t direction)
  {
    pffft_transform(self, input, output, work, direction);
  }

  void reorder(const Scalar* input, Scalar* output, pffft_direction_t direction)
  {
    pffft_zreorder(self, input, output, direction);
  }

  void convolveAccumulate(const Scalar* dft_a,
                          const Scalar* dft_b,
                          Scalar* dft_ab,
                          const Scalar scaling)
  {
    pffft_zconvolve_accumulate(self, dft_a, dft_b, dft_ab, scaling);
  }

  void convolve(const Scalar* dft_a,
                const Scalar* dft_b,
                Scalar* dft_ab,
                const Scalar scaling)
  {
    pffft_zconvolve_no_accu(self, dft_a, dft_b, dft_ab, scaling);
  }
};


template<>
class Setup< std::complex<float> >
{
  PFFFT_Setup* self;

public:
  typedef std::complex<float> value_type;
  typedef Types< value_type >::Scalar Scalar;

  Setup()
    : self(NULL)
  {}

  ~Setup() { pffft_destroy_setup(self); }

  void prepareLength(int length)
  {
    if (self) {
      pffft_destroy_setup(self);
    }
    self = pffft_new_setup(length, PFFFT_COMPLEX);
  }

  bool isValid() const { return (self); }

  void transform_ordered(const Scalar* input,
                         Scalar* output,
                         Scalar* work,
                         pffft_direction_t direction)
  {
    pffft_transform_ordered(self, input, output, work, direction);
  }

  void transform(const Scalar* input,
                 Scalar* output,
                 Scalar* work,
                 pffft_direction_t direction)
  {
    pffft_transform(self, input, output, work, direction);
  }

  void reorder(const Scalar* input, Scalar* output, pffft_direction_t direction)
  {
    pffft_zreorder(self, input, output, direction);
  }

  void convolve(const Scalar* dft_a,
                const Scalar* dft_b,
                Scalar* dft_ab,
                const Scalar scaling)
  {
    pffft_zconvolve_no_accu(self, dft_a, dft_b, dft_ab, scaling);
  }
};

#endif /* defined(PFFFT_ENABLE_FLOAT) || ( !defined(PFFFT_ENABLE_FLOAT) && !defined(PFFFT_ENABLE_DOUBLE) ) */


#if defined(PFFFT_ENABLE_DOUBLE)

template<>
class Setup<double>
{
  PFFFTD_Setup* self;

public:
  typedef double value_type;
  typedef Types< value_type >::Scalar Scalar;

  Setup()
    : self(NULL)
  {}

  ~Setup() { pffftd_destroy_setup(self); }

  void prepareLength(int length)
  {
    if (self) {
      pffftd_destroy_setup(self);
      self = NULL;
    }
    if (length > 0) {
      self = pffftd_new_setup(length, PFFFT_REAL);
    }
  }

  bool isValid() const { return (self); }

  void transform_ordered(const Scalar* input,
                         Scalar* output,
                         Scalar* work,
                         pffft_direction_t direction)
  {
    pffftd_transform_ordered(self, input, output, work, direction);
  }

  void transform(const Scalar* input,
                 Scalar* output,
                 Scalar* work,
                 pffft_direction_t direction)
  {
    pffftd_transform(self, input, output, work, direction);
  }

  void reorder(const Scalar* input, Scalar* output, pffft_direction_t direction)
  {
    pffftd_zreorder(self, input, output, direction);
  }

  void convolveAccumulate(const Scalar* dft_a,
                          const Scalar* dft_b,
                          Scalar* dft_ab,
                          const Scalar scaling)
  {
    pffftd_zconvolve_accumulate(self, dft_a, dft_b, dft_ab, scaling);
  }

  void convolve(const Scalar* dft_a,
                const Scalar* dft_b,
                Scalar* dft_ab,
                const Scalar scaling)
  {
    pffftd_zconvolve_no_accu(self, dft_a, dft_b, dft_ab, scaling);
  }
};

template<>
class Setup< std::complex<double> >
{
  PFFFTD_Setup* self;

public:
  typedef std::complex<double> value_type;
  typedef Types< value_type >::Scalar Scalar;

  Setup()
    : self(NULL)
  {}

  ~Setup() { pffftd_destroy_setup(self); }

  void prepareLength(int length)
  {
    if (self) {
      pffftd_destroy_setup(self);
    }
    self = pffftd_new_setup(length, PFFFT_COMPLEX);
  }

  bool isValid() const { return (self); }

  void transform_ordered(const Scalar* input,
                         Scalar* output,
                         Scalar* work,
                         pffft_direction_t direction)
  {
    pffftd_transform_ordered(self, input, output, work, direction);
  }

  void transform(const Scalar* input,
                 Scalar* output,
                 Scalar* work,
                 pffft_direction_t direction)
  {
    pffftd_transform(self, input, output, work, direction);
  }

  void reorder(const Scalar* input, Scalar* output, pffft_direction_t direction)
  {
    pffftd_zreorder(self, input, output, direction);
  }

  void convolveAccumulate(const Scalar* dft_a,
                          const Scalar* dft_b,
                          Scalar* dft_ab,
                          const Scalar scaling)
  {
    pffftd_zconvolve_accumulate(self, dft_a, dft_b, dft_ab, scaling);
  }

  void convolve(const Scalar* dft_a,
                const Scalar* dft_b,
                Scalar* dft_ab,
                const Scalar scaling)
  {
    pffftd_zconvolve_no_accu(self, dft_a, dft_b, dft_ab, scaling);
  }
};

#endif /* defined(PFFFT_ENABLE_DOUBLE) */

} // end of anonymous namespace for Setup<>


template<typename T>
inline Fft<T>::Fft(int length, int stackThresholdLen)
  : work(NULL)
  , length(0)
  , stackThresholdLen(stackThresholdLen)
{
#if (__cplusplus >= 201103L || (defined(_MSC_VER) && _MSC_VER >= 1900))
  static_assert( sizeof(Complex) == 2 * sizeof(Scalar), "pffft requires sizeof(std::complex<>) == 2 * sizeof(Scalar)" );
#elif defined(__GNUC__)
  char static_assert_like[(sizeof(Complex) == 2 * sizeof(Scalar)) ? 1 : -1]; // pffft requires sizeof(std::complex<>) == 2 * sizeof(Scalar)
#endif
  prepareLength(length);
}

template<typename T>
inline Fft<T>::~Fft()
{
  alignedFree(work);
}

template<typename T>
inline bool
Fft<T>::isValid() const
{
  return setup.isValid();
}

template<typename T>
inline bool
Fft<T>::prepareLength(int newLength)
{
  if(newLength < minFFtsize())
    return false;

  const bool wasOnHeap = ( work != NULL );

  const bool useHeap = newLength > stackThresholdLen;

  if (useHeap == wasOnHeap && newLength == length) {
    return true;
  }

  length = 0;

  setup.prepareLength(newLength);
  if (!setup.isValid())
    return false;

  length = newLength;

  if (work) {
    alignedFree(work);
    work = NULL;
  }

  if (useHeap) {
    work = reinterpret_cast<Scalar*>( alignedAllocType(length) );
  }

  return true;
}


template<typename T>
inline AlignedVector<T>
Fft<T>::valueVector() const
{
  return AlignedVector<T>(length);
}

template<typename T>
inline AlignedVector< typename Fft<T>::Complex >
Fft<T>::spectrumVector() const
{
  return AlignedVector<Complex>( getSpectrumSize() );
}

template<typename T>
inline AlignedVector< typename Fft<T>::Scalar >
Fft<T>::internalLayoutVector() const
{
  return AlignedVector<Scalar>( getInternalLayoutSize() );
}


template<typename T>
inline AlignedVector< typename Fft<T>::Complex > &
Fft<T>::forward(const AlignedVector<T> & input, AlignedVector<Complex> & spectrum)
{
  forward( input.data(), spectrum.data() );
  return spectrum;
}

template<typename T>
inline AlignedVector<T> &
Fft<T>::inverse(const AlignedVector<Complex> & spectrum, AlignedVector<T> & output)
{
  inverse( spectrum.data(), output.data() );
  return output;
}


template<typename T>
inline AlignedVector< typename Fft<T>::Scalar > &
Fft<T>::forwardToInternalLayout(
    const AlignedVector<T> & input,
    AlignedVector<Scalar> & spectrum_internal_layout )
{
  forwardToInternalLayout( input.data(), spectrum_internal_layout.data() );
  return spectrum_internal_layout;
}

template<typename T>
inline AlignedVector<T> &
Fft<T>::inverseFromInternalLayout(
    const AlignedVector<Scalar> & spectrum_internal_layout,
    AlignedVector<T> & output )
{
  inverseFromInternalLayout( spectrum_internal_layout.data(), output.data() );
  return output;
}

template<typename T>
inline void
Fft<T>::reorderSpectrum(
    const AlignedVector<Scalar> & input,
    AlignedVector<Complex> & output )
{
  reorderSpectrum( input.data(), output.data() );
}

template<typename T>
inline AlignedVector< typename Fft<T>::Scalar > &
Fft<T>::convolveAccumulate(
    const AlignedVector<Scalar> & spectrum_internal_a,
    const AlignedVector<Scalar> & spectrum_internal_b,
    AlignedVector<Scalar> & spectrum_internal_ab,
    const Scalar scaling )
{
  convolveAccumulate( spectrum_internal_a.data(), spectrum_internal_b.data(),
                      spectrum_internal_ab.data(), scaling );
  return spectrum_internal_ab;
}

template<typename T>
inline AlignedVector< typename Fft<T>::Scalar > &
Fft<T>::convolve(
    const AlignedVector<Scalar> & spectrum_internal_a,
    const AlignedVector<Scalar> & spectrum_internal_b,
    AlignedVector<Scalar> & spectrum_internal_ab,
    const Scalar scaling )
{
  convolve( spectrum_internal_a.data(), spectrum_internal_b.data(),
            spectrum_internal_ab.data(), scaling );
  return spectrum_internal_ab;
}


template<typename T>
inline typename Fft<T>::Complex *
Fft<T>::forward(const T* input, Complex * spectrum)
{
  assert(isValid());
  setup.transform_ordered(reinterpret_cast<const Scalar*>(input),
                          reinterpret_cast<Scalar*>(spectrum),
                          work,
                          detail::PFFFT_FORWARD);
  return spectrum;
}

template<typename T>
inline T*
Fft<T>::inverse(Complex const* spectrum, T* output)
{
  assert(isValid());
  setup.transform_ordered(reinterpret_cast<const Scalar*>(spectrum),
                          reinterpret_cast<Scalar*>(output),
                          work,
                          detail::PFFFT_BACKWARD);
  return output;
}

template<typename T>
inline typename pffft::Fft<T>::Scalar*
Fft<T>::forwardToInternalLayout(const T* input, Scalar* spectrum_internal_layout)
{
  assert(isValid());
  setup.transform(reinterpret_cast<const Scalar*>(input),
                  spectrum_internal_layout,
                  work,
                  detail::PFFFT_FORWARD);
  return spectrum_internal_layout;
}

template<typename T>
inline T*
Fft<T>::inverseFromInternalLayout(const Scalar* spectrum_internal_layout, T* output)
{
  assert(isValid());
  setup.transform(spectrum_internal_layout,
                  reinterpret_cast<Scalar*>(output),
                  work,
                  detail::PFFFT_BACKWARD);
  return output;
}

template<typename T>
inline void
Fft<T>::reorderSpectrum( const Scalar* input, Complex* output )
{
  assert(isValid());
  setup.reorder(input, reinterpret_cast<Scalar*>(output), detail::PFFFT_FORWARD);
}

template<typename T>
inline typename pffft::Fft<T>::Scalar*
Fft<T>::convolveAccumulate(const Scalar* dft_a,
                           const Scalar* dft_b,
                           Scalar* dft_ab,
                           const Scalar scaling)
{
  assert(isValid());
  setup.convolveAccumulate(dft_a, dft_b, dft_ab, scaling);
  return dft_ab;
}

template<typename T>
inline typename pffft::Fft<T>::Scalar*
Fft<T>::convolve(const Scalar* dft_a,
                 const Scalar* dft_b,
                 Scalar* dft_ab,
                 const Scalar scaling)
{
  assert(isValid());
  setup.convolve(dft_a, dft_b, dft_ab, scaling);
  return dft_ab;
}

template<typename T>
inline void
Fft<T>::alignedFree(void* ptr)
{
  pffft::alignedFree(ptr);
}


template<typename T>
inline T*
pffft::Fft<T>::alignedAllocType(int length)
{
  return alignedAlloc<T>(length);
}

template<typename T>
inline typename pffft::Fft<T>::Scalar*
pffft::Fft<T>::alignedAllocScalar(int length)
{
  return alignedAlloc<Scalar>(length);
}

template<typename T>
inline typename Fft<T>::Complex *
Fft<T>::alignedAllocComplex(int length)
{
  return alignedAlloc<Complex>(length);
}



////////////////////////////////////////////////////////////////////

// Allocator - for std::vector<>:
// origin: http://www.josuttis.com/cppcode/allocator.html
// http://www.josuttis.com/cppcode/myalloc.hpp
//
// minor renaming and utilizing of pffft (de)allocation functions
// are applied to Jossutis' allocator

/* The following code example is taken from the book
 * "The C++ Standard Library - A Tutorial and Reference"
 * by Nicolai M. Josuttis, Addison-Wesley, 1999
 *
 * (C) Copyright Nicolai M. Josuttis 1999.
 * Permission to copy, use, modify, sell and distribute this software
 * is granted provided this copyright notice appears in all copies.
 * This software is provided "as is" without express or implied
 * warranty, and with no claim as to its suitability for any purpose.
 */

template <class T>
class PFAlloc {
  public:
    // type definitions
    typedef T        value_type;
    typedef T*       pointer;
    typedef const T* const_pointer;
    typedef T&       reference;
    typedef const T& const_reference;
    typedef std::size_t    size_type;
    typedef std::ptrdiff_t difference_type;

    // rebind allocator to type U
    template <class U>
    struct rebind {
        typedef PFAlloc<U> other;
    };

    // return address of values
    pointer address (reference value) const {
        return &value;
    }
    const_pointer address (const_reference value) const {
        return &value;
    }

    /* constructors and destructor
     * - nothing to do because the allocator has no state
     */
    PFAlloc() throw() {
    }
    PFAlloc(const PFAlloc&) throw() {
    }
    template <class U>
      PFAlloc (const PFAlloc<U>&) throw() {
    }
    ~PFAlloc() throw() {
    }

    // return maximum number of elements that can be allocated
    size_type max_size () const throw() {
        return std::numeric_limits<std::size_t>::max() / sizeof(T);
    }

    // allocate but don't initialize num elements of type T
    pointer allocate (size_type num, const void* = 0) {
        pointer ret = (pointer)( alignedAlloc<T>(int(num)) );
        return ret;
    }

    // initialize elements of allocated storage p with value value
    void construct (pointer p, const T& value) {
        // initialize memory with placement new
        new((void*)p)T(value);
    }

    // destroy elements of initialized storage p
    void destroy (pointer p) {
        // destroy objects by calling their destructor
        p->~T();
    }

    // deallocate storage p of deleted elements
    void deallocate (pointer p, size_type num) {
        // deallocate memory with pffft
        alignedFree( (void*)p );
    }
};

// return that all specializations of this allocator are interchangeable
template <class T1, class T2>
bool operator== (const PFAlloc<T1>&,
                 const PFAlloc<T2>&) throw() {
    return true;
}
template <class T1, class T2>
bool operator!= (const PFAlloc<T1>&,
                 const PFAlloc<T2>&) throw() {
    return false;
}


} // namespace pffft

