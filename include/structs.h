#include <Arduino.h>

#ifndef structs_h
#define structs_h

struct OilLog {
  float level;
  long  distance;
};

struct DeviceSettings {
   bool touchscreen;    //touchscreen sound
   bool temp;           //temp sound
   bool clogging;       //clogging sound
   bool regeneration;   //regeneration sound
   int  clogging_alert; //clogging threshold
   int  temp_alert;     //temp threshold
   int  alarm_frequency;//buzzer freq
   int  alarm_interval; //buzzer interval in seconds
   bool bt_spp;         //simple secure pairing bluetooth
   int  bt_channel;     //bluetooth channel
   String bt_pin;       //bluetooth pin
   bool boot_auto_connect;   //bluetooth autoconnect
   int  obd_cmd_delay;  //obd read command delay
   int  lost_connection_attemps;  //
   int  car_model; //0 giulia, 1 stelvio
};

//bluetooth item
struct BtItem {
  String    name; 
  String    address;
  int8_t    rssi;
  int       channel;
  int       index;
};

//paired device
struct PairedDevice {
  String name;
  String address;
  int channel;
  bool phantom;
}; 

//PID Struct
struct PidRequest {
  String Group;
  String Name;
  bool AlwaysSendHeader;
  String Header;
  uint8_t  Service;
  uint16_t Pid;
  uint8_t  ExpectedRows;
  uint8_t  ExpectedBytes;
  float  ScaleFactor;
  float  AdjustFactor;
  float  *BindValue;
  void (*ValueCallback)(String pid);
  long ReadInterval; 
  long ReadTime;
}; 

enum class AppStatus{
    Undefined = 0,
    Modal = 1,
    BtScanning = 2,
    BtScanComplete = 3,
    BtListDevices = 4,
    BtPairing = 5,
    BtNoConnection = 6,
    BtLost = 7,
    Main = 8,
    Settings = 9,
    DTC = 10
};

//DWIN VARS AND ADDRESS DEFINITIONS
enum DwinAddress{
    //page bt search
    BT_VP_ADDR = 0x2000, //page address
    BT_SRC_BTN = 0x0d0d, 
    BT_BACK_BTN = 0x00f7,
    BT_RECONNECT_BTN = 0x00F1,
    BT_TXT_LOST = 0x2001, //connection lost text 40ch max
    BT_DEV1_BTN = 0x0001, //button device 1
    BT_DEV2_BTN = 0x0002, //button device 2
    BT_DEV3_BTN = 0x0003, //button device 3
    BT_DEV4_BTN = 0x0004, //button device 4
    BT_DEV1_TXT = 0x2140, //text device 1 max 40ch
    BT_DEV2_TXT = 0x2180, //text device 2 max 40ch
    BT_DEV3_TXT = 0x21c0, //text device 3 max 40ch
    BT_DEV4_TXT = 0x2200, //text device 4 max 40ch
    SETTING_INFO_BTN = 0x2240, //button info
    SETTING_INFO1_TXT = 0x22c0, //device info 1 max 40ch
    SETTING_INFO2_TXT = 0x2300, //device info 2 max 40ch
    SETTING_INFO3_TXT = 0x2340, //device info 3 max 40ch
    SETTING_INFO4_TXT = 0x2380, //device info 4 max 40ch

    //settings
    SETTING_VP_ADDR = 0x3000,
    SETTING_BUTTON = 0x00F1,
    SETTING_BACK_BTN = 0x00f7,
    SETTING_NEXT_BTN = 0x00f8,
    SETTING_CANCEL_BT_BTN = 0x00f0,
    SETTING_INIT_ALL_BTN = 0x00f3,
    SETTING_RESET_BTN = 0x00f2,
    SETTING_TEMP = 0x3200,
    SETTING_CLOGGING = 0x3204,
    SETTING_REGEN = 0x3208,
    SETTING_TEMP_VALUE = 0x320A,
    SETTING_CLOG_VALUE = 0x323A,
    SETTING_ALARM_REPEAT = 0x324A,
    SETTING_READ_TIMEOUT_VALUE = 0x33AA,
    SETTING_RECONNECT_TRIES_VALUE = 0x33BB,
    SETTING_AUTORECONNECT_VALUE = 0x33C0,
    SETTING_SIMPLEAUTH_VALUE = 0x33C4,
    SETTING_BTCHANNEL_VALUE = 0x33C8,
    SETTING_CARVALUE_VALUE = 0x33E4,

    //dashboard 
    DASH_ICO_BATT = 0x4001, //bit from 0-2
    DASH_TXT_BATT = 0x4050, //4bytes float
    DASH_TXT_IBS  = 0x40A0, //2bytes int
    DASH_COOLANT_ICO = 0x40D0, //bit from 0-2
    DASH_TXT_COOLANT = 0x40E0, //2bytes int
    DASH_OIL_ICO = 0x4110, //bit from 0-2
    DASH_TXT_OIL = 0x4150, //2bytes int
    DASH_GAUGE_CLOGGING = 0x4180, //int is %
    DASH_TXT_CLOGG_PERC = 0x4190, //2bytes int
    DASH_TXT_EXAUST = 0x41C0, //2bytes int
    DASH_GAUGE_ICON = 0x41F0, //bit from 0-3
    DASH_TXT_KM_REGEN = 0x4200, //2bytes int
    DASH_TXT_NREGEN = 0x4204, //2bytes int
    DASH_TXT_INTAKE_TEMP = 0x4209, //2bytes int
    DASH_TXT_REGEN_PROCESS = 0x4220, //2bytes int

