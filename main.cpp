////#include "include/globalMacros.hpp"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>
#include "include/webSocket.hpp"
#include "include/JsonParser.hpp"

// Compile with:
// g++ -std=c++17 -O3 -g -o main main.cpp -lOpenCL -lssl -lcrypto
//
using namespace std;


int main(){
/*
curl --request GET 'https://data.alpaca.markets/v1beta3/crypto/us/latest/orderbooks?symbols=BTC/USD,ETH/USD,SOL/USD'
curl --request GET --url 'https://data.alpaca.markets/v1beta3/crypto/us/latest/bars?symbols=BTC%2FUSD%2CLTC%2FUSD' \
     --header 'accept: application/json'
curl --request GET 'https://data.alpaca.markets/v1beta3/crypto/us/latest/orderbooks?symbols=BTC/USD' \
--header 'Apca-Api-Key-Id: PKZLV5L2XPYAFM5HVTLO' \
--header 'Apca-Api-Secret-Key: 4zLDE7vbS1ABhNWlFNCJLgdeqsXyp45f3va0q8AO'

*/

//  const std::string path = "/v1beta3/crypto/us";
//  const std::string host = "stream.data.alpaca.markets";
//  const std::string port = "443"; // Default port for wss

//const string path = "/v1beta3/crypto/us/latest/bars?symbols=BTC%2FUSD";
  const string path = "/v1beta3/crypto/us/latest/orderbooks?symbols=BTC/USD";
  const string host = "data.alpaca.markets";
  const string port = "443";
  const string sdType = "application/json";// "*/*"
  const string ConnStatus = "close";//"keep-alive";//
  string response;
//  string fName = "ARandledger";
//  DeviceHandler     Dhandler;
//  kernelExecutor<DeviceHandler>    executor(&Dhandler);
//  TransactionLedger<DummyIOSocket> TLedger(fName);
//  cout << "Total devices : " << Dhandler.Get_Total_NDevs()  << endl;
//  cout << "Total Cores   : " << Dhandler.Get_Total_NCores() << endl;

  // Formulate the HTTP GET request
  std::string request = "GET "        + path      +" HTTP/1.1\r\n"
                        "Host: "      + host      +         "\r\n"
                        "User-Agent: MyApp (C++ HTTPS Client)\r\n"
                        "Accept: "    + sdType    +         "\r\n"
                        "Connection: "+ ConnStatus+     "\r\n\r\n";

  //Setup the client side
  //web socket
  //
  clientWebSocketIO<double> webData(path,host,port);
//  webData.readDatablock(response, request);
//  Document document;
//  document.Parse(response.c_str());

  //Dissecting orderbook
  //documents
  //
//  Value &orderbook = document["orderbooks"];
//  assert(document["orderbooks"].IsObject());
//  assert(document["bars"].IsObject());
//  Value &bars = document["bars"];
//  assert(bars["BTC/USD"].IsObject());
//  double cVal = BTCUSDbars["l"].GetDouble();
  return 0;
}
