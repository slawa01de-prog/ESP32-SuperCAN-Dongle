# Safety

CAN networks can contain safety-critical traffic. The default firmware is deliberately passive.

Recommended development practice:

- Begin in listen-only mode.
- Verify bitrate and wiring before capture.
- Do not alter vehicle wiring while powered unless the test setup is designed for it.
- Avoid adding termination to an existing vehicle network unless you understand the bus topology.
- Do not test active transmission on a moving vehicle.
- Do not use unvalidated ECU write/coding functions on safety-related controllers.
- Prefer bench setups for protocol development.

The repository's public default sketch contains no arbitrary CAN transmit feature.
