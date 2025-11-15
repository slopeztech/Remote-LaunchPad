# LaunchPad Remote Controller

This repository contains firmware for an ESP8266-based LaunchPad remote controller. It creates a Wi‑Fi access point (soft AP) and serves an HTTPS API that provides basic status endpoints and actions like a countdown-triggered relay activation.

The project uses the Arduino framework and is built with PlatformIO (recommended to use Visual Studio Code with the PlatformIO extension).

---

## Contents / Key files

- `platformio.ini` — PlatformIO project configuration (board: `nodemcu`, framework: `arduino`).
- `src/main.cpp` — Main firmware: Wi‑Fi AP setup, HTTPS server, endpoints (`/ping`, `/countdown`), handlers and main loop.
- `src/config.h` — Project configuration: `ssid`, `password`, pin definitions and hardware constants.
- `src/certs.h` and `src/certs.cpp` — TLS certificate and private key stored in PROGMEM. `certs.h` declares `extern` symbols and `certs.cpp` defines them.
- `src/README.md` — (not present) Use this file as the main repo README.

Notes in the code:
- TLS certificate and private key are stored in flash (PROGMEM). The private key is sensitive — keep it private and avoid committing real production keys to public repos.

---

## Quick overview of how the code works

- On boot the ESP8266 sets up a Wi‑Fi Access Point using the SSID/password in `src/config.h`.
- It starts an HTTPS server (BearSSL via `ESP8266WebServerSecure`) on port 443 and an optional HTTP server on port 80 that redirects to HTTPS.
- Endpoints available:
  - `GET /ping` — returns a JSON status (uptime, wifi RSSI, heap, soft switch state).
  - `GET /countdown?seconds=<n>` — starts a countdown for `<n>` seconds; on completion it checks the soft switch and may activate the relay.

Hardware mapping (default in `config.h`):
- `ledPin` — builtin LED
- `relayPin` — D1 (GPIO5)
- `softSwitchPin` — D2 (GPIO4)
- `buzzerPin` — D5 (GPIO14)

---

## Configuration

Edit `src/config.h` to change network credentials or pin assignments. Example:

```cpp
// AP credentials
static const char* ssid = "REMOTE_LAUNCHPAD";
static const char* password = "12345678";

// Pins
const int ledPin = LED_BUILTIN;
const int relayPin = 5;
const int softSwitchPin = 4;
const int buzzerPin = 14;
```

If you change pins, ensure they match your hardware wiring.

---

## Certificates

Certificates are declared in `src/certs.h` as `extern const char serverCert[];` and `extern const char serverKey[];` and defined in `src/certs.cpp` with `PROGMEM` raw PEM strings.

Important:
- The example includes an RSA private key in the repo for development/testing only. Do NOT use private keys from this repo in production or publish real private keys in public repositories.
- For testing HTTPS clients (curl, browser), use the skip-verify flag (see Test section) or install the certificate into the client trust store.

---

## Build and Upload (VS Code + PlatformIO)

Prerequisites:
- Visual Studio Code
- PlatformIO IDE extension (install from the VS Code Extensions marketplace)

Steps:

1. Open VS Code and open the folder `LaunchPad` (the project root where `platformio.ini` is located).
2. In the PlatformIO side bar you can click `Build` (check mark icon) to compile.
3. Connect your ESP8266 board (NodeMCU) via USB. PlatformIO usually auto-detects the serial port.
4. Click `Upload` (arrow icon) to flash the firmware.
5. Open `PlatformIO → Serial Monitor` (plug icon) to view serial output at `115200` baud.

CLI (PowerShell / Terminal) commands:

```powershell
# from project root
# Build
C:\Users\<you>\.platformio\penv\Scripts\platformio.exe run

# Upload (PlatformIO detects the port/board automatically if configured)
C:\Users\<you>\.platformio\penv\Scripts\platformio.exe run --target upload

# Or using platformio in PATH (if configured)
platformio run
platformio run --target upload

# Serial monitor (PlatformIO)
platformio device monitor --baud 115200
```

If PlatformIO can't detect the port automatically, pass `-p COM3` (or appropriate port) to the upload command.

---

## Testing the server

1. Watch the serial monitor after upload — it prints the AP IP (usually `192.168.4.1` when using softAP mode) and server start messages.
2. From a device connected to the device's Wi‑Fi AP, or from the ESP host machine if routing configured, call the `/ping` endpoint.

Using `curl` and skipping certificate verification (development):

```powershell
curl -k https://<AP_IP>/ping
```

Replace `<AP_IP>` with the IP displayed in the serial monitor (e.g. `192.168.4.1`).

If you use a browser, you will see a certificate warning because the server uses a self-signed certificate.

---

## Troubleshooting

- Server not responding:
  - Check serial monitor for errors or that the HTTPS server started.
  - Verify board booted (serial output) and AP IP printed.
  - Ensure the client is connected to the ESP AP and that you use the correct IP.
- Build errors about undefined symbols (e.g. pins):
  - Ensure `#include "config.h"` is present in `src/main.cpp`.
- TLS / BearSSL errors:
  - Make sure `certs.h` declares `extern` and `certs.cpp` defines the `PROGMEM` strings.

---

## Security notes

- Do not publish real private keys publicly. Replace the example key with a generated one for local development and keep production keys secret.
- For production, consider using a proper CA-signed certificate or an alternative secure provisioning mechanism.

---

## Contributing

1. Fork the repo and create a feature branch.
2. Make changes, test on hardware, and open a pull request with a clear description.

---

Last updated: 2025-11-15
