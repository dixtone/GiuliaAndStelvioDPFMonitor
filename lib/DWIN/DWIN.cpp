#include "DWIN.h"
#include <stdio.h>
#include "driver/uart.h"

#define CMD_HEAD1           0x5A
#define CMD_HEAD2           0xA5
#define CMD_WRITE           0x82
#define CMD_READ            0x83

#define MIN_ASCII           32
#define MAX_ASCII           255

#define READ_TIMEOUT  20
#define CMD_TIMEOUT   20

#define DWIN_ACK_RESPONSE   "5aa503824f4b"

DWIN::DWIN(HardwareSerial& port, uint8_t receivePin, uint8_t transmitPin, long baud){
    _rxPin = receivePin;
    _txPin = transmitPin;
    _baud = baud;
    port.begin(baud, SERIAL_8N1, receivePin, transmitPin);
    init((Stream *)&port);
}

void DWIN::init(Stream* port){
    this->_dwinSerial = port;
    this->_returnResponseEnabled = true;
    this->_readTimeout = READ_TIMEOUT;
    this->_cmdTimeout = CMD_TIMEOUT;
}

void DWIN::echoEnabled(bool echoEnabled){
    _echo = echoEnabled;
}

void DWIN::returnResponseEnabled(bool responseEnabled){
    _returnResponseEnabled = responseEnabled;
}

// Get Hardware Firmware Version of DWIN HMI
double DWIN::getHWVersion(){  //  HEX(5A A5 04 83 00 0F 01)
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x04, CMD_READ, 0x00, 0x0F, 0x01};
    writeDWIN(sendBuffer, sizeof(sendBuffer)); 
    delay(10);
    return readCMDLastByte();
}

// Restart DWIN HMI
void DWIN::restartHMI(){  // HEX(5A A5 07 82 00 04 55 aa 5a a5 )
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x07, CMD_WRITE, 0x00, 0x04, 0x55, 0xaa, CMD_HEAD1, CMD_HEAD2};
    writeDWIN(sendBuffer, sizeof(sendBuffer)); 
    delay(10);
    //readDWIN();
}

// SET DWIN Brightness
void DWIN::setBrightness(byte brightness){
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x04, CMD_WRITE, 0x00, 0x82, brightness };
    writeDWIN(sendBuffer, sizeof(sendBuffer));
    //readDWIN();
}

// GETET DWIN Brightness
byte DWIN::getBrightness(){
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x04, CMD_READ, 0x00, 0x31, 0x01 };
    writeDWIN(sendBuffer, sizeof(sendBuffer));
    return readCMDLastByte();
}

// Change Page 
void DWIN::setPage(byte page){
    //5A A5 07 82 00 84 5a 01 00 02
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x07, CMD_WRITE, 0x00, 0x84, 0x5A, 0x01, 0x00, page};
    writeDWIN(sendBuffer, sizeof(sendBuffer));
    //readDWIN();
}

// Get Current Page ID
int DWIN::getPage(){
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x04, CMD_READ, 0x00 , 0x14, 0x01};
    writeDWIN(sendBuffer, sizeof(sendBuffer)); 
    return int(readCMDLastByte());
}

int DWIN::getScreenRotation(){
    if(_rotation != NULL) return _rotation;

    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x04, CMD_READ, 0x00 , 0x80, 0x02}; //read display configuration
    String resp = writeDWIN(sendBuffer, sizeof(sendBuffer)); 
    int r = (uint64_t) strtoull(resp.substring(resp.length()-2).c_str(), 0, 16); //last byte contains rotation configuration
    //by configuration we have 7 bits: bit 0 and bit 1 are rotation.
    //00 = 0 °, 01 = 90 °, 10 =180 ° ,11 = 270 °
    byte c = (r >> 0) & 1 || (r >> 1) & 1;
    switch(c){
        case 0x0:
            _rotation= 0;
        break;
        case 0x01:
            _rotation= 90;
        break;
        case 0x10:
            _rotation= 180;
        break;
        case 0x11:
            _rotation= 270;
        break;
    }

    return  _rotation;
}

