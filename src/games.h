#ifndef GAMES_H
#define GAMES_H

#include <Arduino.h>
#include <M5StickCPlus.h>

class MenuSystem;

class Games {
public:
    Games();
    void setMenuSystem(MenuSystem* menu);
    
    // Game launchers
    void runDinoJump();
    void runArkanoid();
    void runSpaceInvaders();
    
private:
    MenuSystem* menuSystem;
    
    // Dino Jump
    void drawDino(int x, int y, bool jumping);
    void drawCactus(int x, int y);
    void drawGround(int y);
    
    // Arkanoid
    struct Brick {
        int x, y;
        bool active;
        uint16_t color;
    };
    void drawPaddle(int x, int y, int width);
    void drawBall(int x, int y);
    void drawBrick(int x, int y, uint16_t color);
    
    // Space Invaders
    struct Alien {
        int x, y;
        bool alive;
        int type;
    };
    void drawAlien(int x, int y, int type);
    void drawPlayer(int x, int y);
    void drawBullet(int x, int y, bool playerBullet);
};

#endif
