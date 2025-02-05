/**
 * Obd2Reader 
 * Copyright (c) Dixtone @2025. All rights reserved.
 * Using stand alone Can Controller SN65HVD230
 * Based on Can library from Sandeep Mistry
 * Licensed under the MIT license. See LICENSE file in the project root for full license information.
 */

#include "Can.h"
#include "OBD2.h"

using namespace std;

void OBD2::Begin(int ctxPin, int crxPin, long baudrate){
  
    _ctxPin = ctxPin;
    _crxPin = crxPin;
    _baudrate = baudrate;

    if(OBD2_DEBUG)
    {
        Serial.print("Initializing CANBUS @baudrate ");
        Serial.print(baudrate);
        Serial.print("...");
    }     

    CAN.setPins(crxPin, ctxPin);

    if (!CAN.begin(baudrate)) { // 1000E3 500E3

    if(OBD2_DEBUG)
        Serial.println("[KO]"); 
        while (1);
    }

    if(_handleInterrupt)
    {
        CAN.onReceive(this);
    }

    if(OBD2_DEBUG)
        Serial.println("[OK]");

    status = OBD2StatusType::ready;    
}

void OBD2::addBroadcastFilter(long filter){

  if(_nbroadcastfilters < _maxbroadcastfilters)
  {
    _broadcastfilters[_nbroadcastfilters] = filter;
    _nbroadcastfilters++; 

    if(OBD2_DEBUG)
    { 
      Serial.print("Added listen filter: ");
      Serial.println(filter, HEX);
    } 
  }
  
}
void OBD2::addPacketFilter(long filter){

  if(_nfilters < _maxfilters)
  {
    _filters[_nfilters] = filter;
    _nfilters++; 

    if(OBD2_DEBUG)
    { 
      Serial.print("Added packet filter: ");
      Serial.println(filter, HEX);
    } 
  }
  
}


bool OBD2::sendRequest(OBD2Request* request){

  if(status==OBD2StatusType::ready)
  {    
    if(_isElm)
    {
        return sendElmRequest(request);
    }
    
    //flush
    if(CAN.available()){
      while(CAN.available()){
        CAN.read();
      }
    }

    _flush();
    
    status=OBD2StatusType::sending;   

    _currentRequest = request;

    _sendRequestTime = millis();

    _responseMultiFrames = false;

    _requestPacketId = _currentRequest->Header;
    _requestService = _currentRequest->Service;
    _requestPid = _currentRequest->Pid;
    _responseReadedBytes = 0;
    _responseFrameBytes = 0;

    //29bit request 
    if(_currentRequest->Pid > 0xFF)
    {
      CAN.beginExtendedPacket(_currentRequest->Header, 8);
      CAN.write(0x03); // number of additional bytes
      CAN.write(_currentRequest->Service); //service 
      CAN.write(_currentRequest->Pid>>8); //if 1003 then 10
      CAN.write(_currentRequest->Pid & 0x00ff); //if 1003 then 03   
      CAN.endPacket();
    }
    else{
      CAN.beginPacket(_currentRequest->Header, 8);
      CAN.write(0x02); // number of additional bytes
      CAN.write(_currentRequest->Service);
      CAN.write(_currentRequest->Pid); //if 1003 then 10
      CAN.endPacket();
    }
    
    return true;
  }
  
  return false;

}

void OBD2::callListener(OBD2Request* request, float value, uint8_t* responseBytes){
  if(_currentRequest!=NULL)
  {
      if(_valueListener!=NULL) _valueListener->onOBD2Response(request, value, responseBytes); //listener method

      if(_callBackFunction!=NULL) _callBackFunction(request, value, responseBytes); //listener function
  }
}

