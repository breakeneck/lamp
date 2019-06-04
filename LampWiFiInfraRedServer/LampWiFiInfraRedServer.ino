/*
 * IRremoteESP8266: IRrecvDemo - demonstrates receiving IR codes with IRrecv
 * This is very simple teaching code to show you how to use the library.
 * If you are trying to decode your Infra-Red remote(s) for later replay,
 * use the IRrecvDumpV2.ino example code instead of this.
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * Copyright 2009 Ken Shirriff, http://arcfn.com
 * Example circuit diagram:
 *  https://github.com/markszabo/IRremoteESP8266/wiki#ir-receiving
 * Changes:
 *   Version 0.2 June, 2017
 *     Changed GPIO pin to the same as other examples.
 *     Used our own method for printing a uint64_t.
 *     Changed the baud rate to 115200.
 *   Version 0.1 Sept, 2015
 *     Based on Ken Shirriff's IrsendDemo Version 0.1 July, 2009
 */
 
#include <ESP8266WiFi.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

// LAMP STATES
const int ON = 0;
const int OFF = 1;

// ACTIONS
const int ACTION_UP = 1;
const int ACTION_RIGHT = 2;
const int ACTION_DOWN = 3;
const int ACTION_LEFT = 4;
const int ACTION_TOGGLE_ALL = 5;
const int ACTION_LOW_LIGHT = 6;
const int ACTION_DIODES = 7;
const int ACTION_BRIGHT_LIGHT = 8;
const int ACTION_ALL_OFF = 9;

// WIFI CONSTANT AND VARIABLES
const char SSID[] = "BedroomLamp";
const char PASS[] = "krishnakrishna";
const char REQ_END = '\r';
WiFiServer server(80);
IPAddress IP(192,168,4,15);
IPAddress mask = (255, 255, 255, 0);
const int WIFI_COMMAND_CLICK = 1;
const int WIFI_COMMAND_DB_CLICK = 2;
const int WIFI_COMMAND_LONG_CLICK = 3;

// BOARD PINS CONSTANTS
uint16_t RELAY_PIN4 = 5; // D1
uint16_t RELAY_PIN3 = 4; // D2
uint16_t RELAY_PIN2 = 0; // D3
uint16_t RELAY_PIN1 = 2; // D4
uint16_t IR_RECV_PIN = 12; // D6

// INFRARED CONSTANTS AND VARIABLES
const int LAMPS_COUNT = 4;
uint16_t relays[LAMPS_COUNT] = {RELAY_PIN1, RELAY_PIN2, RELAY_PIN3, RELAY_PIN4};
int lamps[LAMPS_COUNT] = {OFF, OFF, OFF, OFF}; // UP LEFT DOWN RIGHT
const int LAMP_UP = 0;
const int LAMP_RIGHT = 1;
const int LAMP_DOWN = 2;
const int LAMP_LEFT = 3;
const int BRIGHT_LIGHT_LAMPS[] = {LAMP_UP, LAMP_LEFT};
  
IRrecv irrecv(IR_RECV_PIN);
decode_results results;


//----REMOTE IR CONTROLLERS CONSTANTS----------
//-------- FACTORY REMOTE
//--FF18E7--16718055
//--FF5AA5--16734885
//--FF4AB5--16730805
//--FF10EF--16716015
//--FF38C7--16726215
const int SYMBOL_UP = 16718055;
const int SYMBOL_RIGHT = 16734885;
const int SYMBOL_DOWN = 16730805;
const int SYMBOL_LEFT = 16716015;
const int SYMBOL_OK = 16726215;
const int SYMBOL_HASH = 16756815;
const int SYMBOL_STAR = 16738455;
//--------LG 32LD320 REMOTE
//--20DF02FD--551486205
//--20DF609F--551510175
//--20DF827D--551518845
//--20DFE01F--551542815
//--20DF22DD--551494365
const int LG_UP = 551486205;
const int LG_RIGHT = 551510175;
const int LG_DOWN = 551518845;
const int LG_LEFT = 551542815;
const int LG_OK = 551494365;
//-----------ID REMOTE / SONY DSS CABLE
//--27ED--10221
//--17ED--6125
//--67ED--26605
//--57ED--22509
//--68ED--26861
const int SONY_UP = 10221;
const int SONY_RIGHT = 6125;
const int SONY_DOWN = 26605;
const int SONY_LEFT = 22509;
const int SONY_OK = 26861;
//-------IRPLUS /SONY-TV
//--2F0--752
//--CD0--3280
//--AF0--2800
//--2D0--720
//--A70--2672
const int SONY_PLUS_UP = 752;
const int SONY_PLUS_RIGHT = 3280;
const int SONY_PLUS_DOWN = 2800;
const int SONY_PLUS_LEFT = 720;
const int SONY_PLUS_OK = 2672;



void setup() {
  Serial.begin(9600);
  while (!Serial) delay(50);
  
  irrecv.enableIRIn();
  setupWiFiAP();
  
  initLamps();
}

void loop() {
  receiveIr();
  
  processWiFiCommand();
}


void receiveIr() {
  if (irrecv.decode(&results)) {
    Serial.println("--"+uint64ToString(results.value, HEX)+"--"+uint64ToString(results.value));

    processIrCommand(results.value);
    
    irrecv.resume();  // Receive the next value
  }
  delay(100);
}


