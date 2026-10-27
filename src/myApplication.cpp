#include "definitions.h"
#include "myApplication.h"
#include "myDisplay.h"
#include "myUtils.h"
#include "myBtUtils.h"
#include <esp_task_wdt.h>

extern myUtils utils;
extern myBtUtils btUtils;
extern myDisplay display;
extern WS2812Led wsLED;
extern ESP32S3Buzzer buzzer;
extern TaskHandle_t asyncTask;
extern TaskHandle_t asyncTask2;
extern OBD2 obd2;

#if BLUETOOTH_BLE
extern BLEsp32 BluetoothConnector;
#else
extern BluetoothSerial BluetoothConnector;
#endif

extern myApplication app;

void myApplication::lockOBD(){
    if(_obdMutex!=nullptr)
        xSemaphoreTakeRecursive(_obdMutex, portMAX_DELAY);
}

void myApplication::unlockOBD(){
    if(_obdMutex!=nullptr)
        xSemaphoreGiveRecursive(_obdMutex);
}

void myApplication::Init(){

    esp_task_wdt_init(3, true);

    _obdMutex = xSemaphoreCreateRecursiveMutex();

    utils.DebugSerial("Init application");
    
    #if CANBUS_DIRECT
    //ss
    pinMode(START_STOP_PIN, OUTPUT_OPEN_DRAIN);
    digitalWrite(START_STOP_PIN, HIGH);
    #else
    //led
    wsLED.Brightness(255);
    wsLED.Update(0xff,0x99,0xDA,0, 0, true);
    #endif

    //TODO CHECK riscrittura valori olio e visualizzazione quando clicco su

    display.Init();
    
    delay(1000);
    display.SetPage(DwinPage::PAGE_LOADING);
    utils.InitFS();

    //buzzer
    buzzer.begin();
    buzzer.tone(500, 10, 50, 1); 
    
    delay(3000);
    
    utils.readSettings();

    utils.readOilValues();

    //setting up obd reading on core
    xTaskCreatePinnedToCore( readOBD2Values, "readOBD2Values", 8096, NULL, 5, &asyncTask, 1); //core

    #if !DISABLE_OIL_LOG
    xTaskCreatePinnedToCore( manageOilValues, "manageOilValues", 8096, NULL, 1, &asyncTask2, 1); //core
    #endif

    //setup handler
    obd2.onHandleValue(&app);

    #if CANBUS_DIRECT
    //using SJA1000 transceiver
    obd2.Begin(CAN_TX_PIN, CAN_RX_PIN, 500E3);

    //lister for these messages Id (without querying them)
    obd2.addBroadcastFilter(0x226); //ECU for start & stop
    obd2.addBroadcastFilter(0x4B2); //ECU for oil pressure and other
    obd2.addBroadcastFilter(0x545); //ECU for headlights and other
    
    //add some filters for canbus messages: they correspond to reverse header of OBD2Request
    obd2.addPacketFilter(0x18DAF1FA);
    obd2.addPacketFilter(0x18DAF110);
    obd2.addPacketFilter(0x18DAF1C7);
    obd2.addPacketFilter(0x18DAF140);
    obd2.addPacketFilter(0x18DAF160);
    #else
    btUtils.checkBootPairedDevice();
    #endif

    displayHomePage();
    display.InitDisplayVariables();

    _isInitialized = true;
}

void myApplication::displayHomePage(){


  if(ELMInitialized || CANBUS_DIRECT)
  {
     CurrentStatus = AppStatus::Main;     
     switchPidGroup("MAIN_PIDS", false);
     
     if(regenerationInProgress)
     {
       display.SetPage(DwinPage::PAGE_REGEN);
     }
     else{
       display.SetPage(DwinPage::PAGE_DASHBOARD);
     }
     
  }
  else{

      if(CANBUS_DIRECT)
          return;

      CurrentStatus = AppStatus::BtNoConnection;
    
      if(btUtils.pairedDevice.phantom)
      {
        display.SetPage(DwinPage::PAGE_BT_SEARCH);
      }
      else{
        display.SetPage(DwinPage::PAGE_BT_RECONNECT);
      }
  } 
}

