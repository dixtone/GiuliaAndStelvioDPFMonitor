/**
 * BLE Serial
 * Copyright (c) Dixtone @2025. All rights reserved.
 * Commands Interpreter for BluetoothSerial
 * Licensed under the MIT license. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <Arduino.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include "IBLEsp32.h"
#include "BLEsp32Server.h"
#include "BLEsp32Client.h"

//nordic serial uart device
#define NORDIC_SERVICE_UUID "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define NORDIC_RX_UUID "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
#define NORDIC_TX_UUID "6e400003-b5a3-f393-e0a9-e50e24dcca9e"

#define BLEsp32MaxDeviceAdvertise 10

#define BLEsp32ServerDefaultMTU 8 //default ble string length

#define BLEBuffetTimeout 50

class BLEsp32: public IBLEsp32, public BLEAdvertisedDeviceCallbacks, public Stream
{

    public:
        BLEsp32(){};
        
        bool BeginServer(const char* name);
        bool BeginClient(const char* name);
        bool BeginServer(const char* name, const char *service_uuid, const char *rx_uuid, const char *tx_uuid);
        bool BeginClient(const char* name, const char *service_uuid, const char *rx_uuid, const char *tx_uuid);
        void Write(std::string buffer);   
        void WriteChars(std::string buffer);        
        bool ConnectTo(BLEAddress address);
        bool ConnectTo(BLEAdvertisedDevice* device);
        bool isConnected(){ if(_server!=nullptr) return _server->isConnected; if(_client!=nullptr) return _client->isConnected; return false; }
        void Disconnect(){
            if(_server!=nullptr) _server->Disconnect(); 
            if(_client!=nullptr) _client->Disconnect(); 
        };

        //server
        void onConnect(void (*connectlistener)(BLEsp32Server* server));
        void onDisconnect(void (*disconnectlistener)(BLEsp32Server* server));
        
        //client
        void onConnect(void (*connectlistener)(BLEsp32Client* client));
        void onDisconnect(void (*disconnectlistener)(BLEsp32Client* client));
        
        void onReceive(void (*onreceivelistener)(std::string buffer));

        BLEsp32Server* getServer() {return _server;};
        BLEsp32Client* getClient() {return _client;};

        //interface
        void setBuffer(std::string s) override;
        void pushBuffer(uint8_t* c) override;
        int shiftBuffer() override; 
        void flushBuffer() override; 
        std::string getBuffer() override;

        //scan devices
        void ScanForDevices(int timeout = 30);
        int  ScanResultCount(){ return _foundDevicesIndex+1;}
        BLEAdvertisedDevice* GetScanResults(){return _foundDevices;};

        //inherited
        void onResult(BLEAdvertisedDevice advertisedDevice); 

        //inherited stream
        int available() override;
    	int peek() override;
        int read() override;
        size_t readBytes(uint8_t* buffer, size_t bufferSize) override;
        size_t write(uint8_t byte) override;
        size_t write(const uint8_t* buffer, size_t bufferSize) override;
    	void flush() override;
        
       

    private:
        std::string _btbuffer;
        const char* _deviceName;
        char _serialServiceUuid[40] = NORDIC_SERVICE_UUID;
	    char _rxUuid[40] = NORDIC_RX_UUID;
    	char _txUuid[40] = NORDIC_TX_UUID;
        long _bufferMillis = 0;
        long _bufferTimeout = BLEBuffetTimeout;
        
        void (*_onReceivelistener)(std::string buffer) = nullptr;

        BLEsp32Server* _server = nullptr;
        void (*_onConnectServer)(BLEsp32Server* server) = nullptr;
        void (*_onDisconnectServer)(BLEsp32Server* server) = nullptr;
        
        BLEsp32Client* _client = nullptr;
        void (*_onConnectClient)(BLEsp32Client* client) = nullptr;
        void (*_onDisconnectClient)(BLEsp32Client* client) = nullptr;

        BLEScan* _BLEScan;
        int _foundDevicesIndex = -1;
        BLEAdvertisedDevice* _foundDevices = new BLEAdvertisedDevice[BLEsp32MaxDeviceAdvertise];

        void _removeBondedDevices();
            
};  
