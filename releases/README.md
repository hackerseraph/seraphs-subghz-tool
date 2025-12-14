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

#### Method 1: M5Burner (Easiest)
1. Download [M5Burner](https://docs.m5stack.com/en/download)
2. Connect M5StickC Plus via USB
3. Open M5Burner and select your device
4. Click "Custom" and select `seraphs-subghz-tool-v0.01.bin`
5. Click "Burn" to flash

#### Method 2: esptool.py
```bash
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 1500000 \
  write_flash -z 0x10000 seraphs-subghz-tool-v0.01.bin
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

### Notes
- Default module type: M5Stack 2-in-1
- Default frequency: 433.92 MHz
- Requires proper CC1101 wiring (see main README)