// Get Resolution
int DWIN::getHorizontalResolution(){
    if(_width!=NULL) return _width;
    int rot = getScreenRotation();
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x04, CMD_READ, 0x00 , 0x7A, 0x01};
    
    if(rot == 90 || rot == 270) //if rotation is 90 or 270 use command for vertical resolution
    {
        sendBuffer[5] = 0x7B;
    }
    
    String resp = writeDWIN(sendBuffer, sizeof(sendBuffer)); 
    //last 2 bytes are resolution
    _width = (uint64_t) strtoull(resp.substring(resp.length()-4).c_str(), 0, 16);    
    return _width;
}

int DWIN::getVerticalResolution(){
    if(_height!=NULL) return _height;

    int rot = getScreenRotation();
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x04, CMD_READ, 0x00 , 0x7B, 0x01};

    if(rot ==90 || rot == 270) //if rotation is 90 or 270 use command for horizontal resolution
    {
        sendBuffer[5] = 0x7A;
    }

    String resp = writeDWIN(sendBuffer, sizeof(sendBuffer)); 
    //last 2 bytes are resolution
    _height = (uint64_t) strtoull(resp.substring(resp.length()-4).c_str(), 0, 16);    
    return _height;
}


// Set Text on VP Address
void DWIN::setText(long address, String textData){

    int dataLen = textData.length();
    byte startCMD[] = {CMD_HEAD1, CMD_HEAD2, dataLen+3 , CMD_WRITE, 
    (address >> 8) & 0xFF, (address) & 0xFF};
    byte dataCMD[dataLen];textData.getBytes(dataCMD, dataLen+1);
    byte sendBuffer[6+dataLen];

    memcpy(sendBuffer, startCMD, sizeof(startCMD));
    memcpy(sendBuffer+6, dataCMD, sizeof(dataCMD));

    writeDWIN(sendBuffer, sizeof(sendBuffer));    
}

// Set Text on VP Address
void DWIN::setText(long address, String textData, int length){

    int dataLen = textData.length();

    if(dataLen < length)
    {
        for(int i=dataLen;i<length;i++)
        {
            textData+=" ";
            dataLen++;
        }
    }
    
    byte startCMD[] = {CMD_HEAD1, CMD_HEAD2, dataLen+3 , CMD_WRITE, 
    (address >> 8) & 0xFF, (address) & 0xFF};
    byte dataCMD[dataLen];textData.getBytes(dataCMD, dataLen+1);
    byte sendBuffer[6+dataLen];

    memcpy(sendBuffer, startCMD, sizeof(startCMD));
    memcpy(sendBuffer+6, dataCMD, sizeof(dataCMD));

    writeDWIN(sendBuffer, sizeof(sendBuffer));
    //readDWIN();
    
}

// Set Data on VP Address
void DWIN::setVP(long address, byte data){
    
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x05 , CMD_WRITE, (address >> 8) & 0xFF, (address) & 0xFF, 0x00, data};
    
    writeDWIN(sendBuffer, sizeof(sendBuffer));
}

void DWIN::setVPInt(long address, int data){
    //assumiamo di inviare sempre un address di 2 byte e un data di 2byte o più
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x05, CMD_WRITE, 0x00, 0x00, 0x00, 0x00}; // > 5AA5 è l'header, 05 è il numero di byte del frame da inviare cioè 0x82 e [address VP byte 0xFF00] [address VP byte 0x00FF ] [value VP byte 0xFF00] [value VP byte 0x00FF ]
    
    sendBuffer[4] = address >> 8;
    sendBuffer[5] = address & 0x00FF;
    sendBuffer[6] = data >> 8;
    sendBuffer[7] = data & 0x00FF;

    writeDWIN(sendBuffer, sizeof(sendBuffer));

}

