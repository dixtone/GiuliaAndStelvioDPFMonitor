#include "Arduino.h"
#include "structs.h"
#include "definitions.h"

#ifndef myBtUtils_H
#define myBtUtils_H

class myBtUtils: public LoopableClass{

    public:
        myBtUtils(){};
        void Init(){};
        void Loop();

        void reinitScan();
        void beginScan();
        void connectPairedDevice();
        void checkAndConnectDeviceFromList(short i);
        bool pairBtDevice();
        void unpairBtDevice();
        void searchForBluetoothDevices();
        void checkBootPairedDevice();
        void checkLostConnection();

        void getPairedDevice();    
        bool savePairedDevice();
        void resetPairedDevice();
        void clearBoundedDevices();
        bool elm327Init(bool realPairing);

        PairedDevice pairedDevice;
        PairedDevice pairedDeviceStored;

    private:
        unsigned long _previousScanningIndicator = 0;
        unsigned long _previousLostConnectionMillis = 0;
        int _lostConnectionAttempts = -1;     
        BtItem _availBtDevices[MAX_BT_DEVICES];
        String _foundDevices[MAX_BT_DEVICES];
        short _devicesN = -1;

        void btAdvertisedDeviceFound(BTAdvertisedDevice* device);
        bool btAlreadyFound(String s);
};

#endif