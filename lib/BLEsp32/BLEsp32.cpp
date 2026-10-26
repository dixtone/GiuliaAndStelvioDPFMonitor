/**
 * BLE Serial
 * Copyright (c) Dixtone @2025. All rights reserved.
 * Commands Interpreter for BluetoothSerial
 * Licensed under the MIT license. See LICENSE file in the project root for full license information.
 */
#include "IBLEsp32.h"
#include "BLEsp32.h"

//SERVER
bool BLEsp32::BeginServer(const char* name){
   return BeginServer(name,_serialServiceUuid, _rxUuid, _txUuid);
}

bool BLEsp32::BeginServer(const char* name, const char *service_uuid, const char *rx_uuid, const char *tx_uuid){

    _deviceName = name;
    strcpy(_rxUuid, rx_uuid);
    strcpy(_txUuid, tx_uuid);
    strcpy(_serialServiceUuid, service_uuid);

    _server = &BLEsp32Server::getInstance();
    _server->Setup(name, service_uuid, rx_uuid, tx_uuid, this);
    _server->onConnected(_onConnectServer);
    _server->onDisconnected(_onDisconnectServer);

    return true;
}
void BLEsp32::onConnect(void (*connectlistener)(BLEsp32Server* server)){
    _onConnectServer = connectlistener;
    if(_server!=nullptr) _server->onConnected(_onConnectServer);    
};

void BLEsp32::onDisconnect(void (*disconnectlistener)(BLEsp32Server* server)){
    _onDisconnectServer = disconnectlistener;
    if(_server!=nullptr) _server->onDisconnected(_onDisconnectServer);
};

//CLIENT
bool BLEsp32::BeginClient(const char* name){
    _deviceName = name;
    _client = &BLEsp32Client::getInstance();
    _client->Setup(name, this);
    _client->onConnected(_onConnectClient);
    _client->onDisconnected(_onDisconnectClient);

    return true;
 }
 
 bool BLEsp32::BeginClient(const char* name, const char *service_uuid, const char *rx_uuid, const char *tx_uuid){
 
     _deviceName = name;
     strcpy(_rxUuid, rx_uuid);
     strcpy(_txUuid, tx_uuid);
     strcpy(_serialServiceUuid, service_uuid);
 
     _client = &BLEsp32Client::getInstance();
     _client->Setup(name, service_uuid, rx_uuid, tx_uuid, this);
     _client->onConnected(_onConnectClient);
     _client->onDisconnected(_onDisconnectClient);
 
     return true;
 }
 void BLEsp32::onConnect(void (*connectlistener)(BLEsp32Client* client)){
     _onConnectClient = connectlistener;
     if(_client!=nullptr) _client->onConnected(_onConnectClient);    
 };
 
 void BLEsp32::onDisconnect(void (*disconnectlistener)(BLEsp32Client* client)){
     _onDisconnectClient = disconnectlistener;
     if(_client!=nullptr) _client->onDisconnected(_onDisconnectClient);
 };
 
void BLEsp32::onReceive(void (*receivelistener)(std::string buffer)){
    _onReceivelistener = receivelistener;

    if(_server!=nullptr) _server->onReceive(_onReceivelistener);
    if(_client!=nullptr) _client->onReceive(_onReceivelistener);
};

void BLEsp32::Write(std::string buffer){
    if(_server!=nullptr)  _server->Write(buffer);
    if(_client!=nullptr)  _client->Write(buffer);
}
void BLEsp32::WriteChars(std::string buffer){
    if(_server!=nullptr)  _server->WriteChars(buffer);
    if(_client!=nullptr)  _client->WriteChars(buffer);
}

bool BLEsp32::ConnectTo(BLEAddress address){
    if(_client==nullptr) return false;

    _removeBondedDevices();
   return _client->Connect(address);
 }

 bool BLEsp32::ConnectTo(BLEAdvertisedDevice* device){
    if(_client==nullptr) return false;

    _removeBondedDevices();
   return _client->Connect(device);
 }

 void BLEsp32::_removeBondedDevices(){

    int dev_num = esp_ble_get_bond_device_num();
    esp_ble_bond_dev_t *dev_list = (esp_ble_bond_dev_t *)malloc(sizeof(esp_ble_bond_dev_t) * dev_num);
    esp_ble_get_bond_device_list(&dev_num, dev_list);
    for (int i = 0; i < dev_num; i++) {
        esp_ble_remove_bond_device(dev_list[i].bd_addr);
    }

    free(dev_list);
 }

 void BLEsp32::ScanForDevices(int timeout){

    if(DEBUG_MODE) Serial.println("Begin Scan for  devices...");

    _foundDevicesIndex = -1; 
    memset(_foundDevices, 0, BLEsp32MaxDeviceAdvertise);

    _BLEScan = BLEDevice::getScan();
    _BLEScan->setAdvertisedDeviceCallbacks(this);
    _BLEScan->setActiveScan(true);
    _BLEScan->start(timeout);

    if(DEBUG_MODE) Serial.println("End Scan");  
 }

 void BLEsp32::onResult(BLEAdvertisedDevice advertisedDevice){

    //we only want device with services
    if(!advertisedDevice.getServiceUUIDCount() || _foundDevicesIndex >= BLEsp32MaxDeviceAdvertise)
        return;
    
    _foundDevicesIndex++;
    _foundDevices[_foundDevicesIndex] = advertisedDevice;
    

    if(DEBUG_MODE)
    {
        Serial.print("Discover device: ");
        Serial.print(advertisedDevice.getName().c_str());
        Serial.print(" with address: ");
        Serial.println(advertisedDevice.getAddress().toString().c_str());
    }
    

 }

 //implement interface
 std::string BLEsp32::getBuffer(){
    return _btbuffer;
 }

 void BLEsp32::setBuffer(std::string buffer){
    _btbuffer = buffer;
 }

 void BLEsp32::pushBuffer(uint8_t* c){
    
    _bufferMillis = millis();

    int _c = *c;
    char _ch = _c;
    _btbuffer+= (char)_c;
 }

 int BLEsp32::shiftBuffer(){

    if(_btbuffer.length()>0)
    {
        auto c = _btbuffer.substr(0,1);
        _btbuffer = _btbuffer.substr(1);  
       
        return (int)c[0];        
    }

    return -1;
 }

 void BLEsp32::flushBuffer(){
    _btbuffer = "";
 }

//inherit from stream
int BLEsp32::available()
{
	return _btbuffer.length() >0 && (millis()-_bufferMillis)>_bufferTimeout; //check timeout after consecutive pushBuffer
}

int BLEsp32::peek()
{
    if(available()==0)
        return -1;
    return _btbuffer[0];  
}

int BLEsp32::read()
{   
    return shiftBuffer();
}

size_t BLEsp32::readBytes(uint8_t *buffer, size_t bufferSize)
{
    int i = 0;
	while (i < bufferSize && available())
	{
		buffer[i] = (uint8_t)this->_btbuffer[i];
		i++;
	}
	return i;
}

size_t BLEsp32::write(uint8_t byte)
{
    return write((const uint8_t*)byte, strlen((char*)byte));
}

size_t BLEsp32::write(const uint8_t *buffer, size_t bufferSize){
   std::string s = "";
   int i = 0;
   for (i = 0; i < bufferSize; i++)
   {
        s += String((char*)buffer).charAt(i);
   }
   Write(s);
   return i;
}

void BLEsp32::flush()
{
   flushBuffer();
}
