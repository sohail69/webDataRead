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

struct Quotes{
  //Stored Data
  vector<double> bid;
  vector<string> keys;
};

struct QuotesHandler{
  //Stored Data
  vector<double> bid;
  vector<string> keys;
  int ObjectDepth=0, MaxObjectDepth=0;

  //Null Data handler
  bool Null() { return true; }

  //Boolean Data handler
  bool Bool(bool b) { return true; }

  //32-Bit Integer Data handler 
  bool Int(int i) {  return true; }

  //32-Bit Unsigned Data handler
  bool Uint(unsigned u) { return true; }

  //64-Bit Integer Data handler
  bool Int64(int64_t i) { return true; }

  //64-Bit Unsigned Data handler
  bool Uint64(uint64_t u) { return true; }

  //Double precision float Data handler
  bool Double(double d) { return true; }

  //Raw number bit type Data handler
  bool RawNumber(const char* str, SizeType length, bool copy) { return true;}

  //String type Data handler
  bool String(const char* str, SizeType length, bool copy) {  return true;}

  //Object Key Value
  bool Key(const char* str, SizeType length, bool copy) {
    //cout << "Key(" << str << ", " << length << ", " << boolalpha << copy << ")" << endl;
    keys.push_back(str);
    return true;
  }

  //Object start Data handler
  bool StartObject(){
    cout << "StartObject()" << endl; 
    ObjectDepth=ObjectDepth+1;
    MaxObjectDepth = max(MaxObjectDepth,ObjectDepth);
    return true;
  }

  //Object end handler
  bool EndObject(SizeType memberCount) {
    cout << "EndObject(" << memberCount << ")" << endl;
    ObjectDepth=ObjectDepth-1;
    return true;
  }

  //Array start handler
  bool StartArray() { cout << "StartArray()" << endl; return true; }

  //Array end handler
  bool EndArray(SizeType elementCount) { cout << "EndArray(" << elementCount << ")" << endl; return true; }
};
/*

{"quotes":{"BTC/USD":{"ap":105243.44,"as":1.601,"bp":105089.5,"bs":1.60307,"t":"2025-06-15T12:09:56.052328511Z"}}}
*/


template<typename T>
class QuotesParser{
  


};



