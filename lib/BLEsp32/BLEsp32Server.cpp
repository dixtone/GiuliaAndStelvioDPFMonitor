/**
 * BLE Serial Server
 * Copyright (c) Dixtone @2025. All rights reserved.
 * Commands Interpreter for BluetoothSerial
 * Licensed under the MIT license. See LICENSE file in the project root for full license information.
 */

 #include "BLEsp32.h"

 void BLEsp32Server::Setup(const char* name, const char *service_uuid, const char *rx_uuid, const char *tx_uuid, IBLEsp32* intf){
    _deviceName = name;
    _rxUuid = rx_uuid;
    _txUuid = tx_uuid;
    _serialServiceUuid = service_uuid;
    _interface = intf;
    BLEDevice::init(name);
    BLEServer *_bleServer = BLEDevice::createServer();
    

    _bleServer->setCallbacks(this);

    BLEDevice::stopAdvertising();
    
    _service = _bleServer->createService(_serialServiceUuid);

    _rxCharacteristic = _service->createCharacteristic(_rxUuid, BLECharacteristic::PROPERTY_WRITE);
	_txCharacteristic = _service->createCharacteristic(_txUuid, BLECharacteristic::PROPERTY_NOTIFY);
    _txCharacteristic->addDescriptor(new BLE2902());
	_rxCharacteristic->addDescriptor(new BLE2902());
    _txCharacteristic->setReadProperty(true);
	_rxCharacteristic->setWriteProperty(true);
    _txCharacteristic->setWriteNoResponseProperty(true); //todo verify
	_rxCharacteristic->setWriteNoResponseProperty(true); //todo verify
    
    _rxCharacteristic->setCallbacks(this);
    
	_service->start();
    
    _advertise();   
 }

 void BLEsp32Server::_advertise(){

    pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(_serialServiceUuid);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    pAdvertising->start();

    BLEDevice::startAdvertising();
 }

 void BLEsp32Server::onConnect(BLEServer* pServer){
     isConnected = true; 
     _interface->flushBuffer();
    /*
    uint16_t bleID = pServer->getConnId();
    pServer->updatePeerMTU(bleID, BLEsp32ServerDefaultMTU);
    if(DEBUG_MODE) Serial.printf("updateMTU to: %i\n\n", pServer->getPeerMTU(bleID));
    */

     if(_onConnectServer!=nullptr) _onConnectServer(this);
}

 //inherited
void  BLEsp32Server::onWrite(BLECharacteristic* pCharacteristic){
    
    if(isConnected)
    {
        if (pCharacteristic->getUUID().toString() == _rxUuid)
        {
            std::string value = pCharacteristic->getValue().c_str();
            _interface->flushBuffer();
            _interface->setBuffer(value);
            if(_onReceivelistener!=nullptr)
            {
                _onReceivelistener(value);
            }                    
        }
    }
}

void BLEsp32Server::Write(std::string buffer){
    
    //split string in BLEsp32ServerDefaultMTU chunks to compliant
    _interface->flushBuffer();
    std::string buffMTU;

    for (int i = 0; i < buffer.length(); i += BLEsp32ServerDefaultMTU) {
        buffMTU = buffer.substr(i, BLEsp32ServerDefaultMTU);
        _txCharacteristic->setValue(buffMTU);
        _txCharacteristic->notify(true); 
        delay(5);   
    }

}

void BLEsp32Server::WriteChars(std::string buffer){
    
    _interface->flushBuffer();
    std::string buffMTU;

   //notify single char at once
   for (int i = 0; i < buffer.length(); i ++) {
        buffMTU = buffer[i];
        _txCharacteristic->setValue(buffMTU);
        _txCharacteristic->notify(true);
        delay(3);    
    }
}
