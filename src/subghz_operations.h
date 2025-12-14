#ifndef SUBGHZ_OPERATIONS_H
#define SUBGHZ_OPERATIONS_H

#include <Arduino.h>
#include "cc1101_interface.h"
#include "menu_system.h"

#define SPECTRUM_POINTS 120  // Number of points for spectrum display
#define MAX_RECORDING_SAMPLES 512

class SubGhzOperations {
public:
    SubGhzOperations(CC1101Interface* radio, MenuSystem* menu);
    void begin();
    void update();
    void runTeslaChargePortHack();
    void runGarageDoorBruteForce();
    void runHamptonBayFanBruteForce();
    
private:
    CC1101Interface* cc1101;
    MenuSystem* menuSystem;
    OperationMode lastMode;
    
    // Scan mode
    void updateScan();
    int lastRSSI;
    int lastDisplayedRSSI;
    int scanCounter;
    int rssiHistory[120];  // History buffer for waveform
    int historyIndex;
    unsigned long lastScanUpdate;
    void drawRSSIWaveform();
    
    // Spectrum analyzer
    void updateSpectrum();
    int spectrumData[SPECTRUM_POINTS];
    unsigned long lastSpectrumUpdate;
    void drawSpectrum();
    
    // Listen mode
    void updateListen();
    byte rxBuffer[64];
    unsigned long lastListenUpdate;
    int signalCount;
    bool forceListenDraw;
    float lastListenFreq;
    
    // Recording
    void updateRecord();
    int recordedTimings[MAX_RECORDING_SAMPLES];
    int recordedSampleCount;
    bool hasRecording;
    unsigned long recordStartTime;
    
    // Replay
    void updateReplay();
    bool isTransmitting;
    
    // Helper functions
    void displayRSSI(int rssi, int x, int y);
    void displaySignalStrength(int rssi);
    int mapRSSIToBar(int rssi);
    
    // Garage door brute force helpers
    void bruteForceGarageCodes(int bits);
    void sendGarageSync();
    void sendGarageBit(int bit);
    
    void bruteForceHamptonBay();
    void sendHamptonBayCommand(int fanId, int command);
};

#endif
