#ifndef CC1101_INTERFACE_H
#define CC1101_INTERFACE_H

#include <Arduino.h>
#include <ELECHOUSE_CC1101_SRC_DRV.h>

enum ModuleType {
    MODULE_2IN1,      // M5Stack 2-in-1 NRF24/CC1101 module
    MODULE_STANDARD   // Standard CC1101 breakout board
};

// CC1101 SPI pins for M5Stack StickC Plus NRF24&CC1101 2-in-1 Module  
// Correct pinout from Bruce firmware wiki
// https://github.com/BruceDevices/firmware/wiki/CC1101
#define CC1101_GDO0     25  // GPIO25
#define CC1101_CS       26  // GPIO26 (SS/CSN - chip select)
#define CC1101_MOSI     32  // GPIO32 (Grove SCL)
#define CC1101_SCK      0   // GPIO0
#define CC1101_MISO     33  // GPIO33 (Grove SDA)

// Common SubGHz frequencies
#define FREQ_315_MHZ    315.00
#define FREQ_433_MHZ    433.92
#define FREQ_868_MHZ    868.00
#define FREQ_915_MHZ    915.00

// Signal buffer size
#define MAX_SIGNAL_LENGTH 512

class CC1101Interface {
public:
    CC1101Interface();
    bool begin(ModuleType moduleType = MODULE_2IN1);
    void setFrequency(float freq);
    float getFrequency();
    
    // Scanning
    void startScan();
    int getRSSI();
    
    // Spectrum analyzer
    void scanSpectrum(float startFreq, float endFreq, float step, int* rssiValues, int numPoints);
    
    // Receiver
    void setRxMode();
    bool signalDetected();
    int receiveData(byte* buffer, int maxLen);
    
    // Transmitter
    void setTxMode();
    void transmit(byte* data, int len);
    
    // Signal recording
    bool recordSignal(int* timings, int maxSamples);
    void replaySignal(int* timings, int numSamples);
    
private:
    float currentFrequency;
    bool initialized;
};

#endif
