# CC1101 Wiring Guide for M5StickC Plus

## Pin Connections

```
M5StickC Plus          CC1101 Module
━━━━━━━━━━━━━━━       ━━━━━━━━━━━━━━
                      
G36 (GPIO36)    ────→  GDO0 (Data)
G13 (GPIO13)    ────→  SCK  (Clock)
G14 (GPIO14)    ←────  MISO (Master In)
G15 (GPIO15)    ────→  MOSI (Master Out)
G5  (GPIO5)     ────→  CSN  (Chip Select)
3.3V            ────→  VCC  (Power)
GND             ────→  GND  (Ground)
```

## Visual Diagram

```
     M5StickC Plus                    CC1101
    ┌─────────────┐               ┌──────────┐
    │             │               │          │
    │    ┌────┐   │               │  ┌────┐  │
    │    │LCD │   │               │  │RF  │  │
    │    └────┘   │               │  │Chip│  │
    │             │               │  └────┘  │
    │   [A] [B]   │               │          │
    │             │               │   ANT    │
    └──────┬──────┘               └────┬─────┘
           │                           │
           │  G36 ─────────────────────┤ GDO0
           │  G13 ─────────────────────┤ SCK
           │  G14 ─────────────────────┤ MISO
           │  G15 ─────────────────────┤ MOSI
           │  G5  ─────────────────────┤ CSN
           │  3.3V ────────────────────┤ VCC
           │  GND ─────────────────────┤ GND
           │
```

## Notes

1. **Power**: CC1101 runs on 3.3V - never use 5V!
2. **Antenna**: Required for proper operation. Match to your frequency:
   - 315 MHz: ~23.8 cm wire
   - 433 MHz: ~17.3 cm wire
   - 868 MHz: ~8.6 cm wire
   - 915 MHz: ~8.2 cm wire

3. **GDO0**: Used for signal detection and raw data transmission/reception
4. **SPI Pins**: Standard SPI interface for configuration and data transfer

## Testing Connection

After wiring, the device will show "CC1101 FAILED!" if connection is unsuccessful.
If successful, you'll see "Ready!" on the display.

## Common Issues

- **No connection**: Check solder joints and wire continuity
- **Intermittent**: Ensure wires are properly secured
- **CC1101 not responding**: Verify 3.3V power, check for shorts
