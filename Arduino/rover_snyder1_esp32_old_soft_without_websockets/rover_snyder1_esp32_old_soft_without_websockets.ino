/*
 *  Space Mining Rover aka snider1
 *  (wemos webserver with AP and L293D)
 *  creative-lab.lu
 *  
 * D0 (16)      MHET 26
 * D1 (SCL 5)   MHET 22 SCL
 * D2 (SDA 4)   MHET 21 SDA
 * D3 (0)       MHET 17
 * D4 (LED 2)   MHET 16
 * D5 (SCK 14)  MHET 18 SCK
 * D6 (MISO 12) MHET 19 MISO
 * D7 (MOSI 13) MHET 23 MOSI
 * D8 (SS 15)   MHET 5  SS
 * 
 * ESP32 framework is missing analogWrite. 
 * Option: 16 channels LEDC which is PWM
 */

#include <WiFi.h>
//a#include <FS.h>
//a#include <ESPAsyncWebServer.h>
//a#include <AsyncTCP.h>
#include <WiFiClient.h>
#include <WebServer.h>

const char *ssid = "btsiot01";  // AP settings
const char *password = "btsiot01"; //password must have more than 7 characters!!
IPAddress IP_AP(192, 168, 168, 168);
IPAddress MASK_AP(255, 255, 255, 0);

//aAsyncWebServer server(80);
WebServer server(80);

const int slow = 500;
const int medium = 750;           // medium speed (max. 1023)
const int fast = 1023;
const int bowdiff = 150;

const byte LED = 2;

const byte MotorR1_pin = 26; // D0 config (Wemos) pins to motor driver
const byte MotorR2_pin = 17; // D3
const byte MotorL1_pin = 5;  // D8 
const byte MotorL2_pin = 23; // D7
const byte MotorR1_PWM = 1;  // pwm channel
const byte MotorR2_PWM = 2; 
const byte MotorL1_PWM = 3;  
const byte MotorL2_PWM = 4; 

int MotorR_speed = 0;
int MotorL_speed = 0;

// here comes the html and css magic
String myhtml = R"=====(
<!DOCTYPE html>
<html>
  <head>    
    <title>creative-lab.lu</title>
  </head>
  <style>
    body      {font-family: Arial, Verdana, sans-serif; 
               font-size: 12px;
               color: red;
               background-color: green;}
    div       {margin: 0.2em auto;
               width: 100%;
               clear: both;}
    p         {margin: 0.2em;
               padding: 0.1em;
               border-radius: 1em;
               background-color: yellow;
               border: 0.3em red solid;
               font-size: 4em;
               font-weight: bolder;
               text-align: center;}
    p.center  {margin: 0.2em  auto;
               width: 27%;
               clear: both;}
    p.lam     {clear: both;}
    p.main    {background-color: orange;
               width: 27%;
               float: left;}
    p.side    {width: 27%;
               float: left;}
    p.stop    {background-color: red;
               width: 27%; 
               float: left;
               margin-bottom: 0.4em;}
    a         {display: block;
               text-decoration: none;}
  </style>
  <body>
    <br><p class="lam">creative-lab.lu
           <br/>Lyc&eacute;e des Arts et M&eacute;tiers</p><br>
    <div>
    <p class="side"><a href=/forwards>Forward<br/>Slow</a></p>
    <p class="main"><a href=/forwardm>Forward<br/>Medium</a></p>
    <p class="side"><a href=/forwardf>Forward<br/>Fast</a></p>
    </div><div>
    <p class="main"><a href=/lefts>Left<br/>Spot</a></p>
    <p class="stop"><a href=/halt>Stop<br/>Now</a></p>
    <p class="main"><a href=/rights>Right<br/>Spot</a></p>
    </div><div>
    <p class="side"><a href=/leftb>Left<br/>Bow</a></p>
    <p class="stop"><a href=/halt>Stop<br/>Now</a></p>
    <p class="side"><a href=/rightb>Right<br/>Bow</a></p>
    </div><div>
    <p class="side"><a href=/backwards>Backward<br/>Slow</a></p>
    <p class="main"><a href=/backwardm>Backward<br/>Medium</a></p>
    <p class="side"><a href=/backwardf>Backward<br/>Fast</a></p>
    </div>
  </body>
</html>
)=====";
 
void setup() {
  Serial.begin(115200);
  ledcAttachPin(MotorR1_pin, MotorR1_PWM); // assign output pins to PWM channels
  ledcAttachPin(MotorR2_pin, MotorR2_PWM);
  ledcAttachPin(MotorL1_pin, MotorL1_PWM);
  ledcAttachPin(MotorL2_pin, MotorL2_PWM);
  pinMode(LED, OUTPUT);   // blue LED Pin (D4, 2) as output

  digitalWrite(LED, HIGH); //ON (MHET (no neg. logic)
  ledcSetup(MotorR1_PWM, 1000, 10); // 1 kHz PWM, 10-bit resolution 
  ledcSetup(MotorR2_PWM, 1000, 10); 
  ledcSetup(MotorL1_PWM, 1000, 10); 
  ledcSetup(MotorL2_PWM, 1000, 10); 
  
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(IP_AP, IP_AP, MASK_AP);
  WiFi.softAP(ssid, password);    // if you want remove password parameter
  IPAddress myIP = WiFi.softAPIP();
  Serial.println();
  Serial.print("IP address: ");
  Serial.println(myIP);
 
  // handle http requests; root must be called! (don't forget the "/")
  // type: http://192.168.168.168/
//a  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
//a            request->send(200,"text/html",myhtml);});  
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.on("/forwards", forwards); 
  server.on("/forwardm", forwardm); 
  server.on("/forwardf", forwardf); 
  server.on("/lefts", lefts);
  server.on("/rights", rights);
  server.on("/halt", halt);  
  server.on("/leftb", leftb);
  server.on("/rightb", rightb);
  server.on("/backwards", backwards);
  server.on("/backwardm", backwardm);
  server.on("/backwardf", backwardf);

  server.begin();
  digitalWrite(LED, LOW);
//  server.setContentLength(html.length()); // if not given we get
}                                         // net:err_content_length_mismatch

void loop(void) { 
  server.handleClient();
}

void forwards()  { action(slow,0,slow,0); }
void forwardm()  { action(medium,0,medium,0); }
void forwardf()  { action(fast,0,fast,0); }
void lefts()     { action(slow,0,1023-slow,1023); }
void rights()    { action(1023-slow,1023,slow,0); }
void halt()      { action(0,0,0,0); }
void rightb()    { action(slow,0,slow+bowdiff,0); }
void leftb()     { action(slow + bowdiff,0,slow,0); }
void backwards() { action(1023-slow,1023,1023-slow,1023); }
void backwardm() { action(1023-medium,1023,1023-medium,1023); }
void backwardf() { action(1023-fast,1023,1023-fast,1023); }

void action(int MotorR1, int MotorR2, int MotorL1, int MotorL2) {  
  digitalWrite(LED, HIGH);
  ledcWrite(MotorR1_PWM, MotorR1);
  ledcWrite(MotorR2_PWM, MotorR2);
  ledcWrite(MotorL1_PWM, MotorL1);
  ledcWrite(MotorL2_PWM, MotorL2);
  server.send(200, "text/html",myhtml);
  digitalWrite(LED, LOW);
}  

void handleNotFound() { //from arduino example webserver
  digitalWrite(LED, HIGH);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(LED, LOW);
}

void handleRoot() {  
  digitalWrite(LED, HIGH);
  server.send(200,"text/html",myhtml); // HTTP response code 200 (alt. 404)  
  digitalWrite(LED, LOW);
}
  