//need to call in loop everytime
OBD2StatusType OBD2::process(){
  
  if(status==OBD2StatusType::sending){
      
      if(!_isElm)
      {
          //if we dont have interrupt we read manually
          if(!_handleInterrupt)
          {
              int packetSize = CAN.parsePacket();
              if(packetSize)
              {
                onReceivePacket(packetSize);
              }
          }      
      }
      
      checkTimeoutRequest();
  }
  else if(status==OBD2StatusType::hadling){

      checkTimeoutRequest();

      if(_isElm)
      {
        getElmResponse();
      }
      else{
        getResponse();
      }
      
  }
  else if(status==OBD2StatusType::received){

      callListener(_currentRequest, getValue(_currentRequest), _responseBytes);
      
      flushRequest();
      status = OBD2StatusType::ready;
      
  }
  else if(status==OBD2StatusType::timeout){
      //after a bit we return in ready state
      if(millis()-_sendRequestTime > _requestTimeout)
      {
        callListener(_currentRequest, 0.0, _responseBytes);
        _flush();

        status = OBD2StatusType::ready;             
      }     
  }
  else if(status==OBD2StatusType::nodata){
      //after a bit we return in ready state
      if(millis()-_sendRequestTime > _requestTimeout)
      {
        callListener(_currentRequest, 0.0, _responseBytes);
        _flush();

        status = OBD2StatusType::ready;
      }     
  }
  else if(status==OBD2StatusType::error){
      //after a bit we return in ready state
      if(millis()-_sendRequestTime > _requestTimeout)
      {
        callListener(_currentRequest, 0.0, _responseBytes);
        _flush();

        status = OBD2StatusType::ready;
      }     
  }

  return status;

}

float OBD2::getValue(OBD2Request* request){

    int n = request->ExpectedBytes-1;

    /*float v =  (float)
    ( n>=0?(_responseBytes[0]<<(8*n)):0 
       | (n-1)>=0?(_responseBytes[1]<<(8*(n-1))):0
       | (n-2)>=0?(_responseBytes[2]<<(8*(n-2))):0
       | (n-3)>=0?(_responseBytes[3]<<(8*(n-3))):0
    );*/

    float v = 0.00;
    uint8_t bitShift;

    for (uint8_t i = 0; i < request->ExpectedBytes; i++)
    {
        bitShift = 8 * (request->ExpectedBytes - i - 1);
        v = v + ((_responseBytes[i]) << bitShift);
    }

    return v>0.00?(v*request->ScaleFactor + request->AdjustFactor):0.00; 
}

uint8_t* OBD2::getResponseBytes(){
  return _responseBytes;
}

void OBD2::flush(){

  status = OBD2StatusType::ready;
  _flush();
}

void OBD2::_flush(){
  _requestService = 0;
  _requestPid = 0;
  _responseService = 0;
  _responsePid = 0;
  flushResponseBytes();
  flushRequest();
}

void OBD2::checkTimeoutRequest(){

  if(millis()-_sendRequestTime > _requestTimeout){
    _sendRequestTime = millis();
    status=OBD2StatusType::timeout;
  }
}

void OBD2::flowControl(long packetId){
  CAN.beginExtendedPacket(packetId);
  CAN.write(0x30);
  CAN.write(0x0);
  CAN.write(0x0);
  CAN.write(0x0);
  CAN.write(0x0);
  CAN.write(0x0);
  CAN.write(0x0);
  CAN.write(0x0);
  CAN.endPacket();    
}

