#include "menu_system.h"

MenuSystem::MenuSystem() {
    currentState = MENU_MAIN;
    currentMode = MODE_IDLE;
    menuSelection = 0;
    maxMenuItems = 6;  // Added Settings to menu
    moduleType = MODULE_2IN1;  // Default to 2-in-1 module
    settingsSelection = 0;
    freqIndex = 1; // Default to 433MHz
    
    frequencies[0] = 315.00;
    frequencies[1] = 433.92;
    frequencies[2] = 868.00;
    frequencies[3] = 915.00;
    
    lastUpdate = 0;
    buttonAPressed = false;
    buttonBPressed = false;
    buttonPowerPressed = false;
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
                currentState = MENU_SETTINGS;
                currentMode = MODE_IDLE;
                settingsSelection = 0;
                break;
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
    if (currentState == MENU_ABOUT) {
        currentState = MENU_SETTINGS;
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
        menuSelection = (menuSelection - 1 + maxMenuItems) % maxMenuItems;
    } else if (currentState == MENU_SETTINGS) {
        // Toggle between Module Type and About
        settingsSelection = (settingsSelection + 1) % 2;
    } else if (currentState != MENU_ABOUT) {
        // In other screens, cycle frequency
        freqIndex = (freqIndex + 1) % 4;
    }
}

void MenuSystem::drawMainMenu() {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.setTextColor(CYAN, BLACK);
    M5.Lcd.println("Seraph's");
    M5.Lcd.setCursor(10, 25);
    M5.Lcd.println("SubGHz Tool");
    
    M5.Lcd.setTextSize(1);
    int y = 48;
    const char* menuItems[] = {"Scan", "Spectrum", "Listen", "Record", "Replay", "Settings"};
    
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
        y += 15;
    }
    
    // Show current frequency
    M5.Lcd.setCursor(10, 115);
    M5.Lcd.setTextColor(YELLOW, BLACK);
    M5.Lcd.printf("Freq: %.2f MHz", frequencies[freqIndex]);
}

void MenuSystem::drawScanScreen() {
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.setTextColor(CYAN, BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.println("SCANNING");
    
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 40);
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.printf("Freq: %.2fMHz", frequencies[freqIndex]);
    
    M5.Lcd.setCursor(10, 110);
    M5.Lcd.setTextColor(YELLOW, BLACK);
    M5.Lcd.println("B: Back  PWR: Freq");
}

void MenuSystem::drawSpectrumScreen() {
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.setTextColor(CYAN, BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.println("SPECTRUM");
    
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 110);
    M5.Lcd.setTextColor(YELLOW, BLACK);
    M5.Lcd.println("B: Back  PWR: Freq");
}

void MenuSystem::drawListenScreen() {
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.setTextColor(CYAN, BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.println("LISTENING");
    
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 40);
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.printf("Freq: %.2fMHz", frequencies[freqIndex]);
    
    M5.Lcd.setCursor(10, 110);
    M5.Lcd.setTextColor(YELLOW, BLACK);
    M5.Lcd.println("A: Record B: Back");
}

void MenuSystem::drawRecordScreen() {
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.setTextColor(RED, BLACK);
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
}

void MenuSystem::drawReplayScreen() {
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.setTextColor(GREEN, BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.println("REPLAY");
    
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 40);
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.printf("Freq: %.2fMHz", frequencies[freqIndex]);
    
    M5.Lcd.setCursor(10, 110);
    M5.Lcd.setTextColor(YELLOW, BLACK);
    M5.Lcd.println("A: TX  B: Back");
}

void MenuSystem::drawSettingsScreen() {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.setTextColor(CYAN, BLACK);
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
    M5.Lcd.setTextColor(CYAN, BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.println("ABOUT");
    
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 40);
    M5.Lcd.setTextColor(GREEN, BLACK);
    M5.Lcd.println("Seraph's SubGHz Tool");
    
    M5.Lcd.setCursor(10, 60);
    M5.Lcd.setTextColor(YELLOW, BLACK);
    M5.Lcd.println("Version 0.01");
    
    M5.Lcd.setCursor(10, 75);
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.println("Created for Dad");
    
    M5.Lcd.setCursor(10, 120);
    M5.Lcd.setTextColor(YELLOW, BLACK);
    M5.Lcd.println("B: Back");
}
