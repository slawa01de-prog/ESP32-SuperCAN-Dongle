# ESP32 SuperCAN Dongle

> 🚧 **WORK IN PROGRESS / EXPERIMENTAL**  
> This repository is under active development. The default firmware is intentionally **passive / listen-only** and is intended for CAN-bus observation, logging and reverse-engineering workflows.

ESP32 SuperCAN Dongle is an ESP32-S3 + MCP2515 based CAN-bus research platform with a Wi-Fi dashboard, timestamped logging and a modular path toward protocol/profile support.

## Current features

- Passive CAN 2.0 sniffing via MCP2515
- Listen-only mode by default
- 500 / 250 / 125 kbit/s selectable from the web UI
- Wi-Fi station mode with AP fallback
- Browser-based live dashboard
- CAN ID, DLC, payload and timestamp display
- In-memory recent-frame view
- FFat CSV logging and download
- Clear/export controls for captured data
- Vehicle-agnostic architecture
- Placeholder profile area for future vehicle-specific decoders

## Development status

| Area | Status |
|---|---|
| ESP32-S3 + MCP2515 receive | ✅ Working in development |
| Listen-only sniffer | ✅ Implemented |
| Web dashboard | ✅ Implemented / evolving |
| FFat CSV logging | ✅ Implemented |
| Bitrate switching | 🧪 Experimental |
| CAN filtering | 🧪 Basic UI / planned expansion |
| ELM327 emulation | 🚧 Separate experimental branch/workstream |
| ISO-TP / UDS | 📋 Planned |
| DBC/profile decoder | 📋 Planned |
| Vehicle-specific profiles | 📋 Planned |

## Hardware reference

Current development mapping:

| Signal | ESP32-S3 GPIO |
|---|---:|
| MCP2515 CS | 10 |
| MCP2515 INT | 9 |
| SPI SCK | 12 |
| SPI MISO | 13 |
| SPI MOSI | 11 |

The current firmware assumes an **8 MHz MCP2515 crystal**. Change `MCP_CLOCK` if your module uses a different oscillator.

See [docs/HARDWARE.md](docs/HARDWARE.md) before connecting hardware.

## Repository layout

```text
ESP32-SuperCAN-Dongle/
├── firmware/
│   └── SuperCAN_Sniffer/
│       └── SuperCAN_Sniffer.ino
├── profiles/
│   └── README.md
├── examples/
│   └── renault-zoe/
│       └── README.md
├── docs/
│   ├── HARDWARE.md
│   ├── DEVELOPMENT_STATUS.md
│   ├── ELM327.md
│   ├── ROADMAP.md
│   └── SAFETY.md
├── CHANGELOG.md
├── CONTRIBUTING.md
├── LICENSE
└── README.md
```

## Quick start

1. Open `firmware/SuperCAN_Sniffer/SuperCAN_Sniffer.ino` in Arduino IDE.
2. Install ESP32 board support and an MCP2515 library compatible with `mcp_can.h`.
3. Enter your Wi-Fi credentials at the top of the sketch.
4. Verify the GPIO mapping and MCP2515 oscillator.
5. Flash the ESP32-S3.
6. Open the IP printed in Serial Monitor.
7. Start in **listen-only** mode and verify frames before doing any further protocol work.

## Why vehicle-agnostic?

The original development work included experiments on a Renault ZOE, but the core project is intentionally generic. Vehicle-specific parsing belongs in separate profiles/examples so the sniffer remains reusable for other vehicles, ECUs and CAN projects.

## Safety

Do not use experimental transmit/write functions on a moving vehicle or a safety-critical network. The public default firmware deliberately runs in passive listen-only mode. See [docs/SAFETY.md](docs/SAFETY.md).

## License

MIT License — see [LICENSE](LICENSE).
