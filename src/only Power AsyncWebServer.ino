#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <DNSServer.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)

#include <ESPAsyncTCP.h>
#endif
#include "ESPAsyncWebServer.h"

extern "C" {
  #include <osapi.h>
  #include <os_type.h>
}

#include "config.h"

const char* ssid      = "";
const char* password  = "";
const char* server_my = "";

#define ledPin D2

IPAddress ip;
int mysql_channel = 18; // Subwoofers
int OnOff = 0;

AsyncWebServer server(80);
const char* PARAM_MESSAGE = "message";

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

void setup() {
  Serial.begin(9600);
  delay(10);
  pinMode(ledPin, OUTPUT);
   
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    return;
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  ip = WiFi.localIP();
//------------ server ----------------------------
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      AsyncResponseStream *response = request->beginResponseStream("text/html");
      response->print("<!DOCTYPE html><html><head><title>Subwoofers</title>");
      response->print("<link rel='stylesheet' href='https://stackpath.bootstrapcdn.com/bootstrap/4.5.2/css/bootstrap.min.css' integrity='sha384-JcKb8q3iqJ61gNV9KGb8thSsNjpSL0n8PARn9HuZOnIxN0hoP+VmmDGMN5t9UJ0Z' crossorigin='anonymous'>");
     
      response->print("</head><body id='Subwoofers'>");
      response->print("<script src='https://code.jquery.com/jquery-3.5.1.slim.min.js' integrity='sha384-DfXdz2htPH0lsSSs5nCTpuj/zy4C+OGpamoFVy38MVBnE+IbbVYUew+OrCXaRkfj' crossorigin='anonymous'></script>");
      response->print("<script src='https://cdn.jsdelivr.net/npm/popper.js@1.16.1/dist/umd/popper.min.js' integrity='sha384-9/reFTGAW83EW2RDu2S0VKaIzap3H66lZH81PoYlFhbGU+6BZp6G7niu735Sk7lN' crossorigin='anonymous'></script>");
      response->print("<script src='https://stackpath.bootstrapcdn.com/bootstrap/4.5.2/js/bootstrap.min.js' integrity='sha384-B4gt1jrGC7Jh4AgTPSdUtOBvfO8shuf57BaghqFfPlYxofvL8/KUEfYiJOMMV+rV' crossorigin='anonymous'></script>");
      response->print("<H1>Subwoofers</H1>");
      response->print("<div class='container-fluid'>");
      response->printf("<p>You were trying to reach: http://%s%s</p>", request->host().c_str(), request->url().c_str());
      response->print("<p>Power lamp: ");
      response->print(getState().c_str());
      response->print("</p>");
      response->print("<hr>");
      response->print("<p><a class='btn btn-outline-secondary' href='state'>state</a></p>");
      response->print("<p><a class='btn btn-outline-secondary' href='set?power=on'>On</a></p>");
      response->print("<p><a class='btn btn-outline-secondary' href='set?power=off'>Off</a></p>");
      response->print("<p><a class='btn btn-outline-secondary' href='set?power=toggle'>Toggle</a></p>");
      response->print("</div>");
      response->print("</body></html>");
      request->send(response);
    });
    
    server.on("/state", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", getState().c_str());
    });

    // Send a GET request to <IP>/set?message=<message>
    server.on("/set", HTTP_GET, [] (AsyncWebServerRequest *request) {
        String message;
        if (request->hasParam("power")) {
            message = request->getParam("power")->value();
        } else {
            message = "No set sent";
        }
        
        if(message=="on"){
          request->send_P(200, "text/plain", setStart().c_str());
        }
        if(message=="off"){
          request->send_P(200, "text/plain", setStop().c_str());
        }
        if(message=="toggle"){
          request->send_P(200, "text/plain", setToggle().c_str());
        }
    });

    server.onNotFound(notFound);
    server.begin();
//------------ server ----------------------------
}

void loop()
{ 
  long currentMillis = millis();
  if (OnOff == 1) {
    Serial.println("START LOOP");
  } else {
    Serial.println("STOP LOOP");
  }
  delay(1000);
}//loop

int calcTimeStop(long currentMillis, int periodMinute){
    const unsigned long oneSecond = 1000;  //the value is a number of milliseconds, ie 1 second
    long rz = currentMillis+(oneSecond*(periodMinute*60));
    return rz;
}


String setStart() {
 Serial.println("START setStart");
 OnOff = 1;
 digitalWrite(ledPin, HIGH);
 return String(OnOff);
}

String setStop() {
  Serial.println("Stop setStop");
  OnOff = 0;
  digitalWrite(ledPin, LOW);
  return String(OnOff);
}

String getState() {
  return String(digitalRead(ledPin));
}

String setToggle() {
  if (digitalRead(ledPin)) {
    digitalWrite(ledPin, LOW);
    OnOff = 0;
  } else {
    digitalWrite(ledPin, HIGH);
    OnOff = 1;
  }
  return String(OnOff);
}


