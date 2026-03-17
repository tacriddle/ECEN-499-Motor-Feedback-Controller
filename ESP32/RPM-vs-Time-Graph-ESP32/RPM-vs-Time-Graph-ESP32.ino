/**
 * Drill Press Monitoring System - ESP32 Production Firmware
 * --------------------------------------------------------
 * Mode: Access Point (AP)
 * Hardware: ESP32 + 1.9" TFT LCD (TFT_eSPI)
 * Communication: Serial2 (UART) to STM32
 */
#include <Arduino.h>


//start///////////////////////////////////////////////////////////////////////////////////
//TODO - arcade
#include "flappy.h"
#include <Preferences.h>
Preferences prefs;
int highScore = 0;

#include "pacman.h"
int pacHighScore = 0; 
//end/////////////////////////////////////////////////////////////////////////////////////


// 1. WiFi.h (Built-in)
// By: Espressif Systems
// Purpose: Core library for ESP32 WiFi functionality (Station and Access Point modes).
// Installation: Included automatically with the ESP32 Board Package.
#include <WiFi.h>

// 2. AsyncTCP by Mathieu Carbou
// GitHub: https://github.com/mathieucarbou/AsyncTCP
#include <AsyncTCP.h>

// 3. ESPAsyncWebServer by Mathieu Carbou
// GitHub: https://github.com/mathieucarbou/ESPAsyncWebServer
#include <ESPAsyncWebServer.h>

// 4. TFT_eSPI by Bodmer
// Install using Arduino IDE Library Manager
#include <TFT_eSPI.h>

// 5. Local Project Files
// chartjs.h: Stores the Chart.js library in memory (Offline Mode).
// indexhtml.h: Stores the Dashboard UI/Layout.
#include "chartjs.h"    
#include "indexhtml.h"  


// --- Global Objects ---
TFT_eSPI tft = TFT_eSPI();
AsyncWebServer server(80);
AsyncEventSource events("/events"); // For streaming live data to the browser


// --- System Variables ---
int rpmHistory[320];    // History buffer for LCD graph (matches screen width)
int targetRPM = 0;      // Current setpoint sent to STM32
String displayAddress = "";


// --- Function Prototypes ---
void updateLCDGraph(int val);
void handleSystemButton();



void setup() {
  // 1. Hardware Pin Initialization
  pinMode(32, OUTPUT); 
  digitalWrite(32, HIGH);    // Power on LCD Backlight
  pinMode(0, INPUT_PULLUP);  // BOOT button for system reset
  
  // 2. Communication Setup
  Serial.begin(115200);      // USB Debugging
  Serial2.begin(115200, SERIAL_8N1, 21, 22); // RX=21, TX=22 (To STM32)

  // 3. TFT Display Initialization
  tft.init();
  tft.setRotation(1); 
  tft.fillScreen(TFT_RED);
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("WAKING UP...", 120, 60, 4);
  delay(1000);

  // 4. Access Point (AP) Configuration
  const char* ssid = "DrillPress_Monitor";
  const char* password = NULL; // No password for shop ease-of-use

  // Custom static IP for predictable dashboard access
  IPAddress local_IP(10, 0, 0, 1);
  IPAddress gateway(10, 0, 0, 1);
  IPAddress subnet(255, 255, 255, 0);
  
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ssid, password);
  
  displayAddress = String(ssid) + " | " + local_IP.toString();

  // 5. Web Server Routes
  // Serve Dashboard HTML
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  // Handle incoming Target RPM changes from Dashboard
  server.on("/setRPM", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("val")) {
      String rpmValue = request->getParam("val")->value();
      int tempTarget = rpmValue.toInt();

      // Final safety constraint
      targetRPM = constrain(tempTarget, 0, 2000);
      
      // Relay constrained target to STM32
      Serial2.print(targetRPM); 
      Serial2.print("\n");
    }
    request->send(200, "text/plain", "OK");
  });

  // Serve Chart.js library locally from ESP32 memory (Offline Mode)
  server.on("/chart.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse_P(200, "application/javascript", chart_min_js);
    response->addHeader("Cache-Control", "max-age=3600"); // Cache to speed up repeat loads
    request->send(response);
  });


  //start///////////////////////////////////////////////////////////////////////////////////
  //TODO - arcade

  // flappy brick
  server.on("/game", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", flappy_html);
  });

  // 1. Get the current high score
  server.on("/getHighScore", HTTP_GET, [](AsyncWebServerRequest *request){
      prefs.begin("drill-game", true); // open in read-only
      highScore = prefs.getInt("high", 0);
      prefs.end();
      request->send(200, "text/plain", String(highScore));
  });

  // 2. Set a new high score
  server.on("/setHighScore", HTTP_GET, [](AsyncWebServerRequest *request){
      if (request->hasParam("score")) {
          int newScore = request->getParam("score")->value().toInt();
          prefs.begin("drill-game", false); // open in write mode
          int currentHigh = prefs.getInt("high", 0);
          if (newScore > currentHigh) {
              prefs.putInt("high", newScore);
              highScore = newScore;
          }
          prefs.end();
      }
      request->send(200, "text/plain", "OK");
  });


  // pacman
  server.on("/pacman", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", pacman_html);
  });

  // 1. Get Pac-Man High Score (Persistent)
  server.on("/getPacScore", HTTP_GET, [](AsyncWebServerRequest *request){
      prefs.begin("drill-game", true); // Open in read-only
      int currentPacHigh = prefs.getInt("pachigh", 0);
      prefs.end();
      request->send(200, "text/plain", String(currentPacHigh));
  });

  // 2. Set Pac-Man High Score (Persistent)
  server.on("/setPacScore", HTTP_GET, [](AsyncWebServerRequest *request){
      if (request->hasParam("score")) {
          int newScore = request->getParam("score")->value().toInt();
          
          prefs.begin("drill-game", false); // Open in read-write
          prefs.putInt("pachigh", newScore);
          prefs.end();
          
          Serial.print("New Pac-Man High Score Saved: ");
          Serial.println(newScore);
      }
      request->send(200, "text/plain", "OK");
  });
  //end/////////////////////////////////////////////////////////////////////////////////////


  // Start Server and Events
  server.addHandler(&events);
  DefaultHeaders::Instance().addHeader("Connection", "keep-alive");
  server.begin();

  // 6. UI Final Prep
  tft.fillScreen(TFT_WHITE);
  tft.drawFastHLine(0, 146, 320, TFT_MAROON); // Bottom border
  tft.setTextColor(TFT_MAROON);
  tft.drawCentreString(displayAddress, 160, 150, 2);

  // 7. Initialize STM32 State
  // Force Target RPM to 0 on startup/reset for safety
  targetRPM = 0;
  delay(500); // Give STM32 a moment to stabilize UART after power-on
  Serial2.print("0\n");
}