    //values
    PAGE_SLIDE_SWITCH= 0x14, //default sent from component
    VALUES_SWITCH_PAGES= 0x4000, //2bytes int > sends page no to switch
    DASH_VAL_TURBO= 0x4212, //4bytes float
    DASH_VAL_DIFFPRESSURE= 0x4216, //long 4 bytes
    DASH_VAL_DIFFPRESSURE_GAUGE= 0x50A0, //long 4 bytes
    DASH_VAL_TURBO_GAUGE= 0x4222, //int 2 bytes
    DASH_VAL_THR_VALVE= 0x4230, //int 2 bytes %
    DASH_VAL_THR_VALVE_GAUGE= 0x5000, //int 2 bytes
    DASH_VAL_NKEYS= 0x4233, //int 2 bytes
    DASH_VAL_KEYLAST= 0x4236, //int 2 bytes
    DASH_VAL_NREMOTE= 0x4239, //int 2 bytes
    DASH_VAL_LASTREMOTE= 0x4242, //int 2 bytes
    DASH_VAL_OIL_LEVEL= 0x4245, //float 4 bytes
    DASH_VAL_OIL_KM= 0x4250, //long 8 bytes
    DASH_VAL_OIL_TOTAL= 0x4260, //int 2 bytes
    DASH_VAL_OIL_GG= 0x4264, //long 4 bytes
    DASH_VAL_OIL_DATE= 0x4270, //text 14len
    DASH_VAL_BAT_TEMP= 0x4280, //int 2 bytes
    DASH_VAL_BAT_AMP= 0x4284, //int 2 bytes
    DASH_VAL_GAS_TEMP= 0x4287, //int 2 bytes
    DASH_VAL_GAS_PRESS= 0x4290, //float 4 bytes
    DASH_VAL_GAS_H2O= 0x4295, //text 2
    DASH_VAL_P_PDX= 0x4300, //float 4 bytes
    DASH_VAL_P_PSX= 0x4305, //float 4 bytes
    DASH_VAL_P_ASX= 0x4310, //float 4 bytes
    DASH_VAL_P_ADX= 0x4315, //float 4 bytes
    DASH_VAL_T_PDX= 0x4330, //int 2 bytes
    DASH_VAL_T_PSX= 0x4334, //int 2 bytes
    DASH_VAL_T_ASX= 0x4338, //int 2 bytes
    DASH_VAL_T_ADX= 0x4340, //int 2 bytes

    //oil graph
    GRAPH_OIL_LINES= 0x6000, 
    GRAPH_OIL_DOTS = 0x7000, 
    GRAPH_OIL_LEV01 = 0x6600, //text 6
    GRAPH_OIL_LEV02 = 0x6608, //text 6
    GRAPH_OIL_LEV03 = 0x660D, //text 6
    GRAPH_OIL_LEV04 = 0x6615, //text 6
    GRAPH_OIL_LEV05 = 0x661F, //text 6
    GRAPH_OIL_KM01 = 0x6700, //text 6
    GRAPH_OIL_KM02 = 0x6708, //text 6
    GRAPH_OIL_KM03 = 0x670D, //text 6
    GRAPH_OIL_KM04 = 0x6715, //text 6
    GRAPH_OIL_KM05 = 0x671F, //text 6
    GRAPH_OIL_TRASH = 0x6800, //button trash
    GRAPH_OIL_TRASH_CONFIRM = 0x00f1 //button confirm trash
};

enum DwinPage{
    //DWIN Pages definitions
    PAGE_LOGO =0,
    PAGE_LOADING =1,
    PAGE_BT_CONNECT =2,
    PAGE_BT_CONNECT_SUCCESS =3,
    PAGE_BT_CONNECT_FAILED =4,
    PAGE_BT_SEARCHING =5,
    PAGE_BT_LIST_DEVICES =6,
    PAGE_BT_RECONNECT=8,
    PAGE_BT_SEARCH =9,
    PAGE_SETTING_0 =10,
    PAGE_SETTING_1 =11,
    PAGE_SETTING_2 =16,
    PAGE_SETTING_3 =17,
    PAGE_SETTING_4 =19,
    PAGE_SETTING_5 =18,
    PAGE_DASHBOARD =23,
    PAGE_REGEN=24,
    PAGE_VALUES_CHOICE =25,
    PAGE_INTAKE_PRESSURES =27,
    PAGE_KEYSREMOTES =28,
    PAGE_OIL_INFOS =29,
    PAGE_OIL_GRAPH=30,
    PAGE_BATTERY_INFOS =32,
    PAGE_GASOIL_INFOS =33,
    PAGE_TIRES_INFOS =34
    
};

//interface
class LoopableClass{
    public:
        virtual ~LoopableClass(){}
        virtual void Loop(){}
        virtual void Init(){}
};

#endif