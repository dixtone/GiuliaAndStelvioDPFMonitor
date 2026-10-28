/**
 * BLE Serial Client
 * Copyright (c) Dixtone @2025. All rights reserved.
 * Commands Interpreter for BluetoothSerial
 * Licensed under the MIT license. See LICENSE file in the project root for full license information.
 */
#pragma once

#include <Arduino.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEClient.h>
#include "IBLEsp32.h"

class BLEsp32Client: public BLEClientCallbacks, public BLECharacteristicCallbacks
{
    public:
        //singleton
        BLEsp32Client(BLEsp32Client const&) = delete;
        void operator=(BLEsp32Client const&)  = delete;

        static BLEsp32Client& getInstance(){
            static BLEsp32Client instance;
            return instance;
        };
        
        void Setup(const char* name, IBLEsp32* intf);
        void Setup(const char* name, const char *service_uuid, const char *rx_uuid, const char *tx_uuid, IBLEsp32* intf);
    
        bool isConnected = false;
        void onConnected(void (*connectlistener)(BLEsp32Client* client)){_onConnectClient = connectlistener;};
        void onDisconnected(void (*disconnectlistener)(BLEsp32Client* client)){_onDisconnectClient = disconnectlistener;};
        void onReceive(void (*onreceivelistener)(std::string buffer)){_onReceivelistener = onreceivelistener;};
        void Write(std::string buffer);
        void WriteChars(std::string buffer);
        
        bool Connect(BLEAddress address);
        bool Connect(BLEAdvertisedDevice* device);
        void Disconnect(){_bleClient->disconnect();};
        
        static void remoteNotify(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);

        //inherited
        void onConnect(BLEClient* pClient){};
        void onDisconnect(BLEClient* pClient){ isConnected = false; _flush(); if(_onDisconnectClient!=nullptr) _onDisconnectClient(this);};

    private:
        BLEsp32Client(){};
        IBLEsp32* _interface;
        const char* _deviceName;
        const char* _serialServiceUuid;
	    const char* _rxUuid;
    	const char* _txUuid;
        
        bool _doConnect();
        bool _doConnectDefaultService();
        
        void _flush(){
            _services = nullptr;
            _characteristics  = nullptr;
            _service = nullptr;
            _txCharacteristic = nullptr;
            _rxCharacteristic = nullptr;
        }

        BLEClient* _bleClient = nullptr;
        std::map<std::string, BLERemoteService*>* _services = nullptr;
        std::map<std::string, BLERemoteCharacteristic*>* _characteristics  = nullptr;
        BLERemoteService *_service = nullptr;
        BLERemoteCharacteristic *_txCharacteristic = nullptr;
    	BLERemoteCharacteristic *_rxCharacteristic = nullptr;
        void (*_onConnectClient)(BLEsp32Client* client) = nullptr;
        void (*_onDisconnectClient)(BLEsp32Client* client) = nullptr;
        void (*_onReceivelistener)(std::string buffer) = nullptr;        
};