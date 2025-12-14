#include "menu_system.h"
#include "subghz_operations.h"

MenuSystem::MenuSystem() {
    currentState = MENU_MAIN;
    currentMode = MODE_IDLE;
    menuSelection = 0;
    maxMenuItems = 7;  // Scan, Spectrum, Listen, Record, Replay, Hacks, Settings
    moduleType = MODULE_2IN1;  // Default to 2-in-1 module
    settingsSelection = 0;
    hacksSelection = 0;
    operations = nullptr;
    freqIndex = 1; // Default to 433MHz
    
    frequencies[0] = 315.00;
    frequencies[1] = 433.92;
    frequencies[2] = 868.00;
    frequencies[3] = 915.00;
    
    lastUpdate = 0;
    buttonAPressed = false;
    buttonBPressed = false;
    buttonPowerPressed = false;
    redrawNeeded = true;  // Initial draw needed
}

void MenuSystem::begin() {
    M5.begin();
    M5.Lcd.setRotation(3);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(WHITE, BLACK);
}

void MenuSystem::update() {
    M5.update();
    handleButtons();
}

void MenuSystem::draw() {
    switch (currentState) {
        case MENU_MAIN:
            drawMainMenu();
            break;
        case MENU_SCAN:
            drawScanScreen();
            break;
        case MENU_SPECTRUM:
            drawSpectrumScreen();
            break;
        case MENU_LISTEN:
            drawListenScreen();
            break;
        case MENU_RECORD:
            drawRecordScreen();
            break;
        case MENU_REPLAY:
            drawReplayScreen();
            break;
        case MENU_HACKS:
            drawHacksScreen();
            break;
        case MENU_SETTINGS:
            drawSettingsScreen();
            break;
        case MENU_ABOUT:
            drawAboutScreen();
            break;
    }
}

MenuState MenuSystem::getState() {
    return currentState;
}

OperationMode MenuSystem::getMode() {
    return currentMode;
}

void MenuSystem::setMode(OperationMode mode) {
    currentMode = mode;
}

int MenuSystem::getSelectedFreqIndex() {
    return freqIndex;
}

float MenuSystem::getSelectedFrequency() {
    return frequencies[freqIndex];
}

ModuleType MenuSystem::getModuleType() {
    return moduleType;
}

bool MenuSystem::needsRedraw() {
    return redrawNeeded;
}

void MenuSystem::clearRedrawFlag() {
    redrawNeeded = false;
}

void MenuSystem::setOperations(SubGhzOperations* ops) {
    operations = ops;
}

void MenuSystem::handleButtons() {
    if (M5.BtnA.wasPressed()) {
        buttonA();
    }
    
    if (M5.BtnB.wasPressed()) {
        buttonB();
    }
    
    if (M5.Axp.GetBtnPress()) {
        buttonPower();
    }
}

void MenuSystem::buttonA() {
    // Select/Enter button
    redrawNeeded = true;  // Button pressed, need redraw
    if (currentState == MENU_MAIN) {
        switch (menuSelection) {
            case 0:
                currentState = MENU_SCAN;
                currentMode = MODE_SCANNING;
                break;
            case 1:
                currentState = MENU_SPECTRUM;
                currentMode = MODE_SPECTRUM;
                break;
            case 2:
                currentState = MENU_LISTEN;
                currentMode = MODE_LISTENING;
                break;
            case 3:
                currentState = MENU_RECORD;
                currentMode = MODE_RECORDING;
                break;
            case 4:
                currentState = MENU_REPLAY;
                currentMode = MODE_REPLAYING;
                break;
            case 5:
                currentState = MENU_HACKS;
                currentMode = MODE_IDLE;
                hacksSelection = 0;
                break;
            case 6:
                currentState = MENU_SETTINGS;
                currentMode = MODE_IDLE;
                settingsSelection = 0;
                break;
        }
    } else if (currentState == MENU_HACKS) {
        if (operations != nullptr) {
            if (hacksSelection == 0) {
                // Tesla Charge Port - trigger the hack
                operations->runTeslaChargePortHack();
                redrawNeeded = true;
            } else if (hacksSelection == 1) {
                // Garage Door Brute Force
                operations->runGarageDoorBruteForce();
                redrawNeeded = true;
            } else if (hacksSelection == 2) {
                // Hampton Bay Fan Brute Force
                operations->runHamptonBayFanBruteForce();
                redrawNeeded = true;
            } else if (hacksSelection == 3) {
                // TV-B-Gone
                operations->runTVBGone();
                redrawNeeded = true;
            }
        }
    } else if (currentState == MENU_SETTINGS) {
        if (settingsSelection == 0) {
            // Toggle module type
            moduleType = (moduleType == MODULE_2IN1) ? MODULE_STANDARD : MODULE_2IN1;
        } else if (settingsSelection == 1) {
            // Enter About screen
            currentState = MENU_ABOUT;
        }
    }
}

