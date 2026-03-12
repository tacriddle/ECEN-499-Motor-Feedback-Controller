#ifndef FLAPPY_H
#define FLAPPY_H

const char flappy_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <style>
        body { margin: 0; overflow: hidden; background: #70c5ce; font-family: 'Segoe UI', sans-serif; touch-action: manipulation; }
        .score-container { 
            display: flex; justify-content: space-between; 
            padding: 10px 20px; color: white; font-weight: bold; font-size: 1.2rem;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
        }
        canvas { display: block; margin: auto; background: #70c5ce; border: 4px solid #333; border-radius: 8px; width: 95vw; max-width: 500px; height: auto; }
    </style>
</head>
<body>
    <div class="score-container">
        <div>HI: <span id="hi-val">0</span></div>
        <div>SCORE: <span id="score-val">0</span></div>
    </div>
    <canvas id="birdCanvas" width="400" height="600"></canvas>
    <script>
        const canvas = document.getElementById('birdCanvas');
        const ctx = canvas.getContext('2d');
        
        let bird = { y: 300, v: 0, gravity: 0.25, jump: -5.0 };
        let pipes = [];
        let score = 0;
        let highScore = 0;
        let frame = 0;
        let isDead = false;

        // Fetch Initial High Score from ESP32
        fetch('/getHighScore').then(r => r.text()).then(d => {
            highScore = parseInt(d);
            document.getElementById('hi-val').innerText = highScore;
        });

        function resetGame() {
            // Save score to ESP32 if it's a new record
            if (score > highScore) {
                highScore = score;
                document.getElementById('hi-val').innerText = highScore;
                fetch('/setHighScore?score=' + score);
            }
            // Reset state
            bird.y = 300; // Back to center
            bird.v = 0;
            pipes = [];
            score = 0;
            frame = 0;
            document.getElementById('score-val').innerText = "0";
            isDead = false;
        }

        function draw() {
            ctx.fillStyle = "#70c5ce";
            ctx.fillRect(0, 0, canvas.width, canvas.height);
            
            if (!isDead) {
                bird.v += bird.gravity;
                bird.y += bird.v;

                if (frame % 90 === 0) {
                    let gap = 160;
                    let top = Math.random() * (canvas.height - gap - 100) + 50;
                    pipes.push({ x: canvas.width, top: top, bottom: top + gap, passed: false });
                }

                pipes.forEach((p, i) => {
                    p.x -= 3;
                    ctx.fillStyle = "yellow";
                    ctx.fillRect(p.x, 0, 50, p.top);
                    ctx.fillRect(p.x, p.bottom, 50, canvas.height);
                    
                    // Collision
                    if (50 + 25 > p.x && 50 < p.x + 50 && (bird.y < p.top || bird.y + 25 > p.bottom)) isDead = true;
                    
                    // Score counting
                    if (!p.passed && p.x < 50) {
                        p.passed = true;
                        score++;
                        document.getElementById('score-val').innerText = score;
                    }
                    if (p.x < -50) pipes.splice(i, 1);
                });

                if (bird.y > canvas.height || bird.y < 0) isDead = true;
                frame++;
            } else {
                ctx.fillStyle = "rgba(0,0,0,0.5)";
                ctx.fillRect(0,0,canvas.width, canvas.height);
                ctx.fillStyle = "white";
                ctx.font = "bold 30px sans-serif";
                ctx.textAlign = "center";
                ctx.fillText("CRASHED!", 200, 280);
                ctx.font = "20px sans-serif";
                ctx.fillText("Tap to Restart", 200, 320);
            }

            ctx.fillStyle = "#e74c3c"; // Bird Red
            ctx.fillRect(50, bird.y, 25, 25);
            requestAnimationFrame(draw);
        }

        function handleInput(e) {
            if (isDead) resetGame();
            else bird.v = bird.jump;
            if(e) e.preventDefault();
        }

        window.onkeydown = (e) => { if(e.code === "Space") handleInput(e); };
        canvas.ontouchstart = (e) => handleInput(e);
        draw();
    </script>
</body>
</html>
)rawliteral";

#endif