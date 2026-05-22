#define DEBUG_UART 0

//(NAKss

#include <Arduino.h>
#include "esp_task_wdt.h"
#include "esp_int_wdt.h"
#include "soc/rtc_wdt.h"
#include "soc/rtc.h"
#include "esp_system.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <stdarg.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include "page_dashboard.h"

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

#include <WiFiUdp.h>
#include <NTPClient.h>
// ----------------- WIFI -----------------
const char *ssid = "Zain";
const char *password = "Z1234567";

// ----------------- INVERTER UART -----------------
#define WIFI_RX 16
#define WIFI_TX 17

#define INV_RX 4
#define INV_TX 2

#define REST_PIN 18
#define COM_PIN 19
#define SRV_PIN 23
#define FRAME_MAX 256

HardwareSerial invSerial(1);
HardwareSerial wifiSerial(2);

// ----------------- TELNET -----------------
WiFiServer telnetServer(23);
WiFiClient telnetClient;

// ----------------- WEBSERVER -----------------
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// ----------------- TIME SERVER -----------------
const long utcOffsetInSeconds = 18000;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

const char *enumWorkingMode[] = {
    "Power on mode",
    "Standby mode",
    "Bypass mode",
    "Battery mode",
    "Fault mode",
    "Hybrid mode"};

enum Source
{
  SRC_NONE,
  SRC_WIFI,
  SRC_ESP
};

volatile Source activeSource = SRC_NONE;

struct InverterData
{
  float grid_v;
  float grid_hz;

  float ac_v;
  float ac_hz;

  int ac_va;
  int ac_w;
  int load_pct;

  float bat_v;
  float bat_scc;
  float bat_scc2;

  int dis_a;
  int chg_a;
  int bat_pct;

  int temp_inv;
  int temp_mppt1;
  int temp_mppt2;

  int pv1_w;
  int pv2_w;

  float pv1_v;
  float pv2_v;

  int mppt1_st;
  int mppt2_st;
  int load_conn;
  int bat_dir;
  int dcac_dir;
  int line_dir;
  int par_id;

  // calculated
  float ac_i;
  float ac_pf;
  float pv1_i;

  // ===== GS2B0 =====
  int grid_power_w;
  float grid_power_kw;

  // ===== MOD =====
  int inv_mode;
  String inv_mode_s;
  int server_status;
  long last_server_time;
};

InverterData inverterData; // global or static
InverterData lastSent;

// ================= TIMERS =================
unsigned long last_wifi_activity = 0;
unsigned long last_command_activity = 0;
#define WIFI_TIMEOUT 500

unsigned long last_byte_time = 0;

unsigned long last_5s = 0;
unsigned long last_30s = 0;

const unsigned long interval_5s = 5000;
const unsigned long interval_30s = 30000;

int currentCmd = 0;
bool telnet_active = false;
bool web_active = false;
bool ws_active = false;

#define FRAME_TIMEOUT 1000 // ms (adjust: 100–500 ms)

struct Command
{
  const char *cmd;
};

Command cmdList[] = {
    {"^P006GS2B0\r"},
    {"^P005GSX\r"}, // https://github.com/danielschramm/qt_inverter_10k/blob/master/p18_query_general_status_list.h
    {"^P006MOD\r"}, // https://github.com/danielschramm/qt_inverter_10k/blob/master/p18_query_working_mode.h
                    // add more here later
};

bool changed_float(float a, float b, float eps = 0.01)
{
  return fabs(a - b) > eps;
}

#define TIMEOUT_SEC 300  // 5min
#define APB_CLK 80000000 // 80 MHz default APB clock

