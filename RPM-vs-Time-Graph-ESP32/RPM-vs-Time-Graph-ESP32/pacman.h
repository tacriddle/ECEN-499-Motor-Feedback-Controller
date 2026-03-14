#ifndef PACMAN_H
#define PACMAN_H

#include <Arduino.h>

const char pacman_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <style>
        body { margin: 0; background: #1a1a1a; color: white; font-family: sans-serif; display: flex; flex-direction: column; align-items: center; touch-action: none; overflow: hidden; height: 100vh; }
        #ui { display: flex; justify-content: space-between; width: 95vw; max-width: 400px; padding: 10px; font-weight: bold; color: #f1c40f; font-size: 0.9rem; }
        canvas { background: #000; border: 3px solid #333; border-radius: 8px; width: 95vw; max-width: 400px; }
        .controls { display: grid; grid-template-columns: repeat(3, 75px); grid-template-rows: repeat(2, 75px); gap: 10px; margin-top: 10px; }
        .btn { background: #333; border: 2px solid #444; border-radius: 12px; color: white; font-size: 1.8rem; display: flex; align-items: center; justify-content: center; user-select: none; }
        .btn:active { background: #f1c40f; color: black; }
        .up { grid-column: 2; }
        .left { grid-column: 1; grid-row: 2; }
        .down { grid-column: 2; grid-row: 2; }
        .right { grid-column: 3; grid-row: 2; }
    </style>
</head>
<body>
    <div id="ui">
        <div>RECORD: <span id="hi-scr">0</span></div>
        <div>SCORE: <span id="scr">0</span></div>
    </div>
    <canvas id="pacCanvas" width="400" height="360"></canvas>
    
    <div class="controls">
        <div class="btn up" onmousedown="handleBtn(0,-1)" ontouchstart="handleBtn(0,-1)">W</div>
        <div class="btn left" onmousedown="handleBtn(-1,0)" ontouchstart="handleBtn(-1,0)">A</div>
        <div class="btn down" onmousedown="handleBtn(0,1)" ontouchstart="handleBtn(0,1)">S</div>
        <div class="btn right" onmousedown="handleBtn(1,0)" ontouchstart="handleBtn(1,0)">D</div>
    </div>

    <script>
        const canvas = document.getElementById('pacCanvas');
        const ctx = canvas.getContext('2d');
        const size = 20; 

        let score = 0, highScore = 0;
        let gameOver = false, gameWin = false;
        let audioStarted = false, audioCtx;

        const maze = [
            [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1],
            [1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1],
            [1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1],
            [1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1],
            [1,0,1,1,0,1,0,1,1,1,1,1,1,0,1,0,1,1,0,1],
            [1,0,0,0,0,1,0,0,0,1,1,0,0,0,1,0,0,0,0,1],
            [1,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,1],
            [1,2,2,1,0,1,0,0,0,0,0,0,0,0,1,0,1,2,2,1], 
            [1,1,1,1,0,1,0,1,1,2,2,1,1,0,1,0,1,1,1,1],
            [1,0,0,0,0,0,0,1,2,2,2,2,1,0,0,0,0,0,0,1], // Ghost starts in these 2s
            [1,1,1,1,0,1,0,1,1,1,1,1,1,0,1,0,1,1,1,1],
            [1,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,1],
            [1,0,1,1,0,1,0,1,1,1,1,1,1,0,1,0,1,1,0,1],
            [1,0,0,1,0,0,0,0,0,1,1,0,0,0,0,0,1,0,0,1],
            [1,1,0,1,0,1,1,1,0,1,1,0,1,1,1,0,1,0,1,1],
            [1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1],
            [1,0,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,0,1],
            [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]
        ];

        let pac = { x: 1, y: 1, dx: 0, dy: 0, nextDx: 0, nextDy: 0 };
        // GHOST START: Now in the center (row 9, column 10)
        let ghost = { x: 10, y: 9, dx: 0, dy: 0 };

        fetch('/getPacScore').then(r => r.text()).then(d => {
            highScore = parseInt(d) || 0;
            document.getElementById('hi-scr').innerText = highScore;
        });

        function initAudio() {
            if (!audioCtx) {
                audioCtx = new (window.AudioContext || window.webkitAudioContext)();
                startMusic();
            }
            if (audioCtx.state === 'suspended') audioCtx.resume();
        }

        function playClick() {
            initAudio();
            const osc = audioCtx.createOscillator();
            const g = audioCtx.createGain();
            osc.frequency.setValueAtTime(800, audioCtx.currentTime);
            osc.frequency.exponentialRampToValueAtTime(100, audioCtx.currentTime + 0.1);
            g.gain.setValueAtTime(0.1, audioCtx.currentTime);
            osc.connect(g); g.connect(audioCtx.destination);
            osc.start(); osc.stop(audioCtx.currentTime + 0.1);
        }

        function startMusic() {
            if (audioStarted) return;
            audioStarted = true;
            const loop = () => {
                if (gameOver || gameWin) return;
                const osc = audioCtx.createOscillator();
                const g = audioCtx.createGain();
                const notes = [110, 110, 123, 110, 146, 110]; 
                osc.frequency.setValueAtTime(notes[Math.floor(Date.now()/400) % 6], audioCtx.currentTime);
                osc.type = 'triangle';
                g.gain.setValueAtTime(0.5, audioCtx.currentTime);
                g.gain.exponentialRampToValueAtTime(0.001, audioCtx.currentTime + 0.4);
                osc.connect(g); g.connect(audioCtx.destination);
                osc.start(); osc.stop(audioCtx.currentTime + 0.4);
                setTimeout(loop, 400);
            };
            loop();
        }

        function handleBtn(x, y) {
            playClick();
            if(gameOver || gameWin) location.reload();
            pac.nextDx = x; pac.nextDy = y;
        }

        function isWalkable(x, y) {
            return maze[y] && (maze[y][x] === 0 || maze[y][x] === 2);
        }

        function checkWin() {
            for(let r=0; r<maze.length; r++) {
                if(maze[r].includes(0)) return false;
            }
            return true;
        }

        function update() {
            if(gameOver || gameWin) return;

            // Pacman movement logic
            if (isWalkable(pac.x + pac.nextDx, pac.y + pac.nextDy)) {
                pac.dx = pac.nextDx; pac.dy = pac.nextDy;
            }
            if (isWalkable(pac.x + pac.dx, pac.y + pac.dy)) {
                pac.x += pac.dx; pac.y += pac.dy;
            }
            if (maze[pac.y][pac.x] === 0) {
                maze[pac.y][pac.x] = 2;
                score += 10;
                document.getElementById('scr').innerText = score;
                if(checkWin()) {
                    gameWin = true;
                    if(score > highScore) fetch('/setPacScore?score=' + score);
                }
            }

            // GHOST AI: Much more active
            let gx = ghost.x, gy = ghost.y;
            // 85% chance to seek, 15% random wander
            if (Math.random() > 0.15) {
                // Determine horizontal and vertical distance
                let dx = pac.x - ghost.x;
                let dy = pac.y - ghost.y;

                // Move in the direction of the greatest distance
                if (Math.abs(dx) > Math.abs(dy)) {
                    gx += (dx > 0) ? 1 : -1;
                } else {
                    gy += (dy > 0) ? 1 : -1;
                }
            } else {
                const dirs = [[0,1],[0,-1],[1,0],[-1,0]];
                const d = dirs[Math.floor(Math.random()*4)];
                gx += d[0]; gy += d[1];
            }

            // Only move if the target tile is walkable
            if (isWalkable(gx, gy)) { 
                ghost.x = gx; ghost.y = gy; 
            }

            // Check collision
            if (pac.x === ghost.x && pac.y === ghost.y) {
                gameOver = true;
                if(score > highScore) fetch('/setPacScore?score=' + score);
            }
        }

        function draw() {
            ctx.fillStyle = "black"; ctx.fillRect(0, 0, canvas.width, canvas.height);
            for (let r = 0; r < maze.length; r++) {
                for (let c = 0; c < maze[r].length; c++) {
                    if (maze[r][c] === 1) {
                        ctx.fillStyle = "#2980b9"; ctx.fillRect(c*size, r*size, size-1, size-1);
                    } else if (maze[r][c] === 0) {
                        ctx.fillStyle = "#f1c40f"; ctx.beginPath();
                        ctx.arc(c*size + size/2, r*size + size/2, 2, 0, 7); ctx.fill();
                    }
                }
            }
            // Draw Ghost (The Rust Spot)
            ctx.fillStyle = "#e74c3c"; ctx.fillRect(ghost.x*size+2, ghost.y*size+2, size-4, size-4);
            // Draw Pac
            ctx.fillStyle = "yellow"; ctx.beginPath();
            ctx.arc(pac.x*size + size/2, pac.y*size + size/2, size/1.5, 0.2*Math.PI, 1.8*Math.PI);
            ctx.lineTo(pac.x*size + size/2, pac.y*size + size/2); ctx.fill();

            if(gameOver || gameWin) {
                ctx.fillStyle = "rgba(0,0,0,0.8)"; ctx.fillRect(0,0,400,400);
                ctx.fillStyle = gameWin ? "#2ecc71" : "#e74c3c";
                ctx.font = "bold 24px sans-serif"; ctx.textAlign = "center";
                ctx.fillText(gameWin ? "CIRCUIT REPAIRED!" : "SHORT CIRCUIT!", 200, 160);
                ctx.fillStyle = "white"; ctx.font = "18px sans-serif";
                ctx.fillText("SCORE: " + score, 200, 200);
                ctx.fillText("TAP D-PAD TO RESTART", 200, 240);
            }
            setTimeout(() => { update(); requestAnimationFrame(draw); }, 150);
        }
        draw();
    </script>
</body>
</html>
)rawliteral";

#endif