# Features Overview

## What Your SubGHz Tool Can Do

### 1. **Frequency Scanner** 📡
- Real-time RSSI monitoring on selected frequency
- Visual signal strength bars (10-segment display)
- Support for 315, 433.92, 868, 915 MHz
- Quick frequency switching with power button
- Continuous monitoring mode

### 2. **Spectrum Analyzer** 📊
- Live spectrum view ±5 MHz around center frequency
- 120-point resolution across frequency range
- Color-coded signal strength:
  - Green: Weak signals (-100 to -70 dBm)
  - Yellow: Medium signals (-70 to -50 dBm)
  - Red: Strong signals (-50 dBm and above)
- Auto-scaling for optimal visualization
- Useful for finding active frequencies

### 3. **Signal Listener** 👂
- Continuous receiver mode
- Automatic signal detection (threshold: -70 dBm)
- Signal counter tracks detected transmissions
- Data reception display (shows byte count)
- Can transition to Record mode with button press
- Perfect for monitoring RF environment

### 4. **Signal Recorder** 🔴
- Captures raw signal timings
- Records up to 512 timing samples
- Automatic trigger on signal detection
- 30-second recording timeout
- Works with OOK/ASK modulation
- Suitable for:
  - Garage door remotes
  - Car key fobs
  - Wireless doorbells
  - Weather sensors
  - Simple RF remotes

### 5. **Signal Replay** ▶️
- Retransmit recorded signals
- One-button transmission
- Visual feedback during TX
- Frequency-agile replay
- Can replay multiple times
- Useful for:
  - Testing captured signals
  - Analyzing protocols
  - Duplicating remotes
  - Educational purposes

## Technical Capabilities

### Hardware Integration
- **M5StickC Plus Display**: Full-featured TFT menu system
- **CC1101 Radio**: Professional SubGHz transceiver
- **Button Controls**: Intuitive 3-button navigation
- **Battery Powered**: Portable RF analysis tool

### RF Features
- **Modulation**: ASK/OOK (most common for SubGHz devices)
- **Frequency Range**: 300-928 MHz (depending on CC1101 variant)
- **RSSI Range**: -100 to -30 dBm display
- **Recording**: Microsecond timing precision
- **Transmission**: Raw timing-based replay

### Software Architecture
- Modular design (4 separate modules)
- Clean menu system with state management
- Non-blocking operations
- Efficient memory usage
- Real-time display updates

## Use Cases

### Home Automation Analysis
- Identify wireless device frequencies
- Analyze RF remote controls
- Study wireless sensor protocols
- Test wireless coverage

### Security Research
- RF device security assessment
- Protocol reverse engineering
- Signal strength mapping
- Frequency utilization studies

### DIY Projects
- Remote control duplication
- Custom RF remote creation
- Wireless sensor development
- RF protocol learning

### Educational
- Learn about RF communications
- Understand SubGHz protocols
- Practice signal analysis
- Explore wireless technologies

## Limitations

### What It CAN'T Do
- Decrypt encrypted signals
- Break rolling codes (modern car keys)
- Decode complex digital protocols automatically
- Work with very short signals (< 100µs transitions)
- Transmit on restricted frequencies legally

### Best Suited For
- Simple OOK/ASK protocols
- Fixed-code remotes
- Non-encrypted signals
- SubGHz ISM band devices
- Learning and experimentation

## Performance Notes

- **Scan Update Rate**: 10 Hz (100ms intervals)
- **Spectrum Scan**: ~500ms for full sweep
- **Listen Update Rate**: 20 Hz (50ms intervals)
- **Recording Timeout**: 30 seconds
- **Max Signal Length**: 512 timing samples
- **Memory Usage**: 7.7% RAM, 26.8% Flash

## Safety Features

- Automatic RX mode after TX
- Timeout on recording operations
- Visual feedback for all operations
- Safe power management
- Frequency validation

---

**Ready to capture and analyze SubGHz signals with your M5StickC Plus!** 🚀