void OBD2::getResponse(){

  if(status!=OBD2StatusType::hadling)
  {  
    status=OBD2StatusType::nodata;
    return;
  }

  if(_responseService != _requestService  || _responsePid != _requestPid)
  {  
    status=OBD2StatusType::nodata;
    return;
  }

  /*
  Serial.print("Packet id "+String(_responseMultiFrames?"(multiframe)":"(single frame) "));
  Serial.print(_responsePacketId, HEX);
  Serial.printf(" Req: %2x Read: %2x\n", _responseFrameBytes, _responseReadedBytes);
  */

  //until we read all bytes
  if(_responseReadedBytes<_responseFrameBytes)
  {
    if(_responsePacketId>0 && _responseMultiFrames)
    {
      if(millis() - _sendRequestTime > _consecutiveFrameTimeout)
      {
          if(OBD2_DEBUG)
            Serial.println("Sending flow control");

          _sendRequestTime = millis();
          flowControl(_requestPacketId);
          status=OBD2StatusType::sending;
      }
    }  
  }
  else{
      //read complete
      status=OBD2StatusType::received;
      
      if(OBD2_DEBUG)
        Serial.printf("\nRequest Complete!  bytes: %02x readed: %02x\n", _responseFrameBytes, _responseReadedBytes);
           
  }
}
void OBD2::flushRequest(){

   flushBuffer();
    _requestPacketId = 0;
    _requestService = 0;
    _requestPid = 0;
    _responsePCI = 0;
    _responseFrameBytes = 0;
    _currentRequest = nullptr;
}

uint8_t OBD2::getResponseByte(int index){
  return _responseBytes[index];
}

void OBD2::flushBuffer(){
  for(int i=0;i<8;i++){
    _canbuffer[i] = 0x0;
  }
}

void OBD2::flushResponseBytes(){
  for(int i=0;i<64;i++){
    _responseBytes[i] = 0x0;
    _responseElmBytes[i] = 0x0;
  }
}

void OBD2::handleBroadcastPackets(long packetId){

  _broadcastPacket.Header = packetId;
  int index = 0;
  //edit: no DLC!
  //if(CAN.available()) CAN.read(); //read first byte DLC
  while (CAN.available()) {
    _canbuffer[index] = CAN.read();
    index++;
  }
  _broadcastPacket.Byte0 = _canbuffer[0];
  _broadcastPacket.Byte1 = _canbuffer[1];
  _broadcastPacket.Byte2 = _canbuffer[2];
  _broadcastPacket.Byte3 = _canbuffer[3];
  _broadcastPacket.Byte4 = _canbuffer[4];
  _broadcastPacket.Byte5 = _canbuffer[5];
  _broadcastPacket.Byte6 = _canbuffer[6];
  _broadcastPacket.Byte7 = _canbuffer[7];
  
  flushBuffer();
}

