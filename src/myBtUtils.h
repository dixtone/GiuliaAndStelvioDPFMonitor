#include "Arduino.h"
#include "structs.h"
#include "definitions.h"

#ifndef myBtUtils_H
#define myBtUtils_H

class myBtUtils: public LoopableClass{

    public:
        myBtUtils(){btname = String(device_ssid)+String("BT-")+String((uint32_t)ESP.getEfuseMac(), HEX);};
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
        bool checkLostConnection();

        void getPairedDevice();    
        bool savePairedDevice();
        void resetPairedDevice();
        void clearBoundedDevices();
        bool bluetoothClassicInit(bool realPairing);
        bool bluetoothBLEInit(bool realPairing);
        
        PairedDevice pairedDevice;
        PairedDevice pairedDeviceStored;
        
    private:
        String btname;     
        unsigned long _previousScanningIndicator = 0;
        unsigned long _previousLostConnectionMillis = 0;
        int _lostConnectionAttempts = -1;     
        BtItem _availBtDevices[MAX_BT_DEVICES];
        String _foundDevices[MAX_BT_DEVICES];
        short _devicesN = -1;
        #if BLUETOOTH_BLE
        bool bluetoothBLEConnect(String address);
        #else
        void btAdvertisedDeviceFound(BTAdvertisedDevice* device);
        #endif
        bool btAlreadyFound(String s);
};

#endif