#include "structs.h"
#include "DWIN.h"
#include "myUtils.h"
#include "OBD2.h"

#ifndef myApplication_H
#define myApplication_H

class myApplication: public LoopableClass, public IOBD2MessageListener
{

    public:
        myApplication(){};        
        void Init();       
        void Loop();        

        bool isInitialized(){return _isInitialized;};
        void checkAlarms();
        void displayHomePage();
        
        void switchPidGroup(String _group, bool resetValues);
        void flushPidReadInterval(String PidName);
        void onOBD2Response(OBD2Request* request, float value, uint8_t* responseBytes); //message listener  inherited from IOBD2MessageListener
        static void readOBD2Values(void * parameter);
        static void readELMValues(void * parameter);
        
        AppStatus CurrentStatus = AppStatus::Undefined;
        
        //start and stop 
        void checkStartAndStop(OBD2BroadcastPacket* p);
        void disableStartAndStop();

        //oil pressure
        void handleOilPressure(OBD2BroadcastPacket* p);
        
        //pids and values
        String CurrentPidGroup = "DPF_PIDS";
        float NULL_VALUE = 0.0;
        float RPM = 0.0;
        float KM_FROM_LAST_DPF = 0.0;
        float DPF_REGENERATIONS = 0.0;
        float DPF_CLOGGING = 0.0;
        float DPF_EXAUST_TEMP = 0.0;
        float DPF_REGENERATION_PROCESS = 0.0;
        float ENGINE_COOLANT_TEMP_OEM = 0.0;
        float BATTERY_OEM = 0.0;
        float BATTERY_TEMP = 0.0;
        float BATTERY_AMP = 0.0;
        float DIFFERENTIAL_PRESSURE = 0.0;
        float TURBO_BOOST_PRESSURE = 0.0;
        float ATMO_PRESSURE = 0.0;
        float FUEL_FULL_CAPACITY = 0.0;
        float OIL_DEGRADATION = 0.0;
        float OIL_LEVEL = 0.0;
        float OIL_PRESSURE = 0.0;
        float IBS_CHARGE_LEVEL = 0.0;
        float AIR_INTAKE_TEMP = 0.0;
        float OVER_ENG_VALVE = 0.0;
        float KEY_NUMBER = 0.0;
        float KEY_LAST = 0.0;
        float REMOTE_NUMBER = 0.0;
        float REMOTE_LAST = 0.0;
        String LAST_SERVICE_DATE;
        float NUMBER_OF_SERVICES = 0.0;
        float SERVICE_KM = 0.0;
        float SERVICE_DAY = 0.0;
        float GAS_TEMP = 0.0;
        float GAS_PRESSURE = 0.0;
        float GAS_H2O = 0.0;
        float P_ASX = 0.0;
        float P_ADX = 0.0;
        float P_PSX = 0.0;
        float P_PDX = 0.0;
        float T_ASX = 0.0;
        float T_ADX = 0.0;
        float T_PSX = 0.0;
        float T_PDX = 0.0;


        OBD2Request standardPidList[1] ={
           {"STANDARD_PIDS", "RPM", true, 0x18daf1fa, 0x01, 0x0C, 0x02, 1.0/4.1, 0, &NULL_VALUE, &myUtils::NullCb, 60000}
        };
        
