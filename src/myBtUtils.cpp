#include "definitions.h"
#include "myBtUtils.h"
#include "myApplication.h"
#include "myDisplay.h"

extern myApplication app;
extern BluetoothSerial BluetoothConnector;
extern myUtils utils;
extern myDisplay display;
extern OBD2 obd2;

using namespace std;

void myBtUtils::reinitScan(){
    _devicesN = -1;

    memset(_foundDevices, 0, sizeof(_foundDevices)); 
    memset(_availBtDevices, 0, sizeof(_availBtDevices)); 
}

void myBtUtils::beginScan(){
    
    //resetting some vars
    app.CurrentStatus = AppStatus::BtScanning;
    
    
    /* 
    //debug fake devices 
    devicesN++;
    foundDevices[devicesN] = "00:11:22:33";
    availDevices[devicesN].name ="Bt Fake 1";
    availDevices[devicesN].address = "00:11:22:33";
    devicesN++;
    foundDevices[devicesN] = "00:11:22:334";
    availDevices[devicesN].name ="Bt Fake 2";
    availDevices[devicesN].address = "00:11:22:33";
    devicesN++;
    foundDevices[devicesN] = "00:11:22:334";
    availDevices[devicesN].name ="Bt Fake 3";
    availDevices[devicesN].address = "00:11:22:33";
    devicesN++;
    foundDevices[devicesN] = "00:11:22:334";
    availDevices[devicesN].name ="Bt Fake 4";
    availDevices[devicesN].address = "00:11:22:33";
    devicesN++;
    //end debug fake devices
    */

    String btname = String(device_ssid)+String("BT-")+String((uint32_t)ESP.getEfuseMac(), HEX);

    //remove bounded devices on esp32
    clearBoundedDevices();
     
    BluetoothConnector.begin(btname, true); //Bluetooth device name
    
    reinitScan();

    elm327Init(false);
    
    // scroll through the results if we found paired, then connect
    if(!pairedDeviceStored.phantom)
    {

      char a1[18];
      char a2[18];   
      pairedDeviceStored.address.toCharArray(a1,18);
   
      for(short i=0;i<=_devicesN;i++)
      {

        _availBtDevices[i].address.toCharArray(a2,18);
        
        if(strcmp(a1,a2)==0)
        {

          //try to connect
          pairedDevice = pairedDeviceStored;
        
          if(elm327Init(true))
          {
              app.displayHomePage();
              return;
          } 
        }
      }
    }
     
    //show list of paired devices
    app.CurrentStatus = AppStatus::BtScanComplete;
    utils.DebugSerial("List devices total: "+(String(_devicesN)));    

    String s = "";

    display.getHmi()->setText(BT_DEV1_TXT, "Nessun dispositivo trovato!", 30);
    display.getHmi()->setText(BT_DEV2_TXT, " ", 30);
    display.getHmi()->setText(BT_DEV3_TXT, " ", 30);
    display.getHmi()->setText(BT_DEV4_TXT, " ", 30);

    if(_devicesN!=-1)
    {
      for(short i=0,j=1,yy=0;i<=_devicesN;i++,j++)
      {
      
           s= String(j)+") "+String(_availBtDevices[i].name)+" - "+String(_availBtDevices[i].address);
           
           utils.DebugSerial("Device: "+s);
      
           if(_availBtDevices[i].address!="")
           {
              switch(i)
              {
                case 0:
                  display.getHmi()->setText(DwinAddress::BT_DEV1_TXT, s, 30);
                break;
  
                case 1:
                  display.getHmi()->setText(DwinAddress::BT_DEV2_TXT, s, 30);
                break;
  
                case 2:
                  display.getHmi()->setText(DwinAddress::BT_DEV3_TXT, s, 30);
                break;
  
                case 3:
                  display.getHmi()->setText(DwinAddress::BT_DEV4_TXT, s, 30);
                break;
              }
           }
      }  
    }
  
}

void myBtUtils::connectPairedDevice(){
    if(pairBtDevice())
    {
        utils.DebugSerial("Abbinamento avvenuto con successo!");
    }

    app.displayHomePage();
}

