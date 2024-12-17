#pragma once
#include "Arduino.h"

//BASED ON ELMDUINO


//-------------------------------------------------------------------------------------//
// Protocol IDs
//-------------------------------------------------------------------------------------//
const char AUTOMATIC                  = '0';
const char SAE_J1850_PWM_41_KBAUD     = '1';
const char SAE_J1850_PWM_10_KBAUD     = '2';
const char ISO_9141_5_BAUD_INIT       = '3';
const char ISO_14230_5_BAUD_INIT      = '4';
const char ISO_14230_FAST_INIT        = '5';
const char ISO_15765_11_BIT_500_KBAUD = '6';
const char ISO_15765_29_BIT_500_KBAUD = '7';
const char ISO_15765_11_BIT_250_KBAUD = '8';
const char ISO_15765_29_BIT_250_KBAUD = '9';
const char SAE_J1939_29_BIT_250_KBAUD = 'A';
const char USER_1_CAN                 = 'B';
const char USER_2_CAN                 = 'C';




//-------------------------------------------------------------------------------------//
// PIDs (https://en.wikipedia.org/wiki/OBD-II_PIDs)
//-------------------------------------------------------------------------------------//
const uint8_t SERVICE_01                       = 1;
	

const uint8_t SUPPORTED_PIDS_1_20              = 0;   // 0x00 - bit encoded
const uint8_t MONITOR_STATUS_SINCE_DTC_CLEARED = 1;   // 0x01 - bit encoded
const uint8_t FREEZE_DTC                       = 2;   // 0x02 -

const uint8_t SERVICE_02                       = 2;