void setupWatchDog()
{
  // --- TASK WDT ---
  esp_task_wdt_init(TIMEOUT_SEC, true); // Panic on timeout = true
  esp_task_wdt_add(NULL);               // Add current task (loopTask)
  /*
    // --- RTC WDT ---
    rtc_wdt_protect_off();
    rtc_wdt_disable();

    rtc_wdt_set_stage(RTC_WDT_STAGE0, RTC_WDT_STAGE_ACTION_RESET_SYSTEM);
    // rtc_wdt_set_time(RTC_WDT_STAGE0, TIMEOUT_SEC * APB_CLK); // 30 seconds
    rtc_wdt_set_time(RTC_WDT_STAGE0, (uint64_t)TIMEOUT_SEC * (uint64_t)APB_CLK); // 30 seconds

    rtc_wdt_enable();
    rtc_wdt_protect_on();
    */
}

void get_inverter_json(char *json, size_t len)
{

  snprintf(json, len,
           "{"
           "\"ac_v\":%.1f,"
           "\"ac_hz\":%.1f,"
           "\"ac_va\":%d,"
           "\"ac_w\":%d,"
           "\"ac_i\":%.2f,"
           "\"ac_pf\":%.2f,"
           "\"load_pct\":%d,"
           "\"temp_inv\":%d,"
           "\"pv1_v\":%.1f,"
           "\"pv1_w\":%d,"
           "\"pv1_i\":%.2f,"
           "\"load_conn\":%d,"
           "\"mppt1_st\":%d,"
           "\"dcac_dir\":%d,"
           "\"line_dir\":%d,"
           "\"grid_power_w\":%d,"
           "\"server_status\":%d,"
           "\"last_server_time\":%ld,"
           "\"inv_mode_s\":\"%s\""
           "}",

           inverterData.ac_v,
           inverterData.ac_hz,
           inverterData.ac_va,
           inverterData.ac_w,
           inverterData.ac_i,
           inverterData.ac_pf,
           inverterData.load_pct,
           inverterData.temp_inv,
           inverterData.pv1_v,
           inverterData.pv1_w,
           inverterData.pv1_i,
           inverterData.load_conn,
           inverterData.mppt1_st,
           inverterData.dcac_dir,
           inverterData.line_dir,
           inverterData.grid_power_w,
           inverterData.server_status,
           inverterData.last_server_time,
           inverterData.inv_mode_s.c_str());
}

void send_ws_delta()
{
  char json[1024];
  get_inverter_json(json, sizeof(json));
  ws.textAll(json);
}

void reset_wifi_module()
{
  digitalWrite(REST_PIN, LOW);  // reset active
  delay(100);                   // hold reset
  digitalWrite(REST_PIN, HIGH); // release reset
}

// ----------------- SEND PRINT (Serial + Telnet) -----------------
void logPrint(const String &msg, bool newLine = false)
{
  if (!telnetClient || !telnetClient.connected())
    return;
  telnetClient.print(millis());
  telnetClient.print(" ");
  telnetClient.print(msg);
  if (newLine)
  {
    telnetClient.print("\r\n");
  }
}

void logPrint(const char *msg, bool newLine = false)
{
  if (!telnetClient || !telnetClient.connected())
    return;

  telnetClient.print(millis());
  telnetClient.print(" ");
  telnetClient.print(msg);

  if (newLine)
  {
    telnetClient.print("\r\n");
  }
}

void logPrintf(const char *format, ...)
{
  if (!telnetClient || !telnetClient.connected())
    return;

  char buffer[256]; // adjust size if needed

  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  telnetClient.print(millis());
  telnetClient.print(" ");
  telnetClient.print(buffer);
}

void logPrintf(bool newLine, const char *fmt, ...)
{
  if (!telnetClient || !telnetClient.connected())
    return;

  char buf[256];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  telnetClient.print(millis());
  telnetClient.print(" ");
  telnetClient.print(buf);
  if (newLine)
    telnetClient.print("\r\n");
}

void print_task_info()
{

  telnetClient.println("==============================================================");
}

