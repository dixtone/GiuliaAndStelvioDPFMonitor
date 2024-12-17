#include "definitions.h"
#include "myUtils.h"
#include "myBtUtils.h"
#include "myApplication.h"

extern ELM327 elm327;
extern myApplication app;
extern BluetoothSerial BluetoothConnector;
extern myBtUtils btUtils;
extern ESP32S3Buzzer buzzer;

using namespace std;

void myUtils::DebugSerial(String s){
    if(DEBUG_MODE)    {
        Serial.println(s);
    }
}

void myUtils::InitFS(){

    if (!SPIFFS.begin()) {  
        DebugSerial("must format!");
        SPIFFS.format();
        SPIFFS.begin();
    }
}

void myUtils::ResetFS(){
    SPIFFS.format();    
}

void myUtils::readSettings(){

 if (SPIFFS.exists(SETTINGS_FILE)) {
    File f = SPIFFS.open(SETTINGS_FILE, "r");
    char buffer[64];
    if (f) {
        int ln = 0;

        vector<String> v;
        while (f.available()) {
          v.push_back(f.readStringUntil('\n'));
        }
        
        //ensure lines corresponding to settings we want
        if (v.size() > 15)
        {
          f.close();
          DebugSerial("Settings Filesize not valid, removing...");
          SPIFFS.remove(SETTINGS_FILE);
        }
        else{
            for (String buffer : v) 
            {
              switch(ln)
              {
                  case 0:
                    _deviceSettings.touchscreen = buffer.toInt();
                  break;

                  case 1:
                    _deviceSettings.temp =  buffer.toInt();
                  break;

                  case 2:
                    _deviceSettings.clogging =  buffer.toInt();
                  break;
                  
                  case 3:
                    _deviceSettings.regeneration =  buffer.toInt();
                  break;

                  case 4:
                    _deviceSettings.clogging_alert =  buffer.toInt();
                  break;

                  case 5:
                    _deviceSettings.temp_alert = buffer.toInt();
                  break;

                  case 6:
                    _deviceSettings.alarm_frequency = buffer.toInt();
                  break;

                  case 7:
                    _deviceSettings.alarm_interval = buffer.toInt();
                  break;

                  case 8:
                    _deviceSettings.bt_spp = buffer.toInt();
                  break;

                  case 9:
                    _deviceSettings.bt_channel = buffer.toInt();
                  break;

                  case 10:
                    _deviceSettings.bt_pin = buffer;
                  break;

                  case 11:
                    _deviceSettings.boot_auto_connect = buffer.toInt();
                  break;

                  case 12:
                    _deviceSettings.obd_cmd_delay = buffer.toInt();
                  break;

                  case 13:
                    _deviceSettings.lost_connection_attemps = buffer.toInt();
                  break;

                  case 14:
                    _deviceSettings.car_model = buffer.toInt();
                  break;                  
              }
                ln++;
              }   
            }
        }      
        f.close();
   }
}  


void myUtils::writeSettings(){

 File f = SPIFFS.open(SETTINGS_FILE, "w");
 if (f) {

    //same order in _deviceSettings
    f.println(_deviceSettings.touchscreen);
    f.println(_deviceSettings.temp);
    f.println(_deviceSettings.clogging);
    f.println(_deviceSettings.regeneration);
    f.println(_deviceSettings.clogging_alert);
    f.println(_deviceSettings.temp_alert);
    f.println(_deviceSettings.alarm_frequency);
    f.println(_deviceSettings.alarm_interval);
    f.println(_deviceSettings.bt_spp);
    f.println(_deviceSettings.bt_channel);
    f.println(_deviceSettings.bt_pin);
    f.println(_deviceSettings.boot_auto_connect);
    f.println(_deviceSettings.obd_cmd_delay);
    f.println(_deviceSettings.lost_connection_attemps);
    f.println(_deviceSettings.car_model);    
    f.close();
  }
}

