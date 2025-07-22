#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>


using namespace std;
// Compile with:
// g++ -std=c++17 -O3 -g -o main main.cpp
//


//
// Generate 
//
//


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

  for(int I=0; I< 1000; I++){

  }

  return 0;
};