void DWIN::setVPInt4(long address, long data){
    //assumiamo di inviare sempre un address di 2 byte e un data di 2byte o più
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x07 , CMD_WRITE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // > 5AA5 è l'header, 05 è il numero di byte del frame da inviare cioè 0x82 e [address VP byte 0xFF00] [address VP byte 0x00FF ] [value VP byte 0xFF00] [value VP byte 0x00FF ]
    
    sendBuffer[4] = address >> 8;
    sendBuffer[5] = address & 0x00FF;

    byte longv[4] = {0};
    byte* l_byte = reinterpret_cast<byte*>(&data);
    memcpy(longv, l_byte, 4);
  
    //inviamo con ordinamento invertito
    sendBuffer[6] = longv[3];
    sendBuffer[7] = longv[2];
    sendBuffer[8] = longv[1];
    sendBuffer[9] = longv[0];   
    
    writeDWIN(sendBuffer, sizeof(sendBuffer));
    //readDWIN();     
}

// Set Float Data on VP Address
void DWIN::setVPFloat(long address, float data){

    //il float ha sempre 4byte di dati
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x07, CMD_WRITE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // > 5AA5 è l'header, 05 è il numero di byte del frame da inviare cioè 0x82 e [address VP byte 0xFF00] [address VP byte 0x00FF ] [4 byte float ]
    
    sendBuffer[4] = address >> 8;
    sendBuffer[5] = address & 0x00FF;
  
    byte floatv[4] = {0};
    byte* f_byte = reinterpret_cast<byte*>(&data);
    memcpy(floatv, f_byte, 4);
  
    //inviamo con ordinamento invertito
    sendBuffer[6] = floatv[3];
    sendBuffer[7] = floatv[2];
    sendBuffer[8] = floatv[1];
    sendBuffer[9] = floatv[0];  

    writeDWIN(sendBuffer, sizeof(sendBuffer)); 

}




// beep Buzzer for 1 Sec
void DWIN::beepHMI(){
    // 0x5A, 0xA5, 0x05, 0x82, 0x00, 0xA0, 0x00, 0x7D
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x05 , CMD_WRITE, 0x00, 0xA0, 0x00, 0x7D};
    writeDWIN(sendBuffer, sizeof(sendBuffer));
   // readDWIN();
}

void DWIN::playAudio(byte audioId){
  // 5A A5 07 82 00 A0 03 01 40 00
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x07 , CMD_WRITE, 0x00, 0xA0, audioId, 0x01, 0x40, 00};
    writeDWIN(sendBuffer, sizeof(sendBuffer));
    
}


// SET CallBack Event
void DWIN::hmiCallBack(hmiListener callBack){
    _listenerCallback = callBack;
}

void DWIN::hmiCallBack(IDWINHandler* instance){
     _listenerInstance = instance;
}
     
// Listen For incoming callback  event from HMI
void DWIN::listen(){
     handle();
}

void DWIN::listen(int timeout){
     _readTimeout = timeout;
     _cmdTimeout = timeout;
     handle();
}

String DWIN::writeDWIN(const uint8_t* buffer, size_t size)
{   
  _dwinSerial->write(buffer, size);
  return handle();
}
     
String DWIN::checkHex(byte currentNo){
    if (String(currentNo,HEX).length()<2){
        return "0"+String(currentNo, HEX);
    }
    return String(currentNo, HEX);
}