void loop() {
  // Check if user is holding the BOOT button to restart system
  if (digitalRead(0) == LOW) {
    handleSystemButton();
  }

  // Handle incoming Telemetry from STM32
  if (Serial2.available()) {
    // Read CSV formatted data "RPM,PWM"
    String data = Serial2.readStringUntil('\n');
    int commaIndex = data.indexOf(',');

    if (commaIndex != -1) { 
      // 1. Broadcast to all connected web browsers
      events.send(data.c_str(), "message", millis());

      // 2. Extract RPM and update physical LCD display
      int rpm = data.substring(0, commaIndex).toInt();
      updateLCDGraph(rpm);
    }
  }
}



/**
 * Renders the real-time graph and target line on the 1.9" LCD.
 */
void updateLCDGraph(int val) {
  const int pointsToKeep = 100; // Number of horizontal data points
  const int pixelWidth = 3;     // Width scale factor
  const int graphBottom = 135;  // Baseline Y coordinate
  const int graphHeight = 130;  // Maximum pixel height of data

  // 1. Shift data history left by one
  for (int i = 0; i < pointsToKeep - 1; i++) {
    rpmHistory[i] = rpmHistory[i + 1];
  }
  
  // 2. Map current RPM to pixel height (0-2000 range)
  int yNew = graphBottom - (int)(val * graphHeight / 2000.0);
  rpmHistory[pointsToKeep - 1] = constrain(yNew, 5, graphBottom);
  
  // 3. Map Target RPM to pixel height (0-2000 range)
  int yTarget = graphBottom - (int)(targetRPM * graphHeight / 2000.0);
  yTarget = constrain(yTarget, 5, graphBottom);

  // 4. Drawing Pass
  tft.startWrite();
  tft.fillRect(0, 0, 300, graphBottom + 1, TFT_WHITE); // Clear previous frame

  // Draw 0-RPM Baseline (Black - 3 pixels thick)
  tft.drawFastHLine(0, graphBottom - 1, 320, TFT_BLACK);
  tft.drawFastHLine(0, graphBottom, 320, TFT_BLACK); 
  tft.drawFastHLine(0, graphBottom + 1, 320, TFT_BLACK);

  // Draw Target Setpoint Line (Red - 3 pixels thick)
  tft.drawFastHLine(0, yTarget - 1, 300, TFT_BLUE);
  tft.drawFastHLine(0, yTarget, 300, TFT_BLUE);
  tft.drawFastHLine(0, yTarget + 1, 300, TFT_BLUE);

  // Draw Actual RPM Trace (Blue - 3 pixels thick)
  for (int i = 1; i < pointsToKeep; i++) {
    int x0 = (i - 1) * pixelWidth;
    int y0 = rpmHistory[i - 1];
    int x1 = i * pixelWidth;
    int y1 = rpmHistory[i];

    tft.drawLine(x0, y0 - 1, x1, y1 - 1, TFT_RED);
    tft.drawLine(x0, y0, x1, y1, TFT_RED);
    tft.drawLine(x0, y0 + 1, x1, y1 + 1, TFT_RED);
  }
  tft.endWrite();
}



/**
 * Provides a 3-second hold-to-restart mechanism using the BOOT button.
 */
void handleSystemButton() {
  unsigned long startPress = millis();
  tft.fillScreen(TFT_BLUE);
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("HOLD 3S TO RESET", 160, 60, 2);

  while (digitalRead(0) == LOW) {
    if (millis() - startPress > 3000) {
      tft.fillScreen(TFT_BLACK);
      tft.drawCentreString("DEVICE RESET", 160, 75, 4);
      tft.drawCentreString("REBOOTING...", 160, 110, 2);
      delay(2000);
      ESP.restart(); 
    }
  }
  tft.fillScreen(TFT_WHITE); // Clear if released early
}