#ifndef MENU_SYSTEM_H
#define MENU_SYSTEM_H

#include <Arduino.h>
#include <M5StickCPlus.h>
#include "cc1101_interface.h"

enum MenuState {
    MENU_MAIN,
    MENU_SCAN,
    MENU_SPECTRUM,
    MENU_LISTEN,
    MENU_RECORD,
    MENU_REPLAY,
    MENU_SETTINGS,
    MENU_ABOUT
};

enum OperationMode {
    MODE_IDLE,
    MODE_SCANNING,
    MODE_SPECTRUM,
    MODE_LISTENING,
    MODE_RECORDING,
    MODE_REPLAYING
};

class MenuSystem {
public:
    MenuSystem();
    void begin();
    void update();
    void draw();
    
    MenuState getState();
    OperationMode getMode();
    void setMode(OperationMode mode);
    
    int getSelectedFreqIndex();
    float getSelectedFrequency();
    ModuleType getModuleType();
    
private:
    MenuState currentState;
    OperationMode currentMode;
    int menuSelection;
    int maxMenuItems;
    ModuleType moduleType;
    int settingsSelection;
    
    // Frequency selection
    int freqIndex;
    float frequencies[4];
    
    // Button handling
    void handleButtons();
    void buttonA();  // Select/Enter
    void buttonB();  // Back/Cancel
    void buttonPower();  // Up/Down navigation (short press up, long press down)
    
    // Drawing functions
    void drawMainMenu();
    void drawScanScreen();
    void drawSpectrumScreen();
    void drawListenScreen();
    void drawRecordScreen();
    void drawReplayScreen();
    void drawSettingsScreen();
    void drawAboutScreen();
    
    // State tracking
    unsigned long lastUpdate;
    bool buttonAPressed;
    bool buttonBPressed;
    bool buttonPowerPressed;
};

#endif