void handle_telnet_commands()
{
  static char cmd_buffer[64];
  static uint8_t index = 0;

  if (!telnetClient || !telnetClient.connected())
    return;

  while (telnetClient.available())
  {
    char c = telnetClient.read();

    // End of command
    if (c == '\n' || c == '\r')
    {
      cmd_buffer[index] = '\0';
      if (strcmp(cmd_buffer, "reboot") == 0)
      {
        telnetClient.println("Rebooting...");
        delay(1000);

        ESP.restart();
      }
      else if (strcmp(cmd_buffer, "heap") == 0)
      {
        uint32_t free_heap = ESP.getFreeHeap();
        uint32_t largest_block = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
        uint32_t internal_free = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
        uint32_t psram_free = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);

        float frag_index = 0;
        if (free_heap > 0)
          frag_index = 1.0 - ((float)largest_block / free_heap);

        telnetClient.println("\n==== ESP32 HEAP STATUS ====");

        telnetClient.printf("Free Heap        : %u bytes\n\r", free_heap);
        telnetClient.printf("Largest Block    : %u bytes\n\r", largest_block);
        telnetClient.printf("Internal Free    : %u bytes\n\r", internal_free);
        telnetClient.printf("PSRAM Free       : %u bytes\n\r", psram_free);

        telnetClient.printf("Fragmentation    : %.2f\n\r", frag_index);

        telnetClient.println("----------------------------");

        // Health status
        if (largest_block < 8000)
          telnetClient.println("STATUS: WARNING (High fragmentation)");
        else if (largest_block < 20000)
          telnetClient.println("STATUS: OK (Medium fragmentation)");
        else
          telnetClient.println("STATUS: GOOD (Healthy heap)");

        telnetClient.println("============================\n\r");
      }
      else if (strcmp(cmd_buffer, "ping") == 0)
      {
        telnetClient.println("pong");
      }
      else if (strcmp(cmd_buffer, "tasks") == 0)
      {
        print_task_info();
      }
      else if (strcmp(cmd_buffer, "?") == 0 || strcmp(cmd_buffer, "help") == 0)
      {
        telnetClient.println("\n\r==== AVAILABLE COMMANDS ====");

        telnetClient.println("tasks    - Show FreeRTOS tasks");
        telnetClient.println("heap      - Detailed heap info");
        telnetClient.println("reboot   - Restart ESP32");

        telnetClient.println("============================\n\r");
      }
      else
      {
        telnetClient.println("Unknown command");
      }

      index = 0;
    }
    else
    {
      if (index < sizeof(cmd_buffer) - 1)
      {
        cmd_buffer[index++] = c;
      }
    }
  }
}

const int CMD_COUNT = sizeof(cmdList) / sizeof(cmdList[0]);

void sendCommand(const char *cmd)
{
  invSerial.print(cmd);
  last_command_activity = millis();
  activeSource = SRC_ESP;
  // logPrint("sendCommand:" + String(cmd), true);
}

void sendProtocolCommand()
{
  sendCommand("^P005PI\r");
}

void sendNextCommand()
{
  sendCommand(cmdList[currentCmd].cmd);
  currentCmd++;
  if (currentCmd >= CMD_COUNT)
    currentCmd = 0;
}

