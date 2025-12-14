# M5Stack CC1101 Module Pin Configuration Guide

Your M5Stack StickC Plus NRF24&CC1101 2-in-1 module is currently not initializing. This could be due to incorrect pin mappings.

## Tested Pin Configurations

### Configuration 1: Standard ESP32 SPI (FAILED)
```
GDO0: GPIO36
SCK:  GPIO13
MISO: GPIO14
MOSI: GPIO15
CS:   GPIO5
```
**Result**: Init() hangs - doesn't work

### Configuration 2: Grove + HAT pins (FAILED)
```
GDO0: GPIO32
SCK:  GPIO0
MISO: GPIO36
MOSI: GPIO26
CS:   GPIO33
```
**Result**: Init() hangs - doesn't work

## Known Working Configurations (from Bruce firmware)

For the M5Stack official CC1101/NRF24 module, the pins are typically:
- Uses **I2C communication** via the Grove connector, NOT SPI!
- OR uses a different SPI configuration than standard

## Possible Solutions

### Solution 1: Check if your module uses I2C
The M5Stack RF module might communicate via I2C (GPIO 32/33) instead of SPI.
This would explain why the ELECHOUSE_CC1101 library (which expects SPI) doesn't work.

### Solution 2: Use Bruce firmware's CC1101 implementation
The Bruce firmware might use a completely different driver/library for this specific module.

### Solution 3: Verify hardware connection
1. Ensure module is firmly seated in both Grove and HAT connectors
2. Check if module LED lights up (power indicator)
3. Try reseating the module

## Next Steps

To identify the correct configuration, you can:

1. **Check Bruce firmware source code** for M5StickC Plus CC1101 pin definitions
2. **Look at module documentation** - does it say I2C or SPI?
3. **Try with Bruce firmware** - does CC1101 work there?
4. **Measure with multimeter** - which pins have activity during communication?

## Alternative: Use a different CC1101 module
If this specific M5Stack module uses proprietary communication, consider:
- Standard CC1101 breakout board with proper SPI pins
- Ensure it's wired to M5StickC Plus SPI pins (13/14/15/5)
