// User installed libraries:
// "WiFiManager" by tablatronix
// "TFT_eSPI" by Bodmer
// "ESPAsyncWebServer" by mathieucarbou (https://github.com/ESP32Async/ESPAsyncWebServer)
// "AsyncTCP" by mathieucarbou (https://github.com/ESP32Async/AsyncTCP)

/////////////////////////////////////////////////////////////////////////////////////////////
#include <Arduino.h>

// 1. Load Async libraries first
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// 2. Rename ALL potential conflicts for WiFiManager
#define HTTP_GET WM_HTTP_GET
#define HTTP_POST WM_HTTP_POST
#define HTTP_DELETE WM_HTTP_DELETE
#define HTTP_PUT WM_HTTP_PUT
#define HTTP_PATCH WM_HTTP_PATCH
#define HTTP_HEAD WM_HTTP_HEAD
#define HTTP_OPTIONS WM_HTTP_OPTIONS
#define HTTP_COPY WM_HTTP_COPY
#define HTTP_LOCK WM_HTTP_LOCK
#define HTTP_MKCOL WM_HTTP_MKCOL
#define HTTP_MOVE WM_HTTP_MOVE
#define HTTP_PROPFIND WM_HTTP_PROPFIND
#define HTTP_PROPPATCH WM_HTTP_PROPPATCH
#define HTTP_UNLOCK WM_HTTP_UNLOCK

// 3. Include WiFi and WiFiManager (which pulls in the conflicting WebServer.h)
#include <WiFi.h>
#include <WiFiManager.h>

// 4. Wipe the renames so they don't break your own code
#undef HTTP_GET
#undef HTTP_POST
#undef HTTP_DELETE
#undef HTTP_PUT
#undef HTTP_PATCH
#undef HTTP_HEAD
#undef HTTP_OPTIONS
#undef HTTP_COPY
#undef HTTP_LOCK
#undef HTTP_MKCOL
#undef HTTP_MOVE
#undef HTTP_PROPFIND
#undef HTTP_PROPPATCH
#undef HTTP_UNLOCK

// 5. Load your other libraries
#include <ESPmDNS.h>
#include <TFT_eSPI.h>

// 6. Final safety: Restore HTTP_GET for your server routes
#ifndef HTTP_GET
#define HTTP_GET (WebRequestMethod)0b0000000000000001
#endif
//////////////////////////////////////////////////////////////////////////////////////////

void updateLCDGraph(int val);

// WIFI MANAGER MODE
// void checkWifiReset();

// ACCESS POINT MODE
void handleSystemButton();
void showConnectionStats();

TFT_eSPI tft = TFT_eSPI();
AsyncWebServer server(80);
AsyncEventSource events("/events");

int rpmHistory[320]; // Stores data for the physical LCD graph matching the 320px width of the 1.9" screen
int graphPtr = 0; // Tracks current horizontal position of LCD graph

String displayAddress = "";

// The HTML for the web dashboard

