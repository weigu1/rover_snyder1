/*
 *  Space Mining Rover aka snider1
 *  (ESP webserver with AP and L293D)
 *  new websockets version for both ESPs 9/19 
 *  weigu.lu creative-lab.lu
 * 
 *  ESP8266: LOLIN/WEMOS D1 mini pro
 *  ESP32:   MH ET LIVE ESP32-MINI-KIT 
 *  
 *  MHET    | MHET    - LOLIN        |---| LOLIN      - MHET    | MHET
 *  
 *  GND     | RST     - RST          |---| TxD        - RxD(3)  | GND
 *   NC     | SVP(36) -  A0          |---| RxD        - TxD(1)  | 27 
 *  SVN(39) | 26      -  D0(16)      |---|  D1(5,SCL) -  22     | 25
 *   35     | 18      -  D5(14,SCK)  |---|  D2(4,SDA) -  21     | 32
 *   33     | 19      -  D6(12,MISO) |---|  D3(0)     -  17     | TDI(12)
 *   34     | 23      -  D7(13,MOSI) |---|  D4(2,LED) -  16     | 4
 *  TMS(14) | 5       -  D8(15,SS)   |---| GND        - GND     | 0   
 *   NC     | 3V3     - 3V3          |---|  5V        -  5V     | 2
 *  SD2(9)  | TCK(13)                |---|              TD0(15) | SD1(8)
 *  CMD(11) | SD3(10)                |---|              SD0(7)  | CLK(6)
 *  
 *  ESP32 framework is missing analogWrite. Option: LEDC which is PWM
 *  
 *  Install the websockets library from Markus Sattler 
 *  (https://github.com/Links2004/arduinoWebSockets)
 *  Tools > Manage libtraries... > search for websockets
 *  
 *  Options: Relay             GPIO 33 as output 
 *           Measuring voltage GPIO 33 ADC1-7
 *           Measuring current GPIO 35 ADC1-6
 */

#ifdef ESP8266
  #include <ESP8266WiFi.h>           // ESP8266  
  #include <ESP8266WebServer.h>  
#else
  #include <WiFi.h>                  // ESP32
  //#include <WiFiClient.h>
  #include <WebServer.h>
#endif // ifdef ESP8266

#ifndef ESP8266
  //#define RELAY            // uncomment if you use a relay (only ESP32)
  //#define MEASURE          // uncomment if you use measure current (only ESP32)
#endif // ifndef ESP8266

#include <WebSocketsServer.h>
#include "html_css_js.h"

const char *WIFI_SSID = "btsiot01";     // AP settings
const char *WIFI_PASSWORD = "btsiot01"; // password must have min. 8 char.!!
IPAddress IP_AP_LOCAL(192, 168, 168, 168);
IPAddress IP_AP_GW(192, 168, 168, 1);
IPAddress MASK_AP(255, 255, 255, 0);

#ifdef ESP8266
  ESP8266WebServer http_server(80);    // create a web server on port 80
  WebSocketsServer ws_server(81);      // create a ws server on port 81
#else  
  WebServer http_server(80);    // create a web server on port 80
  WebSocketsServer ws_server(81);      // create a ws server on port 81
#endif // ifdef ESP8266
  
const byte SPEED_IDLE = 30;          // *2/1023 e.g. 30 gives 60 of 1023 = 6% 
const byte ANGLE_IDLE = 30;  
long switch_off_after = 1000000;      // switch rover off after milliseconds (3*60*1000) 
long last_action_time, previous_millis;  

#ifdef ESP8266
  const byte MOTOR_R1_PIN = 16;         // D0 config Wemos PINs to motor driver
  const byte MOTOR_R2_PIN = 0;          // D3
  const byte MOTOR_L1_PIN = 15;         // D8 
  const byte MOTOR_L2_PIN = 13;         // D7
#else
const byte MOTOR_R1_PIN = 26; // D0 config pinss to motor driver
const byte MOTOR_R2_PIN = 17; // D3
const byte MOTOR_L1_PIN = 5;  // D8 
const byte MOTOR_L2_PIN = 23; // D7
const byte MOTOR_R1_PWM = 1;  // pwm channel
const byte MOTOR_R2_PWM = 2; 
const byte MOTOR_L1_PWM = 3;  
const byte MOTOR_L2_PWM = 4; 
#endif // ifdef ESP8266

