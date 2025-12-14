#include "cc1101_interface.h"
#include <SPI.h>

CC1101Interface::CC1101Interface() {
    currentFrequency = FREQ_433_MHZ;
    initialized = false;
}

bool CC1101Interface::begin(ModuleType moduleType) {
    Serial.println("[CC1101] Initializing CC1101...");
    Serial.printf("  Pin config: SCK=%d, MISO=%d, MOSI=%d, CS=%d, GDO0=%d\n", 
                  CC1101_SCK, CC1101_MISO, CC1101_MOSI, CC1101_CS, CC1101_GDO0);
    
    // Initialize SPI with correct pins
    Serial.println("[CC1101] Setting up SPI...");
    SPI.begin(CC1101_SCK, CC1101_MISO, CC1101_MOSI, CC1101_CS);
    delay(100);
    
    // Set BeginEndLogic for M5Stack 2-in-1 module (from Bruce firmware)
    if (moduleType == MODULE_2IN1) {
        Serial.println("[CC1101] Setting BeginEndLogic(true) for M5Stack 2-in-1 module...");
        ELECHOUSE_cc1101.setBeginEndLogic(true);
    } else {
        Serial.println("[CC1101] Using standard CC1101 initialization...");
        ELECHOUSE_cc1101.setBeginEndLogic(false);
    }
    
    // Configure CC1101 library pins
    ELECHOUSE_cc1101.setSpiPin(CC1101_SCK, CC1101_MISO, CC1101_MOSI, CC1101_CS);
    delay(100);
    
    // Set GDO0 pin
    ELECHOUSE_cc1101.setGDO0(CC1101_GDO0);
    delay(100);
    
    // Initialize CC1101 (Bruce method with setBeginEndLogic)
    Serial.println("[CC1101] Calling Init() with Bruce initialization sequence...");
    ELECHOUSE_cc1101.Init();
    Serial.println("[CC1101] Init() complete!");
    
    // Verify connection (from Bruce firmware)
    if (ELECHOUSE_cc1101.getCC1101()) {
        Serial.println("[CC1101] *** MODULE DETECTED AND WORKING! ***");
        initialized = true;
        
        // Set default frequency
        setFrequency(433.92);
        
        return true;
    } else {
        Serial.println("[CC1101] ERROR: Module not responding after Init()");
        initialized = false;
        return false;
    }
}

void CC1101Interface::setFrequency(float freq) {
    currentFrequency = freq;
    ELECHOUSE_cc1101.setMHZ(freq);
}

float CC1101Interface::getFrequency() {
    return currentFrequency;
}

void CC1101Interface::startScan() {
    ELECHOUSE_cc1101.SetRx();
}

int CC1101Interface::getRSSI() {
    return ELECHOUSE_cc1101.getRssi();
}

void CC1101Interface::scanSpectrum(float startFreq, float endFreq, float step, int* rssiValues, int numPoints) {
    float freq = startFreq;
    for (int i = 0; i < numPoints && freq <= endFreq; i++) {
        setFrequency(freq);
        delay(10); // Allow settling time
        rssiValues[i] = getRSSI();
        freq += step;
    }
}

void CC1101Interface::setRxMode() {
    ELECHOUSE_cc1101.SetRx();
}

bool CC1101Interface::signalDetected() {
    int rssi = getRSSI();
    return rssi > -70; // Threshold for signal detection
}

int CC1101Interface::receiveData(byte* buffer, int maxLen) {
    if (ELECHOUSE_cc1101.CheckRxFifo(100)) {
        int len = ELECHOUSE_cc1101.ReceiveData(buffer);
        return (len > 0 && len <= maxLen) ? len : 0;
    }
    return 0;
}

void CC1101Interface::setTxMode() {
    ELECHOUSE_cc1101.SetTx();
}

void CC1101Interface::transmit(byte* data, int len) {
    ELECHOUSE_cc1101.SendData(data, len);
    delay(100);
    ELECHOUSE_cc1101.SetRx(); // Return to RX mode
}

bool CC1101Interface::recordSignal(int* timings, int maxSamples) {
    int sampleCount = 0;
    unsigned long lastTime = micros();
    unsigned long timeout = millis() + 5000; // 5 second timeout
    
    pinMode(CC1101_GDO0, INPUT);
    
    int lastState = digitalRead(CC1101_GDO0);
    
    while (sampleCount < maxSamples && millis() < timeout) {
        int state = digitalRead(CC1101_GDO0);
        
        if (state != lastState) {
            unsigned long now = micros();
            timings[sampleCount++] = now - lastTime;
            lastTime = now;
            lastState = state;
        }
        
        if (sampleCount > 0 && micros() - lastTime > 100000) {
            // 100ms of silence, signal complete
            break;
        }
    }
    
    return sampleCount > 10; // Need at least some samples
}

void CC1101Interface::replaySignal(int* timings, int numSamples) {
    pinMode(CC1101_GDO0, OUTPUT);
    setTxMode();
    
    int state = LOW;
    for (int i = 0; i < numSamples; i++) {
        digitalWrite(CC1101_GDO0, state);
        delayMicroseconds(timings[i]);
        state = !state;
    }
    
    pinMode(CC1101_GDO0, INPUT);
    setRxMode();
}

void CC1101Interface::setIdleMode() {
    ELECHOUSE_cc1101.SpiStrobe(0x36);  // SIDLE - Exit RX/TX, turn off frequency synthesizer
}

void CC1101Interface::setModulation(int mode) {
    // Set modulation: 0=2-FSK, 1=GFSK, 2=ASK/OOK, 3=4-FSK, 4=MSK
    ELECHOUSE_cc1101.setModulation(mode);
}