void myBtUtils::checkAndConnectDeviceFromList(short index){

    if(_availBtDevices[index].address!="" && _devicesN>-1)
    {
      pairedDevice = {};
      pairedDevice.name = _availBtDevices[index].name;
      pairedDevice.address = _availBtDevices[index].address;
      pairedDevice.channel = _availBtDevices[index].channel;
      pairedDevice.phantom = false;
    
      utils.DebugSerial("Connetto dispositivo "+_availBtDevices[index].address);
      
      if(pairBtDevice())
      {
        utils.DebugSerial("Abbinamento avvenuto con successo!");
      }

      app.displayHomePage();      
    }
}

bool myBtUtils::pairBtDevice(){

    app.CurrentStatus = AppStatus::BtPairing;

    display.getHmi()->setPage(DwinPage::PAGE_BT_CONNECT);
    
    if(elm327Init(true))
    { 
        if(savePairedDevice())
        {
          display.getHmi()->setPage(DwinPage::PAGE_BT_CONNECT_SUCCESS);
          delay(1000);
          return true;
        }
        else
        {
          app.ELMInitialized = false;
          
          pairedDevice = {};
          pairedDevice.phantom = true;

          display.getHmi()->setPage(DwinPage::PAGE_BT_CONNECT_FAILED); 
          delay(1000);
          return false;
        }
    }
    else{
      display.getHmi()->setPage(DwinPage::PAGE_BT_CONNECT_FAILED);        
    }
 
    delay(1000);
    return false;
}

void myBtUtils::unpairBtDevice(){
    if(!pairedDevice.phantom)
    {
        resetPairedDevice();
        app.displayHomePage();
    }
}

void myBtUtils::searchForBluetoothDevices()
{
   display.getHmi()->setPage(DwinPage::PAGE_BT_SEARCHING);

   beginScan();

   display.getHmi()->setPage(DwinPage::PAGE_BT_LIST_DEVICES);
}

void myBtUtils::checkBootPairedDevice(){

    getPairedDevice();

    if (!pairedDevice.phantom && utils.getSettings().boot_auto_connect)
    {
        pairedDeviceStored = pairedDevice;

        display.getHmi()->setPage(DwinPage::PAGE_BT_CONNECT);

        utils.DebugSerial("Found saved device: '" + pairedDeviceStored.address+"' "+String(pairedDeviceStored.phantom?"phantom":"real"));

        if (elm327Init(true)) {

            display.getHmi()->setPage(DwinPage::PAGE_BT_CONNECT_SUCCESS);
            delay(1000);
        }
        else {
            display.getHmi()->setPage(DwinPage::PAGE_BT_CONNECT_FAILED);
            delay(1000);
        }
    }
}

bool myBtUtils::btAlreadyFound(String s){

  for(short i=0;i<MAX_BT_DEVICES;i++)
  {
      if(_foundDevices[i]==s)
      {
        return true;
      }
  }
  return false;  
}

void myBtUtils::btAdvertisedDeviceFound(BTAdvertisedDevice* device) {

  if (app.CurrentStatus==AppStatus::BtScanning)
  {
    
     if(!btAlreadyFound(device->getAddress().toString().c_str()) && _devicesN<(MAX_BT_DEVICES-1))
     {

        if(DEBUG_MODE)
        {
          Serial.print("Advertise device: ");
          Serial.println(String(device->getAddress().toString().c_str()));
        }

        _devicesN++; 
        _foundDevices[_devicesN] = device->getAddress().toString().c_str();

        _availBtDevices[_devicesN].name =  device->haveName()?String(device->getName().c_str()):"Unknow";
        _availBtDevices[_devicesN].address = String(device->getAddress().toString().c_str());
        _availBtDevices[_devicesN].rssi =  device->haveRSSI()?device->getRSSI():-1;
        _availBtDevices[_devicesN].channel =  utils.getSettings().bt_channel;       
        
     }
  }
}