void processGSX(String data, InverterData &d)
{

  // remove start marker
  int idx = data.indexOf("^D106");
  if (idx < 0)
    return;

  // ===== clear only GSX fields =====
  d.grid_v = 0;
  d.grid_hz = 0;

  d.ac_v = 0;
  d.ac_hz = 0;

  d.ac_va = 0;
  d.ac_w = 0;
  d.ac_i = 0;
  d.ac_pf = 0;

  d.load_pct = 0;

  d.bat_v = 0;
  d.bat_scc = 0;
  d.bat_scc2 = 0;
  d.dis_a = 0;
  d.chg_a = 0;
  d.bat_pct = 0;

  d.temp_inv = 0;
  d.temp_mppt1 = 0;
  d.temp_mppt2 = 0;

  d.pv1_w = 0;
  d.pv2_w = 0;

  d.pv1_v = 0;
  d.pv2_v = 0;
  d.pv1_i = 0;

  d.mppt1_st = 0;
  d.mppt2_st = 0;
  d.load_conn = 0;
  d.bat_dir = 0;
  d.dcac_dir = 0;
  d.line_dir = 0;
  d.par_id = 0;

  // ===== process data =====
  data = data.substring(idx + 5); // remove "^D106"
  // remove garbage chars
  data.replace("\r", "");
  data.replace("\n", "");
  data.replace("▒", "");

  // split CSV
  float v[30];
  int i = 0;

  char buf[200];
  data.toCharArray(buf, sizeof(buf));

  char *token = strtok(buf, ",");

  while (token != NULL && i < 30)
  {
    v[i++] = atof(token);
    token = strtok(NULL, ",");
  }

  // ===== assign values =====
  d.grid_v = v[0] / 10.0;
  d.grid_hz = v[1] / 10.0;

  d.ac_v = v[2] / 10.0;
  d.ac_hz = v[3] / 10.0;

  d.ac_va = v[4];
  d.ac_w = v[5];
  d.load_pct = v[6];

  d.bat_v = v[7] / 10.0;
  d.bat_scc = v[8] / 10.0;
  d.bat_scc2 = v[9] / 10.0;

  d.dis_a = v[10];
  d.chg_a = v[11];
  d.bat_pct = v[12];

  d.temp_inv = v[13];
  d.temp_mppt1 = v[14];
  d.temp_mppt2 = v[15];

  d.pv1_w = v[16];
  d.pv2_w = v[17];

  d.pv1_v = v[18] / 10.0;
  d.pv2_v = v[19] / 10.0;

  d.mppt1_st = v[21];
  d.mppt2_st = v[22];
  d.load_conn = v[23];
  d.bat_dir = v[24];
  d.dcac_dir = v[25];
  d.line_dir = v[26];
  d.par_id = v[27];

  // ===== calculations =====

  // current from VA (correct physics)
  d.ac_i = (d.grid_v > 0) ? (d.ac_va / d.grid_v) : 0;

  // power factor
  d.ac_pf = (d.ac_va > 0) ? (d.ac_w / (float)d.ac_va) : 0;

  // PV current
  d.pv1_i = (d.pv1_v > 0) ? (d.pv1_w / d.pv1_v) : 0;

  // logPrint("Watt:" + String(d.ac_w), true);
}

// =====================
// PROCESS RESPONSE
// =====================
void process_gs2b0(String data, InverterData &d)
{

  // find valid frame
  int idx = data.indexOf("^D082");
  if (idx < 0)
    return;

  d.grid_power_w = 0;
  d.grid_power_kw = 0;

  // remove header
  data = data.substring(idx + 5);

  // clean garbage
  data.replace("\r", "");
  data.replace("\n", "");
  data.replace("▒", "");
  data.trim();

  // extract first value (grid power)
  int commaIndex = data.indexOf(',');
  if (commaIndex < 0)
    return;

  String firstValue = data.substring(0, commaIndex);

  int grid_power_w = firstValue.toInt();
  float grid_power_kw = grid_power_w / 1000.0;

  d.grid_power_w = grid_power_w;
  d.grid_power_kw = grid_power_kw;

  // logPrint("KWatt:" + String(d.grid_power_kw), true);
}

void process_mod(String data, InverterData &d)
{

  // find valid frame
  int idx = data.indexOf("^D005");
  if (idx < 0)
    return;

  // clear only MOD fields
  d.inv_mode = -1;
  d.inv_mode_s = "";

  // clean garbage
  data.replace("\r", "");
  data.replace("\n", "");
  data.replace("▒", "");
  data.trim();

  // mode is after ^D005 → next 2 digits
  data = data.substring(idx + 5, idx + 7);

  int inv_mod = data.toInt();
  if (inv_mod >= 0 && inv_mod < 6)
  {
    d.inv_mode = inv_mod;
    d.inv_mode_s = String(enumWorkingMode[inv_mod]);
  }

  // logPrint("Mod:" + String(d.inv_mode_s), true);
}

