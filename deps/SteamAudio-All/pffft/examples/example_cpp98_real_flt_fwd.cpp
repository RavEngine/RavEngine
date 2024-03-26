
#include "pffft.hpp"

#include <complex>
#include <iostream>


void cxx98_forward_real_float(const int transformLen)
{
  std::cout << "running " << __FUNCTION__ << "()" << std::endl;

  // first check - might be skipped
  typedef pffft::Fft<float> FFT_T;
  if (transformLen < FFT_T::minFFtsize())
  {
    std::cerr << "Error: minimum FFT transformation length is " << FFT_T::minFFtsize() << std::endl;
    return;
  }

  // instantiate FFT and prepare transformation for length N
  pffft::Fft<float> fft(transformLen);

  // one more check
  if (!fft.isValid())
  {
    std::cerr << "Error: transformation length " << transformLen << " is not decomposable into small prime factors. "
              << "Next valid transform size is: " << FFT_T::nearestTransformSize(transformLen)
              << "; next power of 2 is: " << FFT_T::nextPowerOfTwo(transformLen) << std::endl;
    return;
  }

  // allocate aligned vectors for input X and output Y
  pffft::AlignedVector<float> X = fft.valueVector();
  pffft::AlignedVector< std::complex<float> > Y = fft.spectrumVector();

  // alternative access: get raw pointers to aligned vectors
  float *Xs = X.data();
  std::complex<float> *Ys = Y.data();

  // prepare some input data
  for (int k = 0; k < transformLen; k += 2)
  {
    X[k] = k;        // access through AlignedVector<float>
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
  cxx98_forward_real_float(N);
  return 0;
}