DeviceSettings& myUtils::getSettings(){
    return _deviceSettings;
}

void myUtils::resetFactory(){
  
  ResetFS();
  
  btUtils.resetPairedDevice();

  resetDevice();
}

void myUtils::resetDevice(){

  //in some conditions, after restart, buzzer tone continue (BUG?), write high freq to stop it
  buzzer.end();
  ledcWriteTone(BUZZER_CHN, 44000);
  DebugSerial("resetting...");
  ESP.restart(); 
  while(1){};
}

void myUtils::readOilValues(){

  if (!SPIFFS.exists(GRAPH_OIL_FILE))
  {
    File f = SPIFFS.open(GRAPH_OIL_FILE, "w");
    f.println("0.0 100");
    f.println("0.0 200");
    f.println("0.0 300");
    f.println("0.0 400");
    f.println("0.0 500");
    f.close();
  }
 
  File f = SPIFFS.open(GRAPH_OIL_FILE, "r");

  if (f) {
    int ln = 0;
    float lev = 0.0;
    String value;
    char *pvalue;
    
    DebugSerial("Reading oil level...");
    vector<String> v;
    while (f.available()) {
      v.push_back(f.readStringUntil('\n'));
    }
    //ensure lines corresponding to settings we want
    if (v.size() > 5)
    {
      f.close();
      DebugSerial("Graph oil Filesize not valid, removing...");
      resetOilValues();
    }
    else{
      for (String buffer : v) 
      {
        value = buffer.substring(0, buffer.indexOf(" "));
        lev = value.toFloat();
        value = buffer.substring(buffer.indexOf(" ")+1);

        if(lev > 0.0)
        {
          app.oilLevels[ln] = { lev , value.toInt()};
          app.lastOilLevel = lev;
        }         

        ln++;
      }
    }
  }
    
  DebugSerial("Last oil level recorded: "+String(app.lastOilLevel));
  
  f.close();
 
}

void myUtils::writeOilValues(float previousLevel, float currentLevel){

  if(currentLevel==0.0)
    return;

  short currentIndex=0;
  float lastValue = 0.0;
    
  for(int i=0;i<MAX_OIL_LOG;i++)
  {
      if(app.oilLevels[i].level>0.0)
      {
          lastValue = app.oilLevels[i].level;
          currentIndex++;
      }
      else{
        break;
      }
  }

  if(currentLevel==lastValue)
    return;

  app.updatedOilGraphValues = false;
  DebugSerial("Oil Level variation detected: "+String(currentLevel)+" "+String(lastValue));

  if(currentIndex<(MAX_OIL_LOG))
  {
    app.oilLevels[currentIndex].level = currentLevel;
  }
  //shift values and append last
  else{
    for(int i=1;i<MAX_OIL_LOG;i++){
        app.oilLevels[i-1].level = app.oilLevels[i].level;
    }

    app.oilLevels[MAX_OIL_LOG-1].level = currentLevel;
  }
  
  //now write values
  File f = SPIFFS.open(GRAPH_OIL_FILE, "w");

  for(int i=0;i<MAX_OIL_LOG;i++)
  {
     f.println(String(app.oilLevels[i].level)+" "+String(app.oilLevels[i].distance));
  }
 
  f.close();

  //refresh values
  readOilValues();
}

void myUtils::resetOilValues(){
  if (SPIFFS.exists(GRAPH_OIL_FILE))
  {
      SPIFFS.remove(GRAPH_OIL_FILE);      
  }

  for(int i=0; i < MAX_OIL_LOG;i++){
      app.oilLevels[i] = {0.0, 0};
  }

  DebugSerial("Deleted oil log files");
}

