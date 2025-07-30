#include <cmath>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include "../include/modelGARCH.hpp"
#include "../include/RNGs.hpp"

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
// Main function
//
int main(){
  const unsigned N=2, M=2;
  uint32_t rqd_seed = 0UL;
  double mu = 0.9;
  array<double,M> epsi, beta;
  array<double,N> sig, alpha, Rt_s;
  FWMat<double, M> epsilon;
  FWMat<double, N> Sigma;
  ofstream OutFile;                 //The output file
  OutFile.open("randDistTest.dat"); //The random dist tests

  beta[0]=0.49; alpha[0]=0.01;
  beta[1]=0.15; alpha[1]=0.15;

  sig[0]=0.99; epsi[0]=0.3;
  sig[1]=0.80; epsi[1]=0.1;

  Rt_s[0] = mu;
  Rt_s[1] = mu+0.1;


  for(int I=0; I< 500; I++){
    rqd_seed  = randqd_uint32(rqd_seed);    //caclulate random number
    double x  = rqd_seed/ double(1L << 32); //Calc   x
    double Sig2 = GARCH_volatility2<double,M,N>(mu, epsi, beta, sig, alpha);
    double z  = NormalDist(x, Sig2, mu);    //Calc   z
    double Epsi2 = Sig2 * z * z;
    double Rt = mu + sqrt(Sig2)*z;
    Update_series<double,N>(Sig2, sig);
    Update_series<double,M>(Epsi2, epsi);
    Update_series<double,N>(Rt, Rt_s);
//    mu = Calc_mean<double,N>(Rt_s);

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
