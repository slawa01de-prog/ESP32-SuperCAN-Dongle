# Roadmap

## v0.1 — Passive sniffer foundation

- [x] ESP32-S3 + MCP2515 receive
- [x] Listen-only default
- [x] Wi-Fi web dashboard
- [x] FFat CSV logging
- [x] CSV download
- [x] 125/250/500 kbit/s selection

## v0.2 — Better analysis

- [ ] CAN-ID include/exclude filters
- [ ] Frame frequency statistics
- [ ] Changed-byte highlighting
- [ ] ID discovery view
- [ ] Capture start/stop sessions
- [ ] Bookmark interesting IDs
- [ ] Improved memory handling for long captures

## v0.3 — Decoder architecture

- [ ] Generic signal/profile interface
- [ ] DBC import research
- [ ] Vehicle profile loader
- [ ] Decoded signal dashboard
- [ ] Export capture metadata

## v0.4 — Protocol research

- [ ] ISO-TP receive/reassembly
- [ ] Read-only UDS exploration
- [ ] Separate ELM327 compatibility module
- [ ] Automated compatibility tests

## Later

- [ ] Optional desktop companion app
- [ ] WebSocket live stream
- [ ] PCAP-compatible export investigation
- [ ] Multiple CAN interfaces / native ESP32 TWAI variant