void myApplication::switchPidGroup(String _group, bool resetValues){

  lockOBD();

  obd2.flush();

  if(CurrentPidGroup==_group)
  {
    unlockOBD();
    return;
  }

  utils.DebugSerial("Switch to group pids: "+_group);

  switchPidOccours = true;

  CurrentPidGroup = _group;

  if(resetValues)
  {
    utils.resetOBD2RequestValues();
  }

  pidIndex = 0;
  switchPidOccours = false;

  unlockOBD();

  if(resetValues)
  {
    delay(500);
  }
}

String myApplication::getCurrentPidGroup(){
  lockOBD();
  String group = CurrentPidGroup;
  unlockOBD();
  return group;
}

bool myApplication::unreadableData(){
  return obd2.status==OBD2StatusType::timeout || obd2.status==OBD2StatusType::nodata || obd2.status==OBD2StatusType::error;
} 

void myApplication::checkRegenerationStatus(){

   //dpf regerating... 
   if((KM_FROM_LAST_DPF>0 && KM_FROM_LAST_DPF<=0.1) || DPF_REGENERATION_PROCESS > 0.1)
   {
      regenerationInProgress = true;
   }
   else{
      regenerationInProgress = false;
   }
}

void myApplication::checkAlarms(){
  //minimum interval: with 0 tones would be queued every loop until memory runs out
  unsigned long interval = (unsigned long)max(utils.getSettings().alarm_interval, ALARM_MIN_INTERVAL)*1000UL;

  if(millis()-_previousAlarmMillis > interval || !_previousAlarmMillis)
  {
 
      if(regenAlarm)
      {
          buzzer.tone(1500, 100, 50, 2); 
          buzzer.tone(1000, 200, 100, 3);
      }
      else{
        
        if(tempAlarm && clogAlarm)
        {
           buzzer.tone(1000, 125, 50, 3);  
        }
        else if(tempAlarm)
        {
           buzzer.tone(1000, 125, 50, 2);
          
        }
        else if(clogAlarm){
           buzzer.tone(1000, 125, 50, 1);
        }
      }
      _previousAlarmMillis = millis();      
  }
}

//OBD2 handler
void myApplication::onOBD2Response(OBD2Request* request, float value, uint8_t* responseBytes){

  //when value occours set it for display
  //Serial.print("Received message for "+request->Name+" :");
  //Serial.println(value);

  *request->BindValue = value;
  request->ValueCallback(request->Name, responseBytes); //call to valuecallback method
}

//next PID of current group whose read interval is elapsed (round robin), nullptr if none
OBD2Request* myApplication::nextDueRequest(){

  for(size_t n=0;n<pidListSize;n++)
  {
    if(pidIndex<0 || pidIndex>=(short)pidListSize)
    {
      pidIndex = 0;
    }

    OBD2Request* request = &pidList[pidIndex];
    pidIndex++;

    //filter PIDS by group
    if(request->Group!=CurrentPidGroup && request->Group!="ALL_PIDS")
    {
      continue;
    }

    //ReadTime 0: never read or flushed, read now
    if(request->ReadTime==0 || (millis() - (unsigned long)request->ReadTime) > (unsigned long)request->ReadInterval)
    {
      return request;
    }
  }

  return nullptr;
}