// const char index_html[] PROGMEM = R"rawliteral(
// <html>
// <head>
//   <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
//   <style>
//     body { font-family: 'Segoe UI', sans-serif; background: #f4f7f6; color: #333; padding: 20px; }
//     #container { max-width: 1100px; margin: auto; background: #ffffff; padding: 25px; border-radius: 15px; box-shadow: 0 4px 20px rgba(0,0,0,0.08); }
//     .header { text-align: center; margin-bottom: 25px; border-bottom: 2px solid #eee; padding-bottom: 15px; }
//     h1 { color: #222; margin: 0; font-weight: 600; font-size: 1.6rem; }
//     .update-rate { font-size: 1rem; color: #888; margin-top: 5px; }
//     .dashboard-grid { display: flex; gap: 20px; flex-wrap: wrap; }
//     .graph-section { flex: 1; min-width: 450px; background: #fcfcfc; padding: 15px; border-radius: 12px; border: 1px solid #eee; }
//     .stat-header { display: flex; justify-content: space-between; align-items: baseline; margin-bottom: 10px; padding: 0 5px; }
//     .stat-label { font-size: 0.8rem; color: #666; text-transform: uppercase; font-weight: bold; }
//     .stat-value { font-size: 1.8rem; font-weight: 800; font-family: 'Courier New', monospace; color: #1a1a1a; }
//     .unit { font-size: 0.9rem; color: #aaa; margin-left: 3px; }
//     .rpm-accent { border-top: 5px solid #0056b3; }
//     .pwm-accent { border-top: 5px solid #d32f2f; }
//     canvas { width: 100% !important; height: 300px !important; }
//   </style>
// </head>
// <body>
//   <div id="container">
//     <div class="header">
//       <h1>Drill Press Monitoring System</h1>
//       <div class="update-rate">Live Telemetry (Sampling Rate: 100ms)</div>
//     </div>
//     <div class="dashboard-grid">
//       <div class="graph-section rpm-accent">
//         <div class="stat-header">
//           <span class="stat-label">Spindle Speed</span>
//           <span class="stat-value" id="val-rpm">0<span class="unit">RPM</span></span>
//         </div>
//         <canvas id="rpmChart"></canvas>
//       </div>
//       <div class="graph-section pwm-accent">
//         <div class="stat-header">
//           <span class="stat-label">Motor Load</span>
//           <span class="stat-value" id="val-pwm">0<span class="unit">%</span></span>
//         </div>
//         <canvas id="pwmChart"></canvas>
//       </div>
//     </div>
//   </div>
// <script>
//     function createChartConfig(color, label, maxVal) {
//       return {
//         type: 'line',
//         data: { 
//           // CHANGE 1: Increase initial labels to 100 for a 10s window
//           labels: Array(100).fill(''), 
//           datasets: [{ 
//             label: label, 
//             data: [], 
//             borderColor: color, 
//             backgroundColor: 'transparent', 
//             borderWidth: 3, 
//             pointRadius: 0, 
//             tension: 0.1 // Reduced tension for a "sharper" look during stalls
//           }] 
//         },
//         options: {
//           responsive: true, maintainAspectRatio: false,
//           scales: {
//             y: { min: 0, max: maxVal, grid: { color: '#e0e0e0' }, ticks: { font: { weight: 'bold' } } },
//             x: { title: { display: true, text: '10 Second History', color: '#999', font: { size: 13 } }, grid: { display: false } }
//           },
//           plugins: { legend: { display: false } },
//           animation: { duration: 0 } // Keeps it real-time without "sliding" lag
//         }
//       };
//     }

//     var rpmChart = new Chart(document.getElementById('rpmChart').getContext('2d'), createChartConfig('#0056b3', 'RPM', 3500));
//     var pwmChart = new Chart(document.getElementById('pwmChart').getContext('2d'), createChartConfig('#d32f2f', 'PWM %', 100));
    
//     var source = new EventSource('/events');
//     source.onmessage = function(e) {
//       var parts = e.data.split(',');
//       var rpm = parts[0];
//       var pwm = parts[1];
      
//       document.getElementById('val-rpm').innerHTML = rpm + '<span class="unit">RPM</span>';
//       document.getElementById('val-pwm').innerHTML = pwm + '<span class="unit">%</span>';

//       // RPM Chart Update
//       rpmChart.data.datasets[0].data.push(rpm);
//       // CHANGE 2: Shift data after 100 points
//       if (rpmChart.data.datasets[0].data.length > 100) rpmChart.data.datasets[0].data.shift();
//       rpmChart.update('none'); // 'none' makes it snap instantly

//       // PWM Chart Update
//       pwmChart.data.datasets[0].data.push(pwm);
//       // CHANGE 2: Shift data after 100 points
//       if (pwmChart.data.datasets[0].data.length > 100) pwmChart.data.datasets[0].data.shift();
//       pwmChart.update('none');
//     };
//   </script>
// </body>
// </html>
// )rawliteral";
// The HTML for the web dashboard