//-------------------------------------------------------------------------------------//
// AT commands (https://www.sparkfun.com/datasheets/Widgets/ELM327_AT_Commands.pdf)
//-------------------------------------------------------------------------------------//
const char * const DISP_DEVICE_DESCRIPT       = "AT @1";       // General
const char * const DISP_DEVICE_ID             = "AT @2";       // General
const char * const STORE_DEVICE_ID            = "AT @3 %s";    // General
const char * const REPEAT_LAST_COMMAND        = "AT \r";       // General
const char * const ALLOW_LONG_MESSAGES        = "AT AL";       // General
const char * const AUTOMATIC_RECEIVE          = "AT AR";       // OBD
const char * const ADAPTIVE_TIMING_OFF        = "AT AT0";      // OBD
const char * const ADAPTIVE_TIMING_AUTO_1     = "AT AT1";      // OBD
const char * const ADAPTIVE_TIMING_AUTO_2     = "AT AT2";      // OBD
const char * const DUMP_BUFFER                = "AT BD";       // OBD
const char * const BYPASS_INIT_SEQUENCE       = "AT BI";       // OBD
const char * const TRY_BAUD_DIVISOR           = "AT BRD %02d"; // General
const char * const SET_HANDSHAKE_TIMEOUT      = "AT BRT %02d"; // General
const char * const CAN_AUTO_FORMAT_OFF        = "AT CAF0";     // CAN
const char * const CAN_AUTO_FORMAT_ON         = "AT CAF1";     // CAN
const char * const CAN_EXTENDED_ADDRESS_OFF   = "AT CEA";      // CAN
const char * const USE_CAN_EXTENDED_ADDRESS   = "AT CEA %02d"; // CAN
const char * const SET_ID_FILTER              = "AT CF %s";    // CAN
const char * const CAN_FLOW_CONTROL_OFF       = "AT CFC0";     // CAN
const char * const CAN_FLOW_CONTROL_ON        = "AT CFC1";     // CAN
const char * const SET_ID_MASK                = "AT CM %s";    // CAN
const char * const SET_CAN_PRIORITY           = "AT CP %02d";  // CAN
const char * const SHOW_CAN_STATUS            = "AT CS";       // CAN
const char * const CAN_SILENT_MODE_OFF        = "AT CSM0";     // CAN
const char * const CAN_SILENT_MODE_ON         = "AT CSM1";     // CAN
const char * const CALIBRATE_VOLTAGE_CUSTOM   = "AT CV %04d";  // Volts
const char * const RESTORE_CV_TO_FACTORY      = "AT CV 0000";  // Volts
const char * const SET_ALL_TO_DEFAULTS        = "AT D";        // General
const char * const DISP_DLC_OFF               = "AT D0";       // CAN
const char * const DISP_DLC_ON                = "AT D1";       // CAN
const char * const MONITOR_FOR_DM1_MESSAGES   = "AT DM1";      // J1939
const char * const DISP_CURRENT_PROTOCOL      = "AT DP";       // OBD
const char * const DISP_CURRENT_PROTOCOL_NUM  = "AT DPN";      // OBD
const char * const ECHO_OFF                   = "AT E0";       // General
const char * const ECHO_ON                    = "AT E1";       // General
const char * const FLOW_CONTROL_SET_DATA_TO   = "AT FC SD %s"; // CAN
const char * const FLOW_CONTROL_SET_HEAD_TO   = "AT FC SH %s"; // CAN
const char * const FLOW_CONTROL_SET_MODE_TO   = "AT FC SM %c"; // CAN
const char * const FORGE_EVENTS               = "AT FE";       // General
const char * const PERFORM_FAST_INIT          = "AT FI";       // ISO
const char * const HEADERS_OFF                = "AT H0";       // OBD
const char * const HEADERS_ON                 = "AT H1";       // OBD
const char * const DISP_ID                    = "AT I";        // General
const char * const SET_ISO_BAUD_10400         = "AT IB 10";    // ISO
const char * const SET_ISO_BAUD_4800          = "AT IB 48";    // ISO
const char * const SET_ISO_BAUD_9600          = "AT IB 96";    // ISO
const char * const IFR_VAL_FROM_HEADER        = "AT IFR H";    // J1850
const char * const IFR_VAL_FROM_SOURCE        = "AT IFR S";    // J1850
const char * const IFRS_OFF                   = "AT IFR0";     // J1850
const char * const IFRS_AUTO                  = "AT IFR1";     // J1850
const char * const IFRS_ON                    = "AT IFR2";     // J1850
const char * const IREAD_IGNMON_INPUT_LEVEL   = "AT IGN";      // Other
const char * const SET_ISO_SLOW_INIT_ADDRESS  = "AT IIA %02d"; // ISO
const char * const USE_J1939_ELM_DATA_FORMAT  = "AT JE";       // J1850
const char * const J1939_HEAD_FORMAT_OFF      = "AT JHF0";     // J1850
const char * const J1939_HEAD_FORMAT_ON       = "AT JHF1";     // J1850
const char * const USE_J1939_SAE_DATA_FORMAT  = "AT JS";       // J1850
const char * const SET_J1939_TIMER_X_TO_1X    = "AT JTM1";     // J1850
const char * const SET_J1939_TIMER_X_TO_5X    = "AT JTM5";     // J1850
const char * const DISP_KEY_WORDS             = "AT KW";       // ISO
const char * const KEY_WORD_CHECKING_OFF      = "AT KW0";      // ISO
const char * const KEY_WORD_CHECKING_ON       = "AT KW1";      // ISO
const char * const LINEFEEDS_OFF              = "AT L0";       // General
const char * const LINEFEEDS_ON               = "AT L1";       // General
const char * const LOW_POWER_MODE             = "AT LP";       // General
const char * const MEMORY_OFF                 = "AT M0";       // General
const char * const MEMORY_ON                  = "AT M1";       // General
const char * const MONITOR_ALL                = "AT MA";       // OBD
const char * const MONITOR_FOR_PGN            = "AT MP %s";    // J1939
const char * const MONITOR_FOR_RECEIVER       = "AT MR %02d";  // OBD
const char * const MONITOR_FOR_TRANSMITTER    = "AT MT %02d";  // OBD
const char * const NORMAL_LENGTH_MESSAGES     = "AT NL";       // OBD
const char * const SET_PROTO_OPTIONS_AND_BAUD = "AT PB %s";    // OBD
const char * const PROTOCOL_CLOSE             = "AT PC";       // OBD
const char * const ALL_PROG_PARAMS_OFF        = "AT PP FF OFF";// PPs
const char * const ALL_PROG_PARAMS_ON         = "AT PP FF ON"; // PPs
const char * const SET_PROG_PARAM_OFF         = "AT PP %s OFF";// PPs
const char * const SET_PROG_PARAM_ON          = "AT PP %s ON"; // PPs
const char * const SET_PROG_PARAM_VAL         = "AT PP %s SV %s"; // PPs
const char * const DISP_PP_SUMMARY            = "AT PPS";      // PPs
const char * const RESPONSES_OFF              = "AT R0";       // OBD
const char * const RESPONSES_ON               = "AT R1";       // OBD
const char * const SET_RECEIVE_ADDRESS_TO     = "AT RA %02d";  // OBD
const char * const READ_STORED_DATA           = "AT RD";       // General
const char * const SEND_RTR_MESSAGE           = "AT RTR";      // CAN
const char * const READ_VOLTAGE               = "AT RV";       // Volts
const char * const PRINTING_SPACES_OFF        = "AT S0";       // OBD
const char * const PRINTING_SPACES_ON         = "AT S1";       // OBD
const char * const STORE_DATA_BYTE            = "AT SD ";      // General
const char * const SET_HEADER                 = "AT SH %s";    // OBD
const char * const PERFORM_SLOW_INIT          = "AT SI";       // ISO
const char * const SET_PROTOCOL_TO_AUTO_H_SAVE = "AT SP A%c";  // OBD
const char * const SET_PROTOCOL_TO_H_SAVE     = "AT SP %c";    // OBD
const char * const SET_PROTOCOL_TO_AUTO_SAVE  = "AT SP 00";    // OBD
const char * const SET_REC_ADDRESS            = "AT SR %02d";  // OBD
const char * const SET_STANDARD_SEARCH_ORDER  = "AT SS";       // OBD
const char * const SET_TIMEOUT_TO_H_X_4MS     = "AT ST %02d";  // OBD
const char * const SET_WAKEUP_TO_H_X_20MS     = "AT SW %02d";  // ISO
const char * const SET_TESTER_ADDRESS_TO      = "AT TA %02d";  // OBD
const char * const TRY_PROT_H_AUTO_SEARCH     = "AT TP A%c";   // OBD
const char * const TRY_PROT_H                 = "AT TP %c";    // OBD
const char * const VARIABLE_DLC_OFF           = "AT V0";       // CAN
const char * const VARIABLE_DLC_ON            = "AT V1";       // CAN
const char * const SET_WAKEUP_MESSAGE         = "AT WM";       // ISO
const char * const WARM_START                 = "AT WS";       // General
const char * const RESET_ALL                  = "AT Z";        // General




