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
extern ELM327 elm327;
extern TaskHandle_t asyncTask;
extern BluetoothSerial BluetoothConnector;
extern EspSoftwareSerial::UART ElmConnector;

extern myApplication app;

void myApplication::Init(){
    
    utils.DebugSerial("Init application");
    display.Init();
    delay(1000);
    display.SetPage(DwinPage::PAGE_LOADING);
    utils.InitFS();

    if(ELM_DIRECT)
    {
        ElmConnector.begin(ELM_BAUDRATE, SWSERIAL_8N1,ELM_RX, ELM_TX, false);
        if(!ElmConnector)
        {
          utils.DebugSerial("Elm direct not initialized");
          while(1){}
        }
    }

    //led
    wsLED.Brightness(255);
    wsLED.Update(0xff,0x99,0xDA,0, 0, true);
  
    buzzer.begin();
    buzzer.tone(500, 10, 50, 1); 

    delay(3000);

    utils.readSettings();

    utils.readOilValues();

    display.InitDisplayVariables();

    //setting up obd reading on core1
    xTaskCreatePinnedToCore( readELMValues, "readELMValues", 8096, NULL, 2, &asyncTask, 1); //core1

    if(ELM_DIRECT)
    {
       ELMInitialized = elm327.begin(ElmConnector, ELM_DUMP, 2000);
       utils.elm327Setup(); 
    }
    else{
       btUtils.checkBootPairedDevice();
    }

    displayHomePage();

    _isInitialized = true;
}

void myApplication::displayHomePage(){


  if(ELMInitialized || ELM_DIRECT)
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

      if(ELM_DIRECT)
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

  if(CurrentPidGroup==_group)
    return;
    
  utils.DebugSerial("Switch to group pids: "+_group);
  
  switchPidOccours = true;
  
  CurrentPidGroup = _group;

  if(ELMInitialized)
  {
    elm327.flushAll();
  }
  
  if(resetValues)
  {
    utils.resetELMValues();
    delay(500);
  }
  
  pidIndex = 0;
  switchPidOccours = false;
}