const char index_html[] PROGMEM = R"rawliteral(
<html>
<head>
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <style>
    body { font-family: 'Segoe UI', sans-serif; background: #f4f7f6; color: #333; padding: 20px; }
    #container { max-width: 1100px; margin: auto; background: #ffffff; padding: 25px; border-radius: 15px; box-shadow: 0 4px 20px rgba(0,0,0,0.08); }
    .header { text-align: center; margin-bottom: 25px; border-bottom: 2px solid #eee; padding-bottom: 15px; }
    h1 { color: #222; margin: 0; font-weight: 600; font-size: 1.6rem; }
    .update-rate { font-size: 1rem; color: #888; margin-top: 5px; }
    .dashboard-grid { display: flex; gap: 20px; flex-wrap: wrap; }
    .graph-section { flex: 1; min-width: 450px; background: #fcfcfc; padding: 15px; border-radius: 12px; border: 1px solid #eee; }
    .stat-header { display: flex; justify-content: space-between; align-items: baseline; margin-bottom: 10px; padding: 0 5px; }
    .stat-label { font-size: 0.8rem; color: #666; text-transform: uppercase; font-weight: bold; }
    .stat-value { font-size: 1.8rem; font-weight: 800; font-family: 'Courier New', monospace; color: #1a1a1a; }
    .unit { font-size: 0.9rem; color: #aaa; margin-left: 3px; }
    .rpm-accent { border-top: 5px solid #0056b3; }
    .pwm-accent { border-top: 5px solid #d32f2f; }
    
    /* Control Section Styling */
    .control-section { margin-top: 25px; padding: 20px; background: #ebf5fb; border-radius: 12px; border: 1px solid #aed6f1; display: flex; align-items: center; justify-content: center; gap: 15px; }
    .control-label { font-weight: bold; color: #2c3e50; }
    input[type=number] { padding: 10px; border-radius: 8px; border: 1px solid #bdc3c7; font-size: 1rem; width: 120px; text-align: center; }
    .btn-send { padding: 10px 25px; background: #2980b9; color: white; border: none; border-radius: 8px; cursor: pointer; font-weight: bold; transition: 0.2s; }
    .btn-send:hover { background: #1f6391; }
    .btn-send:active { transform: scale(0.95); }

    canvas { width: 100% !important; height: 300px !important; }
  </style>
</head>
<body>
  <div id="container">
    <div class="header">
      <h1>Drill Press Monitoring System</h1>
      <div class="update-rate">Live Telemetry (Sampling Rate: 100ms)</div>
    </div>
    
    <div class="dashboard-grid">
      <div class="graph-section rpm-accent">
        <div class="stat-header">
          <span class="stat-label">Spindle Speed</span>
          <span class="stat-value" id="val-rpm">0<span class="unit">RPM</span></span>
        </div>
        <canvas id="rpmChart"></canvas>
      </div>
      <div class="graph-section pwm-accent">
        <div class="stat-header">
          <span class="stat-label">Motor Load</span>
          <span class="stat-value" id="val-pwm">0<span class="unit">%</span></span>
        </div>
        <canvas id="pwmChart"></canvas>
      </div>
    </div>

    <!-- NEW NUMERIC INPUT SECTION -->
    <div class="control-section">
      <span class="control-label">Target Spindle Speed:</span>
      <input type="number" id="rpmInput" min="0" max="3500" placeholder="0 - 3500">
      <button class="btn-send" onclick="sendRPM()">SET RPM</button>
    </div>

  </div>

<script>
    function sendRPM() {
      var val = document.getElementById('rpmInput').value;
      if (val === "") return;
      
      // Feedback: Flash button color
      const btn = document.querySelector('.btn-send');
      const originalColor = btn.style.background;
      btn.style.background = "#27ae60";
      setTimeout(() => btn.style.background = originalColor, 500);

      // Send the actual value to the ESP32
      fetch('/setRPM?val=' + val);
    }

    function createChartConfig(color, label, maxVal) {
      return {
        type: 'line',
        data: { 
          labels: Array(100).fill(''), 
          datasets: [{ 
            label: label, 
            data: [], 
            borderColor: color, 
            backgroundColor: 'transparent', 
            borderWidth: 3, 
            pointRadius: 0, 
            tension: 0.1 
          }] 
        },
        options: {
          responsive: true, maintainAspectRatio: false,
          scales: {
            y: { min: 0, max: maxVal, grid: { color: '#e0e0e0' }, ticks: { font: { weight: 'bold' } } },
            x: { title: { display: true, text: '10 Second History', color: '#999', font: { size: 13 } }, grid: { display: false } }
          },
          plugins: { legend: { display: false } },
          animation: { duration: 0 } 
        }
      };
    }

    var rpmChart = new Chart(document.getElementById('rpmChart').getContext('2d'), createChartConfig('#0056b3', 'RPM', 3500));
    var pwmChart = new Chart(document.getElementById('pwmChart').getContext('2d'), createChartConfig('#d32f2f', 'PWM %', 100));
    
    var source = new EventSource('/events');
    source.onmessage = function(e) {
      var parts = e.data.split(',');
      var rpm = parts[0];
      var pwm = parts[1];
      
      document.getElementById('val-rpm').innerHTML = rpm + '<span class="unit">RPM</span>';
      document.getElementById('val-pwm').innerHTML = pwm + '<span class="unit">%</span>';

      rpmChart.data.datasets[0].data.push(rpm);
      if (rpmChart.data.datasets[0].data.length > 100) rpmChart.data.datasets[0].data.shift();
      rpmChart.update('none');

      pwmChart.data.datasets[0].data.push(pwm);
      if (pwmChart.data.datasets[0].data.length > 100) pwmChart.data.datasets[0].data.shift();
      pwmChart.update('none');
    };
  </script>
</body>
</html>
)rawliteral";



void setup() {
  pinMode(32, OUTPUT); 
  digitalWrite(32, HIGH); // Turn on the light

  pinMode(0, INPUT_PULLUP); 
  Serial.begin(115200);   
  Serial2.begin(115200, SERIAL_8N1, 21, 22); // RX=21, TX=22

  // 2. TFT INITIALIZATION
  tft.init();
  tft.setRotation(1); 
  tft.fillScreen(TFT_RED); // If this works, you'll see blue immediately
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("WAKING UP...", 120, 60, 4); // Adjusted for 240x135 screen
  delay(1000);
  
  // 3. WIFI MANAGER
  ////////////////////////////////////////////////////////////////////////////////////////////
  // tft.fillScreen(TFT_BLACK);
  // tft.drawString("Connecting Wi-Fi...", 10, 10);

  // WiFiManager wm;
  // wm.setConfigPortalTimeout(60); 
  // if (!wm.autoConnect("DrillPress_Setup")) {
  //   tft.fillScreen(TFT_RED);
  //   tft.drawString("WiFi Offline Mode", 10, 10);
  //   delay(2000);
  // }

  // displayAddress = String(WiFi.SSID()) + " | " + WiFi.localIP().toString();
  /////////////////////////////////////////////////////////////////////////////////////////////

  // 3. ACCESS POINT MODE
  /////////////////////////////////////////////////////////////////////////////////////////////
  tft.fillScreen(TFT_RED);
  tft.drawString("Starting AP...", 10, 10);

  // Set SSID and an optional password (leave password out for open access)
  const char* ssid = "DrillPress_Monitor";
  const char* password = NULL; // Must be at least 8 chars or NULL

  displayAddress = String(ssid) + " | 192.168.4.1";


  // create custom IP (to use default [192.168.4.1], comment out next 5 lines)
  IPAddress local_IP(10, 0, 0, 1);
  IPAddress gateway(10, 0, 0, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  displayAddress = String(ssid) + "  |  " + local_IP.toString();

  WiFi.softAP(ssid, password);
  /////////////////////////////////////////////////////////////////////////////////////////////


  // 4. SERVER SETUP
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  // Handle Numeric Input from Website
  server.on("/setRPM", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("val")) {
      String rpmValue = request->getParam("val")->value();
      
      // SEND TO STM32 over Serial2
      Serial2.println(rpmValue); 
      
      // Optional: Debug to Serial Monitor
      Serial.print("User set target RPM to: ");
      Serial.println(rpmValue);
    }
    request->send(200, "text/plain", "OK");
  });

  server.addHandler(&events);
  server.begin();

  tft.fillScreen(TFT_WHITE);
  // 0 RPM Mark
  tft.drawFastHLine(0, 136, 320, TFT_BLACK); 


  // Footer Line and IP Address
  tft.drawFastHLine(0, 146, 320, TFT_MAROON); 
  tft.setTextColor(TFT_MAROON);
  tft.drawCentreString(displayAddress, 160, 150, 2);

  // DEBUG
  Serial.println("SETUP DONE"); 
}


// void loop() {
//   #ifdef ESP32
//     // mDNS update is handled automatically on newer cores, 
//     // but adding it ensures the name stays active
//   #endif

//   // Pin 0 is the BOOT button
//   if (digitalRead(0) == LOW) {
//     // if BOOT button held for 3 secobds, reset Wifi 
//     checkWifiReset();
//   }
  
//   // if message sent from STM32
//   if (Serial2.available()) {
//     // Get data as a CSV string "RPM,PWM" and split on the comma
//     String data = Serial2.readStringUntil('\n');
//     int commaIndex = data.indexOf(',');

//     // If a comma exists (data in the right format)
//     if (commaIndex != -1) { 
//       // Send the raw CSV string to the browser
//       events.send(data.c_str(), "message", millis());

//       // Update the physical 1.9" LCD with new RPM 
//       int rpm = data.substring(0, commaIndex).toInt();
//       updateLCDGraph(rpm);
//     }
//   }
// }


void loop() {
  // 1. Check for BOOT Button
  if (digitalRead(0) == LOW) {
    // WIFI MANAGER MODE
    // checkWifiReset();

    // ACCESS POINT MODE
    handleSystemButton();
  }

  // HEARTBEAT
  static unsigned long hb = 0;
  if (millis() - hb > 1000) {
    Serial.println("ESP32 is alive - Waiting for STM32...");
    hb = millis();
  }

  // 2. Handle Real Data from STM32
  if (Serial2.available()) {
    // Get data as a CSV string "RPM,PWM" (e.g., "1500,45")
    String data = Serial2.readStringUntil('\n');
    int commaIndex = data.indexOf(',');

    // DEBUG
    Serial.println(data); 

    // If data is in the correct "RPM,PWM" format
    if (commaIndex != -1) { 
      // 1. Send the raw string to the Web Dashboard immediately
      events.send(data.c_str(), "message", millis());

      // 2. Parse the RPM for the physical LCD
      int rpm = data.substring(0, commaIndex).toInt();
      
      // 3. Update the LCD Graph
      updateLCDGraph(rpm);
    }
  }
}


// // Simulated RPM and PWM input
// void loop() {
//   // 1. Check for WiFi Reset (Boot Button)
//   if (digitalRead(0) == LOW) {
//     checkWifiReset();
//   }

//   // 2. Handle Data (Real from STM32 OR Simulated)
//   static unsigned long lastUpdate = 0;
  
//   if (Serial2.available()) {
//     // REAL DATA: Get string from STM32
//     String data = Serial2.readStringUntil('\n');
//     int commaIndex = data.indexOf(',');
//     if (commaIndex != -1) { 
//       events.send(data.c_str(), "message", millis());
//       int rpm = data.substring(0, commaIndex).toInt();
//       updateLCDGraph(rpm);
//     }
//   } 
//   else if (millis() - lastUpdate > 100) { 
//     // SIMULATED DATA: Runs every 100ms if STM32 is silent
//     lastUpdate = millis();
    
//     // FULL RANGE SIMULATOR (0 to 3500)
//     // Using a slower sine wave to sweep the full range
//     float wave = sin(millis() / 2000.0); // Slows it down so you see the climb
//     int simRPM = (wave * 1750) + 1750;   // This results in 0 to 3500

//     // Create a wandering PWM value (40% to 60% range)
//     int simPWM = 50 + (sin(millis() / 1500.0) * 10);
    
//     // Format as CSV "RPM,PWM"
//     String simData = String(simRPM) + "," + String(simPWM);
    
//     // Send to Web Dashboard
//     events.send(simData.c_str(), "message", millis());
    
//     // Send to LCD Graph
//     updateLCDGraph(simRPM);
//   }
// }



void updateLCDGraph(int val) {
  const int pointsToKeep = 100; 
  const int pixelWidth = 3;     
  const int graphBottom = 135;
  const int graphHeight = 130;

  // 1. Shift history (Only need to shift the 100 points we care about)
  for (int i = 0; i < pointsToKeep - 1; i++) {
    rpmHistory[i] = rpmHistory[i + 1];
  }
  
  // 2. Map new value
  int yNew = graphBottom - (int)(val * graphHeight / 3500.0);
  rpmHistory[pointsToKeep - 1] = constrain(yNew, 5, graphBottom);

  // 3. Draw
  tft.startWrite();
  tft.fillRect(0, 0, 300, graphBottom + 1, TFT_WHITE); // Clear graph area

  for (int i = 1; i < pointsToKeep; i++) {
    tft.drawLine((i - 1) * pixelWidth, rpmHistory[i - 1], i * pixelWidth, rpmHistory[i], TFT_RED);
  }
  tft.endWrite();
}


// WIFI MANAGER MODE
///////////////////////////////////////////////////////////////////////////////
// void checkWifiReset() {
//   unsigned long startPress = millis();
  
//   // Immediate visual feedback on the LCD
//   tft.fillScreen(TFT_RED);
//   tft.setTextColor(TFT_WHITE);
//   tft.drawCentreString("HOLD 3S TO RESET", 160, 60, 2);

//   // Stay in this loop as long as the button is held
//   while (digitalRead(0) == LOW) {
//     if (millis() - startPress > 3000) {
//       tft.fillScreen(TFT_BLACK);
//       tft.drawCentreString("WIFI CLEARED", 160, 75, 4);
//       tft.drawCentreString("REBOOTING...", 160, 110, 2);
      
//       WiFiManager wm;
//       wm.resetSettings(); // Wipes saved SSID/Password
//       delay(2000);
//       ESP.restart(); 
//     }
//   }
//   // If released before 3s, clear the red screen and resume
//   tft.fillScreen(TFT_BLACK);
// }
//////////////////////////////////////////////////////////////////////////////


// ACCESS POINT MODE
//////////////////////////////////////////////////////////////////////////////
void handleSystemButton() {
  unsigned long startPress = millis();
  
  // Immediate visual feedback on the LCD
  tft.fillScreen(TFT_BLUE);
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("HOLD 3S TO RESET", 160, 60, 2);

  // Stay in this loop as long as the button is held
  while (digitalRead(0) == LOW) {
    if (millis() - startPress > 3000) {
      tft.fillScreen(TFT_BLACK);
      tft.drawCentreString("DEVICE RESET", 160, 75, 4);
      tft.drawCentreString("REBOOTING...", 160, 110, 2);
      delay(2000);
      ESP.restart(); 
    }
  }
  // If released before 3s, clear the red screen and resume
  tft.fillScreen(TFT_WHITE);
}
//////////////////////////////////////////////////////////////////////////////