//-------------------------------------------------------------------------------------//
// Class constants
//-------------------------------------------------------------------------------------//
const int8_t QUERY_LEN	           = 8;
const int8_t ELM_SUCCESS           = 0;
const int8_t ELM_NO_RESPONSE       = 1;
const int8_t ELM_BUFFER_OVERFLOW   = 2;
const int8_t ELM_GARBAGE           = 3;
const int8_t ELM_UNABLE_TO_CONNECT = 4;
const int8_t ELM_NO_DATA           = 5;
const int8_t ELM_STOPPED           = 6;
const int8_t ELM_TIMEOUT           = 7;
const int8_t ELM_GETTING_MSG       = 8;
const int8_t ELM_MSG_RXD           = 9;
const int8_t ELM_GENERAL_ERROR     = -1;


// Non-blocking (NB) command states
typedef enum { SEND_COMMAND,
               WAITING_RESP,
               RESPONSE_RECEIVED,
               DECODED_OK,
               ERROR } ELM327ObdState;


class ELM327
{
public:
	Stream* elm_port;

	bool connected = false;
	bool debugMode;
	char* payload;
	uint16_t PAYLOAD_LEN;
	int8_t nb_rx_state = ELM_GETTING_MSG;
	uint64_t response;
	uint8_t numPayChars;
	uint16_t timeout_ms;
	unsigned int frameRows;
	char* pframe;
	uint16_t frameBytes;
	uint8_t responseByte[64]; //must match with payloadLen


	bool begin(Stream& stream, const bool& debug = false, const uint16_t& timeout = 1000, const char& protocol = '0', const uint16_t& payloadLen = 64, const byte& dataTimeout = 0);
	bool initializeELM(const char& protocol = '0', const byte& dataTimeout = 0);
	void flushAll();
	void flushInputBuff();
	uint64_t findResponse();
	bool queryPID(const uint8_t& service, const uint16_t& pid, const uint8_t& num_responses = 1);
	bool queryPID(char queryStr[]);
	void  processPID(const uint8_t& service, const uint16_t& pid, const uint8_t& num_responses, const uint8_t& numExpectedBytes);
	float processPID(const uint8_t& service, const uint16_t& pid, const uint8_t& num_responses, const uint8_t& numExpectedBytes, const float& scaleFactor, const float& bias);
	void sendCommand(const char *cmd);
	int8_t sendCommand_Blocking(const char *cmd);
	int8_t get_response();
	void sendHeader(String& header, bool fast);
	bool timeout();
	float conditionResponse(const uint64_t& response, const uint8_t& numExpectedBytes, const float& scaleFactor = 1, const float& bias = 0);
	ELM327ObdState getCmdState();
	uint16_t freezeDTC();
	void printError();

private:
	char* _payload;
	char query[QUERY_LEN] = { '\0' };
	bool longQuery = false;
	uint32_t currentTime;
	uint32_t previousTime;

	ELM327ObdState nb_query_state = SEND_COMMAND; // Non-blocking query state

	void upper(char string[], uint8_t buflen);
	void formatQueryArray(uint8_t service, uint16_t pid, uint8_t num_responses);
	uint8_t ctoi(uint8_t value);
	int8_t nextIndex(char const *str,
	                 char const *target,
	                 uint8_t numOccur);
};