void MenuSystem::buttonB() {
    // Back/Cancel button
    redrawNeeded = true;  // Button pressed, need redraw
    if (currentState == MENU_ABOUT) {
        currentState = MENU_SETTINGS;
    } else if (currentState == MENU_HACKS) {
        // Go back to main menu
        currentState = MENU_MAIN;
        currentMode = MODE_IDLE;
    } else if (currentState == MENU_SETTINGS) {
        currentState = MENU_MAIN;
        currentMode = MODE_IDLE;
    } else if (currentState != MENU_MAIN) {
        currentState = MENU_MAIN;
        currentMode = MODE_IDLE;
    } else {
        // Navigate menu down
        menuSelection = (menuSelection + 1) % maxMenuItems;
    }
}

void MenuSystem::buttonPower() {
    // Power button - navigate menu up or change frequency
    if (currentState == MENU_MAIN) {
        redrawNeeded = true;  // Menu navigation needs redraw
        menuSelection = (menuSelection - 1 + maxMenuItems) % maxMenuItems;
    } else if (currentState == MENU_HACKS) {
        redrawNeeded = true;  // Hacks navigation needs redraw
        hacksSelection = (hacksSelection - 1 + 4) % 4;  // Navigate hacks menu (4 items)
    } else if (currentState == MENU_SETTINGS) {
        redrawNeeded = true;  // Settings navigation needs redraw
        // Toggle between Module Type and About
        settingsSelection = (settingsSelection + 1) % 2;
    } else if (currentState != MENU_ABOUT) {
        // In operational screens, just cycle frequency (no full redraw needed)
        // Operations handle their own display updates
        freqIndex = (freqIndex + 1) % 4;
    }
}

void MenuSystem::drawMainMenu() {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(10, 5);
    M5.Lcd.setTextColor(ORANGE, BLACK);
    M5.Lcd.println("Seraph's");
    M5.Lcd.setCursor(10, 20);
    M5.Lcd.println("SubGHz Tool");
    
    M5.Lcd.setTextSize(1);
    int y = 40;
    const char* menuItems[] = {"Scan", "Spectrum", "Listen", "Record", "Replay", "Hacks", "Settings"};
    
    for (int i = 0; i < maxMenuItems; i++) {
        M5.Lcd.setCursor(10, y);
        if (i == menuSelection) {
            M5.Lcd.setTextColor(BLACK, GREEN);
            M5.Lcd.print(">");
        } else {
            M5.Lcd.setTextColor(WHITE, BLACK);
            M5.Lcd.print(" ");
        }
        M5.Lcd.print(menuItems[i]);
        y += 13;
    }
    
    // Show current frequency (lower right)
    M5.Lcd.setCursor(135, 120);
    M5.Lcd.setTextColor(YELLOW, BLACK);
    M5.Lcd.printf("%.2fMHz", frequencies[freqIndex]);
}

void MenuSystem::drawScanScreen() {
    // Only draw static elements - waveform is drawn by drawRSSIWaveform()
    static MenuState lastDrawnState = MENU_ABOUT;
    static int lastFreqIndex = -1;
    static bool screenValid = false;
    
    // Only execute if we're actually in this mode
    if (currentState != MENU_SCAN) {
        screenValid = false;
        return;
    }
    
    // Redraw if we just entered this screen or frequency changed
    if (!screenValid || currentState != lastDrawnState || freqIndex != lastFreqIndex) {
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(10, 10);
        M5.Lcd.setTextColor(ORANGE, BLACK);
        M5.Lcd.setTextSize(2);
        M5.Lcd.println("SCANNING");
        
        M5.Lcd.setTextSize(1);
        M5.Lcd.setCursor(10, 32);
        M5.Lcd.setTextColor(WHITE, BLACK);
        M5.Lcd.printf("Freq: %.2fMHz", frequencies[freqIndex]);
        
        M5.Lcd.setCursor(10, 110);
        M5.Lcd.setTextColor(YELLOW, BLACK);
        M5.Lcd.println("B: Back  PWR: Freq");
        
        lastDrawnState = currentState;
        lastFreqIndex = freqIndex;
        screenValid = true;
    }
}