//called on interrupt internal callback to process raw data
void OBD2::onReceivePacket(int packetSize){

   _responsePacketId = CAN.packetId();

  //check if we have listen filters
  if(_nbroadcastfilters > 0)    
  {
     bool listen = find(_broadcastfilters, _broadcastfilters+_nbroadcastfilters, _responsePacketId) != ( _broadcastfilters+_nbroadcastfilters);
     if(listen)
     {
       return handleBroadcastPackets(_responsePacketId);
     }
  }   

  //if we have filters, we have to check if packet is included
  if(_nfilters > 0)
  { 
    bool accepted = find(_filters, _filters+_nfilters, _responsePacketId) != ( _filters+_nfilters);  
    if(!accepted) return;   
  }

  flushBuffer();

  if(OBD2_DEBUG)
  {
      Serial.println("Received ");

   /*   if (CAN.packetExtended()) {
        Serial.print("extended ");
      }

      if (CAN.packetRtr()) {
        // Remote transmission request, packet contains no data
        Serial.print("RTR ");
      }

      Serial.print("packet with id 0x");*/
      Serial.print(_responsePacketId, HEX);
  }   

  if (CAN.packetRtr()) {
    if(OBD2_DEBUG)
    {
      Serial.print(" and requested length ");
      Serial.println(CAN.packetDlc());
    }
  } else {
    
    if(OBD2_DEBUG)
    {
      Serial.print(" and length ");
      Serial.println(packetSize);
    }
    
    if(packetSize>0)
    {
      _responseMultiFrames = false;
      
      int index, dataindex = 0;

      while (CAN.available()) {
        _canbuffer[index] = CAN.read();
        index++;
      }
      
      //multiframe response example: 10 0B 6240A4020103  
      if(_canbuffer[0] > 8) //example: 0x10, 0x21 - 0x2F
      {
        _responseMultiFrames = true;
        _responsePCI = _canbuffer[0];
        _consecutiveFrameSendRequestTime = millis();

        //only for firstframe we get service and pid
        if(_canbuffer[0]==0x10)
        {
           _responseDataBytes = 0;
           _responseFrameBytes = _canbuffer[1];
           _responseService = _canbuffer[2]-0x40;   //_responseService xor 40 return original service request
           _responseReadedBytes = 1;
          
           if(CAN.packetExtended())
           {
              _responsePid = (_canbuffer[3]<<8)|(_canbuffer[4]);
              _responseReadedBytes+=2;  
              dataindex = 5;
           }
           else{
               _responsePid = _canbuffer[3];
               _responseReadedBytes+=1;  
               dataindex = 4;
           }

           flushResponseBytes();            
        }else{
          dataindex = 1;
        }
      }
      //single frame example: 046240A45F 
      else{ //single response
     
        dataindex = 0;
        _responseDataBytes = 0;
        _responseReadedBytes = 1;
        _responsePCI = 0;
        _responseFrameBytes = _canbuffer[0];     
        _responseService = _canbuffer[1]-0x40;  //_responseService xor 40 return original service request
        if(CAN.packetExtended())
        {
          _responsePid = (_canbuffer[2]<<8)|(_canbuffer[3]);
          _responseReadedBytes+=2;  
          dataindex = 4;
        }
        else{
          _responsePid = _canbuffer[2];
          _responseReadedBytes+=1;  
          dataindex = 3;
        } 
       
        //Serial.printf("\nCan buffer s:%02x pid:%04x size %d: %02x %02x %02x %02x %02x %02x %02x %02x",_responseService, _responsePid,  packetSize,  _canbuffer[0], _canbuffer[1], _canbuffer[2], _canbuffer[3], _canbuffer[4], _canbuffer[5], _canbuffer[6], _canbuffer[7]);
       
        flushResponseBytes(); 
      }      
      
      for(int d=dataindex;d<packetSize;d++)
      {
        _responseBytes[_responseDataBytes] = _canbuffer[d];
        _responseDataBytes++;
        _responseReadedBytes++;
      }

      status = OBD2StatusType::hadling;
    }    
  }  
}

//elmIntegration
bool OBD2::BeginElm327(Stream& stream, long timeout){

  _isElm = true;
  _elmPort = &stream;
  _elmTimeout = timeout;

  if(!_elmPort) return false;

  status = OBD2StatusType::ready;

  return initializeELM();
}

bool OBD2::initializeELM(){

  bool c1 = sendElmCommandBlocking("AT D");
	delay(100);

  bool c2 = sendElmCommandBlocking("AT Z");
	delay(100);
  
  return c1&&c2;
}

void OBD2::sendElmHeader(long header){

  if(status== OBD2StatusType::ready)
  {
      //if header is of type 0x18XXYYZZ we convert into string XXYYZZ
      String _header = String(header,HEX);
      if(_header.length()>6)
      {
        _header = _header.substring(2);
      }
      
      String cmd = String("AT SH ")+_header;
      sendElmCommandBlocking(cmd);     
  }
 
}

bool OBD2::sendElmCommandBlocking(String cmd){

  if(status== OBD2StatusType::ready)
  {
      sendElmCommand(cmd);

      while (!getElmResponse());

      if(OBD2_DEBUG)
      {
         Serial.println("Query for "+cmd+" completed with status: "+String((int)status));
      }

      if(status== OBD2StatusType::received)
      {
        status= OBD2StatusType::ready;
        return true;
      }      
  }
  
  return false;
}

