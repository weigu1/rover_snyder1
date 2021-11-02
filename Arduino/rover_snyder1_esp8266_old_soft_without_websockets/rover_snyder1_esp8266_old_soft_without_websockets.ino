/*
 *  Space Mining Rover aka snider1
 *  (wemos webserver with AP and L293D)
 *  creative-lab.lu
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char *ssid = "creative23";     // AP settings
const char *password = "creative23"; // password must have more than 7 characters!!
IPAddress IP_AP(192, 168, 168, 168);
IPAddress MASK_AP(255, 255, 255, 0);

ESP8266WebServer server(80);      // create object with name "server" port 80

const int slow = 500;        // speed (500 to get min 2,5V)
const int medium = 750;      
const int fast = 1023;       // max. 1023) 
const int bowdiff = 150;     // if bigger, lesser radius

const byte LED = 2;          // D4 
const byte MotorR1_pin = 16; // D0 config Wemos pins to motor driver
const byte MotorR2_pin = 0;  // D3
const byte MotorL1_pin = 15; // D8 
const byte MotorL2_pin = 13; // D7



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
  pinMode(MotorR1_pin, OUTPUT); // 4 pins to motor driver as output
  pinMode(MotorR2_pin, OUTPUT); 
  pinMode(MotorL1_pin, OUTPUT); 
  pinMode(MotorL2_pin, OUTPUT); 
  pinMode(D4, OUTPUT);   // blue LED Pin (D4) as output

  digitalWrite(D4, LOW);                  // LED on (negative logic)

  action(0,0,0,0);                         // stop the motors
  /*digitalWrite(Motor_R_PWM_1, LOW);
  digitalWrite(Motor_R_PWM_2, LOW);
  digitalWrite(Motor_L_PWM_1, LOW);
  digitalWrite(Motor_L_PWM_2, LOW);*/

  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(IP_AP, IP_AP, MASK_AP);
  IPAddress myIP = WiFi.softAPIP();

  // handle http requests; root must be called! type: http://192.168.168.168/
  server.on("/", handleRoot);              // (don't forget the "/")
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
  server.setContentLength(myhtml.length()); // if not given we get
}                                         // net:err_content_length_mismatch

void loop() {
  server.handleClient();          // wait for http requests
}

void handleRoot() {  
  digitalWrite(LED, LOW);
  server.send(200,"text/html",myhtml); // HTTP response code 200 (alt. 404)  
  digitalWrite(LED, HIGH);
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
  digitalWrite(LED, LOW);
  analogWrite(MotorR1_pin, MotorR1);
  analogWrite(MotorR2_pin, MotorR2);
  analogWrite(MotorL1_pin, MotorL1);
  analogWrite(MotorL2_pin, MotorL2);
  server.send(200, "text/html",myhtml);
  digitalWrite(LED, HIGH);
}  


