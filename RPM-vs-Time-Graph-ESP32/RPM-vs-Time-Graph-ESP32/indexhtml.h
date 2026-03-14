#ifndef INDEXHTML_H
#define INDEXHTML_H

const char index_html[] PROGMEM = R"rawliteral(
<html>
<head>
  <script src="/chart.min.js"></script>
  <style>
    body { font-family: 'Segoe UI', sans-serif; background: #f4f7f6; color: #333; padding: 20px; }
    #container { max-width: 1200px; margin: auto; background: #ffffff; padding: 25px; border-radius: 15px; box-shadow: 0 4px 20px rgba(0,0,0,0.08); }
    .header { text-align: center; margin-bottom: 25px; border-bottom: 2px solid #eee; padding-bottom: 15px; }
    h1 { color: #222; margin: 0; font-weight: 600; font-size: 1.6rem; }
    
    .dashboard-grid { display: flex; gap: 20px; flex-wrap: wrap; }
    .graph-section { flex: 1; min-width: 450px; background: #fcfcfc; padding: 15px; border-radius: 12px; border: 1px solid #eee; }
    
    .stat-header { display: flex; justify-content: space-between; align-items: baseline; margin-bottom: 10px; padding: 0 5px; }
    .stat-group { display: flex; flex-direction: column; }
    .stat-label { font-size: 0.75rem; color: #666; text-transform: uppercase; font-weight: bold; margin-bottom: 2px; }
    .stat-value { font-size: 1.8rem; font-weight: 800; font-family: 'Courier New', monospace; }
    #val-rpm { color: #0056b3; }
    #val-pwm { color: #008000; }
    .target-value { font-size: 1.8rem; font-weight: 800; font-family: 'Courier New', monospace; color: #d32f2f; }
    .unit { font-size: 0.9rem; color: #aaa; margin-left: 3px; }
    
    .rpm-accent { border-top: 5px solid #0056b3; }
    .pwm-accent { border-top: 5px solid #008000; }
    
    .control-section { margin-top: 25px; padding: 20px; background: #ebf5fb; border-radius: 12px; border: 1px solid #aed6f1; display: flex; align-items: center; justify-content: center; gap: 15px; }
    input[type=number] { padding: 10px; border-radius: 8px; border: 1px solid #bdc3c7; font-size: 1rem; width: 120px; text-align: center; }
    .btn-send { padding: 10px 25px; background: #2980b9; color: white; border: none; border-radius: 8px; cursor: pointer; font-weight: bold; transition: 0.2s; }
    .btn-send:hover { background: #1f6391; }

    /* Tall Graphs for better detail */
    .chart-container { position: relative; height: 500px; width: 100%; }
  </style>
</head>
<body>
  <div id="container">
    <div class="header">
      <h1>Drill Press Monitoring System</h1>
    </div>
    
    <div class="dashboard-grid">
      <div class="graph-section rpm-accent">
        <div class="stat-header">
          <div class="stat-group">
            <span class="stat-label">Actual Speed</span>
            <span class="stat-value" id="val-rpm">0<span class="unit">RPM</span></span>
          </div>
          <div class="stat-group" style="text-align: right;">
            <span class="stat-label">Target</span>
            <span class="target-value" id="target-display">0<span class="unit">RPM</span></span>
          </div>
        </div>
        <div class="chart-container">
          <canvas id="rpmChart"></canvas>
        </div>
      </div>

      <div class="graph-section pwm-accent">
        <div class="stat-header">
          <div class="stat-group">
            <span class="stat-label">Motor Load</span>
            <span class="stat-value" id="val-pwm">0<span class="unit">%</span></span>
          </div>
        </div>
        <div class="chart-container">
          <canvas id="pwmChart"></canvas>
        </div>
      </div>
    </div>

    <div class="control-section">
      <span class="stat-label" style="color:#2c3e50">Target RPM:</span>
      <input type="number" id="rpmInput" min="0" max="3500" value="0">
      <button class="btn-send" onclick="sendRPM()">SET SPEED</button>
    </div>




<!--
//start///////////////////////////////////////////////////////////////////////////////////
//TODO - arcade
-->
    <div style="margin-top: 40px; text-align: center; border-top: 2px dashed #ccc; padding-top: 20px; padding-bottom: 40px;">
        <button id="gameToggle" onclick="toggleGame()" style="padding: 12px 25px; background: #7f8c8d; color: white; border: none; border-radius: 8px; cursor: pointer; font-weight: bold; font-size: 1.1rem; margin: 5px;">
            FLAPPY BRICK
        </button>
        
        <button id="pacToggle" onclick="togglePacman()" style="padding: 12px 25px; background: #f1c40f; color: #2c3e50; border: none; border-radius: 8px; cursor: pointer; font-weight: bold; font-size: 1.1rem; margin: 5px;">
            CIRCUIT PAC
        </button>

        <div id="gameContainer" style="display: none; margin-top: 20px;">
            <iframe id="gameFrame" src="" style="border:none; border-radius: 12px; width: 100%; height: 700px; max-width: 500px; margin: auto; display: block;"></iframe>
        </div>

        <div id="pacContainer" style="display: none; margin-top: 20px;">
            <iframe id="pacFrame" src="" style="border:none; border-radius: 12px; width: 100%; height: 700px; max-width: 500px; margin: auto; display: block;"></iframe>
        </div>
    </div>

    <script>
    function toggleGame() {
        var c = document.getElementById('gameContainer');
        var p = document.getElementById('pacContainer');
        var f = document.getElementById('gameFrame');
        if (c.style.display === "none") {
            c.style.display = "block"; p.style.display = "none";
            if (f.src === "" || f.src === window.location.href) f.src = "/game";
        } else { c.style.display = "none"; f.src = ""; }
    }

    function togglePacman() {
        var c = document.getElementById('gameContainer');
        var p = document.getElementById('pacContainer');
        var f = document.getElementById('pacFrame');
        if (p.style.display === "none") {
            p.style.display = "block"; c.style.display = "none";
            if (f.src === "" || f.src === window.location.href) f.src = "/pacman";
        } else { p.style.display = "none"; f.src = ""; }
    }
    </script>
<!--
//end/////////////////////////////////////////////////////////////////////////////////////
-->




  </div>

<script>
    let currentTarget = 0;

    function sendRPM() {
      var val = document.getElementById('rpmInput').value;
      if (val === "") return;
      currentTarget = parseInt(val);
      document.getElementById('target-display').innerHTML = currentTarget + '<span class="unit">RPM</span>';
      
      fetch('/setRPM?val=' + val, { keepalive: true });
      
      const btn = document.querySelector('.btn-send');
      btn.style.background = "#27ae60";
      setTimeout(() => btn.style.background = "#2980b9", 150);
    }

    // High performance config
    function createChartConfig(datasets, maxVal) {
      return {
        type: 'line',
        data: { 
          labels: Array(100).fill(''), 
          datasets: datasets 
        },
        options: {
          responsive: true, maintainAspectRatio: false,
          animation: false,
          elements: { line: { tension: 0 }, point: { radius: 0 } },
          scales: {
            y: { min: 0, max: maxVal, grid: { color: '#e0e0e0' }, ticks: { font: { weight: 'bold' } } },
            x: { grid: { display: false } }
          },
          plugins: { legend: { display: false } }
        }
      };
    }

    const rpmDatasets = [
      { label: 'Actual', data: [], borderColor: '#0056b3', borderWidth: 3 },
      { label: 'Target', data: [], borderColor: '#d32f2f', borderWidth: 3 }
    ];

    const pwmDatasets = [
      { label: 'PWM', data: [], borderColor: '#008000', borderWidth: 3 }
    ];

    var rpmChart = new Chart(document.getElementById('rpmChart').getContext('2d'), createChartConfig(rpmDatasets, 3500));
    var pwmChart = new Chart(document.getElementById('pwmChart').getContext('2d'), createChartConfig(pwmDatasets, 100));
    
    var source = new EventSource('/events');
    source.onmessage = function(e) {
      var parts = e.data.split(',');
      var rpm = parseFloat(parts[0]);
      var pwm = parseFloat(parts[1]);
      
      document.getElementById('val-rpm').innerHTML = rpm + '<span class="unit">RPM</span>';
      document.getElementById('val-pwm').innerHTML = pwm + '<span class="unit">%</span>';

      // Update RPM Chart
      rpmChart.data.datasets[0].data.push(rpm);
      rpmChart.data.datasets[1].data.push(currentTarget); // The target line
      
      // Update PWM Chart
      pwmChart.data.datasets[0].data.push(pwm);

      // Manage Buffer
      if (rpmChart.data.datasets[0].data.length > 100) {
        rpmChart.data.datasets[0].data.shift();
        rpmChart.data.datasets[1].data.shift();
        pwmChart.data.datasets[0].data.shift();
      }

      rpmChart.update('none');
      pwmChart.update('none');
    };
  </script>
</body>
</html>
)rawliteral";

#endif