String DWIN::handle(){
    
    String response;

    unsigned long startTime = millis(); // Start time for Timeout    
    while((millis() - startTime < _readTimeout)){
        if(_dwinSerial->available() > 0){
            int c = _dwinSerial->read();
            response.concat(checkHex(c));
        }
    }
        
    if(response.length()>0)
    { 
      if (_echo){
        Serial.println("->> "+response);
      }

      //5a a5 06 83 23 04 01 00 01   
      //5a a5 06 83 23 1a 01 02 b7

      //5a a5 06 83 23 1a 01 0006
    
       response.replace(DWIN_ACK_RESPONSE, "");

       //puo essere che nella risposta dopo scrittura mi arriva un evento touch, per cui parso la risposta
       String _address;
       int _data;
       int _lastbyte;
           
       if(response.indexOf("5aa5")!=-1) //se ho gli header del pacchetto significa che ho un messaggio in entrata
       {
          String resp = response;
          resp.replace("5aa5","");

          //verifichiamo a questo punto che ci sia il comando di lettura 0x83
          int idxRead = resp.indexOf("83");
          int nBytesData = resp.substring(0, idxRead).toInt();

          if(nBytesData<resp.length())
          {
              _address = resp.substring(idxRead+2, idxRead+6); 
              _data = (uint64_t) strtoull(resp.substring(resp.indexOf(_address)+6).c_str(), 0, 16); //facciamo dopo 6 e non 4 perchè arriva uno 01 sconosciuto prima dei dati
              _lastbyte = (uint64_t) strtoull(resp.substring(resp.length()-2).c_str(), 0, 16);

              /*if(_data.length()<4) //devono arrivare almeno 2 byte
                _data = "0"+_data;*/
              
              if (_echo){
                  Serial.println("Address : " + _address + " | Data : " + String(_lastbyte, HEX) + " | Message : " + String(_data, HEX) + " | Response " +response );
              }

        
              if(_listenerCallback!=NULL)
              {
                if (_echo){ Serial.println("Call callback"); }

                _listenerCallback( (uint64_t) strtoull(_address.c_str(), 0, 16), _lastbyte, _data, response);
              }
              if(_listenerInstance!=NULL){
                
                if (_echo){ Serial.println("Calling listener callback"); }
                
                _listenerInstance->displayEventHandler((uint64_t) strtoull(_address.c_str(), 0, 16), _lastbyte, _data, response);
              }
          }
       }
             
    }
            
    return response;
}


byte DWIN::readCMDLastByte(){
    
    byte lastByte = -1;
    unsigned long startTime = millis(); // Start time for Timeout
    while((millis() - startTime < _cmdTimeout)){
        while(_dwinSerial->available() > 0){
            lastByte = _dwinSerial->read();
        }
    }
    return lastByte;
}

void DWIN::flushSerial(){
  Serial.flush();
  _dwinSerial->flush();
}

void DWIN::drawLine(long address, long color, pointCoord* points, size_t pointsize){
    /*
       Ex: Two points are connected to form a line
        5A A5 13 82 5440 0002 0001 F800   00FC 0168   015E 0168   FF00 
        0x5AA5: Frame header;
        0x13 Data length;
        0x82: Write instruction;
        0x5440: VP;
        0x0002: Draw line segment;
        0x0001: The number of connections; 
        0xF800: Color value;
        0x00FC 0168: (252,360)coordinates; 
        0x015E 0168: (350,360)coordinates; 
        0xFF00: End of drawing operation.
    */
    
    if(_width==NULL) getHorizontalResolution();
    if(_height==NULL) getVerticalResolution();

    int len = 14+pointsize*4;
    byte sendBuffer[len];

    long nconnection = pointsize-1;

    sendBuffer[0] = CMD_HEAD1;
    sendBuffer[1] = CMD_HEAD2;
    sendBuffer[2] = len - 3;  //length = 82 + vp + command + connection + colore + pointsize*4 + end 
    sendBuffer[3] = CMD_WRITE;
    sendBuffer[4] = (address >> 8) & 0xFF;
    sendBuffer[5] = (address) & 0xFF;
    sendBuffer[6] = 0x00; // command Draw line segment 0x0002
    sendBuffer[7] = 0x02; // command Draw line segment 0x0002
    sendBuffer[8] = (nconnection >> 8) & 0xFF;
    sendBuffer[9] = (nconnection) & 0xFF;
    sendBuffer[10] = (color >> 8) & 0xFF;
    sendBuffer[11] = (color) & 0xFF;

    //end coordinates
    int j = 12;
    long x=0, y = 0;
    for(int i=0;i<pointsize;i++)
    {
        x = points[i].x;
        y = _height - points[i].y; //revert y 

        sendBuffer[j]   = (x >> 8) & 0xFF; 
        j++;
        sendBuffer[j] = x & 0xFF;
        j++;
        sendBuffer[j] = (y >> 8) & 0xFF;
        j++;
        sendBuffer[j] = y & 0xFF;
        j++;
    }

    //end command
    sendBuffer[j] = 0xFF;
    j++;
    sendBuffer[j] = 0x00;    

/*
    for(int i=0;i<len;i++)
    {
        if (sendBuffer[i] < 16) {Serial.print("0");}
        Serial.print(sendBuffer[i], HEX);
        Serial.print(" ");
    }
*/
    writeDWIN(sendBuffer, sizeof(sendBuffer));
}

