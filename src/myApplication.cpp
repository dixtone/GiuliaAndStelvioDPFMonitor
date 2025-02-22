#include "definitions.h"
#include "myApplication.h"
#include "myDisplay.h"
#include "myUtils.h"
#include "myBtUtils.h"

extern myUtils utils;
extern myBtUtils btUtils;
extern myDisplay display;
extern WS2812Led wsLED;
extern ESP32S3Buzzer buzzer;
extern TaskHandle_t asyncTask;
extern BluetoothSerial BluetoothConnector;
extern OBD2 obd2;

extern myApplication app;

void myApplication::Init(){
    
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

    //setting up obd reading on core1
    xTaskCreatePinnedToCore( readOBD2Values, "readOBD2Values", 8096, NULL, 2, &asyncTask, 1); //core1
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
     switchPidGroup("DPF_PIDS", false);
     
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

  obd2.flush();

  if(CurrentPidGroup==_group)
    return;
    
  utils.DebugSerial("Switch to group pids: "+_group);
  
  switchPidOccours = true;
  
  CurrentPidGroup = _group;
  
  if(resetValues)
  {
    utils.resetOBD2RequestValues();
    delay(500);
  }
  
  pidIndex = 0;
  switchPidOccours = false;
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
  if(millis()-_previousAlarmMillis > utils.getSettings().alarm_interval*1000 || !_previousAlarmMillis)
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

//OBD2 reading on Task1
void myApplication::readOBD2Values(void * parameter){

  OBD2StatusType obdStatus;
  OBD2BroadcastPacket bp;
  OBD2Request* request;  
  long sendTime = millis();
  int timeoutSend = 1000;
  bool _enginerpm = false;
  long _prevHeader=0x0;

  while(1)
  {

    buzzer.update();//it's faster here

    if(app.isInitialized() && app.CurrentStatus==AppStatus::Main && (obd2.isELM327()?(app.ELMInitialized && !app.ElmIsLost):true))
    { 
           
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
     
      if(app.pidIndex>app.pidListSize)
      {
        app.pidIndex = 0;        
      }
      
      #if CANBUS_DIRECT
      request = &app.pidList[app.pidIndex];
      #else
      //when elm327 we have to send rpm to unlock ecu, once
      if(!_enginerpm)
      {
        request = &app.standardPidList[0];
        app.CurrentPidGroup = request->Group;
        obd2.sendRequest(request);
        
        if(obdStatus==OBD2StatusType::ready)
        {
          utils.DebugSerial("Readinf RPM to init flow");
        }
        
      }
      else{
        request = &app.pidList[app.pidIndex];
      }
      #endif
            

      //filter PIDS by group
      //debugSerial("Current: "+ CurrentPidGroup+" Pid: "+request->Name);
      if(request->Group!=app.CurrentPidGroup && request->Group!="ALL_PIDS")
      {
          app.pidIndex++;
          continue;
      }  

      if(obdStatus == OBD2StatusType::ready)
      {  
        //send every cmd delay setting
        if(millis()-sendTime > utils.getSettings().obd_cmd_delay)
        {           
          sendTime = millis();      
          
          //no timeout for first read
          if(request->ReadTime==0){
            request->ReadTime = -1 & 0xFF0000;
          }

          //check pid readtime
          if((millis() - request->ReadTime) > request->ReadInterval)
          {
              request->ReadTime = millis();
          }
          else{
            app.pidIndex++;
            continue;
          } 

          #if !CANBUS_DIRECT
          //when ELM we have to check if change request header
          if(_prevHeader != request->Header || request->AlwaysSendHeader)
          {
              _prevHeader = request->Header;
            obd2.sendElmHeader(request->Header);
          }
          #endif

          obd2.sendRequest(request);
          app.pidIndex++;
        }         
      }
      else if(obdStatus == OBD2StatusType::received){
       
        #if !CANBUS_DIRECT
        if(request->Name=="RPM")
        {
          _enginerpm = true;
          _prevHeader = request->Header;

          app.CurrentPidGroup = "DPF_PIDS"; //lets start from DPF pids, once
        }        
        #endif
      }      
      else if(obdStatus == OBD2StatusType::timeout){
        //timeout occours from canbus     
        request->ReadTime = 0;
        utils.DebugSerial("Unreadable PID (timeout): "+String(request->Pid, HEX));
      }  
      
      utils.writeOilValues(app.lastOilLevel, app.OIL_LEVEL);

      if(app.OIL_LEVEL>0.0){
        app.lastOilLevel = app.OIL_LEVEL;
      } 
      
    }
    
    delay(10);
  }
}

void myApplication::flushPidReadInterval(String PidName)
{
  OBD2Request* request;
  
  for(int i=0;i<pidListSize;i++)
  {
     request = &pidList[i];
    
     if(request->Name == PidName)
     {
       utils.DebugSerial("Reset PID: "+String(request->Pid, HEX)+"  Last read: "+String(request->ReadTime));
       request->ReadTime = 0;   
       return;
     }
  }
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
