#include "Arduino.h"
#include "structs.h"
#include "definitions.h"

#ifndef myUtils_H
#define myUtils_H

class myUtils{

    public:
        myUtils(){};
        void InitFS();
        void ResetFS();
        void DebugSerial(String s);
        
        void readSettings();
        void writeSettings();
        DeviceSettings& getSettings();

        
        void resetFactory();
        
        void elm327Setup();
        void resetOBD2RequestValues();
        void readOilValues();
        void writeOilValues(float previousLevel, float currentLevel);
        void resetOilValues();
        void resetDevice();

        static void NullCb(String PidName, uint8_t* responseBytes){};
        static void CustomBytesValues(String PidName, uint8_t* responseBytes);

        

    private:    
       
        //default settings values
        DeviceSettings _deviceSettings = {1, 1, 0, 1, 98, 398, 500, 30, 1, 1, "",1,200, 3, 0};
        OilLog _oilValues[5] = {{0.0, 100}, {0.0, 200}, {0.0, 300}, {0.0, 400}, {0.0, 500}};
};

#endif