bool myApplication::unreadableData(){
  return (elm327.nb_rx_state == ELM_NO_DATA || elm327.nb_rx_state == ELM_STOPPED || elm327.nb_rx_state == ELM_TIMEOUT || elm327.nb_rx_state == ELM_GENERAL_ERROR || elm327.nb_rx_state == ELM_NO_RESPONSE);
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

//obd reading on Task1
void myApplication::readELMValues(void * parameter){

  float v = 0.0;
  PidRequest* request;  
  bool _enginerpm = false;
  bool _initEcu = false;
  String _prevHeader="";
    
  while(1)
  {
      buzzer.update();//it's faster here
      delay(10);
      
      if(app.pidListSize<1 || app.switchPidOccours)
        continue;
        
      if(app.isInitialized() && app.ELMInitialized)
      {
        
          if(app.ElmIsLost || app.CurrentStatus != AppStatus::Main)
          {
              continue;
          }

          app.tempAlarm = (app.DPF_EXAUST_TEMP>=utils.getSettings().temp_alert && utils.getSettings().temp);
          app.clogAlarm = (int(app.DPF_CLOGGING)>=utils.getSettings().clogging_alert  && utils.getSettings().clogging);
          app.regenAlarm = (app.regenerationInProgress && utils.getSettings().regeneration);          

          //we send RPM first to unlock ecu
          if(!_enginerpm)
          {
                elm327.processPID(0x01, 0x0C, 1 , 2, 1.0/4.0, 0); //standard ((A*256)+B)/4
                utils.DebugSerial("Readinf RPM to init flow");
          }
          //query pidsd
          else{
              
              app.checkRegenerationStatus();
              
              if(app.pidIndex>app.pidListSize)
              {
                          
                app.pidIndex = 0; 
                //delay for new read cycle
                delay(utils.getSettings().obd_cmd_delay);            
              }

              request = &app.pidList[app.pidIndex];

              //filter PIDS by group
              //debugSerial("Current: "+ CurrentPidGroup+" Pid: "+request->Name);
              if(request->Group!=app.CurrentPidGroup && request->Group!="ALL_PIDS")
              {
                 app.pidIndex++;
                 continue;
              }  
              
              //change header if different or we getting values
              if(_prevHeader != request->Header && elm327.nb_rx_state != ELM_GETTING_MSG)
              {
                 // debugSerial("Send Header: "+request->Header+" prev: "+_prevHeader);
                  elm327.sendHeader(request->Header, false);
              }

              if(elm327.nb_rx_state != ELM_GETTING_MSG)
              {

                //ready to send pid
                if(elm327.getCmdState() == ELM327ObdState::SEND_COMMAND)
                {
                    //first read
                    if(request->ReadTime==0)
                    {
                        request->ReadTime = millis();
                        utils.DebugSerial("Setup ReadTime for pid "+request->Name);
                    }
                    //next read
                    else{
                        if(millis()- request->ReadTime > request->ReadInterval)
                        {
                           request->ReadTime = millis();
                        }
                        else{
                            app.pidIndex++;
                            continue;
                        }
                    }
                }
              }

              //if AlwaysSendHeader we send header before pid
              if(request->AlwaysSendHeader && elm327.nb_rx_state != ELM_GETTING_MSG)
              {
                 elm327.sendHeader(request->Header, true);
              }
              
              v = elm327.processPID(request->Service, request->Pid, request->ExpectedRows , request->ExpectedBytes, request->ScaleFactor, request->AdjustFactor); 

             // debugSerial("Leggo PID: "+String(request->Pid, HEX)+" Valore: "+String(v)+" current:"+String(millis())+" Ultima lettura il "+String(request->ReadTime));
          }        
          
          if(elm327.nb_rx_state == ELM_SUCCESS)
          {
              if(!_enginerpm)
              {
                 _enginerpm = true;

                 utils.DebugSerial("Ecu inizializzata");

              }
              else
              {                                    
                    *request->BindValue = v;

                    //debugSerial("Letto PID: "+String(request->Name)+" valore:"+String(v));
                    
                    app.pidList[app.pidIndex].ValueCallback(request->Name);  
  
                    //only for RPM: we have to send everytime
                    if(!_enginerpm)
                    {
                      _enginerpm = true;
                    }
                                
                    _prevHeader = request->Header;
                    app.pidIndex++;
                
              }                          
          }
          else if(app.unreadableData())
          {
            
            if(_enginerpm)
            {
              app.pidIndex++;    
              *request->BindValue = 0.0;
              request->ReadTime = millis();
              app.pidList[app.pidIndex].ValueCallback(request->Name);
              _prevHeader = request->Header;
              //debugSerial("Unreadable PID: "+String(request->Pid, HEX));      
            }    
         }  

         utils.writeOilValues(app.lastOilLevel, app.OIL_LEVEL);

         if(app.OIL_LEVEL>0.0){
            app.lastOilLevel = app.OIL_LEVEL;
         }       
         
      }
      else{
        // _enginerpm = false;
      }
  } 

}

void myApplication::FlushPidReadInterval(String PidName)
{
  PidRequest* request;
  
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

void myApplication::Loop(){

    //all components loop here
    display.Loop();
    
    if (CurrentStatus == AppStatus::Main)
    {
        display.UpdateDisplayVariables();


        if ((_prevregenerationInProgress != regenerationInProgress))
        {
            FlushPidReadInterval("KM_FROM_LAST_DPF");
            FlushPidReadInterval("DPF_REGENERATIONS");
            _prevregenerationInProgress = regenerationInProgress;
            displayHomePage();
        }

        //if we lost connection
        btUtils.checkLostConnection();
    }
    
    btUtils.Loop();

    delay(10);
}