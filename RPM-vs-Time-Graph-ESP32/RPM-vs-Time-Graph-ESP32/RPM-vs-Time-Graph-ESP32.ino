#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <WiFiManager.h>
#include <ESPmDNS.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();
AsyncWebServer server(80);
AsyncEventSource events("/events");

int rpmHistory[320]; // Stores data for the physical LCD graph matching the 320px width of the 1.9" screen

String displayAddress = "";

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
  </div>
  <script>
    function createChartConfig(color, label, maxVal) {
      return {
        type: 'line',
        data: { labels: Array(50).fill(''), datasets: [{ label: label, data: [], borderColor: color, backgroundColor: 'transparent', borderWidth: 3, pointRadius: 0, tension: 0.2 }] },
        options: {
          responsive: true, maintainAspectRatio: false,
          scales: {
            y: { min: 0, max: maxVal, grid: { color: '#e0e0e0' }, ticks: { font: { weight: 'bold' } } },
            x: { title: { display: true, text: 'Time (100ms intervals)', color: '#999', font: { size: 13 } }, grid: { display: false } }
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
      if (rpmChart.data.datasets[0].data.length > 50) rpmChart.data.datasets[0].data.shift();
      rpmChart.update();

      pwmChart.data.datasets[0].data.push(pwm);
      if (pwmChart.data.datasets[0].data.length > 50) pwmChart.data.datasets[0].data.shift();
      pwmChart.update();
    };
  </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);   
  Serial2.begin(115200); // Connection to STM32 (TX of STM32 to Pin 16 RX2 of ESP32)

  tft.init();
  tft.setRotation(1); // landscape
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("Connecting Wi-Fi...", 10, 10);

  WiFiManager wm;
  if (!wm.autoConnect("DrillPress_Setup")) {
    ESP.restart();
  }

  if (MDNS.begin("drillpress")) {
    MDNS.addService("http", "tcp", 80);
  }

  // Store the address string
  displayAddress = WiFi.localIP().toString() + "  |  drillpress.local";

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.addHandler(&events);
  server.begin();
  tft.fillScreen(TFT_BLACK);
}

void loop() {
  if (Serial2.available()) {
    String data = Serial2.readStringUntil('\n');
    
    // Split the CSV string "RPM,PWM"
    int commaIndex = data.indexOf(',');
    if (commaIndex != -1) {
      int rpm = data.substring(0, commaIndex).toInt();

      // Send the raw CSV string to the browser
      events.send(data.c_str(), "message", millis());

      // Update the physical 1.9" LCD with RPM graph
      updateLCDGraph(currentRPM);
    }
  }
}

void updateLCDGraph(int val) {
  tft.startWrite(); // Start high-speed SPI transaction
  
  tft.fillScreen(TFT_BLACK);

  // 1. Draw the "Status Bar" at the bottom (30 pixels high)
  tft.drawFastHLine(0, 140, 320, TFT_DARKGREY); // Separation line
  tft.setTextColor(TFT_CYAN);
  tft.drawCentreString(displayAddress, 160, 150, 2); // IP and URL

  // 2. Update and Draw the Graph (140 pixels high)
  // Shift history array
  for(int i=0; i<319; i++) {
    rpmHistory[i] = rpmHistory[i+1];
    
    // Scale 3500 RPM to 140 pixels: (rpm / 3500) * 140
    // We subtract from 139 because Y=0 is the top of the screen
    int y1 = 139 - (rpmHistory[i] * 139 / 3500);
    int y2 = 139 - (rpmHistory[i+1] * 139 / 3500);
    
    // Draw the line segment
    if (rpmHistory[i] > 0) { // Only draw if there is data
       tft.drawLine(i, y1, i+1, y2, TFT_BLUE);
    }
  }
  rpmHistory[319] = val;

  tft.endWrite(); // End high-speed SPI transaction
}