#ifdef RELAY
  const byte RELAY_PIN = 33; // GPIO
#endif // ifdef RELAY
#ifdef MEASURE
  const byte VOLT_PIN = 35; // Analog input to measure voltage
  const byte CURR_PIN = 34;  // Analog input to measure current
  int adc_voltage, adc_current;
  double voltage, current;
#endif // ifdef MEASURE

short speed, angle;

/******** SETUP ***************************************************************/ 

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Serial ok");
  delay(500);
  #ifdef RELAY
    pinMode(RELAY_PIN,OUTPUT);    
    delay(100);
    digitalWrite(RELAY_PIN,HIGH);        // hold the relay 
    Serial.println("Relay on");
    delay(500);
  #endif // ifdef RELAY   
  init_LED();
  LED_on();
  init_motors();                     // init motor pins and 
  action(0,0,0,0);                   // stop the motors  
  init_wifi_ap();                    // init wifi access point
  init_ws_server();                  // init websocket_server server
  init_http_server();                // init a http server 
  #ifdef MEASURE
    previous_millis = millis();
  #endif // #ifdef MEASURE  
}
/***** MAINLOOP ***************************************************************/

void loop() {
  #ifdef RELAY
    if (millis()-last_action_time > switch_off_after) {
      digitalWrite(RELAY_PIN,LOW);     // switch off
    }
  #endif // ifdef RELAY
  #ifdef MEASURE    
    adc_voltage = analogRead(VOLT_PIN);
    adc_current = analogRead(CURR_PIN);    
    voltage = adc_voltage*7.0/4096;
    current = adc_current*5000/(4096*19); // g = 19 I = (adc_current/19)/R = *0,2Ohm
    if ((millis()-previous_millis) > 2000) {
      Serial.print("voltage = ");
      Serial.print(voltage);
      Serial.print(" V\t");
      Serial.print("current = ");
      Serial.print(current);
      Serial.println(" mA");
      previous_millis = millis();
    }
  #endif // ifdef MEASURE
  ws_server.loop();                  // check for websocket_server events
  http_server.handleClient();        // wait for http requests
}
/***** Init functions *********************************************************/

// build in LED on
void LED_on() {
  digitalWrite(LED_BUILTIN,LOW);     // LED on (negative logic)
}

// build in LED off
void LED_off() {
  digitalWrite(LED_BUILTIN,HIGH);    // LED off (negative logic)
}

// initialise the build in LED and switch it on
void init_LED() {
  pinMode(LED_BUILTIN,OUTPUT);
  LED_on();
}

// initialise the motor pins as output
void init_motors() {  
  #ifdef ESP8266 
    analogWriteRange(1023);             // needed for ESP8266 core beginning with 3.0
    pinMode(MOTOR_R1_PIN, OUTPUT);      // 4 pins to motor driver as output
    pinMode(MOTOR_R2_PIN, OUTPUT); 
    pinMode(MOTOR_L1_PIN, OUTPUT); 
    pinMode(MOTOR_L2_PIN, OUTPUT); 
  #else
    ledcAttachPin(MOTOR_R1_PIN, MOTOR_R1_PWM); // assign output pins to PWM channels
    ledcAttachPin(MOTOR_R2_PIN, MOTOR_R2_PWM);
    ledcAttachPin(MOTOR_L1_PIN, MOTOR_L1_PWM);
    ledcAttachPin(MOTOR_L2_PIN, MOTOR_L2_PWM);  
    ledcSetup(MOTOR_R1_PWM, 1000, 10); // 1 kHz PWM, 10-bit resolution 
    ledcSetup(MOTOR_R2_PWM, 1000, 10); 
    ledcSetup(MOTOR_L1_PWM, 1000, 10); 
    ledcSetup(MOTOR_L2_PWM, 1000, 10); 
  #endif // ifdef ESP8266        
}

// initialize wifi ap
void init_wifi_ap() {   
  WiFi.softAPConfig(IP_AP_LOCAL, IP_AP_GW, MASK_AP);
  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);       // start the access point
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("Access Point ");
  Serial.println(WIFI_SSID);
  Serial.print("AP IP address: ");
  Serial.println(myIP);
}

