#include "subghz_operations.h"
#include <M5StickCPlus.h>

SubGhzOperations::SubGhzOperations(CC1101Interface* radio, MenuSystem* menu) {
    cc1101 = radio;
    menuSystem = menu;
    lastRSSI = -100;
    lastScanUpdate = 0;
    lastSpectrumUpdate = 0;
    lastListenUpdate = 0;
    signalCount = 0;
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
}

void SubGhzOperations::update() {
    OperationMode mode = menuSystem->getMode();
    
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
        
        // Display RSSI
        M5.Lcd.fillRect(10, 60, 220, 40, BLACK);
        M5.Lcd.setCursor(10, 60);
        M5.Lcd.setTextSize(2);
        M5.Lcd.setTextColor(GREEN, BLACK);
        M5.Lcd.printf("RSSI: %d dBm", rssi);
        
        // Display signal strength bar
        displaySignalStrength(rssi);
        
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
    // Clear spectrum area
    M5.Lcd.fillRect(0, 35, 240, 70, BLACK);
    
    // Find min and max for scaling
    int minRSSI = 0;
    int maxRSSI = -100;
    for (int i = 0; i < SPECTRUM_POINTS; i++) {
        if (spectrumData[i] > maxRSSI) maxRSSI = spectrumData[i];
        if (spectrumData[i] < minRSSI) minRSSI = spectrumData[i];
    }
    
    // Draw spectrum bars
    int barWidth = 240 / SPECTRUM_POINTS;
    if (barWidth < 1) barWidth = 1;
    
    for (int i = 0; i < SPECTRUM_POINTS; i++) {
        int barHeight = map(spectrumData[i], -100, -30, 0, 60);
        if (barHeight < 0) barHeight = 0;
        if (barHeight > 60) barHeight = 60;
        
        uint16_t color = GREEN;
        if (spectrumData[i] > -50) color = RED;
        else if (spectrumData[i] > -70) color = YELLOW;
        
        M5.Lcd.fillRect(i * barWidth, 105 - barHeight, barWidth - 1, barHeight, color);
    }
    
    // Display center frequency
    M5.Lcd.setCursor(10, 35);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.printf("Center: %.2f MHz", menuSystem->getSelectedFrequency());
    
    M5.Lcd.setCursor(10, 50);
    M5.Lcd.printf("Range: +/- 5 MHz");
}

void SubGhzOperations::updateListen() {
    if (millis() - lastListenUpdate > 50) {
        // Set frequency and RX mode
        cc1101->setFrequency(menuSystem->getSelectedFrequency());
        cc1101->setRxMode();
        int rssi = cc1101->getRSSI();
        
        // Display RSSI
        M5.Lcd.fillRect(10, 60, 220, 50, BLACK);
        M5.Lcd.setCursor(10, 60);
        M5.Lcd.setTextSize(1);
        M5.Lcd.setTextColor(WHITE, BLACK);
        M5.Lcd.printf("RSSI: %d dBm", rssi);
        
        // Check for real signal detection
        if (cc1101->signalDetected()) {
            signalCount++;
            M5.Lcd.setCursor(10, 75);
            M5.Lcd.setTextColor(GREEN, BLACK);
            M5.Lcd.printf("Signals: %d", signalCount);
            
            // Try to receive data
            int len = cc1101->receiveData(rxBuffer, sizeof(rxBuffer));
            if (len > 0) {
                M5.Lcd.setCursor(10, 90);
                M5.Lcd.printf("RX: %d bytes", len);
            }
        }
        
        displaySignalStrength(rssi);
        
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

void SubGhzOperations::displaySignalStrength(int rssi) {
    // Draw signal strength bars
    int barCount = mapRSSIToBar(rssi);
    int barX = 10;
    int barY = 90;
    int barWidth = 10;
    int barSpacing = 3;
    
    M5.Lcd.fillRect(barX, barY, 240 - 20, 20, BLACK);
    
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
