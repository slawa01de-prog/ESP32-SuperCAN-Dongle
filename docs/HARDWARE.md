# Hardware

## Reference setup

The current development target is:

- ESP32-S3 development board
- MCP2515 CAN controller module
- CAN transceiver on the MCP2515 module
- 8 MHz MCP2515 oscillator in the current configuration

## Development pin mapping

| MCP2515 | ESP32-S3 |
|---|---:|
| CS | GPIO 10 |
| INT | GPIO 9 |
| SCK | GPIO 12 |
| MISO | GPIO 13 |
| MOSI | GPIO 11 |
| GND | GND |

Power the module according to the actual board/transceiver design. Do not assume every MCP2515 breakout accepts the same logic or supply voltage.

## CAN connection

The sniffer needs CAN-H, CAN-L and a common reference/ground where appropriate for the test setup. Do not add termination blindly: an in-vehicle CAN network is normally already terminated by the existing bus topology.

## Oscillator

The sketch currently uses:

```cpp
#define MCP_CLOCK MCP_8MHZ
```

If your board uses a 16 MHz oscillator, update the define to the matching library constant.

## Default mode

The firmware initializes the MCP2515 with:

```cpp
CAN0.setMode(MCP_LISTENONLY);
```

This is intentional. The public default firmware is for passive observation first.
