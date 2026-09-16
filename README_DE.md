# ESP32 SuperCAN Dongle

> 🚧 **WORK IN PROGRESS / EXPERIMENTELL**  
> Das Projekt befindet sich aktiv in Entwicklung. Die Standard-Firmware arbeitet bewusst **passiv / Listen-Only** und ist für CAN-Analyse, Logging und Reverse Engineering gedacht.

ESP32 SuperCAN Dongle ist eine ESP32-S3 + MCP2515 CAN-Forschungsplattform mit WLAN-Weboberfläche, Zeitstempel-Logging und modularer Struktur für spätere Protokoll- und Fahrzeugprofile.

## Aktuelle Funktionen

- Passiver CAN-2.0-Sniffer über MCP2515
- Listen-Only als Standard
- 500 / 250 / 125 kbit/s über WebGUI wählbar
- WLAN-Client mit Access-Point-Fallback
- Live-Web-Dashboard
- Anzeige von CAN-ID, DLC, Datenbytes und Zeitstempel
- Anzeige der letzten Frames im RAM
- FFat-CSV-Logging und Download
- Fahrzeugunabhängige Grundstruktur
- Profilordner für spätere Decoder

## Status

| Bereich | Status |
|---|---|
| ESP32-S3 + MCP2515 Empfang | ✅ Im Entwicklungsbetrieb funktionsfähig |
| Listen-Only Sniffer | ✅ Implementiert |
| Web-Dashboard | ✅ Implementiert / wird erweitert |
| FFat CSV Logging | ✅ Implementiert |
| Bitratenwechsel | 🧪 Experimentell |
| CAN-Filter | 🧪 Basis / Ausbau geplant |
| ELM327-Emulation | 🚧 Separater Entwicklungszweig |
| ISO-TP / UDS | 📋 Geplant |
| DBC-/Profildecoder | 📋 Geplant |
| Fahrzeugprofile | 📋 Geplant |

## Hardware

Aktuelle Entwicklungsbelegung:

| Signal | ESP32-S3 GPIO |
|---|---:|
| MCP2515 CS | 10 |
| MCP2515 INT | 9 |
| SPI SCK | 12 |
| SPI MISO | 13 |
| SPI MOSI | 11 |

Die Firmware ist aktuell für einen **8-MHz-MCP2515** eingestellt.

Weitere Hinweise: [docs/HARDWARE.md](docs/HARDWARE.md)

## Start

1. `firmware/SuperCAN_Sniffer/SuperCAN_Sniffer.ino` in der Arduino IDE öffnen.
2. ESP32-Boardpaket und eine zu `mcp_can.h` kompatible MCP2515-Bibliothek installieren.
3. WLAN-Daten oben im Sketch eintragen.
4. GPIOs und Quarzfrequenz des MCP2515 prüfen.
5. Auf den ESP32-S3 laden.
6. Die im seriellen Monitor angezeigte IP im Browser öffnen.
7. Zuerst ausschließlich passiv im Listen-Only-Modus testen.

## Fahrzeugprofile

Die Entwicklung entstand teilweise aus Renault-ZOE-CAN-Tests. Das Repository selbst bleibt aber absichtlich allgemein. Fahrzeugspezifische Decoder und Forschungsnotizen kommen getrennt unter `profiles/` bzw. `examples/`.

## Sicherheit

Die öffentliche Standard-Firmware sendet keine CAN-Frames. Aktive Diagnose- oder Schreibfunktionen werden erst nach separater Validierung aufgenommen. Siehe [docs/SAFETY.md](docs/SAFETY.md).
