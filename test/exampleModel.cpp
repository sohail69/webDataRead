#include <cmath>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <vector>
#include <array>

using namespace std;
// Compile with:
// g++ -std=c++17 -O3 -g -o main exampleModel.cpp
//

//Fixed width matrix
template<typename T, unsigned N>
using  FWMat = vector<array<T,N>>;

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
  uint32_t rqd_seed = 0UL;
  double Sig2 = 0.05, mu = 0.5;
  FWMat<double, 10> epsilon, Sigma;
  ofstream OutFile;    //The output file
  OutFile.open("randDistTest.dat"); //The random dist tests

  for(int I=0; I< 500; I++){
    rqd_seed  = randqd_uint32(rqd_seed);    //sample x
    double x  = rqd_seed/ double(1L << 32); //Calc   x
    double z  = NormalDist(x, Sig2, mu);    //Calc   z
    double Rt = mu + Sig2*z;

    OutFile << setw(16) << I
            << setw(16) << x
            << setw(16) << z
            << setw(16) << Rt << endl;
  }
  OutFile.close();
  return 0;
};
