void OBD2::sendElmCommand(String cmd){

  if(status != OBD2StatusType::ready)
    return;


  //flush port buffer
  while(_elmPort->available())
  {
    _elmPort->read();
  }
    
  status = OBD2StatusType::sending;

  if(OBD2_DEBUG)
    Serial.println("Sending: "+cmd);

  flushBuffer();
  flushResponseBytes();
  _currentRequest = nullptr;
  _responseReadedBytes = 0;
  _elmBuffer = "";
  _elmPort->print(cmd);
  _elmPort->print('\r');
  _sendRequestTime = millis();    
  status=OBD2StatusType::hadling;
}

bool OBD2::getElmResponse(){

  if(status!=OBD2StatusType::hadling)
  {  
    status=OBD2StatusType::nodata;
    return true;
  }

  if(!_elmPort->available())
  {
    status = OBD2StatusType::hadling;
    checkTimeoutRequest();
  }
  else{

      char recChar = _elmPort->read();
      if (recChar == '>')
      {
          if(OBD2_DEBUG)
            Serial.println("Elm response complete.");

          status = OBD2StatusType::received;
      }
      else if (!isalnum(recChar) && (recChar != ':') && (recChar != '.'))
      {
          status = OBD2StatusType::hadling;
          return false;
      }
      else{
          _elmBuffer += (char)recChar;
          status = OBD2StatusType::hadling;
          _responseReadedBytes++;
          return false;
      }

      if(status == OBD2StatusType::received)
      {
          if(OBD2_DEBUG)
          {
              Serial.print("ELM327 response: ");
              Serial.println(_elmBuffer);
          }

          decodeElmResponse();

          //check if response has errors or no data
          std::string r = _elmBuffer.c_str();
          if(r.find("UNABLETOCONNECT")!=std::string::npos)
          {
              if(OBD2_DEBUG)
                Serial.println("ELM327 ERROR: UNABLE TO CONNECT");

              status = OBD2StatusType::error;
          }
          if(r.find("NODATA")!=std::string::npos)
          {
              if(OBD2_DEBUG)
                Serial.println("ELM327 ERROR: NO DATA");

              status = OBD2StatusType::nodata;
          }
          if(r.find("STOPPED")!=std::string::npos)
          {
              if(OBD2_DEBUG)
                Serial.println("ELM327 ERROR: STOPPED");

              status = OBD2StatusType::error;
          }
          if(r.find("ERROR")!=std::string::npos)
          {
              if(OBD2_DEBUG)
                Serial.println("ELM327 generic ERROR");

              status = OBD2StatusType::error;
          }
      }
            
      return true;
  }

  return false;
}

