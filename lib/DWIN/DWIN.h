/*
* DWIN DGUS DWIN Library for Arduino Uno | ESP32 
* This Library Supports all Basic Function
* Created by Tejeet ( tejeet@dwin.com.cn ) 
* Please Checkout Latest Offerings FROM DWIN 
* Here : https://www.dwin-global.com/
*/


#ifndef DWIN_H
#define DWIN_H

#if defined(ARDUINO) && ARDUINO >= 100
    #include "Arduino.h"
#else
    #include "WProgram.h"
#endif

#define DWIN_DEFAULT_BAUD_RATE      115200

class IDWINHandler{
    public:
        virtual ~IDWINHandler(){}
        virtual void displayEventHandler(long address, int lastByte, int message, String response){};
};

class DWIN{

public:
    
    DWIN(HardwareSerial& port, uint8_t receivePin, uint8_t transmitPin, long baud=DWIN_DEFAULT_BAUD_RATE);
    DWIN(HardwareSerial* port, uint8_t receivePin, uint8_t transmitPin, long baud=DWIN_DEFAULT_BAUD_RATE) : DWIN(*port, receivePin, transmitPin, baud) {};
    
    // PUBLIC Methods

    void echoEnabled(bool enabled);
    void returnResponseEnabled(bool enabled);
    
    // Listen Touch Events & Messages from HMI
    void listen();
    void listen(int timeout);
    double getHWVersion();
    void restartHMI();
    void setPage(byte pageID);
    int getPage();
    int getHorizontalResolution();
    int getVerticalResolution();
    int getScreenRotation();
    void setBrightness(byte pConstrast);
    byte getBrightness();
    void setText(long address, String textData);
    void setText(long address, String textData, int length);
    void setVP(long address, byte data);
    void setVPInt(long address, int data);
    void setVPInt4(long address, long data);
    void setVPFloat(long address, float data);

    void beepHMI();
    void playAudio(byte audioId);

    // Callback Function
    typedef void (*hmiListener) (long address, int lastByte, int message, String response);
    // CallBack Method
    void hmiCallBack(hmiListener callBackFunction);

    //template<typename T>
    void hmiCallBack(IDWINHandler* instance);

    //graph
    struct pointCoord{
        long x;
        long y;
        long color;
        long w;
        long h;
    };

    void drawLine(long address, long color, pointCoord* points, size_t pointsize);
    void drawSquarePoints(long address, pointCoord* points, size_t pointsize);
    
    

private:

    Stream* _dwinSerial;   // DWIN Serial interface
    bool _isSoft;          // Is serial interface software
    long _baud;              // DWIN HMI Baud rate
    uint8_t _rxPin;
    uint8_t _txPin;
    bool _echo;            // Response Command Show
    bool _returnResponseEnabled;  //Expected return response
    bool _isConnected;     // Flag set on successful communication
    byte _longBuffer[1024]; //buffer where we save multiple data messages
    long _longBufferIdx;
    bool _useLongBuffer;
    bool cbfunc_valid;
    int _readTimeout;
    int _cmdTimeout;
    int _width;
    int _height;
    int _rotation;
    
    hmiListener _listenerCallback;
    IDWINHandler* _listenerInstance;    

    void init(Stream* port); 
    byte readCMDLastByte();
    String handle();
    String checkHex(byte currentNo);
    String writeDWIN(const uint8_t* buffer, size_t size);
    void flushSerial();    
};

#endif // DWIN_H