//OBD2 reading on Task1
void myApplication::readOBD2Values(void * parameter){

  esp_task_wdt_add(NULL); // Registra il task al watchdog

  OBD2StatusType obdStatus;
  OBD2BroadcastPacket bp;
  OBD2Request* request;
  OBD2Request* lastSentRequest = nullptr;
  unsigned long sendTime = millis();
  bool _enginerpm = false;
  long _prevHeader=0x0;

  while(1)
  {

    if(app.isInitialized())
       buzzer.update();//it's faster here

    if(app.isInitialized() && app.CurrentStatus==AppStatus::Main && (obd2.isELM327()?(app.ELMInitialized && !app.ElmIsLost):true))
    {
      app.lockOBD();

      #if CANBUS_DIRECT
      bp = obd2.getBroadcastPacket();
      if(bp.Header>0x0)
      {
          //S&S Status
          if(bp.Header==0x226 && DISABLE_STARTSTOP)
          {
              app.checkStartAndStop(&bp);
          }

          //Oil pressure
          if(bp.Header==0x4B2)
          {
              app.handleOilPressure(&bp);
          }

          //headlights command came in broadcast with packet id 545
          //response is similar to: 68 20 c8 26 00 0b a0 00 -> Byte4 can be: 00 (off) or 80 (on)
          if(bp.Header==0x545)
          {
              app.headlightsCommand = (bp.Byte4 == 0x80);

          }

      //  Serial.printf("\n>> Broadcast packet %04x : %02x %02x %02x %02x %02x %02x %02x %02x\n", bp.Header, bp.Byte0, bp.Byte1, bp.Byte2, bp.Byte3, bp.Byte4, bp.Byte5, bp.Byte6, bp.Byte7);
      }
      #endif

      //perform some checks
      app.checkRegenerationStatus();
      app.tempAlarm = (app.DPF_EXAUST_TEMP>=utils.getSettings().temp_alert && utils.getSettings().temp);
      app.clogAlarm = (int(app.DPF_CLOGGING)>=utils.getSettings().clogging_alert  && utils.getSettings().clogging);
      app.regenAlarm = (app.regenerationInProgress && utils.getSettings().regeneration);

      //obd2 process flow
      obdStatus = obd2.process();

      #if !CANBUS_DIRECT
      //when elm327 we have to send rpm to unlock ecu, once
      if(!_enginerpm)
      {
        request = &app.standardPidList[0];
        app.CurrentPidGroup = request->Group;

        if(obd2.sendRequest(request))
        {
          lastSentRequest = request;
          utils.DebugSerial("Reading RPM to init flow");
        }
      }
      else
      #endif
      if(obdStatus == OBD2StatusType::ready)
      {
        //send every cmd delay setting
        if(millis()-sendTime > (unsigned long)utils.getSettings().obd_cmd_delay)
        {
          request = app.nextDueRequest();

          if(request!=nullptr)
          {
            sendTime = millis();
            request->ReadTime = millis();

            #if !CANBUS_DIRECT
            //when ELM we have to check if change request header
            if(_prevHeader != request->Header || request->AlwaysSendHeader)
            {
                _prevHeader = request->Header;
              obd2.sendElmHeader(request->Header);
            }
            #endif

            if(obd2.sendRequest(request))
            {
              lastSentRequest = request;
            }
          }
        }
      }

      if(obdStatus == OBD2StatusType::received){

        #if !CANBUS_DIRECT
        if(lastSentRequest!=nullptr && lastSentRequest->Name=="RPM")
        {
          _enginerpm = true;
          _prevHeader = lastSentRequest->Header;

          app.CurrentPidGroup = "MAIN_PIDS"; //lets start from DPF pids, once
        }
        #endif
      }
      else if(obdStatus == OBD2StatusType::timeout){
        //timeout occours from canbus: retry the request that was sent
        if(lastSentRequest!=nullptr && lastSentRequest->ReadTime!=0)
        {
          lastSentRequest->ReadTime = 0;
          utils.DebugSerial("Unreadable PID (timeout): "+String(lastSentRequest->Pid, HEX));
        }
      }

      app.unlockOBD();
    }

    //always reached: feed watchdog and let loop() run
    esp_task_wdt_reset();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

//managment task for oil values and rw/file
void myApplication::manageOilValues(void * parameter){

  float preValue = 0.0;
  float stableValue = 0.0; //level we are checking for stability
  unsigned long stableSince = 0;

  while(1)
  {
    float level = app.OIL_LEVEL;

    //0 means not read (page switch, timeout): don't reset stability
    if(level>0.0)
    {
      //level changed (ex. engine start): restart stability time
      if(stableValue==0.0 || fabs(level-stableValue) > OIL_LOG_TOLERANCE + 0.0001) //epsilon: float rounding
      {
        stableValue = level;
        stableSince = millis();
      }

      //write only after power on delay and if level is stable, so not during engine cranking
      if(millis() > OIL_LOG_START_DELAY
        && millis()-stableSince > OIL_LOG_STABLE_TIME
        && stableValue!=preValue)
      {
          utils.writeOilValues(stableValue);

          preValue = stableValue;
      }
    }

    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void myApplication::flushPidReadInterval(String PidName)
{
  OBD2Request* request;

  lockOBD();

  for(int i=0;i<app.pidListSize;i++)
  {
     request = &pidList[i];

     if(request->Name == PidName)
     {
       utils.DebugSerial("Reset PID: "+String(request->Pid, HEX)+"  Last read: "+String(request->ReadTime));
       request->ReadTime = 0;
       break;
     }
  }

  unlockOBD();
}

//oil pressure came in broadcast with packet id 0x4B2
//then response is similar to: 28 32 16 80 00 00 00 00
//to take oil pressure we have to take from byte0 bit0 and from byte1 bit 7 ~ bit 1 
//then we apple mask: 0b0000000111111110
void myApplication::handleOilPressure(OBD2BroadcastPacket* p){
    app.OIL_PRESSURE = (float)((p->Byte0 & 0b00000001) << 7 | ((p->Byte1 >> 1) & 0b01111111))/10.0;   
}

//headlights are on, lower display brightness
void myApplication::handleHeadlightsCommand(){
    
  if(app.headlightsCommand!=app.headlightsCommandPrevius)
  {
      app.headlightsCommandPrevius = app.headlightsCommand;
      display.getHmi()->setBrightness(app.headlightsCommand?0x07:0x64);//0x00-0x64
  }
}

void myApplication::checkStartAndStop(OBD2BroadcastPacket* p){
    //0xF1: ss ON  and indicator light OFF
    //0x05: ss OFF and indicator light ON
    startAndStopIsOn = (p->Byte2 == 0xF1); 
 //   Serial.printf("\nhead 0x226 SS: %2x %2x %2x %2x %2x %2x %2x %2x \n", p->Byte0, p->Byte1, p->Byte2, p->Byte3, p->Byte4, p->Byte5, p->Byte6, p->Byte7 );
    if(!startAndStopIsOn)
    {
       startAndStopExecutedTries = 0;
       startAndStopCheckMillis = 0;

       if(DEBUG_MODE)
       {
         Serial.println("Start and Stop is disabled!");
       }
    }
}

void myApplication::disableStartAndStop(){
       
    
    if(startAndStopIsOn && startAndStopExecutedTries < startAndStopTries)
    {
        if(!startAndStopCheckMillis)
        {
          startAndStopCheckMillis = millis();
        }

        //after startAndStopCheckTimeout pulldown pin
        if(millis()-startAndStopCheckMillis > startAndStopCheckTimeout)
        {
            digitalWrite(START_STOP_PIN, LOW);
            startAndStopPulledDown = true;
            
            if(!startAndStopPullUpMillis)
              startAndStopPullUpMillis = millis();

            if(DEBUG_MODE)
            {
               Serial.println("PullDown Start & Stop");
            }
        }

        if(startAndStopPulledDown && (millis()-startAndStopCheckDisableTimeout > startAndStopPullUpMillis))
        { 
            startAndStopExecutedTries++;
            
            digitalWrite(START_STOP_PIN, HIGH);
            startAndStopCheckMillis = 0;
            startAndStopPullUpMillis = 0;
            startAndStopPulledDown = false;

            if(DEBUG_MODE)
            {
               Serial.println("PullUp Start & Stop");
            }

        }
        
    }
    else{
      if(startAndStopPulledDown)
      {
        digitalWrite(START_STOP_PIN, HIGH);
        startAndStopPulledDown = false;
      }
    }
}

void myApplication::Loop(){

    //all components loop here
    display.Loop();
    
    if (CurrentStatus == AppStatus::Main)
    {
        display.UpdateDisplayVariables();

        if ((_prevregenerationInProgress != regenerationInProgress))
        {
            flushPidReadInterval("KM_FROM_LAST_DPF");
            flushPidReadInterval("DPF_REGENERATIONS");
            _prevregenerationInProgress = regenerationInProgress;
            displayHomePage();
        }
    }
    
    btUtils.Loop();
    
    #if CANBUS_DIRECT
      #if DISABLE_STARTSTOP
      disableStartAndStop();
      #endif
      handleHeadlightsCommand();
    #endif

    delay(10);
}

//PID DECLARATIONS
OBD2Request myApplication::standardPidList[] = { 
  {"STANDARD_PIDS", "RPM", true, 0x18daf1fa, 0x01, 0x0C, 0x02, 1.0/4.1, 0,  &app.NULL_VALUE, &myUtils::CustomBytesValues, 500} //0x18daf1fa
};

OBD2Request myApplication::pidList[] = { 
  //Group, name, SendHeaderBeforePid, header, service, pid, expectedbytes, scalefactor, adjust factor, bind value, ValueCallback, ReadInterval 
  
  //main page pids  
  #if BLUETOOTH_BLE && !GAS_ENGINE
  //finally oil pressure!!!
  {"MAIN_PIDS", "OIL_PRESSURE", true, 0x18DA10F1, 0x99, 0x04B2, 0x08, 1, 0, &app.OIL_PRESSURE, &myUtils::CustomBytesValues, 10}, 
  #endif
  
  //diesel pids
  #if !GAS_ENGINE
  {"MAIN_PIDS", "ENGINE_COOLANT_TEMP_OEM",false, 0x18DA10F1, 0x22, 0x1003, 0x02, 0.02, -40, &app.ENGINE_COOLANT_TEMP_OEM, &myUtils::NullCb, 1000},        //ENGINE_COOLANT_TEMP_OEM  = 4099;  // 0x1003 => (((A*256)+B)*0.02)-40
  {"MAIN_PIDS", "KM_FROM_LAST_DPF", true, 0x18DA10F1, 0x22, 0x3807, 0x03, 0.1, 0, &app.KM_FROM_LAST_DPF, &myUtils::NullCb, 60000},                //KM_FROM_LAST_DPF         = 14343; // 0x3807 => ((A*65536)+(B*256)+C)*0.1
  {"MAIN_PIDS", "DPF_REGENERATIONS",false,0x18DA10F1, 0x22, 0x18A4, 0x02, 1, 0, &app.DPF_REGENERATIONS, &myUtils::NullCb, 60000 },                  //DPF_REGENERATIONS        = 6308;  // 0x18A4 => (A*256)+B
  {"MAIN_PIDS", "DPF_CLOGGING",false, 0x18DA10F1, 0x22, 0x18E4, 0x02, 1000.0/65535.0, 0, &app.DPF_CLOGGING, &myUtils::NullCb, 1000},             //DPF_CLOGGING             = 6372;  // 0x18E4 => ((A*256)+B)*(1000/65535)
  {"MAIN_PIDS", "DPF_EXAUST_TEMP",false, 0x18DA10F1, 0x22, 0x18DE, 0x02, 0.02, -40, &app.DPF_EXAUST_TEMP, &myUtils::NullCb, 1000},                  //DPF_EXAUST_TEMP          = 6366;  // 0x18DE => (((A*256)+B)*0.02)-40 
  {"MAIN_PIDS", "DPF_REGENERATION_PROCESS", true, 0x18DA10F1, 0x22, 0x380B, 0x02, 100.0/65535.0, 0, &app.DPF_REGENERATION_PROCESS, &myUtils::NullCb,500},      //DPF_REGENERATION_PROCESS = 14347; // 0x380B => ((A*256)+B)*(100/65535) 
  {"MAIN_PIDS", "BATTERY_OEM",false, 0x18DA10F1, 0x22, 0x1955, 0x02, 0.5/1000.0, 0, &app.BATTERY_OEM, &myUtils::NullCb, 2000},                  //BATTERY_OEM              = 6485;  // 0x1955 => ((A*256)+B)*(0.5/1000) 
  {"MAIN_PIDS", "OIL_DEGRADATION", false,0x18DA10F1, 0x22, 0x3813, 0x02, 100.0/65535.0, 0, &app.OIL_DEGRADATION, &myUtils::NullCb,30002}, //OIL_DEGRADATION          = 14355; // 0x3813 => ((A*256)+B)*(100/65535) //% degrado olio
  {"MAIN_PIDS", "OIL_LEVEL", true, 0x18DA10F1, 0x22, 0x194E, 0x02, 0.1, 0, &app.OIL_LEVEL, &myUtils::NullCb,3000}, //OIL_LEVEL mm ((A*256)+B)*0.1
  #else
  {"MAIN_PIDS", "ENGINE_COOLANT_TEMP_OEM",false, 0x18DA10F1, 0x22, 0x1003, 0x01, 1, -40, &app.ENGINE_COOLANT_TEMP_OEM, &myUtils::NullCb, 1000},        //ENGINE_COOLANT_TEMP_OEM  = 4099; A-40
  {"MAIN_PIDS", "BATTERY_OEM",false, 0x18DA10F1, 0x22, 0x1004, 0x02, 0.1, 0, &app.BATTERY_OEM, &myUtils::NullCb, 2000},                   //BATTERY_OEM     // ((A*256)+B)*(0.1)
  {"MAIN_PIDS", "OIL_DEGRADATION", false,0x18DA10F1, 0x22, 0x3813, 0x02, 100.0/65535.0, 0, &app.OIL_DEGRADATION, &myUtils::NullCb,5000}, //OIL_DEGRADATION          = 14355; // 0x3813 => ((A*256)+B)*(100/65535) //% degrado olio
  {"MAIN_PIDS", "OIL_LEVEL", true, 0x18DA10F1, 0x22, 0x3A41, 0x02, 0.001, 0, &app.OIL_LEVEL, &myUtils::NullCb,3000}, //OIL_LEVEL litri ((A*256)+B)*0.001
  #endif

  {"MAIN_PIDS", "IBS_CHARGE_LEVEL",false, 0x18DA10F1, 0x22, 0x19BD, 0x01, 1, 0, &app.IBS_CHARGE_LEVEL, &myUtils::NullCb,10000},                  //IBS_CHARGE_LEVEL         = 6589;  // 0x19BD => A //% di carica
  
  //gasoline pids
  #if GAS_ENGINE
  {"MAIN_PIDS", "OIL_PRESSURE", true, 0x18DA10F1, 0x22, 0x130A, 0x01, 10.0/255.0, 0, &app.OIL_PRESSURE, &myUtils::NullCb, 100}, 
  {"MAIN_PIDS", "EXAUST_TEMP",false, 0x18DA10F1, 0x22, 0x18BA, 0x01, 5.0, -50, &app.EXAUST_TEMP, &myUtils::NullCb, 1000},    
  {"MAIN_PIDS", "OIL_TEMP",false, 0x18DA10F1, 0x22, 0x1302, 0x02, 1.0, 0, &app.OIL_TEMP, &myUtils::NullCb, 3000},    //A*256 + B
  {"MAIN_PIDS", "GEAR_OIL_TEMP",false, 0x18DA18F1, 0x22, 0x04FE, 0x01, 1.0, -40, &app.GEAR_OIL_TEMP, &myUtils::NullCb, 3000},    
  #endif


  //engine pressure values
  #if !GAS_ENGINE
  {"ENG_PIDS","DIFFERENTIAL_PRESSURE",true, 0x18DA10F1, 0x22, 0x18E2, 0x02, 1, -32767, &app.DIFFERENTIAL_PRESSURE, &myUtils::NullCb,100},//DIFFERENTIAL_PRESSURE    = 6370;  // 0x18E2 => ((A*256)+B)-32767
  {"ENG_PIDS","TURBO_BOOST_PRESSURE", false, 0x18DA10F1, 0x22, 0x195A, 0x02, 1, -32767, &app.TURBO_BOOST_PRESSURE, &myUtils::CustomBytesValues,50},         //TURBO_BOOST_PRESSURE     = 6490;  // 0x195A => ((A*256)+B)-32767
  {"ENG_PIDS","OVER_ENG_VALVE",false, 0x18DA10F1, 0x22, 0x1944, 0x01, 1, 0, &app.OVER_ENG_VALVE, &myUtils::NullCb,50}, //0x1944 => A
  #else
  {"ENG_PIDS", "TURBO_AIR_TEMP",false, 0x18DA10F1, 0x22, 0x3A58, 0x01, 1.0, -40, &app.TURBO_AIR_TEMP, &myUtils::NullCb, 1000}, 
  {"ENG_PIDS", "PERC_TURBO_PRESSURE",false, 0x18DA10F1, 0x22, 0x1819, 0x02, 100.0/65535.0, 0, &app.PERC_TURBO_PRESSURE, &myUtils::NullCb, 50}, 
  {"ENG_PIDS", "OVER_ENG_VALVE",false, 0x18DA10F1, 0x22, 0x1864, 0x01, 1, 0, &app.OVER_ENG_VALVE, &myUtils::NullCb,50},
  #endif

  {"ENG_PIDS","ATMO_PRESSURE", false, 0x18DA10F1, 0x22, 0x1956, 0x02, 1, -32767, &app.ATMO_PRESSURE, &myUtils::CustomBytesValues,10000},         //ATMO_BOOST_PRESSURE     = 6490;  // 0x195A => ((A*256)+B)-32767
  //{"AIR_INTAKE_TEMP", 0x18DA10F1, 0x22, 0x1935, 0x02, 0.02, -40, &app.AIR_INTAKE_TEMP, nullptr, 2003}         //AIR_INTAKE_TEMP          = 6453;  // 0x1935 => (((A*256)+B)*0.02)-40 //gradi celsius
  //{"ENG_PIDS","OVER_BOOST_PERC", false,0x18DA10F1, 0x22, 0x193A,  0x02, 100.0/65535.0, 0, &app.OVER_BOOST_PERC, nullptr,70},   //NOT WORKS!!!      //0x193A => ((A*256)+B)*(100/65535)
  
  /*
  Pressione Atmosferica:1009 bar   221956 62195683F0  ((A*256)+B)-32767
  Pressione MAP:1012bar  22195A 62195A83F3 ((A*256)+B)-32767
  Posizione Pompa Olio: bassa 22194F 62194F01 ?
  Temp Ingresso turbina: 153,26 223836 62383625D8 (((A*256)+B)*0.02)-40 
  */

  //radio hub
  {"REMOTEKEY_PIDS", "KEY_NUMBER_AND_LAST", true, 0x18DAC7F1, 0x22, 0x0102, 0x03, 1, 0, &app.NULL_VALUE, &myUtils::CustomBytesValues,6000},//KEY_NUMBER_AND_LAST multirow pid
  /*
      008
      2 62 01 02 00 A6 02 02 FF FF FF FF FF FF  
      1 62 01 02 00 FA 02 01 FF FF FF FF FF FF

      key num: byte[2]
      key last: byte[3] 
      
  */
  {"REMOTEKEY_PIDS", "REMOTE_NUMBER_AND_LAST", true, 0x18DAC7F1, 0x22, 0x40A4, 0x03, 1, 0, &app.NULL_VALUE, &myUtils::CustomBytesValues,5000},//REMOTE_NUMBER_AND_LAST multirow pid
  /*
      00B
      2 62 40 A4 02 02 FF FF FF FF FF FF FF FF 
      1 62 40 A4 02 01 FF FF FF FF FF FF FF FF
      remote num: byte[0]
      remote last: byte[1] 
      ID: byte[3] byte[4] byte[5] byte[6]
  */

  //waring: 0x18xxxxxx is priority as diagnostics. We can use 0x30xxxxxx to lower priority

  //service values 
  {"SERVICE_PIDS", "OIL_DEGRADATION", true, 0x18DA10F1, 0x22, 0x3813, 0x02, 100.0/65535.0, 0, &app.OIL_DEGRADATION, &myUtils::NullCb,30002}, //OIL_DEGRADATION          = 14355; // 0x3813 => ((A*256)+B)*(100/65535) //% degrado olio
  #if !GAS_ENGINE
  {"SERVICE_PIDS", "OIL_LEVEL", true, 0x18DA10F1, 0x22, 0x194E, 0x02, 0.1, 0, &app.OIL_LEVEL, &myUtils::NullCb,3000}, //OIL_LEVEL mm ((A*256)+B)*0.1
  #else
  {"SERVICE_PIDS", "OIL_LEVEL", true, 0x18DA10F1, 0x22, 0x3A41, 0x02, 0.001, 0, &app.OIL_LEVEL, &myUtils::NullCb,3000}, //OIL_LEVEL litri ((A*256)+B)*0.001
  #endif
  {"SERVICE_PIDS", "LAST_SERVICE_DATE", true, 0x18DA60F1, 0x22, 0x2801, 0x04, 1, 0, &app.NULL_VALUE, &myUtils::CustomBytesValues,120000},//LAST_SERVICE_DATE es. 62280120240208 => 2024 02 08 
  {"SERVICE_PIDS", "NUMBER_OF_SERVICES", true, 0x18DA60F1, 0x22, 0x2800, 0x01, 1, 0, &app.NUMBER_OF_SERVICES, &myUtils::NullCb,120001},  //NUMBER_OF_SERVICES         = 6589;  // 0x2800 => A //% di carica
  {"SERVICE_PIDS", "SERVICE_KM", true, 0x18DA60F1, 0x22, 0x0101, 0x03, 0.1, 0, &app.SERVICE_KM, &myUtils::NullCb,120002},  //SERVICE_KM      ((A*65536)+(B*256)+C)*0.1
  {"SERVICE_PIDS", "SERVICE_DAY", true, 0x18DA60F1, 0x22, 0x2807, 0x02, 1, 0, &app.SERVICE_DAY, &myUtils::NullCb,120002},  //SERVICE_DAY      (A*256)+B

  //battery 
  {"BATTERY_PIDS","BATTERY_OEM", true, 0x18DA10F1, 0x22, 0x1955, 0x02, 0.5/1000.0, 0, &app.BATTERY_OEM, &myUtils::NullCb, 2000},                  //BATTERY_OEM              = 6485;  // 0x1955 => ((A*256)+B)*(0.5/1000) 
  {"BATTERY_PIDS","IBS_CHARGE_LEVEL", true, 0x18DA10F1, 0x22, 0x19BD, 0x01, 1, 0, &app.IBS_CHARGE_LEVEL, &myUtils::NullCb,10000},                  //IBS_CHARGE_LEVEL         = 6589;  // 0x19BD => A //% di carica
  {"BATTERY_PIDS","BATTERY_TEMP_AMP", true, 0x18DA40F1, 0x22, 0x1005, 0x04, 1, 0, &app.NULL_VALUE, &myUtils::CustomBytesValues,100},//BATTERY_TEMP_AMP multirow pid : temp:byte[6]-40   amp: byte[10] + byte[12] . in test
  /*
      010
      0:621005005378
      1:0000C83401682C
      2:FD81EB
      
  */   

  //GAS PIDs
  #if !GAS_ENGINE
  {"GAS_PIDS", "GAS_TEMP",  true, 0x18DA10F1, 0x22, 0x1900, 0x02, 0.02, -40, &app.GAS_TEMP, &myUtils::NullCb, 10000},          //GAS_TEMP (((A*256)+B)*0.02)-40 
  {"GAS_PIDS", "GAS_PRESSURE", false, 0x18DA10F1, 0x22, 0x1947, 0x02, 0.05, 0, &app.GAS_PRESSURE, &myUtils::NullCb,700},    //GAS_PRESSURE  ((A*256)+B)*0.05)   
  {"GAS_PIDS", "GAS_H2O", false, 0x18DA10F1, 0x22, 0x18ff, 0x01, 1, 0, &app.GAS_H2O, &myUtils::NullCb,10002},               //GAS_H2O 1 byte 00 o 01
  #endif

  //TIRES PIDS
  {"TIRES_PIDS", "P_ASX", true, 0x18DAC7F1, 0x22, 0x31D0, 0x02, 0.001, 0, &app.P_ASX, &myUtils::NullCb, 3000},          //Pressione A SX: 2231D0 ((A*256)+B)/1000
  {"TIRES_PIDS", "P_ADX", true, 0x18DAC7F1, 0x22, 0x31D1, 0x02, 0.001, 0, &app.P_ADX, &myUtils::NullCb, 3000},          //Pressione A DX: 2231D1 ((A*256)+B)/1000
  {"TIRES_PIDS", "P_PSX", true, 0x18DAC7F1, 0x22, 0x31D2, 0x02, 0.001, 0, &app.P_PSX, &myUtils::NullCb, 3000},          //Pressione P SX: 2231D2 ((A*256)+B)/1000
  {"TIRES_PIDS", "P_PDX", true, 0x18DAC7F1, 0x22, 0x31D3, 0x02, 0.001, 0, &app.P_PDX, &myUtils::NullCb, 3000},          //Pressione P DX: 2231D3 ((A*256)+B)/1000

  {"TIRES_PIDS", "T_ASX", true, 0x18DAC7F1, 0x22, 0x40B1, 0x03, 1, 0, &app.NULL_VALUE, &myUtils::CustomBytesValues,3000},        //T_ASX multirow pid : byte[4]-50
  {"TIRES_PIDS", "T_ADX", true, 0x18DAC7F1, 0x22, 0x40B2, 0x03, 1, 0, &app.NULL_VALUE, &myUtils::CustomBytesValues,3000},        //T_ADX multirow pid : byte[4]-50
  {"TIRES_PIDS", "T_PSX", true, 0x18DAC7F1, 0x22, 0x40B3, 0x03, 1, 0, &app.NULL_VALUE, &myUtils::CustomBytesValues,3000},        //T_PSX multirow pid : byte[4]-50
  {"TIRES_PIDS", "T_PDX", true, 0x18DAC7F1, 0x22, 0x40B4, 0x03, 1, 0, &app.NULL_VALUE, &myUtils::CustomBytesValues,3000},        //T_PSX multirow pid : byte[4]-50
  /*
      00B
      0:6240B10A4B00
      1:00450182DC4B00    

      Rif P anteriore: 2.207 2231C6 6231C6089F  ((A*256)+B)/1000
      Rif P Posteriore: 2.414 2231C7 6231C7096E ((A*256)+B)/1000

  */
};

const size_t myApplication::pidListSize = sizeof(myApplication::pidList) / sizeof(OBD2Request);