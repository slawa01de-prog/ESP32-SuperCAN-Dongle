/*
  ESP32 SuperCAN Dongle - Passive CAN Sniffer
  -------------------------------------------
  Development firmware: ESP32-S3 + MCP2515 + Wi-Fi + FFat

  IMPORTANT:
  - Default CAN mode is MCP_LISTENONLY.
  - No arbitrary CAN transmit function is included.
  - Verify GPIO mapping and MCP2515 oscillator before use.
*/

#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <FS.h>
#include <FFat.h>
#include <mcp_can.h>

// ---------- Wi-Fi ----------
const char* WIFI_SSID = "DEIN_WLAN";
const char* WIFI_PASS = "DEIN_PASSWORT";
const char* AP_SSID   = "SuperCAN-Sniffer";
const char* AP_PASS   = "12345678";

// ---------- MCP2515 ----------
#define CAN_CS     10
#define CAN_INT     9
#define CAN_SCK    12
#define CAN_MISO   13
#define CAN_MOSI   11
#define MCP_CLOCK  MCP_8MHZ

// ---------- Logging ----------
#define LOG_FILE          "/canlog.csv"
#define MAX_ROWS_HTML     120
#define MAX_RECENT_FRAMES 160

struct FrameRow {
  unsigned long ms;
  unsigned long id;
  bool ext;
  byte len;
  byte data[8];
};

WebServer server(80);
MCP_CAN CAN0(CAN_CS);

bool wifiConnected = false;
bool ffatReady = false;
bool canReady = false;
bool capturePaused = false;
bool loggingEnabled = true;

byte canBitrate = CAN_500KBPS;
String canBitrateText = "500 kbit/s";
String canStatus = "not initialized";

unsigned long bootMs = 0;
unsigned long frameCounter = 0;
unsigned long lastFrameMs = 0;

FrameRow recentFrames[MAX_RECENT_FRAMES];
int recentCount = 0;

String bytesToHex(const byte* data, byte len) {
  String out;
  for (byte i = 0; i < len; i++) {
    if (data[i] < 0x10) out += "0";
    out += String(data[i], HEX);
    if (i + 1 < len) out += " ";
  }
  out.toUpperCase();
  return out;
}

String idToHex(unsigned long id, bool ext) {
  String s = String(id, HEX);
  s.toUpperCase();
  while (s.length() < (ext ? 8 : 3)) s = "0" + s;
  return "0x" + s;
}

String uptimeText() {
  unsigned long sec = (millis() - bootMs) / 1000UL;
  unsigned long h = sec / 3600UL;
  unsigned long m = (sec % 3600UL) / 60UL;
  unsigned long s = sec % 60UL;
  char buf[24];
  snprintf(buf, sizeof(buf), "%02lu:%02lu:%02lu", h, m, s);
  return String(buf);
}

String ipText() {
  return wifiConnected ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
}

bool initFFat() {
  if (!FFat.begin(true)) {
    ffatReady = false;
    return false;
  }
  ffatReady = true;
  if (!FFat.exists(LOG_FILE)) {
    File f = FFat.open(LOG_FILE, FILE_WRITE);
    if (f) {
      f.println("millis,id,ext,dlc,data");
      f.close();
    }
  }
  return true;
}

void appendLog(unsigned long id, bool ext, byte len, byte* data) {
  if (!ffatReady || !loggingEnabled) return;
  File f = FFat.open(LOG_FILE, FILE_APPEND);
  if (!f) return;
  f.print(millis());
  f.print(',');
  f.print(idToHex(id, ext));
  f.print(',');
  f.print(ext ? 1 : 0);
  f.print(',');
  f.print(len);
  f.print(',');
  f.println(bytesToHex(data, len));
  f.close();
}

void clearLog() {
  if (!ffatReady) return;
  FFat.remove(LOG_FILE);
  File f = FFat.open(LOG_FILE, FILE_WRITE);
  if (f) {
    f.println("millis,id,ext,dlc,data");
    f.close();
  }
}

size_t logSize() {
  if (!ffatReady || !FFat.exists(LOG_FILE)) return 0;
  File f = FFat.open(LOG_FILE, FILE_READ);
  if (!f) return 0;
  size_t size = f.size();
  f.close();
  return size;
}