void myUtils::CustomBytesValues(String PidName){
                
    if(PidName=="TURBO_BOOST_PRESSURE")
    {   
        //atmo pressure 1009 mbar
        //boost pressure ((A*256)+B)-32767
        app.TURBO_BOOST_PRESSURE = ((elm327.responseByte[0]*256 + elm327.responseByte[1])-32767)/(app.ATMO_PRESSURE>0.0?app.ATMO_PRESSURE:1009);
    }
    else if(PidName=="KEY_NUMBER_AND_LAST")
    {
        app.KEY_NUMBER = elm327.responseByte[2];
        app.KEY_LAST = elm327.responseByte[3];
    }
    else if(PidName=="REMOTE_NUMBER_AND_LAST")
    {
        app.REMOTE_NUMBER = elm327.responseByte[0];
        app.REMOTE_LAST = elm327.responseByte[1];
    } 
    else if(PidName=="LAST_SERVICE_DATE"){
        app.LAST_SERVICE_DATE = String(elm327.responseByte[3],HEX)+"/"+ String(elm327.responseByte[2],HEX)+"/"+String(elm327.responseByte[0],HEX)+String(elm327.responseByte[1],HEX);
    }
    else if(PidName=="BATTERY_TEMP_AMP"){
        app.BATTERY_TEMP = elm327.responseByte[6]>0?elm327.responseByte[6]-40:0;
        app.BATTERY_AMP = elm327.responseByte[10]+elm327.responseByte[12]; //test
    }
    else if(PidName=="T_ASX"){
        app.T_ASX = elm327.responseByte[4]>0?elm327.responseByte[4]-50:0;
    }
    else if(PidName=="T_ADX"){
        app.T_ADX = elm327.responseByte[4]>0?elm327.responseByte[4]-50:0;
    }
    else if(PidName=="T_PSX"){
        app.T_PSX = elm327.responseByte[4]>0?elm327.responseByte[4]-50:0;
    }
    else if(PidName=="T_PDX"){
        app.T_PDX = elm327.responseByte[4]>0?elm327.responseByte[4]-50:0;
    }         
}

void myUtils::elm327Setup(){

  elm327.sendCommand_Blocking(SET_ALL_TO_DEFAULTS);
  delay(100);
  
  elm327.sendCommand_Blocking(RESET_ALL);
  delay(100);

  //elm327.sendCommand_Blocking("AT V1"); //VARIABLE_DLC_ON
  //delay(100);

  elm327.sendCommand_Blocking("AT AR"); //AUTOMATICALLY RESPOND
  delay(100);
  
  elm327.sendCommand_Blocking("AT E0"); //ECHO_OFF
  delay(100);

  elm327.sendCommand_Blocking("AT S0"); //PRINTING_SPACES_OFF
  delay(100);

  elm327.sendCommand_Blocking("AT AL"); //ALLOW_LONG_MESSAGES
  delay(100);
 
  elm327.sendCommand_Blocking("AT AT1"); //ADAPTIVE_TIMING_AUTO_1
  delay(100);  

  elm327.sendCommand_Blocking("AT ST32"); //SET_TIMEOUT_TO_H_X_4MS //> FF = 1 secondo, 99 default, 32 (hex) = 50ms (dec)
  delay(100);  

  elm327.sendCommand_Blocking("AT TP 7"); //ISO_15765_29_BIT_500_KBAUD  /6=ISO_15765_11_BIT_500_KBAUD 7=ISO_15765_29_BIT_500_KBAUD << questo è il protocollo alfa
  delay(100);  
  
  elm327.sendCommand_Blocking("AT SP 7"); //ISO_15765_29_BIT_500_KBAUD  /6=ISO_15765_11_BIT_500_KBAUD 7=ISO_15765_29_BIT_500_KBAUD << questo è il protocollo alfa
  delay(100);  

  //elm327.sendCommand_Blocking("AT TP A0"); //AUTO PROTOCOL  
}

void myUtils::resetELMValues(){
  
  for(int i=0;i<app.pidListSize;i++)
  {
    *app.pidList[i].BindValue = 0.0;
     app.pidList[i].ReadTime = 0;
     app.pidList[i].ValueCallback(app.pidList[i].Name);  
  }
}