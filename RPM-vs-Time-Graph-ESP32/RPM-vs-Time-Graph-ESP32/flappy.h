#ifndef FLAPPY_H
#define FLAPPY_H

#include <Arduino.h>

const char flappy_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <style>
        body { 
            margin: 0; 
            overflow: hidden; 
            background: #2c3e50; 
            font-family: sans-serif; 
            /* This is the magic line that kills double-tap zoom */
            touch-action: none; 
            display: flex;
            flex-direction: column;
            align-items: center;
            color: white; 
        }
        .score-container { 
            display: flex; 
            justify-content: space-between; 
            width: 95vw;
            max-width: 550px;
            padding: 10px 0; 
            font-weight: bold; 
            font-size: 1.2rem; 
        }
        canvas { 
            display: block; 
            background: #70c5ce; 
            border: 4px solid #333; 
            border-radius: 10px; 
            
            /* Mobile First: Take up almost full width */
            width: 98vw; 
            /* Laptop/Landscape: Prevent it from getting too tall for the screen */
            max-width: 550px;
            max-height: 85vh; 
            
            height: auto; 
            image-rendering: pixelated; /* Keeps the brick sharp */
        }
    </style>
</head>
<body>
    <div class="score-container">
        <div>RECORD: <span id="hi-val">0</span></div>
        <div>RPM: <span id="score-val">0</span></div>
    </div>
    <canvas id="birdCanvas" width="550" height="750"></canvas>

    <script>
        const canvas = document.getElementById('birdCanvas');
        const ctx = canvas.getContext('2d');
        
        let audioCtx;
        let nextNoteTime = 0;
        let currentNote = 0;
        const melody = [
            43.65, 43.65, 87.31, 43.65, // F1, F1, F2, F1
            51.91, 51.91, 58.27, 43.65, // Ab1, Ab1, Bb1, F1
            43.65, 43.65, 87.31, 43.65, // F1, F1, F2, F1
            77.78, 65.41, 51.91, 58.27  // Eb2, C2, Ab1, Bb1
        ];
        const tempo = 0.25;

        function initAudio() {
            if (!audioCtx) {
                audioCtx = new (window.AudioContext || window.webkitAudioContext)();
            }
            if (audioCtx.state === 'suspended') audioCtx.resume();
        }

        function scheduler() {
            while (nextNoteTime < audioCtx.currentTime + 0.1) {
                if (!isDead && gameStarted) {
                    playMelodyNote(melody[currentNote], nextNoteTime);
                }
                nextNoteTime += tempo;
                currentNote = (currentNote + 1) % melody.length;
            }
            requestAnimationFrame(scheduler);
        }

        function playMelodyNote(freq, time) {
            const osc = audioCtx.createOscillator();
            const gain = audioCtx.createGain();
            
            // Change 'triangle' to 'sawtooth' if you want it to sound "buzzier" and louder
            osc.type = 'triangle'; 
            osc.frequency.setValueAtTime(freq, time);
            
            // VOLUME CONTROL: Changed from 0.25 to 0.5
            gain.gain.setValueAtTime(0.5, time); 
            
            // This controls how fast the note fades. 
            // Changing tempo to tempo*0.9 makes it sound "punchier"
            gain.gain.exponentialRampToValueAtTime(0.001, time + tempo);
            
            osc.connect(gain);
            gain.connect(audioCtx.destination);
            osc.start(time);
            osc.stop(time + tempo);
        }

        function playSound(type) {
            initAudio();
            const osc = audioCtx.createOscillator();
            const gain = audioCtx.createGain();
            osc.connect(gain); gain.connect(audioCtx.destination);
            if(type === 'jump') {
                osc.type = 'square';
                osc.frequency.setValueAtTime(200, audioCtx.currentTime);
                osc.frequency.exponentialRampToValueAtTime(500, audioCtx.currentTime + 0.1);
                gain.gain.setValueAtTime(0.1, audioCtx.currentTime);
                osc.start(); osc.stop(audioCtx.currentTime + 0.1);
            } else {
                osc.type = 'sawtooth';
                osc.frequency.setValueAtTime(120, audioCtx.currentTime);
                gain.gain.setValueAtTime(0.1, audioCtx.currentTime);
                osc.start(); osc.stop(audioCtx.currentTime + 0.4);
            }
        }

        let bird = { y: 350, v: 0, gravity: 0.35, jump: -7.0 };
        let pipes = [];
        let score = 0;
        let highScore = 0;
        let frame = 0;
        let isDead = false;
        let gameStarted = false;

        fetch('/getHighScore').then(r => r.text()).then(d => {
            highScore = parseInt(d) || 0;
            document.getElementById('hi-val').innerText = highScore;
        });

        function resetGame() {
            if (score > highScore) {
                highScore = score;
                document.getElementById('hi-val').innerText = highScore;
                fetch('/setHighScore?score=' + score);
            }
            bird.y = 350; bird.v = 0; pipes = []; score = 0; frame = 0;
            document.getElementById('score-val').innerText = "0";
            isDead = false;
        }

        function draw() {
            ctx.fillStyle = "#70c5ce";
            ctx.fillRect(0, 0, canvas.width, canvas.height);
            if (!isDead) {
                if (gameStarted) {
                    bird.v += bird.gravity;
                    bird.y += bird.v;
                    if (frame % 90 === 0) {
                        let gap = 180;
                        let top = Math.random() * (canvas.height - gap - 200) + 100;
                        pipes.push({ x: canvas.width, top: top, bottom: top + gap, passed: false });
                    }
                    pipes.forEach((p, i) => {
                        p.x -= 4;
                        ctx.fillStyle = "yellow";
                        ctx.fillRect(p.x, 0, 60, p.top);
                        ctx.fillRect(p.x, p.bottom, 60, canvas.height);
                        if (80 + 35 > p.x && 80 < p.x + 60 && (bird.y < p.top || bird.y + 35 > p.bottom)) {
                            isDead = true; playSound('crash');
                        }
                        if (!p.passed && p.x < 80) { p.passed = true; score++; document.getElementById('score-val').innerText = score; }
                        if (p.x < -60) pipes.splice(i, 1);
                    });
                    if (bird.y > canvas.height || bird.y < 0) { isDead = true; playSound('crash'); }
                    frame++;
                }
            } else {
                ctx.fillStyle = "rgba(0,0,0,0.6)"; ctx.fillRect(0,0,canvas.width, canvas.height);
                ctx.fillStyle = "white"; ctx.font = "bold 30px sans-serif"; ctx.textAlign = "center";
                ctx.fillText("DRILL BIT SNAPPED!", canvas.width/2, 330);
                ctx.font = "20px sans-serif"; ctx.fillText("Space (Laptop) or Tap (Phone)", canvas.width/2, 380);
            }
            ctx.fillStyle = "#e74c3c"; ctx.fillRect(80, bird.y, 35, 35);
            ctx.strokeStyle = "white"; ctx.lineWidth = 2; ctx.strokeRect(80, bird.y, 35, 35);
            if (!gameStarted && !isDead) {
                ctx.fillStyle = "white"; ctx.font = "bold 25px sans-serif"; ctx.textAlign = "center";
                ctx.fillText("READY TO DRILL?", canvas.width/2, 350);
            }
            requestAnimationFrame(draw);
        }

        function handleInput(e) {
            if (e && e.type === 'touchstart') e.preventDefault();
            if (e && e.code === "Space") e.preventDefault();
            
            initAudio();
            if (!gameStarted) {
                gameStarted = true;
                nextNoteTime = audioCtx.currentTime;
                scheduler();
            }
            if (isDead) resetGame();
            else { bird.v = bird.jump; playSound('jump'); }
        }

        window.addEventListener('keydown', (e) => {
            if (e.code === "Space") handleInput(e);
        });

        canvas.addEventListener('touchstart', (e) => {
            handleInput(e);
        }, {passive: false});

        draw();
    </script>
</body>
</html>
)rawliteral";

#endif