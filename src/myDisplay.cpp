#include "myDisplay.h"
#include "myApplication.h"
#include "myUtils.h"
#include "myBtUtils.h"
#include "version.h"

extern myApplication app;
extern myUtils utils;
extern myBtUtils btUtils;

myDisplay::myDisplay(DWIN* hmi){
    _hmi = hmi;
}    

void myDisplay::Init(){

    _hmi->hmiCallBack(this);
    _hmi->echoEnabled(DEBUG_MODE);
    _hmi->returnResponseEnabled(false); //set to false if T5L_OS_xxx_noresponse_xxx.BIN
    _hmi->restartHMI();
}

void myDisplay::SetPage(DwinPage p) {
    CurrentPage = p;
    _hmi->setPage(CurrentPage);
}

void myDisplay::InitDisplayVariables()
{

   DeviceSettings settings = utils.getSettings();

  _hmi->setText(DwinAddress::SETTING_INFO1_TXT, "SD4 DPF Monitor - build "+String(BUILD_NUMBER), 40);
  _hmi->setText(DwinAddress::SETTING_INFO2_TXT, "Esp32 rev. "+String(ESP.getChipRevision())+" cores "+ String(ESP.getChipCores()) , 40);
  _hmi->setText(DwinAddress::SETTING_INFO3_TXT, VERSION, 40);
  _hmi->setText(DwinAddress::SETTING_INFO4_TXT, "2024 @ Dixtone - All right reserved", 40);

   //settings
  _hmi->setVP(DwinAddress::SETTING_TEMP, settings.temp);
  _hmi->setVP(DwinAddress::SETTING_CLOGGING, settings.clogging);
  _hmi->setVP(DwinAddress::SETTING_REGEN, settings.regeneration);
  _hmi->setVP(DwinAddress::SETTING_AUTORECONNECT_VALUE, settings.boot_auto_connect);
  _hmi->setVP(DwinAddress::SETTING_SIMPLEAUTH_VALUE, settings.bt_spp);
  _hmi->setVPInt(DwinAddress::SETTING_CLOG_VALUE, settings.clogging_alert);
  _hmi->setVPInt(DwinAddress::SETTING_TEMP_VALUE, settings.temp_alert);
  _hmi->setVPInt(DwinAddress::SETTING_ALARM_REPEAT, settings.alarm_interval);
  _hmi->setVPInt(DwinAddress::SETTING_BTCHANNEL_VALUE, settings.bt_channel);
  _hmi->setVPInt(DwinAddress::SETTING_READ_TIMEOUT_VALUE, settings.obd_cmd_delay);
  _hmi->setVPInt(DwinAddress::SETTING_RECONNECT_TRIES_VALUE, settings.lost_connection_attemps);
  _hmi->setVPInt(DwinAddress::SETTING_CARVALUE_VALUE, settings.car_model);

  //valori dashboard
  UpdateDisplayVariables();
}

