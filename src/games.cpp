#include "games.h"
#include "menu_system.h"

Games::Games() {
    menuSystem = nullptr;
}

void Games::setMenuSystem(MenuSystem* menu) {
    menuSystem = menu;
}

// ==================== DINO JUMP ====================
void Games::drawDino(int x, int y, bool jumping) {
    // Chrome T-Rex dino sprite (24x24)
    M5.Lcd.fillRect(x, y, 24, 24, BLACK);
    
    // Head (rectangular with notch for mouth)
    M5.Lcd.fillRect(x + 14, y, 10, 8, WHITE);
    M5.Lcd.fillRect(x + 14, y, 4, 2, BLACK);  // Mouth notch
    M5.Lcd.drawPixel(x + 20, y + 2, BLACK);  // Eye
    
    // Neck connecting to body
    M5.Lcd.fillRect(x + 12, y + 6, 4, 4, WHITE);
    
    // Main body (rounded rectangle)
    M5.Lcd.fillRect(x + 6, y + 8, 12, 8, WHITE);
    
    // Belly extension
    M5.Lcd.fillRect(x + 8, y + 14, 8, 4, WHITE);
    
    // Tiny arms
    M5.Lcd.fillRect(x + 12, y + 10, 2, 3, WHITE);
    
    // Tail
    M5.Lcd.fillRect(x + 2, y + 10, 6, 2, WHITE);
    M5.Lcd.fillRect(x, y + 12, 4, 2, WHITE);
    
    // Legs
    if (!jumping) {
        M5.Lcd.fillRect(x + 10, y + 18, 3, 6, WHITE);
        M5.Lcd.fillRect(x + 15, y + 18, 3, 6, WHITE);
    }
}

void Games::drawCactus(int x, int y) {
    // Larger cactus sprite (12x24)
    M5.Lcd.fillRect(x, y, 12, 24, BLACK);
    M5.Lcd.fillRect(x + 4, y + 4, 4, 20, GREEN);
    M5.Lcd.fillRect(x, y + 8, 3, 3, GREEN);
    M5.Lcd.fillRect(x + 9, y + 12, 3, 3, GREEN);
}

void Games::drawGround(int y) {
    M5.Lcd.drawFastHLine(0, y, 240, WHITE);
}

void Games::runDinoJump() {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(5, 5);
    M5.Lcd.setTextColor(YELLOW, BLACK);
    M5.Lcd.println("DINO JUMP - A:Jump B:Exit");
    
    int dinoX = 30;
    int groundY = 120;
    int dinoY = groundY - 24;  // Updated for 24px tall dino (20x24 sprite)
    int jumpVelocity = 0;
    bool jumping = false;
    const int jumpPower = -10;  // Stronger jump for bigger dino
    const int gravity = 1;
    
    int cactusX = 240;
    int cactusSpeed = 3;
    int score = 0;
    bool gameOver = false;
    
    drawGround(groundY);
    
    unsigned long lastUpdate = millis();
    
    while (!gameOver) {
        M5.update();
        
        // Check exit button
        if (M5.BtnB.wasPressed()) {
            M5.Lcd.fillScreen(BLACK);
            M5.Lcd.setTextSize(1);
            menuSystem->draw();
            return;
        }
        
        // Check jump button immediately
        if (M5.BtnA.wasPressed() && !jumping) {
            jumping = true;
            jumpVelocity = jumpPower;
        }
        
        if (millis() - lastUpdate < 30) continue;
        lastUpdate = millis();
        
        // Physics
        if (jumping) {
            dinoY += jumpVelocity;
            jumpVelocity += gravity;
            
            if (dinoY >= groundY - 24) {
                dinoY = groundY - 24;
                jumping = false;
                jumpVelocity = 0;
            }
        }
        
        // Move cactus
        cactusX -= cactusSpeed;
        if (cactusX < -10) {
            cactusX = 240;
            score++;
            if (score % 5 == 0) cactusSpeed++;
        }
        
        // Collision detection (adjusted for larger sprites)
        if (cactusX < dinoX + 24 && cactusX + 12 > dinoX &&
            dinoY + 24 > groundY - 24) {
            gameOver = true;
        }
        
        // Draw
        M5.Lcd.fillRect(0, 20, 240, 100, BLACK);
        drawDino(dinoX, dinoY, jumping);
        drawCactus(cactusX, groundY - 24);  // Cactus is 24px tall, sits on ground line
        drawGround(groundY);
        
        // Score
        M5.Lcd.fillRect(200, 5, 35, 10, BLACK);
        M5.Lcd.setCursor(200, 5);
        M5.Lcd.setTextColor(WHITE, BLACK);
        M5.Lcd.printf("%d", score);
    }
    
    // Game Over
    M5.Lcd.fillRect(50, 50, 140, 30, RED);
    M5.Lcd.setCursor(60, 60);
    M5.Lcd.setTextColor(WHITE, RED);
    M5.Lcd.setTextSize(2);
    M5.Lcd.printf("GAME OVER!");
    M5.Lcd.setCursor(70, 90);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(YELLOW, BLACK);
    M5.Lcd.printf("Score: %d", score);
    
    delay(3000);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(1);
    menuSystem->draw();
}