void MenuSystem::drawSpectrumScreen() {
    // Only draw static elements - spectrum bars are drawn by updateSpectrum()
    static MenuState lastDrawnState = MENU_ABOUT;
    static int lastFreqIndex = -1;
    static bool screenValid = false;
    
    // Only execute if we're actually in this mode
    if (currentState != MENU_SPECTRUM) {
        screenValid = false;
        return;
    }
    
    // Redraw if we just entered this screen or frequency changed
    if (!screenValid || currentState != lastDrawnState || freqIndex != lastFreqIndex) {
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(10, 10);
        M5.Lcd.setTextColor(ORANGE, BLACK);
        M5.Lcd.setTextSize(2);
        M5.Lcd.println("SPECTRUM");
        
        M5.Lcd.setTextSize(1);
        M5.Lcd.setCursor(10, 35);
        M5.Lcd.setTextColor(WHITE, BLACK);
        M5.Lcd.printf("Center: %.2fMHz", frequencies[freqIndex]);
        
        float startFreq = frequencies[freqIndex] - 5.0;
        float endFreq = frequencies[freqIndex] + 5.0;
        M5.Lcd.setCursor(10, 45);
        M5.Lcd.setTextColor(DARKGREY, BLACK);
        M5.Lcd.printf("%.2f - %.2fMHz", startFreq, endFreq);
        
        M5.Lcd.setCursor(10, 120);
        M5.Lcd.setTextColor(YELLOW, BLACK);
        M5.Lcd.println("B: Back  PWR: Freq");
        
        lastDrawnState = currentState;
        lastFreqIndex = freqIndex;
        screenValid = true;
    }
}

void MenuSystem::drawListenScreen() {
    // Only draw static elements - RSSI/signals updated by updateListen()
    static MenuState lastDrawnState = MENU_ABOUT;
    static int lastFreqIndex = -1;
    static bool screenValid = false;
    
    // Only execute if we're actually in this mode
    if (currentState != MENU_LISTEN) {
        screenValid = false;
        return;
    }
    
    // Full redraw only when entering this screen
    if (!screenValid || currentState != lastDrawnState) {
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(10, 10);
        M5.Lcd.setTextColor(ORANGE, BLACK);
        M5.Lcd.setTextSize(2);
        M5.Lcd.println("LISTENING");
        
        M5.Lcd.setTextSize(1);
        M5.Lcd.setCursor(10, 40);
        M5.Lcd.setTextColor(WHITE, BLACK);
        M5.Lcd.printf("Freq: %.2fMHz", frequencies[freqIndex]);
        
        // Draw placeholder signal bars (will be updated by updateListen)
        int barX = 10;
        int barY = 93;
        int barWidth = 10;
        int barSpacing = 3;
        for (int i = 0; i < 10; i++) {
            M5.Lcd.fillRect(barX + (i * (barWidth + barSpacing)), barY, barWidth, 15, DARKGREY);
        }
        
        M5.Lcd.setCursor(10, 110);
        M5.Lcd.setTextColor(YELLOW, BLACK);
        M5.Lcd.println("B: Back  PWR: Freq");
        
        lastDrawnState = currentState;
        lastFreqIndex = freqIndex;
        screenValid = true;
    } 
    // Update frequency display without clearing screen
    else if (freqIndex != lastFreqIndex) {
        M5.Lcd.fillRect(10, 40, 220, 10, BLACK);
        M5.Lcd.setCursor(10, 40);
        M5.Lcd.setTextSize(1);
        M5.Lcd.setTextColor(WHITE, BLACK);
        M5.Lcd.printf("Freq: %.2fMHz", frequencies[freqIndex]);
        lastFreqIndex = freqIndex;
    }
}

void MenuSystem::drawRecordScreen() {
    static MenuState lastDrawnState = MENU_ABOUT;
    static int lastFreqIndex = -1;
    static bool screenValid = false;
    
    // Only execute if we're actually in this mode
    if (currentState != MENU_RECORD) {
        screenValid = false;
        return;
    }
    
    // Full redraw only when entering this screen or frequency changed
    if (!screenValid || currentState != lastDrawnState || freqIndex != lastFreqIndex) {
        M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.setTextColor(ORANGE, BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.println("RECORDING");
    
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 40);
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.printf("Freq: %.2fMHz", frequencies[freqIndex]);
    
    M5.Lcd.setCursor(10, 60);
    M5.Lcd.println("Waiting for signal...");
    
        M5.Lcd.setCursor(10, 110);
        M5.Lcd.setTextColor(YELLOW, BLACK);
        M5.Lcd.println("B: Cancel");
        
        lastDrawnState = currentState;
        lastFreqIndex = freqIndex;
        screenValid = true;
    }
}