int hasLight() {
  for(int i = 0; i < LAMPS_COUNT; i++)
    if(lamps[i] == ON)
      return true;

  return false;
}


void initLamps() {
  for(int i = 0; i < LAMPS_COUNT; i++)
    pinMode(relays[i], OUTPUT);

  syncLampState();
}


bool processIrCommand(int key) {
  switch(key) {
      case LG_UP:
      case SONY_UP:
      case SONY_PLUS_UP:
      case SYMBOL_UP:
        runAction(ACTION_UP);
        break;
      case LG_RIGHT:
      case SONY_RIGHT:
      case SONY_PLUS_RIGHT:
      case SYMBOL_RIGHT:
        runAction(ACTION_RIGHT);
        break;
      case LG_DOWN:
      case SONY_DOWN:
      case SONY_PLUS_DOWN:
      case SYMBOL_DOWN:
        runAction(ACTION_DOWN);
        break;
      case LG_LEFT:
      case SONY_LEFT:
      case SONY_PLUS_LEFT:
      case SYMBOL_LEFT:
        runAction(ACTION_LEFT);
        break;
      case LG_OK:
      case SONY_OK:
      case SONY_PLUS_OK:
      case SYMBOL_OK:
        runAction(ACTION_TOGGLE_ALL);
        break;
      default:
        return false;
    }

    return true;
}


int setStateForAllLamps(int state) {
  for(int i = 0; i < LAMPS_COUNT; i++) {
    lamps[i] = state;
  }
}


void syncLampState() {
  for(int i = 0; i < LAMPS_COUNT; i++)
      digitalWrite(relays[i], lamps[i]);
}


void setupWiFiAP() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(SSID, PASS);
  WiFi.softAPConfig(IP, IP, mask);
  
  server.begin();
  Serial.println("Server started.");
  Serial.print("IP: "+WiFi.softAPIP());
}


void processWiFiCommand() {
  WiFiClient client = server.available();
  if (!client) {return;}
  String request = client.readStringUntil('\r');
  client.flush();

  String response;
  switch(request.toInt()) {
    case WIFI_COMMAND_CLICK:
       response = runAction(ACTION_LOW_LIGHT);
       break;
    case WIFI_COMMAND_DB_CLICK:
       response = runAction(ACTION_BRIGHT_LIGHT);
       break;
    case WIFI_COMMAND_LONG_CLICK:
       response = runAction(ACTION_DIODES);
       break;
  }

  client.println(response + REQ_END);
//  Serial.println("********************************");
//  Serial.println("WIFI: " + response);
}

String runAction(int action) {
  String actionStr = "";
  
  switch(action) {
    case ACTION_UP: 
      actionStr = "ACTION_UP";
      lamps[LAMP_UP] = lamps[LAMP_UP]^1; 
      break;
      
    case ACTION_RIGHT:
      actionStr = "ACTION_RIGHT";
      lamps[LAMP_RIGHT] = lamps[LAMP_RIGHT]^1; 
      break;
      
    case ACTION_DOWN:
      actionStr = "ACTION_DOWN";
      lamps[LAMP_DOWN] = lamps[LAMP_DOWN]^1; 
      break;
      
    case ACTION_LEFT: 
      actionStr = "ACTION_LEFT";
      lamps[LAMP_LEFT] = lamps[LAMP_LEFT]^1; 
      break;
      
    case ACTION_TOGGLE_ALL:
      actionStr = "ACTION_TOGGLE_ALL";
      if(hasLight())
        return runAction(ACTION_ALL_OFF);
      else
        return runAction(ACTION_BRIGHT_LIGHT);
      break;
      
    case ACTION_LOW_LIGHT: // SINGLE CLICK
      actionStr = "ACTION_LOW_LIGHT";
      
      if(hasLight())
        return runAction(ACTION_ALL_OFF);
      else
        return runAction(ACTION_RIGHT);
      break;
      
    case ACTION_DIODES: // DOUBLE CLICK
      actionStr = "ACTION_DIODES";
      
      setStateForAllLamps(OFF);
      return runAction(ACTION_DOWN);
      break;
      
    case ACTION_BRIGHT_LIGHT: // LONG CLICK
      actionStr = "ACTION_BRIGHT_LIGHT";

      setStateForAllLamps(OFF);
      for(int i = 0; i < (sizeof(BRIGHT_LIGHT_LAMPS)/sizeof(int)); i++) {
        lamps[BRIGHT_LIGHT_LAMPS[i]] = ON;
      }
      break;

    case ACTION_ALL_OFF:
      actionStr = "ACTION_ALL_OFF";
      
      setStateForAllLamps(OFF);
      break;
      
    default: return "ERROR: NO ACTION FOUND";
  }

  syncLampState();

  // LOGGING
  
//  Serial.print(" Light: "); Serial.println(hasLight());
  for(int i = 0; i < LAMPS_COUNT; i++)
    Serial.print(String(lamps[i] ^ 1));
  Serial.print("*****" + actionStr);
//    Serial.print(String(relays[i]) + "=>" + String(lamps[i] ^ 1)+", ");
  Serial.println();
  
  return actionStr;
}
