
#include "pffft.hpp"

#include <complex>
#include <iostream>


void cxx11_forward_real_double(const int transformLen)
{
  std::cout << "running " << __FUNCTION__ << "()" << std::endl;

  // first check - might be skipped
  using FFT_T = pffft::Fft<double>;
  if (transformLen < FFT_T::minFFtsize())
  {
    std::cerr << "Error: minimum FFT transformation length is " << FFT_T::minFFtsize() << std::endl;
    return;
  }

  // instantiate FFT and prepare transformation for length N
  pffft::Fft<double> fft { transformLen };

  // one more check
  if (!fft.isValid())
  {
    std::cerr << "Error: transformation length " << transformLen << " is not decomposable into small prime factors. "
              << "Next valid transform size is: " << FFT_T::nearestTransformSize(transformLen)
              << "; next power of 2 is: " << FFT_T::nextPowerOfTwo(transformLen) << std::endl;
    return;
  }

  // allocate aligned vectors for (real) input X and (complex) output Y
  auto X = fft.valueVector();     // input vector;  type is AlignedVector<double>
  auto Y = fft.spectrumVector();  // output vector; type is AlignedVector< std::complex<double> >

  // alternative access: get raw pointers to aligned vectors
  double *Xs = X.data();
  std::complex<double> *Ys = Y.data();

  // prepare some input data
  for (int k = 0; k < transformLen; k += 2)
  {
    X[k] = k;        // access through AlignedVector<double>
    Xs[k+1] = -1-k;  // access through raw pointer
  }

  // do the forward transform; write complex spectrum result into Y
  fft.forward(X, Y);

  // print spectral output
  std::cout << "output should be complex spectrum with " << fft.getSpectrumSize() << " bins" << std::endl;
  std::cout << "output vector has size " << Y.size() << " (complex bins):" << std::endl;
  for (unsigned k = 0; k < Y.size(); k += 2)
  {
    std::cout << "Y[" << k << "] = " << Y[k] << std::endl;
    std::cout << "Y[" << k+1 << "] = " << Ys[k+1] << std::endl;
  }
}


int main(int argc, char *argv[])
{
  int N = (1 < argc) ? atoi(argv[1]) : 32;
  cxx11_forward_real_double(N);
  return 0;
}