void storeRecent(unsigned long id, bool ext, byte len, byte* data) {
  for (int i = MAX_RECENT_FRAMES - 1; i > 0; i--) recentFrames[i] = recentFrames[i - 1];
  recentFrames[0].ms = millis();
  recentFrames[0].id = id;
  recentFrames[0].ext = ext;
  recentFrames[0].len = len;
  for (byte i = 0; i < 8; i++) recentFrames[0].data[i] = (i < len) ? data[i] : 0;
  if (recentCount < MAX_RECENT_FRAMES) recentCount++;
}

bool initCan(byte bitrate, const String& bitrateText) {
  SPI.begin(CAN_SCK, CAN_MISO, CAN_MOSI, CAN_CS);
  pinMode(CAN_INT, INPUT_PULLUP);

  byte result = CAN0.begin(MCP_ANY, bitrate, MCP_CLOCK);
  if (result != CAN_OK) {
    canReady = false;
    canStatus = "init failed";
    return false;
  }

  CAN0.setMode(MCP_LISTENONLY);
  canReady = true;
  canBitrate = bitrate;
  canBitrateText = bitrateText;
  canStatus = "listen-only";
  return true;
}

void pollCan() {
  if (!canReady || capturePaused) return;
  if (CAN0.checkReceive() != CAN_MSGAVAIL) return;

  unsigned long rxId = 0;
  byte ext = 0;
  byte len = 0;
  byte data[8] = {0};

  if (CAN0.readMsgBuf(&rxId, &ext, &len, data) == CAN_OK) {
    if (len > 8) return;
    frameCounter++;
    lastFrameMs = millis();
    storeRecent(rxId, ext != 0, len, data);
    appendLog(rxId, ext != 0, len, data);
  }
}

String framesTable() {
  String html;
  int n = recentCount;
  if (n > MAX_ROWS_HTML) n = MAX_ROWS_HTML;
  if (n == 0) return "<tr><td colspan='5'>No CAN frames captured yet.</td></tr>";

  html.reserve(n * 120);
  for (int i = 0; i < n; i++) {
    html += "<tr><td>" + String(recentFrames[i].ms) + "</td>";
    html += "<td>" + idToHex(recentFrames[i].id, recentFrames[i].ext) + "</td>";
    html += "<td>" + String(recentFrames[i].ext ? "EXT" : "STD") + "</td>";
    html += "<td>" + String(recentFrames[i].len) + "</td>";
    html += "<td><code>" + bytesToHex(recentFrames[i].data, recentFrames[i].len) + "</code></td></tr>";
  }
  return html;
}

String statusJson() {
  String j = "{";
  j += "\"mode\":\"" + String(wifiConnected ? "STA" : "AP") + "\",";
  j += "\"ip\":\"" + ipText() + "\",";
  j += "\"uptime\":\"" + uptimeText() + "\",";
  j += "\"can\":\"" + canStatus + "\",";
  j += "\"bitrate\":\"" + canBitrateText + "\",";
  j += "\"frames\":" + String(frameCounter) + ",";
  j += "\"paused\":" + String(capturePaused ? "true" : "false") + ",";
  j += "\"logging\":" + String(loggingEnabled ? "true" : "false") + ",";
  j += "\"ffat\":" + String(ffatReady ? "true" : "false") + ",";
  j += "\"logSize\":" + String((unsigned long)logSize());
  j += "}";
  return j;
}