void MenuSystem::drawReplayScreen() {
    static MenuState lastDrawnState = MENU_ABOUT;
    static int lastFreqIndex = -1;
    static bool screenValid = false;
    
    // Only execute if we're actually in this mode
    if (currentState != MENU_REPLAY) {
        screenValid = false;
        return;
    }
    
    // Full redraw only when entering this screen or frequency changed
    if (!screenValid || currentState != lastDrawnState || freqIndex != lastFreqIndex) {
        M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.setTextColor(ORANGE, BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.println("REPLAY");
    
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 40);
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.printf("Freq: %.2fMHz", frequencies[freqIndex]);
    
        M5.Lcd.setCursor(10, 110);
        M5.Lcd.setTextColor(YELLOW, BLACK);
        M5.Lcd.println("A: TX  B: Back");
        
        lastDrawnState = currentState;
        lastFreqIndex = freqIndex;
        screenValid = true;
    }
}

void MenuSystem::drawHacksScreen() {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.setTextColor(RED, BLACK);
    M5.Lcd.println("HACKS");
    
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 40);
    M5.Lcd.setTextColor(YELLOW, BLACK);
    M5.Lcd.println("WARNING: Use responsibly!");
    
    M5.Lcd.setCursor(10, 60);
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.println("Select hack:");
    
    // Hack menu items
    int y = 75;
    const char* hackItems[] = {"Tesla Charge Port", "Garage Brute Force", "Hampton Bay Fan", "TV-B-Gone"};
    
    for (int i = 0; i < 4; i++) {
        M5.Lcd.setCursor(10, y);
        if (i == hacksSelection) {
            M5.Lcd.setTextColor(BLACK, GREEN);
            M5.Lcd.print(">");
        } else {
            M5.Lcd.setTextColor(WHITE, BLACK);
            M5.Lcd.print(" ");
        }
        M5.Lcd.print(hackItems[i]);
        y += 12;
    }
    
    M5.Lcd.setCursor(10, 120);
    M5.Lcd.setTextColor(YELLOW, BLACK);
    M5.Lcd.println("A: Run  B: Back");
}

void MenuSystem::drawSettingsScreen() {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.setTextColor(ORANGE, BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.println("SETTINGS");
    
    M5.Lcd.setTextSize(1);
    int y = 35;
    
    // Module Type option
    M5.Lcd.setCursor(10, y);
    if (settingsSelection == 0) {
        M5.Lcd.setTextColor(BLACK, GREEN);
        M5.Lcd.print(">Module Type");
    } else {
        M5.Lcd.setTextColor(WHITE, BLACK);
        M5.Lcd.print(" Module Type");
    }
    
    // Show current module type
    M5.Lcd.setCursor(20, y + 12);
    M5.Lcd.setTextColor(YELLOW, BLACK);
    if (moduleType == MODULE_2IN1) {
        M5.Lcd.println("M5Stack 2-in-1");
    } else {
        M5.Lcd.println("Standard CC1101");
    }
    
    y += 27;
    
    // About option
    M5.Lcd.setCursor(10, y);
    if (settingsSelection == 1) {
        M5.Lcd.setTextColor(BLACK, GREEN);
        M5.Lcd.println(">About");
    } else {
        M5.Lcd.setTextColor(WHITE, BLACK);
        M5.Lcd.println(" About");
    }
    
    M5.Lcd.setCursor(10, 105);
    M5.Lcd.setTextColor(ORANGE, BLACK);
    M5.Lcd.println("*Reboot to apply");
    
    M5.Lcd.setCursor(10, 120);
    M5.Lcd.setTextColor(YELLOW, BLACK);
    M5.Lcd.println("A: Select  B: Back");
}

void MenuSystem::drawAboutScreen() {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.setTextColor(ORANGE, BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.println("ABOUT");
    
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 40);
    M5.Lcd.setTextColor(GREEN, BLACK);
    M5.Lcd.println("Seraph's SubGHz Tool");
    
    M5.Lcd.setCursor(10, 60);
    M5.Lcd.setTextColor(YELLOW, BLACK);
    M5.Lcd.println("Version 0.2.9");
    
    M5.Lcd.setCursor(10, 75);
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.println("Created for Dad");
    
    M5.Lcd.setCursor(10, 120);
    M5.Lcd.setTextColor(YELLOW, BLACK);
    M5.Lcd.println("B: Back");
}
