#pragma once
#include <array>
#include <vector>
#include "RNGs.hpp"

//
// Generate a simple 
// sample from a normal
// distribution
//
template<typename Real>
Real NormalDist(Real x, Real Sig2, Real mu){
  Real PI = Real(3.1415926535897932384626433832795028841971);
  Real a  = 1.00/sqrt(2.00*PI*Sig2);
  Real b  = -(x - mu)*(x - mu)/(2.00*Sig2);
  return a*exp(b);
};

//
// GARCH(P,Q) model coefficients
//
template<typename Real, unsigned P, unsigned Q>
struct GARCH_coef{
  Real mu, Omega0;
  Real alpha[P], beta[Q];
};

//
// GARCH(P,Q) model Volatility
// data
//
template<typename Real, unsigned P, unsigned Q>
struct GARCH_data{
  Real sigma[P], epsilon[Q];
};


//
// Shift the time series
// by adding values
//
template<typename Real, unsigned P, unsigned Q>
void Shift_GARCH_series(const Real & Sig2, const Real & epsil2, GARCH_data<Real,P,Q> & data)
{
  for(int I=1; I<P; I++) data.sigma[P-I]   = data.sigma[P-1-I];
  for(int I=1; I<Q; I++) data.epsilon[Q-I] = data.epsilon[Q-1-I];
  data.sigma[0] = Sig2;
  data.epsilon[0] = epsil2;
};


//
// GARCH(P,Q) model predictor
//
template<typename Real, unsigned P, unsigned Q, unsigned T>
void GARCH_predictor(const GARCH_coef<Real,P,Q> & mod
                   , const GARCH_data<Real,P,Q> & VolInitDat
                   , Real Data[T]){

  //
  // Set the model data
  //
  GARCH_data<Real,P,Q> Voldat = VolInitDat;
  Real Sig2, epsil2, Rt;
  uint32_t rqd_seed = 0UL;

  //
  // Run GARCH model simulation
  // for T-time-steps
  //
  for(unsigned t=0; t<T; t++){
    //Sample a random number
    rqd_seed = randqd_uint32(rqd_seed);             //Calc random number
    Real x   = rqd_seed/Real(1L << 32);             //Calc x
    Real z   = NormalDist<Real>(x, Sig2, Voldat.mu);//Calc z

    //Calculate return rate
    Rt = Voldat.mu + sqrt(Sig2)*z;

    //Shift the GARCH coeffs
    Shift_GARCH_series(Sig2, epsil2, VolInitDat);
    

  }


};

