// ==================== ARKANOID ====================
void Games::drawPaddle(int x, int y, int width) {
    M5.Lcd.fillRect(x, y, width, 4, BLUE);
}

void Games::drawBall(int x, int y) {
    M5.Lcd.fillCircle(x, y, 2, WHITE);
}

void Games::drawBrick(int x, int y, uint16_t color) {
    M5.Lcd.fillRect(x, y, 28, 8, color);
    M5.Lcd.drawRect(x, y, 28, 8, BLACK);
}

void Games::runArkanoid() {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(5, 5);
    M5.Lcd.setTextColor(YELLOW, BLACK);
    M5.Lcd.println("ARKANOID - Tilt:Move B:Exit");
    
    int paddleX = 120;
    int paddleY = 125;
    int paddleWidth = 30;
    
    int ballX = 120;
    int ballY = 60;
    int ballVX = 1;  // Reduced from 2 to 1
    int ballVY = 1;  // Reduced from 2 to 1
    
    // Bricks (8 columns x 4 rows)
    const int brickCols = 8;
    const int brickRows = 4;
    Brick bricks[brickRows * brickCols];
    uint16_t colors[] = {RED, ORANGE, YELLOW, GREEN};
    
    for (int r = 0; r < brickRows; r++) {
        for (int c = 0; c < brickCols; c++) {
            int idx = r * brickCols + c;
            bricks[idx].x = c * 30;
            bricks[idx].y = 20 + r * 10;
            bricks[idx].active = true;
            bricks[idx].color = colors[r];
            drawBrick(bricks[idx].x, bricks[idx].y, bricks[idx].color);
        }
    }
    
    int score = 0;
    int activeBricks = brickRows * brickCols;
    bool gameOver = false;
    unsigned long lastUpdate = millis();
    
    while (!gameOver && activeBricks > 0) {
        M5.update();
        
        if (M5.BtnB.wasPressed()) {
            M5.Lcd.fillScreen(BLACK);
            M5.Lcd.setTextSize(1);
            menuSystem->draw();
            return;
        }
        
        if (millis() - lastUpdate < 30) continue;  // Increased from 20ms to 30ms
        lastUpdate = millis();
        
        // Move paddle with accelerometer (get fresh data each frame)
        float accX, accY, accZ;
        M5.Imu.getAccelData(&accX, &accY, &accZ);
        paddleX += (int)(accY * -25);  // Reduced sensitivity from -30 to -25
        paddleX = constrain(paddleX, 0, 240 - paddleWidth);
        
        // Clear old positions
        M5.Lcd.fillCircle(ballX, ballY, 3, BLACK);
        M5.Lcd.fillRect(0, paddleY, 240, 6, BLACK);
        
        // Move ball
        ballX += ballVX;
        ballY += ballVY;
        
        // Wall collision
        if (ballX <= 2 || ballX >= 238) ballVX = -ballVX;
        if (ballY <= 2) ballVY = -ballVY;
        
        // Paddle collision
        if (ballY >= paddleY - 2 && ballY <= paddleY + 2 &&
            ballX >= paddleX && ballX <= paddleX + paddleWidth) {
            ballVY = -abs(ballVY);
            ballVX += (ballX - (paddleX + paddleWidth/2)) / 5;
        }
        
        // Brick collision
        for (int i = 0; i < brickRows * brickCols; i++) {
            if (bricks[i].active) {
                if (ballX >= bricks[i].x && ballX <= bricks[i].x + 28 &&
                    ballY >= bricks[i].y && ballY <= bricks[i].y + 8) {
                    bricks[i].active = false;
                    M5.Lcd.fillRect(bricks[i].x, bricks[i].y, 28, 8, BLACK);
                    ballVY = -ballVY;
                    score += 10;
                }
            }
        }
        
        // Bottom out
        if (ballY > 135) gameOver = true;
        
        // Draw
        drawBall(ballX, ballY);
        drawPaddle(paddleX, paddleY, paddleWidth);
        
        // Score
        M5.Lcd.fillRect(200, 5, 35, 10, BLACK);
        M5.Lcd.setCursor(200, 5);
        M5.Lcd.setTextColor(WHITE, BLACK);
        M5.Lcd.printf("%d", score);
    }
    
    // Game Over
    M5.Lcd.fillRect(50, 50, 140, 30, RED);
    M5.Lcd.setCursor(60, 60);
    M5.Lcd.setTextColor(WHITE, RED);
    M5.Lcd.setTextSize(2);
    M5.Lcd.printf("GAME OVER!");
    M5.Lcd.setCursor(70, 90);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(YELLOW, BLACK);
    M5.Lcd.printf("Score: %d", score);
    
    delay(3000);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(1);
    menuSystem->draw();
}