void processFrame(String data)
{

  if (data.indexOf("^D106") >= 0)
  {
    processGSX(data, inverterData);
    send_ws_delta();
  }
  else if (data.indexOf("^D082") >= 0)
  {
    process_gs2b0(data, inverterData);
  }
  else if (data.indexOf("^D005") >= 0)
  {
    process_mod(data, inverterData);
  }
  /*else if (data.indexOf("^D00518") >= 0)
 {
   process_mod(data, inverterData);
 }*/
}

uint32_t last_heap_check = 0;
uint32_t worst_largest_block = 999999;

void check_heap_health()
{
  uint32_t now = millis();

  // run every 5 seconds WITHOUT blocking
  if (now - last_heap_check < 60000)
    return;

  last_heap_check = now;

  uint32_t free_heap = ESP.getFreeHeap();
  uint32_t largest_block = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);

  // track worst case over time
  if (largest_block < worst_largest_block)
    worst_largest_block = largest_block;

  if (telnetClient && telnetClient.connected())
  {
    telnetClient.printf("Heap: free=%u | largest=%u | worst=%u\n\r",
                        free_heap,
                        largest_block,
                        worst_largest_block);
  }

  // immediate fragmentation check
  if (largest_block < 8000)
  {
    // Serial.println("CRITICAL: heap fragmentation → reboot");
    ESP.restart();
  }

  // long-term degradation check
  if (worst_largest_block < 5000)
  {
    // Serial.println("CRITICAL: heap degraded over time → reboot");
    ESP.restart();
  }
}

String formatTime(time_t rawTime)
{
  struct tm *timeInfo = localtime(&rawTime);
  char buffer[30];
  strftime(buffer, sizeof(buffer), "%d-%B-%Y %I:%M:%S %p", timeInfo);
  return String(buffer);
}

// ----------------- OTA SETUP -----------------
void setupOTA()
{

  ArduinoOTA.setHostname("Knox Helper");

  ArduinoOTA.onStart([]()
                     { logPrint("OTA Start..."); });

  ArduinoOTA.onEnd([]()
                   { logPrint("OTA End"); });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        {
                          char buf[64];
                          snprintf(buf, sizeof(buf),
                                   "OTA Progress: %u%%\r",
                                   (progress * 100) / total);

                          logPrint(buf,true); });

  ArduinoOTA.onError([](ota_error_t error)
                     { logPrint("OTA Error"); });

  ArduinoOTA.begin();
}

// ----------------- WIFI  -----------------

void WiFiEvent(WiFiEvent_t event)
{
  switch (event)
  {
  case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
    logPrint("Wi-Fi disconnected");
    break;
  case ARDUINO_EVENT_WIFI_STA_GOT_IP:
    logPrint("Wi-Fi reconnected");
    server.end();   // Stop the server
    server.begin(); // Restart the server
                    ///
    ArduinoOTA.end();
    ArduinoOTA.begin();
    ///
    timeClient.end();
    timeClient.begin();
    timeClient.update();
    break;
  default:
    break;
  }
}

void setupWiFi()
{

  WiFi.setHostname("Knox Helper"); // Set the custom hostname
  WiFi.onEvent(WiFiEvent);
  WiFi.begin(ssid, password);

  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);

  logPrint("Connecting WiFi...");

  /*while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.print(".");
   }*/

  // logPrint("\nWiFi Connected");
  // logPrint(WiFi.localIP().toString());
}

void setupTelnetServer()
{
  telnetServer.begin();
  telnetServer.setNoDelay(true);
}

String processor(const String &var)
{
  return String();
}

unsigned long last_web_activity = 0;

void onWsEvent(AsyncWebSocket *server,
               AsyncWebSocketClient *client,
               AwsEventType type,
               void *arg,
               uint8_t *data,
               size_t len)
{
  if (type == WS_EVT_CONNECT)
  {
    ws_active = true;
    logPrint("WS Client Connected", true);
  }
  else if (type == WS_EVT_DISCONNECT)
  {
    ws_active = false;
    logPrint("WS Client Disconnected", true);
  }
}

