/**
 * BLE Serial Server
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

class BLEsp32Server: public BLEServerCallbacks, public BLECharacteristicCallbacks
{
    public:
        BLEsp32Server(BLEsp32Server const&) = delete;
        void operator=(BLEsp32Server const&)  = delete;
        
        static BLEsp32Server& getInstance(){
            static BLEsp32Server instance;
            return instance;
        };
        
        void Setup(const char* name, const char *service_uuid, const char *rx_uuid, const char *tx_uuid, IBLEsp32* intf);

        bool isConnected = false;
        void onConnected(void (*connectlistener)(BLEsp32Server* server)){_onConnectServer = connectlistener;};
        void onDisconnected(void (*disconnectlistener)(BLEsp32Server* server)){_onDisconnectServer = disconnectlistener;};
        void onReceive(void (*onreceivelistener)(std::string buffer)){_onReceivelistener = onreceivelistener;};
        void Write(std::string buffer);
        void WriteChars(std::string buffer);
        void Disconnect(){};        
        //inherited
        void onConnect(BLEServer* pServer);
        void onDisconnect(BLEServer* pServer){ isConnected = false; if(_onDisconnectServer!=nullptr) _onDisconnectServer(this);  _advertise();};
	    void onWrite(BLECharacteristic* pCharacteristic);
        
    private:
        BLEsp32Server(){};
        IBLEsp32* _interface;
        const char* _deviceName;
        const char* _serialServiceUuid;
	    const char* _rxUuid;
    	const char* _txUuid;

        void _advertise();

        BLEServer* _bleServer = nullptr;
        BLEAdvertising* pAdvertising = nullptr;
        BLEService *_service = nullptr;
        BLECharacteristic *_txCharacteristic;
    	BLECharacteristic *_rxCharacteristic;
        void (*_onConnectServer)(BLEsp32Server* server) = nullptr;
        void (*_onDisconnectServer)(BLEsp32Server* server) = nullptr;
        void (*_onReceivelistener)(std::string buffer) = nullptr;
        
};