// init websocket_server server
void init_ws_server() {                       
  ws_server.begin();                 // start the ws server
  ws_server.onEvent(ws_server_event);// if ws message, websocket_server_event()
}                                      

// init a http server and handle http requests: http://192.168.168.168/
void init_http_server() {   
  http_server.onNotFound(handle_not_found);  
  http_server.on("/", handle_root);  // (don't forget the "/")
  http_server.begin();               // start the HTTP server
  http_server.setContentLength(html_css_js.length()); // needed to avoid net:err
}
/***** SERVER handlers ********************************************************/

// if the requested page doesn't exist, return a 404 not found error
void handle_not_found() {   
  http_server.send(404, "text/plain", "404: Not Found");
}

// handle root (deliver webpage)
void handle_root() {   
  LED_on();
  http_server.send(200,"text/html",html_css_js); // HTTP response code 200
  LED_off();
}

// do this when a websocket_server message is received
void ws_server_event(byte num, WStype_t type, byte * payload, size_t lenght) {   
  switch (type) {
    case WStype_DISCONNECTED:        // if the websocket_server is disconnected         
      break;
    case WStype_CONNECTED: {         // if new websocket_server con. established
        //IPAddress ip = ws_server.remoteIP(num);
        ws_server.remoteIP(num);
      }
      break;
    case WStype_TEXT:                // if new text data is received            
      if (payload[0] == '#') {       // we get slider data
        unsigned long pl = (unsigned long) strtol((const char *) &payload[1],
                           NULL, 16);// string data to number 
        int speed = pl & 0x3FF;      // retrieve both slider data (0-1023)
        int angle= (pl >> 12) & 0x3FF; // G: bits 12-24
        Serial.print(speed);
        Serial.print('\t');
        Serial.println(angle);
        if ((angle <= (512 + ANGLE_IDLE)) &&
            (angle >= (512 - ANGLE_IDLE))) {     // straight
          if (speed > (512 + SPEED_IDLE)) {
            action(speed,0,speed,0);             // straight forward
          }
          else if (speed < (512 - SPEED_IDLE)) {          
            action(speed,1023,speed,1023);       // straight backwards
          }
          else {
            action(0,0,0,0);                     // stop
          }
        }  
        else if (angle < (512 - ANGLE_IDLE)) {   // turn right 
          angle = -((512 + ANGLE_IDLE)-angle); 
          if (speed > (512 + SPEED_IDLE)) {      // right forward
            action(speed,0,speed+angle,0);
          }
          else if (speed < (512 - SPEED_IDLE)) {          
            action(speed,1023,speed-angle,1023); // right backwards
          }
          else {
            action(0,0,0,0);
          }
        }  
        else {                                   // turn left 
          angle = (512 - ANGLE_IDLE)-angle; 
          if (speed > (512 + SPEED_IDLE)) {      // left forward
            action(speed+angle,0,speed,0);
          }
          else if (speed < (512 - SPEED_IDLE)) { // left backwards
            action(speed-angle,1023,speed,1023);
          }
          else {
            action(0,0,0,0);
          }
        }
      } 
      break;
  }
}
/***** HELPER functions********************************************************/

void action(int MOTOR_R1, int MOTOR_R2, int MOTOR_L1, int MOTOR_L2) {  
  LED_on();
  #ifdef ESP8266        
    analogWrite(MOTOR_R1_PIN, MOTOR_R1);
    analogWrite(MOTOR_R2_PIN, MOTOR_R2);
    analogWrite(MOTOR_L1_PIN, MOTOR_L1);
    analogWrite(MOTOR_L2_PIN, MOTOR_L2);  
  #else
    ledcWrite(MOTOR_R1_PWM, MOTOR_R1);
    ledcWrite(MOTOR_R2_PWM, MOTOR_R2);
    ledcWrite(MOTOR_L1_PWM, MOTOR_L1);
    ledcWrite(MOTOR_L2_PWM, MOTOR_L2);
  #endif // ifdef ESP8266        
  LED_off();
  last_action_time = millis();
}