void DWIN::drawSquarePoints(long address, pointCoord* points, size_t pointsize){
    /*
       Ex:  Fill two rectangles
        5A A5 1D 82 5440 0004 0002    011E 012C 01AA 018C F800    01AB 018D 01E6 01E6 F800   FF00
        0x5AA5: Frame header;
        0x1D Data length;
        0x82: Write instruction;
        0x5440: VP;
        0x0004: Fill rectangle;
        0x0002: Fill two rectangles;
        0x011E 012C (286,00) upper left coordinate; 
        0x01AA 018C (426, 96) lower right coordinate;
        0xF800: Color; 
        0x01AB 018D (427,97) upper left coordinate; 
        0x01E6 01E6 (486,486) lower right coordinate; 
        0xF800: Color; 
        0xFF00: The drawing operation ends.
    */
    
    if(_width==NULL) getHorizontalResolution();
    if(_height==NULL) getVerticalResolution();
    
    int len = 12+pointsize*10;
    byte sendBuffer[len];

    long fillRect = pointsize;

    sendBuffer[0] = CMD_HEAD1;
    sendBuffer[1] = CMD_HEAD2;
    sendBuffer[2] = len - 3;  //length = 82 + vp + command + connection + colore + pointsize*4 + end 
    sendBuffer[3] = CMD_WRITE;
    sendBuffer[4] = (address >> 8) & 0xFF;
    sendBuffer[5] = (address) & 0xFF;
    sendBuffer[6] = 0x00; // command Fill rectangle 0x0004
    sendBuffer[7] = 0x04; // command Fill rectangle 0x0004
    sendBuffer[8] = (fillRect >> 8) & 0xFF;
    sendBuffer[9] = (fillRect) & 0xFF;
   
    //end coordinates
    int j = 10;
    long x=0, y = 0, x1=0, y1=0;
    for(int i=0;i<pointsize;i++)
    {
        if(points[i].w>0 && points[i].h>0)
        {
            x = points[i].x - points[i].w/2;
            y = _height - points[i].y - points[i].h/2; //revert y 
            x1 = x+points[i].w;
            y1 = y+points[i].h;

            //upper left    
            sendBuffer[j]   = (x >> 8) & 0xFF; 
            j++;
            sendBuffer[j] = x & 0xFF;
            j++;
            sendBuffer[j] = (y >> 8) & 0xFF;
            j++;
            sendBuffer[j] = y & 0xFF;
            j++;

            //lower right
            sendBuffer[j]   = (x1 >> 8) & 0xFF; 
            j++;
            sendBuffer[j] = x1 & 0xFF;
            j++;
            sendBuffer[j] = (y1 >> 8) & 0xFF;
            j++;
            sendBuffer[j] = y1 & 0xFF;
            j++;

            //color
            sendBuffer[j]   = (points[i].color >> 8) & 0xFF; 
            j++;
            sendBuffer[j] = points[i].color & 0xFF;
            j++;
        }
    }

    //end command
    sendBuffer[j] = 0xFF;
    j++;
    sendBuffer[j] = 0x00;    

/*
    for(int i=0;i<len;i++)
    {
        if (sendBuffer[i] < 16) {Serial.print("0");}
        Serial.print(sendBuffer[i], HEX);
        Serial.print(" ");
    }
*/
    writeDWIN(sendBuffer, sizeof(sendBuffer));
}