bool myBtUtils::elm327Init(bool realPairing){

    app.ELMInitialized = false;
    
    if(realPairing)
    {
      utils.DebugSerial("Inizio bluetooth su: "+pairedDevice.name+" "+pairedDevice.address);
    }
    else{
      
      utils.DebugSerial("Inizio bluetooth su: FAKEDEVICEFORSCANNING012345");
    }
    
    //BluetoothConnector.begin("SD4-DPF-MONITOR", true);
    String btname = String(device_ssid)+String("BT-")+String((uint32_t)ESP.getEfuseMac(), HEX);

    clearBoundedDevices();
    //BluetoothConnector.setPin("1234");

    /*if(deviceSettings.bt_spp)
    {
      BluetoothConnector.enableSSP();
    }*/
   
    BluetoothConnector.begin(btname, true); //Bluetooth device name
          
    utils.resetOBD2RequestValues();
         
    //convert bluetooth address ex. da 30:37:66:6b:87:94 to uint8t    
    uint8_t data[6];
    sscanf(pairedDevice.address.c_str(), "%x:%x:%x:%x:%x:%x", &data[0], &data[1], &data[2], &data[3], &data[4], &data[5]);
    uint8_t address[6] = { data[0], data[1], data[2], data[3], data[4], data[5]};

    //debugSerial("pair with "+pairedDevice.address); 
      
    //while connecting it scan, then show device list
    //bool connectionResult = BluetoothConnector.connect(address, pairedDevice.channel);
    bool connectionResult = realPairing?BluetoothConnector.connect(address):BluetoothConnector.connect("FAKEDEVICEFORSCANNING012345@");

    utils.DebugSerial("Connection status: "+ String(connectionResult?"success":"failed")); 
     
    if(connectionResult)
    {
       utils.DebugSerial("Connessione bluetooth avvenuta.");
     
       app.ELMInitialized = obd2.BeginElm327(BluetoothConnector, 2000);
       
       if(app.ELMInitialized)
       {
         utils.DebugSerial("elm327 inizializzato.");
         utils.elm327Setup(); 
         utils.resetOBD2RequestValues();
       }
       else{
        utils.DebugSerial("elm327 non inizializzato.");
       }
    }
    else{

      app.CurrentStatus = AppStatus::BtScanning;
      
      BTScanResults *results = BluetoothConnector.getScanResults();
  
      for(int i=0;i<results->getCount();i++)
      {
          btAdvertisedDeviceFound(results->getDevice(i));
      }
          
      BluetoothConnector.disconnect();
      BluetoothConnector.end();
                
      app.CurrentStatus = AppStatus::BtScanComplete;
      utils.DebugSerial("Connessione bluetooth non avvenuta.");
      delay(1000);
    }    

    return app.ELMInitialized;
}

void myBtUtils::getPairedDevice(){

  pairedDevice = {};
  pairedDevice.phantom = true;
    
 //first row: name, second address, last: channel
 if (SPIFFS.exists(PAIRED_DEVICE_FILE)) {
   
    utils.DebugSerial("File "+String(PAIRED_DEVICE_FILE)+" exists, read...");

    File f = SPIFFS.open(PAIRED_DEVICE_FILE, "r");
    
    if (f){
      
      utils.DebugSerial("Open file "+String(PAIRED_DEVICE_FILE));
      int ln = 0;

      vector<String> v;
      while (f.available()) {
        v.push_back(f.readStringUntil('\n'));
      }

      //ensure lines corresponding to settings we want
      if (v.size() > 3)
      {
        f.close();
        utils.DebugSerial("Paired device Filesize not valid, removing...");
        SPIFFS.remove(PAIRED_DEVICE_FILE);
      }
      else{
        for (String buffer : v) 
        {
          switch(ln)
          {
            case 0:
              pairedDevice.name= buffer;
            break;

            case 1:
              pairedDevice.address =  buffer;
            break;
            case 2:
              pairedDevice.channel = buffer.toInt();
            break;
          }
          ln++;
        }
        pairedDevice.phantom = pairedDevice.address.length() < 1;
        f.close();
      }
     
    }
    else{
        utils.DebugSerial("Unable to open "+String(PAIRED_DEVICE_FILE));
    }   
  }
  else{
    utils.DebugSerial("Unexisting file "+String(PAIRED_DEVICE_FILE));
  }
}


bool myBtUtils::savePairedDevice(){


 File f = SPIFFS.open(PAIRED_DEVICE_FILE, "w");
 if (f) {
    //same order in _pairedDevice
    f.println(pairedDevice.name);
    f.println(pairedDevice.address);
    f.println(String(pairedDevice.channel));
    f.close();
    utils.DebugSerial("Saved paired device: "+pairedDevice.name+" "+pairedDevice.address);  
    return true;
  }
  utils.DebugSerial("Unable to save paired device: "+pairedDevice.name+" "+pairedDevice.address); 
  return false;
}

void myBtUtils::resetPairedDevice(){

  pairedDevice = {};
  pairedDevice.phantom = true;
  app.ELMInitialized = false;

  File f = SPIFFS.open(PAIRED_DEVICE_FILE, "r");
  if (f) {
      f.close();
      SPIFFS.remove(PAIRED_DEVICE_FILE);
  }
  
  clearBoundedDevices();
}

