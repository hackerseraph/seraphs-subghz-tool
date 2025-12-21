#ifndef WIFI_AP_H
#define WIFI_AP_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "cc1101_interface.h"

// Forward declarations
class SubGhzOperations;
class MenuSystem;

class WiFiAP {
public:
    WiFiAP(SubGhzOperations* ops, MenuSystem* menu);
    void begin();
    void update();
    void stop();
    bool isActive();
    String getIPAddress();
    int getClientCount();
    
private:
    SubGhzOperations* operations;
    MenuSystem* menuSystem;
    WebServer* server;
    bool active;
    unsigned long lastStatusUpdate;
    
    // Recording state
    bool recordingActive;
    String recordedSignalData;
    
    // Web handlers
    void handleRoot();
    void handleListen();
    void handleRecord();
    void handleReplay();
    void handleStatus();
    void handleStop();
    void handleNotFound();
    
    // Helper functions
    String getWebPage();
    String getCurrentStatus();
};

#endif
