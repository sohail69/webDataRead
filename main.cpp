////#include "include/globalMacros.hpp"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>
#include "include/webSocket.hpp"

// Compile with:
// g++ -std=c++17 -O3 -g -o main main.cpp -lOpenCL -lssl -lcrypto
//
using namespace std;


int main(){
//////curl --request GET 'https://data.alpaca.markets/v1beta3/crypto/us/latest/orderbooks?symbols=BTC/USD,ETH/USD,SOL/USD'
//////curl --request GET --url 'https://data.alpaca.markets/v1beta3/crypto/us/latest/bars?symbols=BTC%2FUSD%2CLTC%2FUSD' \
     --header 'accept: application/json'

//  const std::string path = "/v1beta3/crypto/us";
//  const std::string host = "stream.data.alpaca.markets";
//  const std::string port = "443"; // Default port for wss

  const string path = "/v1beta3/crypto/us/latest/bars?symbols=BTC%2FUSD";
  const string host = "data.alpaca.markets";
  const string port = "443";
  const string sdType = "application/json";// "*/*"
  const string ConnStatus = "close";//"keep-alive";
  string response;
//  string fName = "ARandledger";
//  DeviceHandler     Dhandler;
//  kernelExecutor<DeviceHandler>    executor(&Dhandler);
//  TransactionLedger<DummyIOSocket> TLedger(fName);




    // Formulate the HTTP GET request
    std::string request = "GET " + path + " HTTP/1.1\r\n"
                          "Host: " + host + "\r\n"
                          "User-Agent: MyApp (C++ HTTPS Client)\r\n"
                          "Accept: "+sdType+"\r\n"
                          "Connection: "+ConnStatus+"\r\n\r\n";

    std::cout << request << std::endl;
    std::cout << std::endl;



  clientWebSocketIO<double> webData(path,host,port);
  webData.readDatablock(response, request);
  Document document;
  document.Parse(response.c_str());

  std::cout << response << std::endl;
  std::cout << std::endl;
//  cout << "Total devices : " << Dhandler.Get_Total_NDevs()  << endl;
//  cout << "Total Cores   : " << Dhandler.Get_Total_NCores() << endl;
//  string body;
//  for(int I=0; I<5; I++) webData.readDatablock(body, true);

  // Dissecting the document
  //
  assert(document["bars"].IsObject());
  Value &bars = document["bars"];
  assert(bars["BTC/USD"].IsObject());

/*
  std::vector<std::string> DataNames;
  DataNames.push_back();
  DataNames.push_back();
  DataNames.push_back();
  DataNames.push_back();
*/

  Value &BTCUSDbars = bars["BTC/USD"];
  assert(BTCUSDbars["l"].IsDouble());
  assert(BTCUSDbars["t"].IsString());

  uint64_t cVal = BTCUSDbars["l"].GetDouble();
  std::cout << std::setw(5) << "l : " << std::setw(15) << std::setprecision(15) << cVal << std::endl;
  std::cout << "Hello "<< std::endl;

/*
{"bars":{"BTC/USD":{"c":108107.1635,"h":108107.1635,"l":108107.1635,"n":0,"o":108107.1635,"t":"2025-07-08T05:43:00Z","v":0,"vw":108107.1635}}}
*/
  return 0;
}












