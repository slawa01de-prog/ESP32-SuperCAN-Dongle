# Development status

This project is intentionally marked **experimental**.

## Stable enough for development use

- Wi-Fi connection with AP fallback
- MCP2515 initialization
- Passive CAN receive
- Browser status page
- Recent-frame table
- FFat CSV logging
- CSV download

## Still being validated

- Behavior across different ESP32-S3 boards
- Different MCP2515 crystal frequencies
- Long-duration FFat logging
- Browser performance on very busy CAN buses
- Bitrate switching without reboot
- Filtering large traffic volumes

## Not part of the safe default firmware

- Arbitrary CAN transmission
- ECU coding/writes
- DTC clearing
- Security access
- Active vehicle control

These areas may be explored later in isolated development branches after bench validation.