void handleRoot() {
  const char page[] PROGMEM = R"rawliteral(
<!doctype html><html><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>ESP32 SuperCAN Dongle</title>
<style>
body{margin:0;background:#0e1118;color:#e8edf5;font-family:Arial,sans-serif}.w{max-width:1400px;margin:auto;padding:16px}
.card{background:#171c27;border:1px solid #2a3243;border-radius:16px;padding:16px;margin-bottom:14px}.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(220px,1fr));gap:12px}
.k{background:#111722;border-radius:12px;padding:14px}.v{font-size:24px;font-weight:700}.muted{color:#9aa8bc}.btn{border:0;border-radius:10px;padding:10px 12px;margin:4px;background:#d6a44b;color:#111;font-weight:700;cursor:pointer}
.btn2{background:#7aa7ff}.btn3{background:#77d6ae}.btn4{background:#d27ca6}table{width:100%;border-collapse:collapse}th,td{padding:8px;border-bottom:1px solid #2a3243;text-align:left}code{font-family:Consolas,monospace}.warn{background:#3c2c16;border:1px solid #7e5c25;border-radius:12px;padding:12px;margin-bottom:14px}
</style></head><body><div class="w">
<h1>ESP32 SuperCAN Dongle</h1><div class="warn">🚧 Development build — passive MCP2515 listen-only sniffer.</div>
<div class="grid">
<div class="k"><div class="muted">IP</div><div class="v" id="ip">--</div></div>
<div class="k"><div class="muted">CAN</div><div class="v" id="can">--</div></div>
<div class="k"><div class="muted">Bitrate</div><div class="v" id="br">--</div></div>
<div class="k"><div class="muted">Frames</div><div class="v" id="frames">0</div></div>
<div class="k"><div class="muted">FFat log</div><div class="v" id="size">0 B</div></div>
</div>
<div class="card"><h2>Capture</h2>
<button class="btn" onclick="act('resume')">Resume</button><button class="btn4" onclick="act('pause')">Pause</button>
<button class="btn3" onclick="act('logon')">Logging ON</button><button class="btn4" onclick="act('logoff')">Logging OFF</button>
<button class="btn2" onclick="location='/download.csv'">Download CSV</button><button class="btn4" onclick="if(confirm('Clear log?'))act('clearlog')">Clear log</button>
</div>
<div class="card"><h2>Bitrate</h2><button class="btn" onclick="act('500')">500 kbit/s</button><button class="btn" onclick="act('250')">250 kbit/s</button><button class="btn" onclick="act('125')">125 kbit/s</button></div>
<div class="card"><h2>Recent frames</h2><table><thead><tr><th>ms</th><th>ID</th><th>Type</th><th>DLC</th><th>Data</th></tr></thead><tbody id="rows"></tbody></table></div>
</div><script>
async function refresh(){try{let s=await(await fetch('/api/status')).json();ip.textContent=s.ip;can.textContent=s.can+(s.paused?' / paused':'');br.textContent=s.bitrate;frames.textContent=s.frames;size.textContent=s.logSize+' B';rows.innerHTML=await(await fetch('/api/frames')).text();}catch(e){}}
async function act(a){await fetch('/api/action?a='+encodeURIComponent(a));refresh();}
setInterval(refresh,1200);refresh();
</script></body></html>
)rawliteral";
  server.send(200, "text/html; charset=utf-8", page);
}

void handleAction() {
  String a = server.arg("a");
  if (a == "pause") capturePaused = true;
  else if (a == "resume") capturePaused = false;
  else if (a == "logon") loggingEnabled = true;
  else if (a == "logoff") loggingEnabled = false;
  else if (a == "clearlog") clearLog();
  else if (a == "500") initCan(CAN_500KBPS, "500 kbit/s");
  else if (a == "250") initCan(CAN_250KBPS, "250 kbit/s");
  else if (a == "125") initCan(CAN_125KBPS, "125 kbit/s");
  server.send(200, "text/plain", "OK");
}

void handleDownload() {
  if (!ffatReady || !FFat.exists(LOG_FILE)) {
    server.send(404, "text/plain", "No log file");
    return;
  }
  File f = FFat.open(LOG_FILE, FILE_READ);
  if (!f) {
    server.send(500, "text/plain", "Cannot open log");
    return;
  }
  server.sendHeader("Content-Disposition", "attachment; filename=supercan_capture.csv");
  server.streamFile(f, "text/csv");
  f.close();
}

void startWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 12000) delay(250);

  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
  } else {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASS);
    wifiConnected = false;
  }
}

void setup() {
  bootMs = millis();
  Serial.begin(115200);
  delay(500);

  initFFat();
  startWifi();
  initCan(CAN_500KBPS, "500 kbit/s");

  server.on("/", handleRoot);
  server.on("/api/status", [](){ server.send(200, "application/json", statusJson()); });
  server.on("/api/frames", [](){ server.send(200, "text/html; charset=utf-8", framesTable()); });
  server.on("/api/action", handleAction);
  server.on("/download.csv", handleDownload);
  server.begin();

  Serial.println();
  Serial.println("ESP32 SuperCAN Dongle");
  Serial.print("IP: ");
  Serial.println(ipText());
  Serial.print("CAN: ");
  Serial.println(canStatus);
}

void loop() {
  server.handleClient();
  pollCan();
}
