//----- SOCKET WIRES
// BLUE - PHASE
// YELLOW - PHASE TO LAMP
// BROWN - 0
// CONNECT YELLOW AND BROWN
//------ LAMP WIRES
// BLACK_LENT - PHASE
// YELLOW_LENT - PHASE TO SOCKET
// WHITE - 0
// CONNECT BLACK AND YELLOW

#include <jled.h>
#include <ESP8266WiFi.h>
#include <AceButton.h>
using namespace ace_button;

// LED RELATED VARIABLES
const int LED_ON = HIGH;
const int LED_OFF = LOW;
const int LED_PIN = 2; // D4
const int LED_DELAY = 100; //100ms
//JLed led = JLed(LED_PIN);

// BUTTON RELATED VARIABLES
const int BUTTON_PIN = 14; // D5
AceButton button(BUTTON_PIN);
AdjustableButtonConfig adjustableButtonConfig;

// WIFI RELATED VARIABLES
const char SSID[] = "BedroomLamp";
const char PASS[] = "krishnakrishna";
const char REQ_END = '\r';
const String COMMAND_CLICK = "1";
const String COMMAND_DB_CLICK = "2";
const String COMMAND_LONG_CLICK = "3";
IPAddress server(192, 168, 4, 15);  // IP address of the AP
WiFiClient client;

const int ledPin = 12;
JLed led = JLed(ledPin);

//JLed ledInternal = JLed(LED_BUILTIN).Blink(1000, 1000).LowActive().Forever();

void setup() {
//  led2.Blink(100, 30).Forever().Update();
  
  Serial.begin(9600);
  while (! Serial);

  setupButton();
  connectWiFi();
}

void loop() {
  button.check();
  
  led.Update();
//  ledInternal.Update();
}

void setupButton() {
  pinMode(BUTTON_PIN, INPUT);
  button.init(BUTTON_PIN, LOW);
  button.setButtonConfig(&adjustableButtonConfig);

  adjustableButtonConfig.setEventHandler(handleClickEvent);
  adjustableButtonConfig.setFeature(ButtonConfig::kFeatureClick);
  adjustableButtonConfig.setFeature(ButtonConfig::kFeatureLongPress);
  adjustableButtonConfig.setFeature(ButtonConfig::kFeatureDoubleClick);

  Serial.println("Button: ready");
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(650); led.On().Update(); delay(50); led.Off().Update();
  }

  Serial.println("WIFI IP: " + WiFi.localIP());
  Serial.println("WiFi Gateway: " + WiFi.gatewayIP());
}

String sendWiFiCommand(String command) {
  client.connect(server, 80);
  client.print(command + REQ_END);
  String answer = client.readStringUntil(REQ_END);
  client.flush();
  client.stop();
  
//  Serial.println("********************************");
//  Serial.println("REQUEST: " + command);
  Serial.println("WIFI_RESPONSE: " + answer);

  return answer;
}

// The event handler for the button.
void handleClickEvent(AceButton*, uint8_t eventType, uint8_t buttonState) {
  // CLICK HANDLER
  switch (eventType) {
    case AceButton::kEventClicked:
      sendWiFiCommand(COMMAND_CLICK);
    
      led.Blink(100, 100).Repeat(1);
      break;
      
    case AceButton::kEventDoubleClicked:
      sendWiFiCommand(COMMAND_DB_CLICK);
      
      led.Blink(50, 50).Repeat(4);
      break;
      
    case AceButton::kEventLongPressed:
      sendWiFiCommand(COMMAND_LONG_CLICK);
      
      led.Breathe(500).Repeat(2);
      break;
  }
}

