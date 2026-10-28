/**
 * BLE Serial
 * Copyright (c) Dixtone @2025. All rights reserved.
 * Commands Interpreter for BluetoothSerial
 * Licensed under the MIT license. See LICENSE file in the project root for full license information.
 */
#pragma once
#include <Arduino.h>

class IBLEsp32{
    public:
        virtual ~IBLEsp32(){}
        virtual void setBuffer(std::string buffer){}
        virtual void pushBuffer(uint8_t* c){}
        virtual int shiftBuffer(){return -1;}
        virtual void flushBuffer(){}
        virtual std::string getBuffer(){return "";}
    protected:
        std::string _btbuffer;    
};