        OBD2Request pidList[36] = { 
        //Group, name, SendHeaderBeforePid, header, service, pid, expectedbytes, scalefactor, adjust factor, bind value, ValueCallback, ReadInterval 
        
        //main page pids  
        {"DPF_PIDS", "KM_FROM_LAST_DPF", true, 0x18DA10F1, 0x22, 0x3807, 0x03, 0.1, 0, &KM_FROM_LAST_DPF, &myUtils::NullCb, 60000}, 
        {"DPF_PIDS", "DPF_REGENERATIONS",false,0x18DA10F1, 0x22, 0x18A4, 0x02, 1, 0, &DPF_REGENERATIONS, &myUtils::NullCb, 60000 }, 
        {"DPF_PIDS", "DPF_CLOGGING",false, 0x18DA10F1, 0x22, 0x18E4, 0x02, 1000.0/65535.0, 0, &DPF_CLOGGING, &myUtils::NullCb, 1000}, 
        {"DPF_PIDS", "DPF_EXAUST_TEMP",false, 0x18DA10F1, 0x22, 0x18DE, 0x02, 0.02, -40, &DPF_EXAUST_TEMP, &myUtils::NullCb, 1000}, 
        {"DPF_PIDS", "ENGINE_COOLANT_TEMP_OEM",false, 0x18DA10F1, 0x22, 0x1003, 0x02, 0.02, -40, &ENGINE_COOLANT_TEMP_OEM, &myUtils::NullCb, 1000},
        {"DPF_PIDS", "BATTERY_OEM",false, 0x18DA10F1, 0x22, 0x1955, 0x02, 0.5/1000.0, 0, &BATTERY_OEM, &myUtils::NullCb, 2000}, 
        {"DPF_PIDS", "OIL_DEGRADATION", false,0x18DA10F1, 0x22, 0x3813, 0x02, 100.0/65535.0, 0, &OIL_DEGRADATION, &myUtils::NullCb,30002}, 
        {"DPF_PIDS", "OIL_LEVEL", true, 0x18DA10F1, 0x22, 0x194E, 0x02, 0.1, 0, &OIL_LEVEL, &myUtils::NullCb,3000}, 
        {"DPF_PIDS", "IBS_CHARGE_LEVEL",false, 0x18DA10F1, 0x22, 0x19BD, 0x01, 1, 0, &IBS_CHARGE_LEVEL, &myUtils::NullCb,10000}, 
        {"DPF_PIDS", "DPF_REGENERATION_PROCESS", true, 0x18DA10F1, 0x22, 0x380B, 0x02, 100.0/65535.0, 0, &DPF_REGENERATION_PROCESS, &myUtils::NullCb,500}, 

        //engine pressure values
        {"ENG_PIDS","DIFFERENTIAL_PRESSURE",true, 0x18DA10F1, 0x22, 0x18E2, 0x02, 1, -32767, &DIFFERENTIAL_PRESSURE, &myUtils::NullCb,100},
        {"ENG_PIDS","ATMO_PRESSURE", false, 0x18DA10F1, 0x22, 0x1956, 0x02, 1, -32767, &ATMO_PRESSURE, &myUtils::CustomBytesValues,10000},
        {"ENG_PIDS","TURBO_BOOST_PRESSURE", false, 0x18DA10F1, 0x22, 0x195A, 0x02, 1, -32767, &TURBO_BOOST_PRESSURE, &myUtils::CustomBytesValues,50},
        {"ENG_PIDS","OVER_ENG_VALVE",false, 0x18DA10F1, 0x22, 0x1944, 0x01, 1, 0, &OVER_ENG_VALVE, &myUtils::NullCb,50}, 

        //radio hub
        {"REMOTEKEY_PIDS", "KEY_NUMBER_AND_LAST", true, 0x18DAC7F1, 0x22, 0x0102, 0x03, 1, 0, &NULL_VALUE, &myUtils::CustomBytesValues,6000},
        {"REMOTEKEY_PIDS", "REMOTE_NUMBER_AND_LAST", true, 0x18DAC7F1, 0x22, 0x40A4, 0x03, 1, 0, &NULL_VALUE, &myUtils::CustomBytesValues,5000},
      
        //service values 
        {"SERVICE_PIDS", "OIL_DEGRADATION", true, 0x18DA10F1, 0x22, 0x3813, 0x02, 100.0/65535.0, 0, &OIL_DEGRADATION, &myUtils::NullCb,30002},
        {"SERVICE_PIDS", "OIL_LEVEL", true, 0x18DA10F1, 0x22, 0x194E, 0x02, 0.1, 0, &OIL_LEVEL, &myUtils::NullCb,3000}, 
        {"SERVICE_PIDS", "LAST_SERVICE_DATE", true, 0x18DA60F1, 0x22, 0x2801, 0x04, 1, 0, &NULL_VALUE, &myUtils::CustomBytesValues,120000},
        {"SERVICE_PIDS", "NUMBER_OF_SERVICES", true, 0x18DA60F1, 0x22, 0x2800, 0x01, 1, 0, &NUMBER_OF_SERVICES, &myUtils::NullCb,120001}, 
        {"SERVICE_PIDS", "SERVICE_KM", true, 0x18DA60F1, 0x22, 0x0101, 0x03, 0.1, 0, &SERVICE_KM, &myUtils::NullCb,120002},  
        {"SERVICE_PIDS", "SERVICE_DAY", true, 0x18DA60F1, 0x22, 0x2807, 0x02, 1, 0, &SERVICE_DAY, &myUtils::NullCb,120002},

        //battery 
        {"BATTERY_PIDS","BATTERY_OEM", true, 0x18DA10F1, 0x22, 0x1955, 0x02, 0.5/1000.0, 0, &BATTERY_OEM, &myUtils::NullCb, 2000}, 
        {"BATTERY_PIDS","IBS_CHARGE_LEVEL", true, 0x18DA10F1, 0x22, 0x19BD, 0x01, 1, 0, &IBS_CHARGE_LEVEL, &myUtils::NullCb,10000},
        {"BATTERY_PIDS","BATTERY_TEMP_AMP", true, 0x18DA40F1, 0x22, 0x1005, 0x04, 1, 0, &NULL_VALUE, &myUtils::CustomBytesValues,100},

        //GAS PIDs
        {"GAS_PIDS", "GAS_TEMP",  true, 0x18DA10F1, 0x22, 0x1900, 0x02, 0.02, -40, &GAS_TEMP, &myUtils::NullCb, 10000}, 
        {"GAS_PIDS", "GAS_PRESSURE", false, 0x18DA10F1, 0x22, 0x1947, 0x02, 0.05, 0, &GAS_PRESSURE, &myUtils::NullCb,700}, 
        {"GAS_PIDS", "GAS_H2O", false, 0x18DA10F1, 0x22, 0x18ff, 0x01, 1, 0, &GAS_H2O, &myUtils::NullCb,10002}, 

        //TIRES PIDS
        {"TIRES_PIDS", "P_ASX", true, 0x18DAC7F1, 0x22, 0x31D0, 0x02, 0.001, 0, &P_ASX, &myUtils::NullCb, 3000}, 
        {"TIRES_PIDS", "P_ADX", true, 0x18DAC7F1, 0x22, 0x31D1, 0x02, 0.001, 0, &P_ADX, &myUtils::NullCb, 3000}, 
        {"TIRES_PIDS", "P_PSX", true, 0x18DAC7F1, 0x22, 0x31D2, 0x02, 0.001, 0, &P_PSX, &myUtils::NullCb, 3000}, 
        {"TIRES_PIDS", "P_PDX", true, 0x18DAC7F1, 0x22, 0x31D3, 0x02, 0.001, 0, &P_PDX, &myUtils::NullCb, 3000}, 

        {"TIRES_PIDS", "T_ASX", true, 0x18DAC7F1, 0x22, 0x40B1, 0x03, 1, 0, &NULL_VALUE, &myUtils::CustomBytesValues,3000}, 
        {"TIRES_PIDS", "T_ADX", true, 0x18DAC7F1, 0x22, 0x40B2, 0x03, 1, 0, &NULL_VALUE, &myUtils::CustomBytesValues,3000},  
        {"TIRES_PIDS", "T_PSX", true, 0x18DAC7F1, 0x22, 0x40B3, 0x03, 1, 0, &NULL_VALUE, &myUtils::CustomBytesValues,3000},    
        {"TIRES_PIDS", "T_PDX", true, 0x18DAC7F1, 0x22, 0x40B4, 0x03, 1, 0, &NULL_VALUE, &myUtils::CustomBytesValues,3000}, 
    };

