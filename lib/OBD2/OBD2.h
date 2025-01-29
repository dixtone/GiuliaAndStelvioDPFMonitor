/**
 * Obd2Reader 
 * Copyright (c) Dixtone @2025. All rights reserved.
 * Using stand alone Can Controller SN65HVD230
 * Based on Can library from Sandeep Mistry
 * Licensed under the MIT license. See LICENSE file in the project root for full license information.
 */

#ifndef Obd2Reader_H
#define Obd2Reader_H

#ifndef OBD2_DEBUG
#define OBD2_DEBUG  false
#endif

#ifndef DEBUG_MODE
#define DEBUG_MODE  false
#endif


#include "CAN.h"

//define maxbuffer lenght for response bytes
#define OBD2_MAX_BUFFER_LENGTH 64

//PID Struct
struct OBD2Request {
  String Group;
  String Name;
  bool AlwaysSendHeader;
  long Header;
  uint8_t  Service;
  uint16_t Pid;
  uint8_t  ExpectedBytes;
  float  ScaleFactor;
  float  AdjustFactor;
  float  *BindValue;
  void (*ValueCallback)(String pid, uint8_t* responseBytes);
  long ReadInterval; 
  long ReadTime;
};

struct OBD2BroadcastPacket {
    long Header;
    uint8_t Byte0;
    uint8_t Byte1;
    uint8_t Byte2;
    uint8_t Byte3;
    uint8_t Byte4;
    uint8_t Byte5;
    uint8_t Byte6;
    uint8_t Byte7;
};

enum class OBD2StatusType {
    undefined,
    ready,
    sending,
    hadling,
    received,
    timeout,
    nodata,
    error
};

class IOBD2MessageListener{
    public:
        virtual ~IOBD2MessageListener(){}
        virtual void onOBD2Response(OBD2Request* request, float value, uint8_t* responseBytes){};
};
    
class OBD2: CANHandler
{
    public:
        OBD2(){};
        void Begin(int ctxPin, int crxPin, long baudrate= 500E3);
        bool sendRequest(OBD2Request* request);
        OBD2StatusType process();
        void onReceivePacket(int packetSize);
        void addPacketFilter(long filter);
        void addBroadcastFilter(long filter);

        // CallBack Method
        //void(*callback)(int)
        void onHandleValue(void (*obd2listenerfn)(OBD2Request* request, float value, uint8_t* responseBytes)){_callBackFunction = obd2listenerfn;}; //simple fn
        void onHandleValue(IOBD2MessageListener* instance){_valueListener = instance;}; //interface
        
        float getValue(OBD2Request* request);
        uint8_t  getResponseByte(int index);
        uint8_t  getResponseService(){ return _responseService;}
        uint16_t getResponsePid(){ return _responsePid;}
        OBD2BroadcastPacket getBroadcastPacket(){ return _broadcastPacket;}
        OBD2StatusType status = OBD2StatusType::undefined;  
        uint8_t* getResponseBytes();     
        void flush();

        //elm integration
        bool BeginElm327(Stream& stream,long timeout = 1000);
        void sendElmCommand(String cmd); 
        bool sendElmCommandBlocking(String cmd); 
        bool isELM327() {return _isElm;};
        void sendElmHeader(long header);
        

    private:
        int _sendRequestTime = 0;
        int _requestTimeout = 1000;
        int _consecutiveFrameSendRequestTime = 0;
        int _consecutiveFrameTimeout = 100;
        int _ctxPin;
        int _crxPin;
        long _baudrate; 
        int _nfilters = 0; //max 10 sw filters    
        int _maxfilters = 10; //max 10 sw filters      
        long _filters[10]; //max 10 sw filters
        int _nbroadcastfilters = 0; //max 10 sw filters    
        int _maxbroadcastfilters = 10; //max 10 sw filters      
        long _broadcastfilters[10]; //max 10 sw filters
        uint8_t _canbuffer[8];
        uint8_t _responseBytes[OBD2_MAX_BUFFER_LENGTH];
        bool _handleInterrupt = true;
        bool _responseMultiFrames;
        long _requestPacketId = 0;
        long _responsePacketId = 0;
        uint8_t _requestService = 0;
        uint8_t _responseService = 0;
        uint16_t _requestPid = 0;
        uint16_t _responsePid = 0;
        uint8_t _responsePCI = 0;
        uint8_t _responseFrameBytes = 0;
        uint8_t _responseReadedBytes = 0;
        uint8_t _responseDataBytes = 0;
        OBD2BroadcastPacket _broadcastPacket = {0,0,0,0,0,0,0,0,0};
        void checkTimeoutRequest();
        void flushRequest();
        void flushBuffer();
        void flushResponseBytes();
        void getResponse();
        void handleBroadcastPackets(long packetId);
        void flowControl(long packetId);
        void (*onReceiveCallback)();
        IOBD2MessageListener* _valueListener;
        void (*_callBackFunction)(OBD2Request* request, float value, uint8_t* responseBytes);
        void callListener(OBD2Request* request, float value, uint8_t* responseBytes);
        OBD2Request* _currentRequest; 

        //elm integration
        bool _isElm = false;
        bool _elmConnected = false;
        long _elmTimeout = 1000;
        String _elmBuffer;
        uint8_t _responseElmBytes[OBD2_MAX_BUFFER_LENGTH];
        Stream* _elmPort;
        bool initializeELM();
        bool getElmResponse();
        bool sendElmRequest(OBD2Request* request);
        void upper(char string[], uint8_t buflen);
        void decodeElmResponse();
        void _flush();
        
};

#endif