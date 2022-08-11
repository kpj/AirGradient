/*
  AirGradient DIY Air Quality Sensor with an ESP8266 Microcontroller.
*/

#include <AirGradient.h>

#include <WiFiManager.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

#include <Wire.h>
#include "SSD1306Wire.h"

////
// configuration
////

// web server
const int port = 9926;

// set unused sensors to false
boolean hasPM = true;
boolean hasCO2 = true;
boolean hasSHT = true;

// enable WIFi connection
boolean connectWIFI = true;

// frequency of screen updates
const int updateFrequency = 5000;

////
// global objects
////

AirGradient ag = AirGradient();

SSD1306Wire display(0x3c, SDA, SCL);

ESP8266WebServer server(port);

String deviceId = String(ESP.getChipId(), HEX);

// misc
long lastUpdate;
int counter = 0;

////
// Core functions
////

void setup() {
  // init serial connection
  Serial.begin(9600);

  // init screen
  display.init();
  display.flipScreenVertically();
  showTextRectangle("Booting", deviceId, true);

  // init sensors
  if (hasPM) ag.PMS_Init();
  if (hasCO2) ag.CO2_Init();
  if (hasSHT) ag.TMP_RH_Init(0x44);

  // init wifi
  if (connectWIFI) connectToWifi();

  // server setup
  server.on("/", handleRequest);
  server.on("/metrics", handleRequest);
  server.onNotFound(handleNotFound);
  server.begin();

  Serial.println("HTTP server started at ip " + WiFi.localIP().toString() + ":" + String(port));
  showTextRectangle("Listening on", WiFi.localIP().toString() + ":" + String(port), true);
}

void loop() {
  long t = millis();

  server.handleClient();
  updateScreen(t);
}

////
// Helper functions
////

void showTextRectangle(String ln1, String ln2, boolean small) {
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);

  if (small) {
    display.setFont(ArialMT_Plain_16);
  } else {
    display.setFont(ArialMT_Plain_24);
  }

  display.drawString(32, 16, ln1);
  display.drawString(32, 36, ln2);
  display.display();
}

void connectToWifi() {
  Serial.println("Connecting to WiFi");

  WiFiManager wifiManager;
  //WiFi.disconnect(); //to delete previous saved hotspot
  String HOTSPOT = "AIRGRADIENT-" + String(ESP.getChipId(), HEX);
  wifiManager.setTimeout(120);

  showTextRectangle("Connect", "to Wifi", true);

  if (!wifiManager.autoConnect((const char * ) HOTSPOT.c_str())) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    ESP.restart();
    delay(5000);
  }

}

String generateMetrics() {
  String message = "";
  String idString = "{id=\"" + deviceId + "\",mac=\"" + WiFi.macAddress().c_str() + "\"}";

  if (hasPM) {
    int stat = ag.getPM2_Raw();

    message += "# HELP pm02 Particulate Matter PM2.5 value\n";
    message += "# TYPE pm02 gauge\n";
    message += "pm02";
    message += idString;
    message += String(stat);
    message += "\n";
  }

  if (hasCO2) {
    int stat = ag.getCO2_Raw();

    message += "# HELP rco2 CO2 value, in ppm\n";
    message += "# TYPE rco2 gauge\n";
    message += "rco2";
    message += idString;
    message += String(stat);
    message += "\n";
  }

  if (hasSHT) {
    TMP_RH stat = ag.periodicFetchData();

    message += "# HELP atmp Temperature, in degrees Celsius\n";
    message += "# TYPE atmp gauge\n";
    message += "atmp";
    message += idString;
    message += String(stat.t);
    message += "\n";

    message += "# HELP rhum Relative humidity, in percent\n";
    message += "# TYPE rhum gauge\n";
    message += "rhum";
    message += idString;
    message += String(stat.rh);
    message += "\n";
  }

  return message;
}

void handleRequest() {
  server.send(200, "text/plain", generateMetrics());
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/html", message);
}

void updateScreen(long now) {
  if ((now - lastUpdate) > updateFrequency) {
    switch (counter) {
      case 0:
        if (hasPM) {
          int stat = ag.getPM2_Raw();
          showTextRectangle("PM2", String(stat), false);
        }
        break;
      case 1:
        if (hasCO2) {
          int stat = ag.getCO2_Raw();
          showTextRectangle("CO2", String(stat), false);
        }
        break;
      case 2:
        if (hasSHT) {
          TMP_RH stat = ag.periodicFetchData();
          showTextRectangle("TMP", String(stat.t, 1) + "C", false);
        }
        break;
      case 3:
        if (hasSHT) {
          TMP_RH stat = ag.periodicFetchData();
          showTextRectangle("HUM", String(stat.rh) + "%", false);
        }
        break;
    }

    counter++;
    if (counter > 3) counter = 0;

    lastUpdate = millis();
  }
}
