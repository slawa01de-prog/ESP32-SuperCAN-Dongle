# Contributing

Contributions are welcome, especially for:

- passive CAN analysis
- MCP2515 compatibility
- ESP32-S3 board support
- filtering and visualization
- logging/export formats
- protocol correctness
- documentation

Please keep generic sniffer functionality separate from vehicle-specific profiles.

For signal definitions, indicate the evidence level:

- **verified** — confirmed from reliable documentation or repeated measurements
- **inferred** — correlation observed but not fully confirmed
- **unknown** — raw observation only

Avoid adding active ECU write/control behavior to the default firmware.