void myDisplay::UpdateDisplayVariables(){

    if(app.CurrentStatus!= AppStatus::Main){
        return;
    }

    app.checkAlarms();

    DeviceSettings settings = utils.getSettings();

    if(app.CurrentPidGroup=="DPF_PIDS")
    {
        _hmi->setVP(DwinAddress::DASH_ICO_BATT, (app.IBS_CHARGE_LEVEL<65.0 || app.BATTERY_OEM<12.0)?0:(app.BATTERY_OEM<12.6)?1:2);
        _hmi->setVPFloat(DwinAddress::DASH_TXT_BATT, app.BATTERY_OEM);   
        _hmi->setVPInt(DwinAddress::DASH_TXT_IBS, (int)app.IBS_CHARGE_LEVEL);

        _hmi->setVP(DwinAddress::DASH_COOLANT_ICO, app.ENGINE_COOLANT_TEMP_OEM>103.0?0:(app.ENGINE_COOLANT_TEMP_OEM<=103.0 && app.ENGINE_COOLANT_TEMP_OEM>95.0)?1:2);
        _hmi->setVPInt(DwinAddress::DASH_TXT_COOLANT, (int)app.ENGINE_COOLANT_TEMP_OEM);

        _hmi->setVP(DwinAddress::DASH_OIL_ICO, app.OIL_DEGRADATION>85.0?2:(app.OIL_DEGRADATION<=85.0 && app.OIL_DEGRADATION>50.0)?1:0);
        _hmi->setVPInt(DwinAddress::DASH_TXT_OIL, (int)app.OIL_DEGRADATION);

        _hmi->setVPInt(DwinAddress::DASH_GAUGE_CLOGGING, map((int)app.DPF_CLOGGING, 0, 107, 0, 7)); //map gauge areas(7) and clogging value( 0 to 107%)
        _hmi->setVPInt(DwinAddress::DASH_TXT_CLOGG_PERC, (int)app.DPF_CLOGGING);  
        _hmi->setVPInt(DwinAddress::DASH_TXT_EXAUST, (int)app.DPF_EXAUST_TEMP);  

        if(app.DPF_EXAUST_TEMP>=settings.temp_alert && app.DPF_CLOGGING>= settings.clogging_alert)
        {
            _hmi->setVP(DwinAddress::DASH_GAUGE_ICON, 1);
        }
        else if(app.DPF_EXAUST_TEMP>=settings.temp_alert)
        {
            _hmi->setVP(DwinAddress::DASH_GAUGE_ICON, 3);
        }
        else if(app.DPF_CLOGGING>=settings.clogging_alert)
        {
            _hmi->setVP(DwinAddress::DASH_GAUGE_ICON, 2);
        }
        else{
            _hmi->setVP(DwinAddress::DASH_GAUGE_ICON, 0);
        }  

        _hmi->setVPInt(DwinAddress::DASH_TXT_KM_REGEN, (int)app.KM_FROM_LAST_DPF);
        _hmi->setVPInt(DwinAddress::DASH_TXT_NREGEN, (int)app.DPF_REGENERATIONS);
        _hmi->setVPInt(DwinAddress::DASH_TXT_REGEN_PROCESS, (int)app.DPF_REGENERATION_PROCESS);
    }
    else if(app.CurrentPidGroup=="ENG_PIDS")
    {
        //page engine
        _hmi->setVPInt(DwinAddress::DASH_VAL_TURBO_GAUGE, (int)(app.TURBO_BOOST_PRESSURE*100));
        _hmi->setVPFloat(DwinAddress::DASH_VAL_TURBO, app.TURBO_BOOST_PRESSURE);
        _hmi->setVPInt4(DwinAddress::DASH_VAL_DIFFPRESSURE, (int)app.DIFFERENTIAL_PRESSURE);
        _hmi->setVPInt(DwinAddress::DASH_VAL_DIFFPRESSURE_GAUGE, (int)(app.DIFFERENTIAL_PRESSURE/10));
        _hmi->setVPInt(DwinAddress::DASH_VAL_THR_VALVE, (int)app.OVER_ENG_VALVE);
        _hmi->setVPInt(DwinAddress::DASH_VAL_THR_VALVE_GAUGE, 100-(int)app.OVER_ENG_VALVE); //reverse rotation for gauge
    }
    else if(app.CurrentPidGroup=="REMOTEKEY_PIDS")
    {
        //page key e remotes
        _hmi->setVPInt(DwinAddress::DASH_VAL_NKEYS, (int)app.KEY_NUMBER);
        _hmi->setVPInt(DwinAddress::DASH_VAL_KEYLAST, (int)app.KEY_LAST);
        _hmi->setVPInt(DwinAddress::DASH_VAL_NREMOTE, (int)app.REMOTE_NUMBER);
        _hmi->setVPInt(DwinAddress::DASH_VAL_LASTREMOTE, (int)app.REMOTE_LAST);
    }
    else if(app.CurrentPidGroup=="SERVICE_PIDS")
    {
        //page oil service
        _hmi->setVPInt(DwinAddress::DASH_TXT_OIL, (int)app.OIL_DEGRADATION);
        _hmi->setVPFloat(DwinAddress::DASH_VAL_OIL_LEVEL, app.OIL_LEVEL);
        _hmi->setVPInt4(DwinAddress::DASH_VAL_OIL_KM, (int)app.SERVICE_KM);
        _hmi->setVPInt(DwinAddress::DASH_VAL_OIL_TOTAL, (int)app.NUMBER_OF_SERVICES);
        _hmi->setVPInt4(DwinAddress::DASH_VAL_OIL_GG, (int)app.SERVICE_DAY);
        _hmi->setText(DwinAddress::DASH_VAL_OIL_DATE, app.LAST_SERVICE_DATE, 14);
        drawOilGraphValues();
    }
    else if(app.CurrentPidGroup=="BATTERY_PIDS")
    {
        //battery values
        _hmi->setVPInt(DwinAddress::DASH_VAL_BAT_TEMP, (int)app.BATTERY_TEMP);
        _hmi->setVPFloat(DwinAddress::DASH_VAL_BAT_AMP, app.BATTERY_AMP);
        _hmi->setVPInt(DwinAddress::DASH_TXT_IBS, (int)app.IBS_CHARGE_LEVEL);
        _hmi->setVPFloat(DwinAddress::DASH_TXT_BATT, app.BATTERY_OEM);   
    }
    else if(app.CurrentPidGroup=="GAS_PIDS")
    {
        //gasoil values
        _hmi->setVPInt(DwinAddress::DASH_VAL_GAS_TEMP, (int)app.GAS_TEMP);
        _hmi->setVPFloat(DwinAddress::DASH_VAL_GAS_PRESS, app.GAS_PRESSURE);
        _hmi->setText(DwinAddress::DASH_VAL_GAS_H2O, (int)app.GAS_H2O>0?"SI":"No", 2);
    }
    else if(app.CurrentPidGroup=="TIRES_PIDS")
    {
        //tires values
        _hmi->setVPFloat(DwinAddress::DASH_VAL_P_ASX, app.P_ASX);
        _hmi->setVPFloat(DwinAddress::DASH_VAL_P_ADX, app.P_ADX);
        _hmi->setVPFloat(DwinAddress::DASH_VAL_P_PSX, app.P_PSX);
        _hmi->setVPFloat(DwinAddress::DASH_VAL_P_PDX, app.P_PDX);
        _hmi->setVPInt(DwinAddress::DASH_VAL_T_ASX, (int)app.T_ASX);
        _hmi->setVPInt(DwinAddress::DASH_VAL_T_ADX, (int)app.T_ADX);
        _hmi->setVPInt(DwinAddress::DASH_VAL_T_PSX, (int)app.T_PSX);
        _hmi->setVPInt(DwinAddress::DASH_VAL_T_PDX, (int)app.T_PDX);
        
    }
}

