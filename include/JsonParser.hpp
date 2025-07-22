#pragma once
#include <map>
#include <array>
#include <tuple>
#include <cmath>
#include <vector>
#include <iostream>
#include "rapidjson/reader.h"
#include "rapidjson/document.h"

using namespace rapidjson;
using namespace std;
/***************************************\
!
!
! Breaks up a JSon Data string into a
! Hierarchical pair structure
!
!
\***************************************/
// Compile with:
// g++ -std=c++17 -O3 -g -o main main.cpp
//
template<typename T>
class QuotesParser{
  private:
    vector<array<int,2>> BidTimeSeries;

  public:



};
