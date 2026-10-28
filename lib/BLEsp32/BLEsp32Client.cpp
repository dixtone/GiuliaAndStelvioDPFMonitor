/**
 * BLE Serial Client
 * Copyright (c) Dixtone @2025. All rights reserved.
 * Commands Interpreter for BluetoothSerial
 * Licensed under the MIT license. See LICENSE file in the project root for full license information.
 */

 #include "BLEsp32Client.h"

 void BLEsp32Client::Setup(const char* name, IBLEsp32* intf){
    _deviceName = name;
    _interface = intf;
    BLEDevice::init(name);
    _bleClient = BLEDevice::createClient();
    _bleClient->setClientCallbacks(this);
 }
 
 void BLEsp32Client::Setup(const char* name, const char *service_uuid, const char *rx_uuid, const char *tx_uuid, IBLEsp32* intf){
    _deviceName = name;
    _rxUuid = rx_uuid;
    _txUuid = tx_uuid;
    _serialServiceUuid = service_uuid;
    _interface = intf;
    BLEDevice::init(name);
    _bleClient = BLEDevice::createClient();
    _bleClient->setClientCallbacks(this);
 }

 bool BLEsp32Client::Connect(BLEAddress address){

    _interface->flushBuffer();
    
    if(!_bleClient->connect(address)) return false;

    return _service!=nullptr?_doConnectDefaultService():_doConnect();
 }

 bool BLEsp32Client::Connect(BLEAdvertisedDevice* device){

    _interface->flushBuffer();
    
    if(!_bleClient->connect(device)) return false;

    return _service!=nullptr?_doConnectDefaultService():_doConnect();
 }

 //generic service with notifications
 bool BLEsp32Client::_doConnect(){

    _interface->flushBuffer();
    _services = _bleClient->getServices();

    if(!_services->size() >0)
    {
        if(DEBUG_MODE)
           Serial.print("Failed to get services");

        _bleClient->disconnect();
        return (false);
    }
   
    for (auto const & [uuid, srv] : *_services)
    {
        _characteristics = srv->getCharacteristics();
        
        for (auto const & [uuid2, chr] : *_characteristics)
        {
            if(chr->canWrite())
            {
                _rxCharacteristic = chr;
            }
            else if(chr->canNotify())
            {
                _txCharacteristic = chr;
            }
        }

        if(_rxCharacteristic!=nullptr && _txCharacteristic!=nullptr) 
        {
            _service = srv;
            break;
        }   
    }
    
    if(_rxCharacteristic==nullptr || _txCharacteristic==nullptr) 
    {
        if(DEBUG_MODE)
        {
            Serial.println("Failed to get remote tx and rx characteristic");
        }

        _bleClient->disconnect();
        return (false);
    }

    _txCharacteristic->registerForNotify(remoteNotify);
   
    isConnected = true; 

    if(_onConnectClient!=nullptr)
    {
        _onConnectClient(this);
    } 

    return true;
 }

 //connect to specified service and characteristics
 bool BLEsp32Client::_doConnectDefaultService(){

    _service = _bleClient->getService(_serialServiceUuid);

    if (_service == nullptr) {
        if(DEBUG_MODE)
        {
            Serial.print("Failed to get service UUID: ");
            Serial.println(_serialServiceUuid);
        }

        _bleClient->disconnect();
        return (false);
    }

    _rxCharacteristic = _service->getCharacteristic(_rxUuid);
    if (_rxCharacteristic == nullptr) {
        if(DEBUG_MODE)
        {
            Serial.print("Failed to get remote rx characteristic UUID: ");
            Serial.println(_rxUuid);
        }

        _bleClient->disconnect();
        return (false);
    }

    _txCharacteristic = _service->getCharacteristic(_txUuid);
    if (_txCharacteristic == nullptr) {
        if(DEBUG_MODE)
        {
            Serial.print("Failed to get remote tx characteristic UUID: ");
            Serial.println(_txUuid);
        }

        _bleClient->disconnect();
        return (false);
    }

    _txCharacteristic->registerForNotify(remoteNotify);

    isConnected = true; 

    if(_onConnectClient!=nullptr)
    {
        _onConnectClient(this);
    } 

    return true;
 }

 void  BLEsp32Client::remoteNotify(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify){

    if(isNotify)
    {
        BLEsp32Client* i = &BLEsp32Client::getInstance();
        
        for(int c=0;c<length;c++)
            i->_interface->pushBuffer(&pData[c]);
       
        if(i->_onReceivelistener!=nullptr)
        {
            i->_onReceivelistener((char*)pData);
        }
    }       
 }


void BLEsp32Client::Write(std::string buffer){
    
    _interface->flushBuffer();
    _rxCharacteristic->writeValue(buffer);
   /* Serial.print("Sending buffer: ");
    Serial.println(buffer.c_str());*/
    
}

void BLEsp32Client::WriteChars(std::string buffer){
    
    _interface->flushBuffer();
    std::string buffMTU;

   //notify single char at once
   for (int i = 0; i < buffer.length(); i ++) {
        buffMTU = buffer[i];
        _rxCharacteristic->writeValue(buffMTU);
        delay(5);    
    }
}