// ==================== SPACE INVADERS ====================
void Games::drawAlien(int x, int y, int type) {
    // Simple alien sprite
    uint16_t color = (type == 0) ? RED : (type == 1) ? ORANGE : YELLOW;
    M5.Lcd.fillRect(x, y, 8, 6, color);
    M5.Lcd.drawPixel(x + 1, y, BLACK);
    M5.Lcd.drawPixel(x + 6, y, BLACK);
}

void Games::drawPlayer(int x, int y) {
    // Ship body (10x6)
    M5.Lcd.fillRect(x, y, 10, 6, GREEN);
    // Turret (2x2)
    M5.Lcd.fillRect(x + 4, y - 2, 2, 2, GREEN);
}

void Games::drawBullet(int x, int y, bool playerBullet) {
    M5.Lcd.fillRect(x, y, 2, 4, playerBullet ? CYAN : RED);
}

void Games::runSpaceInvaders() {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(5, 5);
    M5.Lcd.setTextColor(YELLOW, BLACK);
    M5.Lcd.println("SPACE INVADERS A:Shoot B:Exit");
    
    int playerX = 115;
    int playerY = 120;
    
    // Aliens (11 columns x 3 rows)
    const int alienCols = 11;
    const int alienRows = 3;
    Alien aliens[alienRows * alienCols];
    
    for (int r = 0; r < alienRows; r++) {
        for (int c = 0; c < alienCols; c++) {
            int idx = r * alienCols + c;
            aliens[idx].x = c * 20 + 5;
            aliens[idx].y = r * 12 + 20;
            aliens[idx].alive = true;
            aliens[idx].type = r;
            drawAlien(aliens[idx].x, aliens[idx].y, aliens[idx].type);
        }
    }
    
    int alienDirX = 1;
    int alienStepDown = 0;
    
    int bulletX = -10;
    int bulletY = -10;
    bool bulletActive = false;
    
    int score = 0;
    bool gameOver = false;
    unsigned long lastUpdate = millis();
    unsigned long lastAlienMove = millis();
    
    while (!gameOver) {
        M5.update();
        
        if (M5.BtnB.wasPressed()) {
            M5.Lcd.fillScreen(BLACK);
            M5.Lcd.setTextSize(1);
            menuSystem->draw();
            return;
        }
        
        // Check shoot button immediately (before frame delay)
        if (M5.BtnA.wasPressed() && !bulletActive) {
            bulletActive = true;
            bulletX = playerX + 4;
            bulletY = playerY - 4;
        }
        
        if (millis() - lastUpdate < 20) continue;
        lastUpdate = millis();
        
        // Move player with accelerometer
        float accX, accY, accZ;
        M5.Imu.getAccelData(&accX, &accY, &accZ);
        // Clear old player sprite completely (including the turret)
        M5.Lcd.fillRect(playerX, playerY - 2, 10, 8, BLACK);
        playerX += (int)(accY * -20);  // Use accY for left/right tilt, increased sensitivity
        playerX = constrain(playerX, 0, 230);
        drawPlayer(playerX, playerY);
        
        // Move bullet
        if (bulletActive) {
            M5.Lcd.fillRect(bulletX, bulletY, 2, 4, BLACK);
            bulletY -= 4;
            if (bulletY < 0) {
                bulletActive = false;
            } else {
                drawBullet(bulletX, bulletY, true);
                
                // Check alien collision
                for (int i = 0; i < alienRows * alienCols; i++) {
                    if (aliens[i].alive) {
                        if (bulletX >= aliens[i].x && bulletX <= aliens[i].x + 8 &&
                            bulletY >= aliens[i].y && bulletY <= aliens[i].y + 6) {
                            aliens[i].alive = false;
                            M5.Lcd.fillRect(aliens[i].x, aliens[i].y, 8, 6, BLACK);
                            bulletActive = false;
                            score += (3 - aliens[i].type) * 10;
                            break;
                        }
                    }
                }
            }
        }
        
        // Move aliens
        if (millis() - lastAlienMove > 500) {
            lastAlienMove = millis();
            
            bool hitEdge = false;
            for (int i = 0; i < alienRows * alienCols; i++) {
                if (aliens[i].alive) {
                    M5.Lcd.fillRect(aliens[i].x, aliens[i].y, 8, 6, BLACK);
                    aliens[i].x += alienDirX * 5;
                    if (aliens[i].x <= 0 || aliens[i].x >= 232) hitEdge = true;
                }
            }
            
            if (hitEdge) {
                alienDirX = -alienDirX;
                for (int i = 0; i < alienRows * alienCols; i++) {
                    if (aliens[i].alive) {
                        aliens[i].y += 8;
                        if (aliens[i].y >= playerY - 10) gameOver = true;
                    }
                }
            }
            
            // Redraw aliens
            for (int i = 0; i < alienRows * alienCols; i++) {
                if (aliens[i].alive) {
                    drawAlien(aliens[i].x, aliens[i].y, aliens[i].type);
                }
            }
        }
        
        // Score
        M5.Lcd.fillRect(200, 5, 35, 10, BLACK);
        M5.Lcd.setCursor(200, 5);
        M5.Lcd.setTextColor(WHITE, BLACK);
        M5.Lcd.printf("%d", score);
        
        // Check win
        bool anyAlive = false;
        for (int i = 0; i < alienRows * alienCols; i++) {
            if (aliens[i].alive) {
                anyAlive = true;
                break;
            }
        }
        if (!anyAlive) gameOver = true;
    }
    
    // Game Over / Win
    M5.Lcd.fillRect(30, 50, 180, 40, score > 200 ? GREEN : RED);
    M5.Lcd.setCursor(50, 55);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(2);
    M5.Lcd.printf(score > 200 ? "YOU WIN!" : "GAME OVER!");
    M5.Lcd.setCursor(70, 75);
    M5.Lcd.setTextSize(1);
    M5.Lcd.printf("Score: %d", score);
    
    delay(3000);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(1);
    menuSystem->draw();
}
