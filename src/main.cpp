/**
 * Seraph's SubGHz Tool
 * Dedicated tool for CC1101 SubGHz operations
 * 
 * Features:
 * - Scan: Monitor RSSI on selected frequency
 * - Spectrum: View spectrum analyzer
 * - Listen: Receive and decode signals
 * - Record: Capture signal timings
 * - Replay: Retransmit recorded signals
 * 
 * Hardware:
 * - M5StickC Plus
 * - CC1101 module connected via SPI
 * 
 * Button Controls:
 * - Button A: Select/Enter/Transmit
 * - Button B: Back/Cancel/Navigate Down
 * - Power Button: Navigate Up/Cycle Frequency
 */

#include <M5StickCPlus.h>
#include "cc1101_interface.h"
#include "menu_system.h"
#include "subghz_operations.h"

// Global objects
CC1101Interface cc1101;
MenuSystem menu;
SubGhzOperations operations(&cc1101, &menu);

// Include orca image data
#include "orca_m5.h"

// Splash screen
void showSplashScreen() {
    // Draw orca image (240x135, 24-bit RGB)
    // Convert 24-bit RGB to RGB565 on the fly
    uint16_t* lineBuffer = (uint16_t*)malloc(240 * sizeof(uint16_t));
    
    for (int y = 0; y < 135; y++) {
        for (int x = 0; x < 240; x++) {
            int idx = (y * 240 + x) * 3;
            uint8_t r = image_data_orca_m5[idx];
            uint8_t g = image_data_orca_m5[idx + 1];
            uint8_t b = image_data_orca_m5[idx + 2];
            
            // Convert to RGB565
            lineBuffer[x] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
        }
        M5.Lcd.pushImage(0, y, 240, 1, lineBuffer);
    }
    
    free(lineBuffer);
    delay(2000);
}

void setup() {
    // Initialize M5StickC Plus
    M5.begin();
    M5.Lcd.setRotation(3);
    
    // Initialize IMU (accelerometer/gyroscope)
    M5.Imu.Init();
    
    // Initialize Serial for debugging
    Serial.begin(115200);
    delay(500);
    Serial.println("\n\n=== SubGHz Tool Starting ===");
    
    // Show splash screen
    showSplashScreen();
    
    // Initialize menu system
    menu.begin();
    
    Serial.println("\n[MAIN] Menu initialized");
    
    // Initialize CC1101
    M5.Lcd.fillRect(30, 100, 180, 30, BLACK);
    M5.Lcd.setCursor(30, 100);
    M5.Lcd.setTextColor(YELLOW, BLACK);
    M5.Lcd.setTextSize(1);
    M5.Lcd.println("Init CC1101...");
    
    Serial.println("[MAIN] Initializing CC1101...");
    bool cc1101_ok = cc1101.begin(menu.getModuleType());
    
    if (cc1101_ok) {
        M5.Lcd.fillRect(30, 100, 180, 20, BLACK);
        M5.Lcd.setCursor(30, 100);
        M5.Lcd.setTextColor(GREEN, BLACK);
        M5.Lcd.println("CC1101 Ready!");
        Serial.println("[MAIN] CC1101 initialized successfully!");
        cc1101.setFrequency(menu.getSelectedFrequency());
        delay(1500);
    } else {
        M5.Lcd.fillRect(30, 100, 180, 30, BLACK);
        M5.Lcd.setCursor(30, 100);
        M5.Lcd.setTextColor(RED, BLACK);
        M5.Lcd.println("CC1101 FAILED!");
        M5.Lcd.setCursor(30, 115);
        M5.Lcd.println("Check module");
        Serial.println("[MAIN] CC1101 init failed - check wiring");
        delay(3000);
    }
    
    // Initialize operations
    operations.begin();
    
    // Connect operations to menu for hacks
    menu.setOperations(&operations);
    
    // Seed random for dummy data
    randomSeed(analogRead(0));
    
    Serial.println("[MAIN] Setup complete!");
    
    // Clear screen for menu
    M5.Lcd.fillScreen(BLACK);
}

void loop() {
    // Update menu system (handles button inputs)
    menu.update();
    
    // Draw menu/screen only when needed
    static MenuState lastState = MENU_MAIN;
    MenuState currentState = menu.getState();
    
    bool shouldDraw = false;
    
    // Only redraw if state changed or menu explicitly flagged redraw needed
    if (currentState != lastState || menu.needsRedraw()) {
        shouldDraw = true;
        lastState = currentState;
    }
    
    if (shouldDraw) {
        menu.draw();
        menu.clearRedrawFlag();
    }
    
    // Update operations based on current mode (AFTER menu draw so operations draw on top)
    operations.update();
    
    delay(20);
}