void myBtUtils::clearBoundedDevices(){

    if(!btStart()) {
      utils.DebugSerial("Failed to initialize controller");
      return;
    }
  
    if(esp_bluedroid_init() != ESP_OK) {
      utils.DebugSerial("Failed to initialize bluedroid");
      return;
    }
  
    if(esp_bluedroid_enable() != ESP_OK) {
      utils.DebugSerial("Failed to enable bluedroid");
      return;
    }
  
    int count = esp_bt_gap_get_bond_device_num();
    
    uint8_t pairedDeviceBtAddr[20][6];
    char bda_str[18];

    esp_err_t tError =  esp_bt_gap_get_bond_device_list(&count, pairedDeviceBtAddr);
    
    if(ESP_OK == tError) {
      
      for(int i = 0; i < count; i++) {

        if(DEBUG_MODE)
        {
          Serial.print("Found bonded device # :");
          if (pairedDeviceBtAddr[i] != NULL) {
             sprintf(bda_str, "%02x:%02x:%02x:%02x:%02x:%02x", pairedDeviceBtAddr[i][0], pairedDeviceBtAddr[i][1], pairedDeviceBtAddr[i][2], pairedDeviceBtAddr[i][3], pairedDeviceBtAddr[i][4], pairedDeviceBtAddr[i][5]);
          }
          Serial.println(bda_str);
        }
         
        esp_err_t tError = esp_bt_gap_remove_bond_device(pairedDeviceBtAddr[i]);

        if(DEBUG_MODE)
        {
            if(ESP_OK == tError) {
              Serial.print("Removed bonded device # "); 
            } else {
              Serial.print("Failed to remove bonded device # ");
            }
            Serial.println(i);
        }
      }        
    }
}

bool myBtUtils::checkLostConnection(){
    
    if(CANBUS_DIRECT) return false;
   
    if (!BluetoothConnector.hasClient() && !pairedDevice.phantom && app.CurrentStatus == AppStatus::Main)
    {
        utils.DebugSerial("Bluetooth has no client: Set app status to BtLost");
        app.CurrentStatus = AppStatus::BtLost;
       
    }

    return false;
}

void myBtUtils::Loop(){
  
  if(CANBUS_DIRECT) return;

  //if we lost connection
  checkLostConnection();

  if (app.CurrentStatus == AppStatus::BtLost)
  {

      //first time we enter in btlost
      if (_lostConnectionAttempts == -1)
      {
          app.ElmIsLost = true; 
          _previousLostConnectionMillis = millis();
          _lostConnectionAttempts = utils.getSettings().lost_connection_attemps;
          display.getHmi()->setPage(DwinPage::PAGE_BT_CONNECT_FAILED);
          display.getHmi()->setText(BT_TXT_LOST, "Prova di riconnessione: " + String(_lostConnectionAttempts), 40);
          utils.DebugSerial("Init lost connection");
      }
      else{

          if (millis() - _previousLostConnectionMillis > 3000)
          {

              _previousLostConnectionMillis = millis();

              //until trying reconnect
              if(_lostConnectionAttempts > 0)
              {
                  //attempt new connection
                  utils.DebugSerial("Connessione persa: tentativo di riconnessione "+String(_lostConnectionAttempts));

                  //if reinit, we are ok!
                  if (elm327Init(true))
                  {
                      app.ElmIsLost = false;
                      _lostConnectionAttempts = -1;
                      utils.elm327Setup();
                      app.CurrentStatus = AppStatus::Main;
                      app.displayHomePage();
                      delay(1000);
                  }
                  else {  
                      _lostConnectionAttempts--;
                      app.CurrentStatus = AppStatus::BtLost;                      
                  }
                  
                  display.getHmi()->setText(BT_TXT_LOST, "Prova di riconnessione: " + String(_lostConnectionAttempts), 40); 
              }
              //trying finished, goto home
              else{
                  app.ELMInitialized = false;
                  app.ElmIsLost = false;
                  utils.DebugSerial("Attempt end, goto home");
                  _lostConnectionAttempts = -1;
                  app.CurrentStatus = AppStatus::BtNoConnection;
                  app.displayHomePage();
                  return;   
              }
              
          }
      }
  }
}