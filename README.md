# M5StickC Plus SubGHz Tool

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

## Wiring

Connect CC1101 to M5StickC Plus:

| CC1101 Pin | M5StickC Plus Pin | GPIO |
|------------|-------------------|------|
| GDO0       | G36               | 36   |
| SCK        | G13               | 13   |
| MISO       | G14               | 14   |
| MOSI       | G15               | 15   |
| CSN        | G5                | 5    |
| VCC        | 3.3V              | -    |
| GND        | GND               | -    |

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

## License

MIT License - Use at your own risk

## Credits

Built for M5StickC Plus using:
- M5StickCPlus library
- SmartRC-CC1101-Driver-Lib

Inspired by projects like Flipper Zero and Bruce, but focused specifically on SubGHz operations.