    int pidListSize = sizeof(pidList)/sizeof(pidList[0])-1;
    bool switchPidOccours = false;
    bool ElmIsLost = false;
    bool tempAlarm = false;
    bool clogAlarm = false;
    bool regenAlarm = false;
    bool regenerationInProgress = false;
    short pidIndex;
    bool ELMInitialized = false;
    
    OilLog oilLevels[MAX_OIL_LOG] = {{0.0, 0}, {0.0, 0}, {0.0, 0}, {0.0, 0}, {0.0, 0}};
    float lastOilLevel = 0.0;
    bool updatedOilGraphValues = false;
    
    private:
       
        bool _isInitialized = false;
               
        long _loopmillis = 0;
        bool _prevregenerationInProgress = false;
        unsigned long _previousResetMillis = 0;
        unsigned long _previousAlarmMillis = 0;
        bool unreadableData();
        void checkRegenerationStatus();

        //start and stop 
        bool startAndStopIsOn = true;
        bool startAndStopPulledDown = false;
        long startAndStopCheckMillis = 0;
        long startAndStopCheckTimeout = 10000;
        long startAndStopPullUpMillis = 0;
        long startAndStopCheckDisableTimeout = 500;
        int startAndStopTries = 5;
        int startAndStopExecutedTries = 0;
};

#endif