bool myDisplay::isHomePage(){
  return  _hmi->getPage()<19;   
}

void myDisplay::drawOilGraphValues(){

  if(app.CurrentStatus != AppStatus::Main || app.updatedOilGraphValues || app.OIL_LEVEL<0.1)
    return;

  int displayWidth = getHmi()->getHorizontalResolution();
  int displayHeight = getHmi()->getVerticalResolution();

  app.updatedOilGraphValues = true;
  utils.readOilValues();

  /* get values then send graph coords
  DWIN::pointCoord coordinates[] = {
    {100, 180, 0xFF00}, {290, 180, 0xF000}, {490, 180, 0x0f0f}, {680, 180, 0x0f0f}, {870, 60, 0x0f0f}
  };
  */

  _hmi->setText(DwinAddress::GRAPH_OIL_LEV01, "", 6);
  _hmi->setText(DwinAddress::GRAPH_OIL_KM01, "", 6);
  _hmi->setText(DwinAddress::GRAPH_OIL_LEV02, "", 6);
  _hmi->setText(DwinAddress::GRAPH_OIL_KM02, "", 6);
  _hmi->setText(DwinAddress::GRAPH_OIL_LEV03, "", 6);
  _hmi->setText(DwinAddress::GRAPH_OIL_KM03, "", 6);
  _hmi->setText(DwinAddress::GRAPH_OIL_LEV04, "", 6);
  _hmi->setText(DwinAddress::GRAPH_OIL_KM04, "", 6);
  _hmi->setText(DwinAddress::GRAPH_OIL_LEV05, "", 6);
  _hmi->setText(DwinAddress::GRAPH_OIL_KM05, "", 6);        

  int addr_level[5] = {DwinAddress::GRAPH_OIL_LEV01, DwinAddress::GRAPH_OIL_LEV02, DwinAddress::GRAPH_OIL_LEV03, DwinAddress::GRAPH_OIL_LEV04, DwinAddress::GRAPH_OIL_LEV05};
  int addr_distance[5] = {DwinAddress::GRAPH_OIL_KM01, DwinAddress::GRAPH_OIL_KM02, DwinAddress::GRAPH_OIL_KM03, DwinAddress::GRAPH_OIL_KM04, DwinAddress::GRAPH_OIL_KM05};
  

  long GRAPH_X_MIN = 40;
  long GRAPH_X_MAX = (displayWidth-GRAPH_X_MIN);
  long GRAPH_TICK = (GRAPH_X_MAX/4);
  long GRAPH_Y_MIN = displayHeight>400?((displayHeight/2)-100):60; //custom different display
  long GRAPH_Y_MAX = displayHeight>400?((displayHeight/2)+60):180; //custom different display
  long GRAPH_COLOR = 0xf000;
  long GRAPH1_X = GRAPH_X_MIN;
  long GRAPH1_Y = 0;
  long GRAPH2_X = GRAPH_X_MIN/2+(GRAPH_TICK*1);
  long GRAPH2_Y = 0;
  long GRAPH3_X = GRAPH_X_MIN/2+(GRAPH_TICK*2);
  long GRAPH3_Y = 0;
  long GRAPH4_X = GRAPH_X_MIN/2+(GRAPH_TICK*3);
  long GRAPH4_Y = 0;
  long GRAPH5_X = (GRAPH_TICK*4);
  long GRAPH5_Y = 0;

  //init with zero
  //max Y: 180
  //min Y: 60
  DWIN::pointCoord coordinates[5] = {
    {GRAPH1_X, 0, 0x0000, 0, 0}, {GRAPH2_X, 0, 0x0000, 0, 0}, {GRAPH3_X, 0, 0x0000, 0, 0}, {GRAPH4_X, 0, 0x0000, 0, 0}, {GRAPH5_X, 0, 0x0000, 0, 0}
  };
   
  int count = 0;
  for(int i=0,j=1;i<MAX_OIL_LOG;i++,j++)
  {
      if(app.oilLevels[i].level>0.0)
      {

        if(DEBUG_MODE)
        {
          Serial.print("Value: ");
          Serial.print(app.oilLevels[i].level);
          Serial.print(" Second Value: ");
          Serial.println(app.oilLevels[i].distance);    
        }
        
        _hmi->setText(addr_level[i], String(app.oilLevels[i].level), 6);
        _hmi->setText(addr_distance[i], String(j), 6); 

        //set Y coords impostiamo le coordinate Y tenendo conto che Ymax = 180 e Ymin = 60, quindi mappiamo i valori
        coordinates[i].y = map(app.oilLevels[i].level, 0, 70, GRAPH_Y_MIN, GRAPH_Y_MAX);       
        coordinates[i].color = GRAPH_COLOR;   
        coordinates[i].w = 20;
        coordinates[i].h = 20;
        count++;
      }
      else{
        // line for zero values
        _hmi->setText(addr_level[i], String(""), 6);
        _hmi->setText(addr_distance[i], String(""), 6); 
        coordinates[i].y = coordinates[i-1].y;  
        coordinates[i].color = GRAPH_COLOR;
        coordinates[i].w = 0;
        coordinates[i].h = 0;
      }
      
  }

  //no values: show current
  if(count==0)
  {
      _hmi->setText(addr_level[0], String(app.OIL_LEVEL), 6);
      _hmi->setText(addr_distance[0], String(1), 6); 
      coordinates[0].y = map(app.OIL_LEVEL, 0.0, 80.0, GRAPH_Y_MIN, GRAPH_Y_MAX);        
      coordinates[0].color = GRAPH_COLOR;
      coordinates[0].w = 10;
      coordinates[0].h = 10;

      for(int i=1,j=2;i<MAX_OIL_LOG;i++,j++)
      {
        _hmi->setText(addr_level[i], String(""), 6);
        _hmi->setText(addr_distance[i], String(""), 6); 
        coordinates[i].y = coordinates[0].y;  
        coordinates[i].color = GRAPH_COLOR;
        coordinates[i].w = 0;
        coordinates[i].h = 0;
      }
  }
  
  _hmi->drawLine(0x6000, 0xf000, coordinates, sizeof(coordinates)/sizeof(coordinates[0]));
  _hmi->drawSquarePoints(0x7000, coordinates, sizeof(coordinates)/sizeof(coordinates[0]));
}