void setupWebServer()
{
  // Route for JSON data
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { last_web_activity = millis();
              //request->send_P(200, "text/html", P_DASH_BOARD, processor);
                request->send_P(200, "text/html", P_DASH_BOARD); });

  server.on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              reset_wifi_module();
              request->send_P(200, "text/html", P_DASH_BOARD); });

  server.on("/rebootme", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              esp_restart();
              request->send_P(200, "text/html", P_DASH_BOARD); });

  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request)
            {
last_web_activity = millis();
  char json[1024];
  get_inverter_json(json, sizeof(json));
  request->send(200, "application/json", json); });
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  server.begin();
}

void sendServerStatus()
{
  String msg = String(inverterData.server_status);
  ws.textAll(msg);
}

int getSignalStrengthPercentage(long rssi)
{
  if (rssi <= -100)
  {
    return 0;
  }
  else if (rssi >= -50)
  {
    return 100;
  }
  else
  {
    return 2 * (rssi + 100);
  }
}

volatile bool com_changed = false;
volatile bool srv_changed = false;

void IRAM_ATTR com_isr()
{
  com_changed = true;
}

void IRAM_ATTR srv_isr()
{
  srv_changed = true;
}

int last_day = -1;

void check_reboot_daily()
{
    timeClient.update();

    int day = timeClient.getDay();  // changes every day

    if (last_day == -1)
        last_day = day;

    if (day != last_day)
    {
        delay(1000);
        ESP.restart();
    }
}

// ----------------- SETUP -----------------
void setup()
{
  Serial.begin(115200);
  wifiSerial.begin(2400, SERIAL_8N1, WIFI_RX, WIFI_TX);
  invSerial.begin(2400, SERIAL_8N1, INV_RX, INV_TX);

  pinMode(REST_PIN, OUTPUT);
  digitalWrite(REST_PIN, HIGH);

  setupWiFi();
  setupTelnetServer();
  setupWebServer();
  setupOTA();
  timeClient.begin();
  timeClient.update();

  reset_wifi_module();
  setupWatchDog();
}

void handleTelnet()
{
  if (telnetServer.hasClient())
  {
    telnet_active = true;
    telnetClient = telnetServer.available();
    logPrint(String("Telnet Client Connected | Wifi Strength ") + getSignalStrengthPercentage(WiFi.RSSI()), true);
  }

  if (!telnetClient || !telnetClient.connected())
  {
    telnet_active = false;
  }
}
// ================= BUFFERS =================

char wifiFrame[FRAME_MAX];
char invFrame[FRAME_MAX];
uint16_t wifi_idx = 0;
uint16_t inv_idx = 0;

// String wifiFrame = "";
// String invFrame  = "";
long lastPoll = 0;
unsigned long previousWiFiMillis = 0;
unsigned long lastCleanup = 0;

unsigned long last_print = 0;
int last_com_state = -1;
int last_srv_state = -1;

