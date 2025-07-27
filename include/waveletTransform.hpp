#pragma once
#include <cmath>

//
// Mexican hat Mother wavelet
// packet (Ricker Wavelet)
//
template<typename Real>
Real MexHatWaveFunc(const Real & z){
  return (1.00 - z*z) * std::exp(z*z/2.0);
}

//
// Tensor wavelet transform
// for a vector
//
template<typename Real>
void WaveTensorTransform(Real z){

}
