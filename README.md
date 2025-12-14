# M5StickC Plus SubGHz Tool

**Version 0.2.2**

A dedicated SubGHz tool for the M5StickC Plus with CC1101 radio module. Simple, focused interface for scanning, analyzing, listening, recording, and replaying SubGHz signals.

## Features

- **Scan Mode**: Monitor RSSI on selected frequency with signal strength visualization
- **Spectrum Analyzer**: View frequency spectrum (±5 MHz around center frequency)
- **Listen Mode**: Receive and decode signals in real-time
- **Record Mode**: Capture signal timings for later replay
- **Replay Mode**: Retransmit recorded signals

## Hardware Requirements

- M5StickC Plus
- CC1101 SPI module
- Antenna appropriate for your target frequency

## Hardware Compatibility

This tool is specifically designed for the **M5Stack StickC Plus NRF24&CC1101 2-in-1 RF Module**. 

### Important: M5Stack 2-in-1 Module Support

The M5Stack 2-in-1 module requires special initialization that differs from standard CC1101 breakout boards. This project uses a modified CC1101 driver from [Bruce firmware's fork](https://github.com/bmorcelli/SmartRC-CC1101-Driver-Lib) which includes the `setBeginEndLogic()` method required for proper initialization of the M5Stack module.

**Key differences:**
- Standard CC1101 libraries (LSatan original) will **hang on Init()** with the M5Stack 2-in-1 module
- Bruce's fork adds `setBeginEndLogic(true)` which enables proper SPI begin/end logic needed for the StickC Plus shared pin configuration
- The tool includes a settings menu to toggle between 2-in-1 and standard module types

### Pin Configuration

For M5Stack StickC Plus NRF24/CC1101 2-in-1 Module (as documented in [Bruce firmware wiki](https://github.com/BruceDevices/firmware/wiki/CC1101)):

| CC1101 Pin | M5StickC Plus Pin | GPIO | Notes |
|------------|-------------------|------|-------|
| GDO0       | -                 | 25   | -     |
| CS         | -                 | 26   | Chip Select |
| MOSI       | Grove SCL         | 32   | Shared with Grove |
| SCK        | -                 | 0    | -     |
| MISO       | Grove SDA         | 33   | Shared with Grove |
| VCC        | 3.3V              | -    | -     |
| GND        | GND               | -    | -     |

## Button Controls

### Main Menu
- **Button A**: Select menu item / Enter
- **Button B**: Navigate down through menu
- **Power Button**: Navigate up through menu / Cycle frequency

### In Operation Modes
- **Button A**: Action (Record in Listen mode, Transmit in Replay mode)
- **Button B**: Back to main menu
- **Power Button**: Cycle through frequencies (315, 433.92, 868, 915 MHz)

## Frequency Support

Pre-configured frequencies:
- 315.00 MHz (US garage doors, car keys)
- 433.92 MHz (EU common ISM band)
- 868.00 MHz (EU ISM band)
- 915.00 MHz (US ISM band)

Press Power button while in any mode to cycle through frequencies.

## Usage Guide

### Scanning
1. Select "Scan" from main menu
2. Monitor RSSI value and signal strength bars
3. Press Power button to change frequency
4. Press B to return to menu

### Spectrum Analysis
1. Select "Spectrum" from main menu
2. View real-time spectrum display
3. Center frequency ±5 MHz range shown
4. Green bars = weak, Yellow = medium, Red = strong signals
5. Press B to return to menu

### Listening/Receiving
1. Select "Listen" from main menu
2. Device enters continuous receive mode
3. Signal counter increments when signals detected
4. Press A to switch to Record mode
5. Press B to return to menu

### Recording Signals
1. Select "Record" from main menu (or press A in Listen mode)
2. Wait for signal to be detected (30 second timeout)
3. Signal is automatically captured when detected
4. After successful recording, automatically switches to Replay mode
5. Press B to cancel and return to menu

### Replaying Signals
1. Select "Replay" from main menu (or record a signal first)
2. Press A to transmit the recorded signal
3. "TRANSMITTING!" appears during transmission
4. Press B to return to menu

## Building and Flashing

```bash
# Navigate to project directory
cd /home/marcus/Documents/PlatformIO/Projects/m5-rf-tools

# Build project
pio run

# Upload to M5StickC Plus
pio run --target upload

# Monitor serial output
pio device monitor
```

## Project Structure

```
m5-rf-tools/
├── src/
│   ├── main.cpp                 # Main application entry point
│   ├── cc1101_interface.h/cpp   # CC1101 radio driver
│   ├── menu_system.h/cpp        # Menu and display management
│   └── subghz_operations.h/cpp  # SubGHz operation modes
├── platformio.ini               # PlatformIO configuration
└── README.md                    # This file
```

## Signal Recording Format

Signals are recorded as timing arrays capturing the duration of HIGH and LOW states. Maximum 512 samples per recording. Suitable for simple OOK/ASK protocols like:
- Garage door openers
- Car key fobs
- Weather sensors
- Simple remote controls

## Safety and Legal Notice

⚠️ **IMPORTANT**: 
- Only use on frequencies legal in your jurisdiction
- Do not transmit on licensed frequencies
- Respect privacy - don't capture or replay signals you don't own
- Some signals may be encrypted or rolling code (won't work)
- Use responsibly and ethically

## Troubleshooting

### CC1101 Failed Error
- Check wiring connections
- Verify 3.3V power supply
- Ensure CC1101 module is functional
- Check SPI pin definitions match your setup

### No Signals Detected
- Ensure antenna is connected
- Verify frequency is correct for target device
- Check that signal strength is sufficient (RSSI > -70 dBm)
- Try different frequencies

### Recording Not Working
- Signal must be present for at least 100ms
- Signal must have clear HIGH/LOW transitions
- Try moving closer to signal source

## Changelog

### v0.2.2 (December 2025)
**Major Display System Overhaul:**
- **Fixed critical display race condition**: Corrected draw order in main loop - menu.draw() now executes before operations.update(), ensuring static UI elements render before dynamic content
- **Resolved 433MHz RSSI display issue**: RSSI text now properly appears at all frequencies in Listen mode
- **Fixed mode switching contamination**: Implemented early return pattern with screenValid flags to prevent UI elements from bleeding between modes (Scan/Spectrum/Listen/Record/Replay)
- **Eliminated screen flashing**: Removed periodic 200ms redraws, now only redraws on state changes or explicit needsRedraw flag
- **Fixed text overlap in Scan mode**: Adjusted vertical spacing between frequency text (y=32), RSSI display (y=42), and waveform visualization (y=52)
- **Corrected Record/Replay mode draw logic**: Fixed inverted draw conditions that prevented screens from loading when switching modes multiple times
- **Improved CC1101 stability**: Added 10ms stabilization delay after setRxMode() and frequency tracking reset on mode entry

**Technical improvements:**
- All operational mode draw functions now use consistent lastDrawnState pattern
- Static screenValid variables properly invalidate when leaving modes
- Frequency changes update text without clearing dynamic content
- Clean mode transitions with no UI remnants

### v0.2.1 (December 2025)
**Bug Fixes:**
- Fixed excessive screen flashing on home screen by implementing smart redraw logic
- Fixed menu remnants showing at bottom when switching to operational modes
- Static menus now only redraw on button press instead of every 200ms
- Active modes maintain real-time updates while idle screens stay stable

### v0.01 (Initial Release)
- Initial implementation with all 5 operational modes
- M5Stack 2-in-1 module support via Bruce's CC1101 driver
- Settings menu with module type selection
- About screen with version information

## License

MIT License - Use at your own risk

## Technical Details

### CC1101 Driver

This project uses a modified CC1101 driver from [Bruce firmware](https://github.com/BruceDevices/firmware). The standard ELECHOUSE CC1101 library does not support the M5Stack 2-in-1 module due to differences in SPI initialization requirements.

**Solution discovered by analyzing Bruce firmware source code:**
1. Bruce uses a [forked version](https://github.com/bmorcelli/SmartRC-CC1101-Driver-Lib) of the ELECHOUSE library
2. The fork adds `setBeginEndLogic(bool)` method for proper SPI management
3. For M5StickC Plus, `setBeginEndLogic(true)` must be called before `Init()`
4. This enables proper SPI begin/end calls needed for the shared pin configuration

Reference: [Bruce firmware rf_utils.cpp](https://github.com/BruceDevices/firmware/blob/main/src/modules/rf/rf_utils.cpp) lines 116-120

## Credits

Built for M5StickC Plus using:
- [M5StickCPlus library](https://github.com/m5stack/M5StickC-Plus)
- [Bruce's CC1101 driver fork](https://github.com/bmorcelli/SmartRC-CC1101-Driver-Lib) - Critical for M5Stack 2-in-1 module support

Special thanks to:
- [Bruce firmware project](https://github.com/BruceDevices/firmware) for CC1101 2-in-1 module initialization solution
- Original [ELECHOUSE CC1101 library](https://github.com/LSatan/SmartRC-CC1101-Driver-Lib)

Inspired by projects like Flipper Zero and Bruce, but focused specifically on SubGHz operations.