void OBD2::decodeElmResponse(){

  char byte[2];
  _responseReadedBytes = 0;
  _responseFrameBytes = 0;
  _responseMultiFrames = false;

  if(_currentRequest!=NULL)
  {
 
    if(_currentRequest->Header >0x0 && _currentRequest->Pid > 0x0)
    {
      //check if multiframe response
      //ex: 00B0:6201020000021:0100000000
      std::string ck = _elmBuffer.c_str();
      _responsePCI = std::count(ck.begin(), ck.end(), ':');
      if(_responsePCI > 0)
      {
         _responseMultiFrames = true;
         _responseFrameBytes = strtol(_elmBuffer.substring(0, ck.find(":")-1).c_str(), NULL, 16);

          if(OBD2_DEBUG)
          {
            Serial.print("Found multiple frame response with number of bytes: ");
            Serial.println(_responseFrameBytes, HEX);
          }

          //clean buffer bytes from pci 
          _elmBuffer = _elmBuffer.substring(ck.find(":")+1);
          ck = _elmBuffer.c_str();

          while(ck.find(":")!=std::string::npos){
             _elmBuffer = _elmBuffer.substring(0, ck.find(":")-1) + _elmBuffer.substring(ck.find(":")+1);
             ck = _elmBuffer.c_str();
          }   

          if(OBD2_DEBUG)
          {
              Serial.print("Cleaned buffer: ");
              Serial.println(_elmBuffer);
          }
      }
      else{
          if(OBD2_DEBUG)
          {
              Serial.print("Cleaned buffer: ");
              Serial.println(_elmBuffer);
          }
      }

      //convert response string to hexbytes
      for(int i=0;i<_elmBuffer.length();i++){
        strcpy(byte, String(_elmBuffer[i]).c_str());
        strcat(byte, String(_elmBuffer[i+1]).c_str());
        _responseElmBytes[_responseReadedBytes] = strtol(byte, NULL, 16);
        _responseReadedBytes++;
        i++;
      }

      //check if match request
      if(_responseElmBytes[0] > 0)
      {
         _responseReadedBytes = 1;
         _responseService = _responseElmBytes[0]-0x40;  //_responseService xor 40 return original service request
         
        if(_currentRequest->Pid > 0xFF){
          _responsePid = (_responseElmBytes[1]<<8)|(_responseElmBytes[2]);
          _responseReadedBytes+=2;
        }
        else{
          _responsePid = _responseElmBytes[1];
          _responseReadedBytes+=1;
        }

        _responseFrameBytes = _responseMultiFrames? _responseFrameBytes :(_responseReadedBytes+_currentRequest->ExpectedBytes);
  
        if(_currentRequest->Service == _responseService && _currentRequest->Pid == _responsePid)
        {
          //if(OBD2_DEBUG)
           // Serial.printf("\nResponse service: %2x Response Pid: %4x \n\nBYTES: ", _responseService, _responsePid);
            
            _responseDataBytes = 0;           
            for(int i= _responseReadedBytes;i<_responseFrameBytes;i++){
              _responseBytes[_responseDataBytes] = _responseElmBytes[i];
              //Serial.print(_responseBytes[_responseDataBytes], HEX);
              //Serial.print(" ");
              _responseDataBytes++;
            }
        }
        else{
          if(OBD2_DEBUG)
            Serial.printf("\nRequest service: %2x  and Request Pid: %4x not matches response: %2x %4x\n", _currentRequest->Service, _currentRequest->Pid, _responseService, _responsePid);
          status = OBD2StatusType::nodata;
        }

      }
      else{
        status = OBD2StatusType::nodata;
      }     
    }  
  }  
}


void OBD2::upper(char string[], uint8_t buflen)
{
	for (uint8_t i = 0; i < buflen; i++)
	{
		if (string[i] > 'Z')
			string[i] -= 32;
		else if ((string[i] > '9') && (string[i] < 'A'))
			string[i] += 7;
	}
}

bool OBD2::sendElmRequest(OBD2Request* request){
  
  if(status==OBD2StatusType::ready)
  {

    flushRequest();
    flushResponseBytes();

    _requestPacketId = request->Header;
    _requestService = request->Service;
    _requestPid = request->Pid;
    _responseReadedBytes = 0;
    _responseFrameBytes = 0;

    char query[8] = { '\0' }; 
    
    query[0] = ((_requestService >> 4) & 0xF) + '0';
	  query[1] = (_requestService & 0xF) + '0';

    //29bit request 
    if(request->Pid > 0xFF)
    {
      query[2] = ((_requestPid >> 12) & 0xF) + '0';
      query[3] = ((_requestPid >> 8) & 0xF) + '0';
      query[4] = ((_requestPid >> 4) & 0xF) + '0';
      query[5] = (_requestPid & 0xF) + '0';
      query[6] = request->ExpectedBytes + '0';
      query[7] = '\0';

      upper(query, 6);
    }
    else{
      query[2] = ((_requestPid >> 4) & 0xF) + '0';
      query[3] = (_requestPid & 0xF) + '0';
      query[4] = request->ExpectedBytes + '0';
      query[5] = '\0';

      upper(query, 4);
    }

    sendElmCommand(query);
    _currentRequest = request;
    return true;
  }
  
  return false;
}