unsigned long wifi_start_time = 0;
unsigned long wifi_end_time = 0;
unsigned long wifi_last_end_time = 0;
int last_server_status = -1;
// ----------------- LOOP -----------------
void loop()
{
  // Feed both watchdogs
  esp_task_wdt_reset();

  // OTA handler (IMPORTANT)
  ArduinoOTA.handle();

  // Telnet client handler
  handleTelnet();
  handle_telnet_commands();
  check_heap_health();
  web_active = (millis() - last_web_activity) < 30000;
  if (millis() - lastCleanup > 5000)
  {
    ws.cleanupClients();
    lastCleanup = millis();
    timeClient.update();
    check_reboot_daily();
  }

  // ---------------- WIFI → INV ----------------
  if (activeSource != SRC_ESP)
  {
    while (wifiSerial.available())
    {
      uint8_t c = wifiSerial.read();
      invSerial.write(c);
      activeSource = SRC_WIFI;
      last_wifi_activity = millis();
      last_command_activity = last_wifi_activity;

      if (c == 0x0D)
      {
        wifiFrame[wifi_idx] = '\0'; // terminate string

        if (strstr(wifiFrame, "^P005PI"))
        {
          wifi_start_time = millis();
          wifi_end_time = 0;
        }
        else if (strstr(wifiFrame, "^P013FD2"))
        {
          wifi_end_time = millis();
          wifi_last_end_time = wifi_end_time;
        }

#if DEBUG_UART
        char logbuf[300];
        snprintf(logbuf, sizeof(logbuf), "%lu WIFI-> %s", millis(), wifiFrame);
        logPrint(logbuf, true);
#endif
        // memset(wifiFrame, 0, FRAME_MAX);
        wifi_idx = 0;               // reset buffer
        wifiFrame[wifi_idx] = '\0'; // terminate string
      }
      else
      {
        // wifiFrame += (char)c;
        if (wifi_idx < FRAME_MAX - 1)
        {
          wifiFrame[wifi_idx++] = (char)c;
        }
      }
    }
  };

  // ---------------- INV → WIFI ----------------
  while (invSerial.available())
  {
    uint8_t c = invSerial.read();
    last_byte_time = millis();
    // forward ONLY if WiFi initiated
    if (activeSource == SRC_WIFI)
    {
      wifiSerial.write(c);
#if DEBUG_UART
      if (inv_idx < FRAME_MAX - 1)
      {
        invFrame[inv_idx++] = (char)c;
      }
#endif
    }

    else if (activeSource == SRC_ESP)
    {
      if (inv_idx < FRAME_MAX - 1)
      {
        invFrame[inv_idx++] = (char)c;
      }
    }

    if (c == 0x0D)
    {
      invFrame[inv_idx] = '\0';
#if DEBUG_UART
      if (activeSource == SRC_WIFI)
      {
        char logbuf[300];
        snprintf(logbuf, sizeof(logbuf), "%lu INV-> %s", millis(), invFrame);

        logPrint(logbuf, true);
      }
#endif

      if (activeSource == SRC_ESP)
      {
        processFrame(String(invFrame));
      }
      // memset(wifiFrame, 0, FRAME_MAX);
      inv_idx = 0;
      invFrame[inv_idx] = '\0';
      activeSource = SRC_NONE;
    }
  }

  if (activeSource != SRC_NONE)
  {
    // no CR received within timeout
    if ((millis() - last_byte_time) > FRAME_TIMEOUT && (millis() - last_command_activity) > FRAME_TIMEOUT)
    {
      activeSource = SRC_NONE;
    }
  }

  inverterData.server_status = ((millis() - last_wifi_activity) < 5000 || activeSource == SRC_WIFI) ? 1 : 0;

  if (inverterData.server_status != last_server_status)
  {

    if (last_server_status == 1 && inverterData.server_status == 0)
    {
      timeClient.update();
      inverterData.last_server_time = timeClient.getEpochTime();
    }
    last_server_status = inverterData.server_status;
    send_ws_delta();
  }

  // =========================
  // ESP polling (only when idle)
  // =========================
  unsigned long now = millis();

  if ((now - last_command_activity > WIFI_TIMEOUT) &&
      (now - last_wifi_activity > WIFI_TIMEOUT) &&
      (activeSource == SRC_NONE) &&
      (telnet_active || web_active || ws_active))
  {

    if (currentCmd == 0)
    {
      sendCommand(cmdList[0].cmd);
      currentCmd = 1;
    }
    else if (currentCmd == 1)
    {
      sendCommand(cmdList[1].cmd); // CMD 2
      currentCmd = 3;
    }

    else if (millis() - last_5s >= interval_5s && currentCmd == 3)
    {
      currentCmd = 0;
      last_5s = millis();
    }

    if (millis() - last_30s >= interval_30s && activeSource == SRC_NONE)
    {
      last_30s = millis();
      sendCommand(cmdList[2].cmd); // MOD
    }
  }

  if ((WiFi.status() != WL_CONNECTED) && (millis() - previousWiFiMillis >= 30000))
  {
    logPrint("Reconnecting to Wi-Fi...", true);
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    previousWiFiMillis = millis();
  }

  if ((millis() - wifi_last_end_time >= 900000))
  {
    reset_wifi_module();
  }
  delay(0);
}
