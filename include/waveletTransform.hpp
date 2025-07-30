#pragma once
#include <cmath>
#include <array>
#include <vector>

//
// Definition of a 
// fixed width Matrix
//
template<typename Num, unsigned N>
using FWMatrix = std::vector<std::array<Num, N>>;

//
// Definition of a
// Vector
//
template<typename Num, unsigned N>
using Vector = std::vector<Num>;

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
template<typename Real, unsigned N>
void WaveTensorTransform(const FWMatrix<Real,N> & Z_ij, Vector<Real> & Psi_j){
  for(int J=0; J<Psi_j.size(); J++){
    Psi_j[J] = 1.0;
    for(int I=0; I<N; I++){
      Psi_j[J] *= MexHatWaveFunc(Z_ij[I][J]);
    }
  }
};
