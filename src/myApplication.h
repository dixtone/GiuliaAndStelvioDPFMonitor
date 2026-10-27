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
        static void manageOilValues(void * parameter);
        
        AppStatus CurrentStatus = AppStatus::Undefined;
        
        //start and stop 
        void checkStartAndStop(OBD2BroadcastPacket* p);
        void disableStartAndStop();

        //oil pressure
        void handleOilPressure(OBD2BroadcastPacket* p);

        //light command
        void handleHeadlightsCommand();
        
        //pids and values
        String CurrentPidGroup = "MAIN_PIDS";
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
        float TURBO_AIR_TEMP = 0.0;
        float PERC_TURBO_PRESSURE = 0.0;
        float ATMO_PRESSURE = 0.0;
        float FUEL_FULL_CAPACITY = 0.0;
        float OIL_DEGRADATION = 0.0;
        float OIL_LEVEL = 0.0;
        float OIL_PRESSURE = 0.0;
        float OIL_TEMP = 0.0;
        float EXAUST_TEMP = 0.0;
        float GEAR_OIL_TEMP = 0.0;
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


        static OBD2Request standardPidList[];
        
        static OBD2Request pidList[];
        static const size_t pidListSize;
        
        bool switchPidOccours = false;
        bool ElmIsLost = false;
        bool tempAlarm = false;
        bool clogAlarm = false;
        bool regenAlarm = false;
        bool regenerationInProgress = false;
        short pidIndex = 0;
        bool ELMInitialized = false;

        OilLog oilLevels[MAX_OIL_LOG] = {{0.0, 0}, {0.0, 0}, {0.0, 0}, {0.0, 0}, {0.0, 0}};
        bool updatedOilGraphValues = false;

        //shared OBD state (pidList, CurrentPidGroup, obd2) is used by loop() and readOBD2Values task
        void lockOBD();
        void unlockOBD();
        String getCurrentPidGroup(); //thread safe copy of CurrentPidGroup

        private:

            bool _isInitialized = false;
            SemaphoreHandle_t _obdMutex = nullptr;
            OBD2Request* nextDueRequest();
                
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
            
            //light command
            bool headlightsCommand = false;
            bool headlightsCommandPrevius = false;
        
};

#endif