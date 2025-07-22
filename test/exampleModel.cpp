#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>


using namespace std;
// Compile with:
// g++ -std=c++17 -O3 -g -o main main.cpp
//


//
// Generate random number
// using a linear congruential
// generators
//
template<typename INT>
INT LCG_RNG(INT Seed, INT A, INT C, INT M){
  return (A*Seed + C)%M;
};

//
// Generate a simple 
// sample from a normal
// distribution
//
template<typename Real>
Real NormalDist(Real x, Real Sig2, Real mu){
  Real PI = Real(3.1415926535897932384626433832795028841971);
  Real a  = 1.00/sqrt(2*PI*Sig2);
  Real b  = -(x - mu)*(x - mu)/(2.00*Sig2);
  return a*exp(b);
};

template<typename Real>
void MetropolisHastings(){
  //Initialise


  //Iterate
  for(int I=0; I<; I++){
//    randVecGen(x_dash);  
//    Real alpha = Func(x_dash);

  };
};

int main(){
  int Seed = 3;
  int M = 2147483648; //2^31
  int A = 22695477;
  int C = 1;
  double Sig2 = 0.03, mu = 0.6;


  for(int I=0; I< 1000; I++){
    Seed = LCG_RNG<int> (Seed, A, C, M);
    double x = double(Seed);
    double z = NormalDist(x, Sig2, mu);
    cout << setw(10) << I
         << setw(10) << x
         << setw(10) << z << endl;
  }

  return 0;
};
























