#include <cmath>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include "../include/rickerKernel.hpp"

using namespace std;
// Compile with:
// g++ -std=c++17 -O3 -g -o main exampleModel.cpp
//

//
//Fixed width matrix
//
template<typename T, unsigned N>
using  FWMat = vector<array<T,N>>;

//
// Update the Coefficients
// in a series
//
template<typename Real, unsigned N>
void Update_series(Real & Sig2, array<Real,N> & sig)
{
  for(int I=1; I<N; I++) sig[N-I]  = sig[N-1-I];
  sig[0]  = Sig2;
};


//
// Calculate the mean of a series
//
template<typename Real, unsigned N>
Real Calc_mean(array<Real,N> & sig)
{
  Real mean=0.00;
  for(int I=1; I<N; I++) mean += sig[I];
  mean *= Real(1.00)/Real(N);
  return mean;
};

//
// Calculate the unconditional
// volatility
//
template<typename Real, unsigned M, unsigned N>
Real GARCH_volatility2(Real & mu
                     , array<Real,M> & epsi
                     , array<Real,M> & beta
                     , array<Real,N> & sig
                     , array<Real,N> & alpha)
{
  Real Sig2 = mu;
  for(unsigned I=0; I<M; I++) Sig2 = Sig2 + epsi[I]*beta[I];
  for(unsigned I=0; I<N; I++) Sig2 = Sig2 + sig[I]*alpha[I];
  return Sig2;
};

//
// Generate random number
// using a linear congruential
// generators
//
uint32_t randqd_uint32(uint32_t rqd_seed){    
    rqd_seed = (uint32_t) (1664525UL * rqd_seed + 1013904223UL);
    return rqd_seed;
};

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
// Main function
//
int main(){
  const unsigned N=4, M=4;
  uint32_t rqd_seed = 0UL;
  double mu = 0.9;
  array<double,M> epsi, beta;
  array<double,N> sig, alpha, Rt_s;
  FWMat<double, M> epsilon;
  FWMat<double, N> Sigma;
  ofstream OutFile;                 //The output file
  OutFile.open("randDistTest.dat"); //The random dist tests

  beta[0]=0.30; alpha[0]=0.30;
  beta[1]=0.15; alpha[1]=0.15;
  beta[2]=0.05; alpha[2]=0.05;
  beta[3]=0.30; alpha[3]=0.30;

  sig[0]=0.010; epsi[0]=0.3;
  sig[1]=0.050; epsi[1]=0.0001;
  sig[2]=0.005; epsi[2]=0.05;
  sig[3]=0.010; epsi[3]=0.3;

  Rt_s[0] = mu;
  Rt_s[1] = mu+0.1;
  Rt_s[2] = mu-0.2;
  Rt_s[3] = mu-0.5;


  for(int I=0; I< 1000; I++){
    rqd_seed  = randqd_uint32(rqd_seed);    //caclulate random number
    double x  = rqd_seed/ double(1L << 32); //Calc   x
    double Sig2 = GARCH_volatility2<double,M,N>(mu, epsi, beta, sig, alpha);
    double z  = NormalDist(x, Sig2, mu);    //Calc   z
    double Epsi2 = Sig2 * z * z;
    double Rt = mu + sqrt(Sig2)*z;
    Update_series<double,N>(Sig2, sig);
    Update_series<double,M>(Epsi2, epsi);
    Update_series<double,N>(Rt, Rt_s);
    mu = Calc_mean<double,N>(Rt_s);

    OutFile << setw(16) << I
            << setw(16) << x
            << setw(16) << z*z
            << setw(16) << Sig2
            << setw(16) << Epsi2
            << setw(16) << Rt << endl;
  }
  OutFile.close();
  return 0;
};
