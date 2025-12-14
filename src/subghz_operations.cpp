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
