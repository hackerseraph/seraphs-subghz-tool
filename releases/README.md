# Firmware Releases

## v0.01 - Initial Release

**File:** `seraphs-subghz-tool-v0.01.bin`

### Features
- Full SubGHz RF tool for M5StickC Plus with CC1101
- 5 operational modes: Scan, Spectrum, Listen, Record, Replay
- Settings menu with module type selection (2-in-1 vs Standard)
- Support for M5Stack 2-in-1 NRF24/CC1101 module
- About screen with version info

### How to Flash

#### Method 1: M5Burner (Easiest - All Platforms)
1. Download [M5Burner](https://docs.m5stack.com/en/download) for Windows/Mac/Linux
2. Connect M5StickC Plus via USB
3. Open M5Burner and select your device
4. Click "Custom" and select `seraphs-subghz-tool-v0.01.bin`
5. Click "Burn" to flash

#### Method 2: ESP Flash Download Tool (Windows)
1. Download [ESP Flash Download Tool](https://www.espressif.com/en/support/download/other-tools)
2. Extract and run `flash_download_tool_x.x.x.exe`
3. Select **ESP32** chip type
4. Configuration:
   - Add `seraphs-subghz-tool-v0.01.bin` at address `0x10000` ✓
   - SPI Speed: `40MHz`
   - SPI Mode: `DIO`
   - COM Port: Select your device (usually COM3 or higher)
   - Baud Rate: `921600` or `1500000`
5. Click **START** to flash
6. Wait for "FINISH" message

#### Method 3: esptool.py (Linux/Mac/Windows)
```bash
# Install esptool first: pip install esptool
esptool.py --chip esp32 --port COM3 --baud 1500000 \
  write_flash -z 0x10000 seraphs-subghz-tool-v0.01.bin

# Linux/Mac use /dev/ttyUSB0 instead of COM3
```

#### Method 3: PlatformIO (From Source)
```bash
git clone https://github.com/hackerseraph/seraphs-subghz-tool.git
cd seraphs-subghz-tool
pio run --target upload
```

### Hardware Required
- M5StickC Plus
- M5Stack StickC Plus NRF24&CC1101 2-in-1 RF Module (or standard CC1101 - configure in Settings)

### First Boot
1. Device will show splash screen: "Seraph's SubGHz Tool"
2. CC1101 module initialization (should show "MODULE DETECTED AND WORKING!")
3. Main menu appears with 6 options

### Configuration
- Navigate to Settings → Module Type to select your CC1101 module type
- Reboot after changing module type

### Troubleshooting Flash Issues

**Device not detected:**
- Windows: Install [CP210x USB driver](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)
- Ensure M5StickC Plus is powered on (long press power button)
- Try a different USB cable (data cable, not charge-only)

**Flash fails:**
- Hold power button during flash to keep device awake
- Lower baud rate to 115200 if higher speeds fail
- Use USB 2.0 port instead of USB 3.0

**Wrong firmware address:**
- ⚠️ **CRITICAL**: Use address `0x10000` NOT `0x0000`
- Address `0x0000` is for bootloader, will brick the device temporarily

### Notes
- Default module type: M5Stack 2-in-1
- Default frequency: 433.92 MHz
- Requires proper CC1101 wiring (see main README)
- Flash time: ~30 seconds at 1.5Mbps