void myDisplay::displayEventHandler(long address, int lastByte, int message, String response){ 

  if(DEBUG_MODE)
  {
     Serial.println("On Event : " + String(address, HEX) + " "+String(address)+" | Data : " + String(lastByte, HEX) + " | Message : " + String(message, HEX) + " "+String(message)+ " | Response " +response );
  }

  switch(address)
  {
      //page slide switch e change page
      case DwinAddress::PAGE_SLIDE_SWITCH:
      case DwinAddress::VALUES_SWITCH_PAGES:

        switch(message)
        {
           case DwinPage::PAGE_INTAKE_PRESSURES:
            app.switchPidGroup("ENG_PIDS", true);
           break;

           case DwinPage::PAGE_KEYSREMOTES:
             app.switchPidGroup("REMOTEKEY_PIDS", true);
           break;

           case DwinPage::PAGE_OIL_INFOS:
             app.switchPidGroup("SERVICE_PIDS", true);
           break; 

           case DwinPage::PAGE_OIL_GRAPH:
             app.switchPidGroup("SERVICE_PIDS", true);
             drawOilGraphValues();
           break;
                    
           case DwinPage::PAGE_GASOIL_INFOS:
             app.switchPidGroup("GAS_PIDS", true);;
           break;   

           case DwinPage::PAGE_BATTERY_INFOS:
             app.switchPidGroup("BATTERY_PIDS", true);;
           break;

           case DwinPage::PAGE_TIRES_INFOS:
             app.switchPidGroup("TIRES_PIDS", true);
           break;

           case DwinPage::PAGE_SETTING_0:
              app.CurrentStatus = AppStatus::Settings;
           break;
           
           default:
              app.switchPidGroup("DPF_PIDS", true);
           break;
        }
        delay(250);
      break;

      case DwinAddress::BT_VP_ADDR:
        if(message==DwinAddress::BT_SRC_BTN) //search
        {
          return btUtils.searchForBluetoothDevices();
        }
        else if(message==DwinAddress::BT_RECONNECT_BTN)
        {
          app.switchPidGroup("DPF_PIDS", true);
          btUtils.connectPairedDevice(); //bluetooth pairing
        }
        else if(message==DwinAddress::BT_BACK_BTN) //back
        {
           utils.writeSettings();
           app.switchPidGroup("DPF_PIDS", true);
           app.displayHomePage();
        }
        else if(message==DwinAddress::BT_DEV1_BTN){ //connect device 1
          app.switchPidGroup("DPF_PIDS", true);
          btUtils.checkAndConnectDeviceFromList(0);
        }
        else if(message==DwinAddress::BT_DEV2_BTN){ //connect device 2
          app.switchPidGroup("DPF_PIDS", true);
          btUtils.checkAndConnectDeviceFromList(1);
        }
        else if(message==DwinAddress::BT_DEV3_BTN){ //connect device 3
          app.switchPidGroup("DPF_PIDS", true);
          btUtils.checkAndConnectDeviceFromList(2);
        }
        else if(message==DwinAddress::BT_DEV4_BTN){ //connect device 4
          app.switchPidGroup("DPF_PIDS", true);
          btUtils.checkAndConnectDeviceFromList(3);
        }
      break;

      case DwinAddress::SETTING_VP_ADDR:
        app.CurrentStatus = AppStatus::Settings;
        if(message == DwinAddress::SETTING_BACK_BTN)
        {
           utils.writeSettings();
           InitDisplayVariables();
           app.switchPidGroup("DPF_PIDS", true);  
           app.displayHomePage();
        }
        else if(message==DwinAddress::SETTING_CANCEL_BT_BTN)
        {
            utils.writeSettings();
            InitDisplayVariables();
            btUtils.unpairBtDevice();
        }
        else if(message == DwinAddress::SETTING_INIT_ALL_BTN)
        {
           _hmi->setPage(0);
           utils.resetFactory();
        }
        else if(message == DwinAddress::SETTING_RESET_BTN)
        {
          utils.writeSettings();
          utils.resetDevice();
        }
        else if(message == DwinAddress::SETTING_BUTTON)
        {
            _hmi->setPage(DwinPage::PAGE_SETTING_0);
        }
      break;

      case DwinAddress::SETTING_TEMP:
        utils.getSettings().temp = message;
      break;
  
      case DwinAddress::SETTING_CLOGGING:
        utils.getSettings().clogging = message;
      break;
  
      case DwinAddress::SETTING_REGEN:
        utils.getSettings().regeneration = message;
      break;

      case DwinAddress::SETTING_TEMP_VALUE:
        utils.getSettings().temp_alert = message;
      break;
  
      case DwinAddress::SETTING_CLOG_VALUE:
        utils.getSettings().clogging_alert = message;
      break;
      
      case DwinAddress::SETTING_ALARM_REPEAT:
        utils.getSettings().alarm_interval = message;
      break; 
    
     case DwinAddress::SETTING_READ_TIMEOUT_VALUE:
        utils.getSettings().obd_cmd_delay = message;
     break;

     case DwinAddress::SETTING_RECONNECT_TRIES_VALUE:
        utils.getSettings().lost_connection_attemps = message;
     break;

    case DwinAddress::SETTING_AUTORECONNECT_VALUE:
       utils.getSettings().boot_auto_connect = message;
    break;

    case DwinAddress::SETTING_SIMPLEAUTH_VALUE:
      utils.getSettings().bt_spp = message;
    break;

    case DwinAddress::SETTING_BTCHANNEL_VALUE:
      utils.getSettings().bt_channel = message;
    break;

    case DwinAddress::SETTING_CARVALUE_VALUE:
      utils.getSettings().car_model = message;
      utils.writeSettings();
      InitDisplayVariables();
    break;

    case DwinAddress::GRAPH_OIL_TRASH:
      if(message == DwinAddress::GRAPH_OIL_TRASH_CONFIRM)
      {
        utils.resetOilValues();
        app.updatedOilGraphValues = false;
        drawOilGraphValues();
      }
    break;
    
    //TODO DTC
  }
  
}

void myDisplay::Loop(){

    _hmi->listen();
}