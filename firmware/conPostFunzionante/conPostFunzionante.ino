#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <SD.h>
#include <ArduinoJson.h>
#include <WiFiUDP.h>
#include <ArduinoMDNS.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "bitmaps.h"

const int chipSelect = 10;
const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 64;
const int OLED_RESET = -1;
const int displayPin = 9;

WiFiUDP udp;
MDNS mdns(udp);
WiFiServer server(80);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int DEFAULT_ACCELERATION_X = 0;
int DEFAULT_ACCELERATION_Y = 0;
int DEFAULT_ACCELERATION_Z = 0;
float DEFAULT_DELAY_X = 0;
float DEFAULT_DELAY_Y = 0;
float DEFAULT_DELAY_Z = 0;

int wh1[24][3];
String wh1_strings[24][3];
int wh2[24][3];
String wh2_strings[24][3];

int sizewh1 = sizeof(wh1) / sizeof(wh1[0]);
int sizewh2 = sizeof(wh2) / sizeof(wh2[0]);

String wifi[99][2];
int numNets = 0;

int xtime = 0;
int ytime = 0;
int ztime = 0;
int xstep = 0;
int ystep = 0;
int zstep = 0;

float calcSpeed(float step_delay_ms, int microStep) {
  const int steps_per_revolution = 200;
  int microStepFactor;

  switch (microStep) {
    case 0: microStepFactor = 1; break;
    case 1: microStepFactor = 2; break;
    case 2: microStepFactor = 4; break;
    case 3: microStepFactor = 8; break;
    case 4: microStepFactor = 16; break;
    case 5: microStepFactor = 32; break;
    default: microStepFactor = 1; break;
  }

  float effective_steps_per_revolution = steps_per_revolution * microStepFactor;

  if (step_delay_ms <= 0 || effective_steps_per_revolution <= 0) {
    Serial.println("Errore: divisione per zero");
    return 0;
  }

  float rpm = (60.0 * 1000.0) / (step_delay_ms * effective_steps_per_revolution);
  Serial.print("RPM: ");
  Serial.println(rpm);
  return rpm;
}


void setupDisplay() {
  pinMode(displayPin, OUTPUT);
  digitalWrite(displayPin, HIGH);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (true)
      ;
  }
}

void printMessage(const String& title, const String& text, bool clientConnected, bool m5Connected) {
  display.clearDisplay();

  if (WiFi.status() == WL_CONNECTED) {
    display.drawBitmap(SCREEN_WIDTH - WIFI_ICON_WIDTH, 0, wifiConnectedIcon, WIFI_ICON_WIDTH, WIFI_ICON_HEIGHT, WHITE);
  }
  if (clientConnected) {
    display.drawBitmap(0, 0, clientConnectedIcon, WIFI_ICON_WIDTH, WIFI_ICON_HEIGHT, WHITE);
  }
  if (m5Connected) {
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(WIFI_ICON_WIDTH + 2, 0);
    display.println("M5");
  }

  int topMargin = WIFI_ICON_HEIGHT + 2;

  display.setTextSize(1);
  display.setTextColor(WHITE);
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
  int x_centered = (SCREEN_WIDTH - w) / 2;
  display.setCursor(x_centered, topMargin);
  display.println(title);

  int text_y = topMargin + h + 4;

  display.setTextSize(1);
  int maxWidth = SCREEN_WIDTH;
  String line = "";

  for (int i = 0; i < text.length(); i++) {
    char c = text.charAt(i);

    if (c == '\n') {
      display.getTextBounds(line, 0, 0, &x1, &y1, &w, &h);
      int x_text_centered = (SCREEN_WIDTH - w) / 2;
      display.setCursor(x_text_centered, text_y);
      display.println(line);
      line = "";
      text_y += h + 2;
    } else {
      String testLine = line + c;
      display.getTextBounds(testLine, 0, 0, &x1, &y1, &w, &h);
      if (w > maxWidth) {
        display.getTextBounds(line, 0, 0, &x1, &y1, &w, &h);
        int x_text_centered = (SCREEN_WIDTH - w) / 2;
        display.setCursor(x_text_centered, text_y);
        display.println(line);
        line = String(c);
        text_y += h + 2;
      } else {
        line = testLine;
      }
    }
  }
  if (line.length() > 0) {
    display.getTextBounds(line, 0, 0, &x1, &y1, &w, &h);
    int x_text_centered = (SCREEN_WIDTH - w) / 2;
    display.setCursor(x_text_centered, text_y);
    display.println(line);
  }
  display.display();
}

bool initializeIntArrayFromCSV(const char* filename, int array[][3], int rows) {
  File file = SD.open(filename);
  if (!file) {
    printMessage("ERROR", "error open: " + String(filename), false, false);
    return false;
  }

  int i = 0;
  while (file.available() && i < rows) {
    String line = file.readStringUntil('\n');
    int idx1 = line.indexOf(',');
    int idx2 = line.lastIndexOf(',');

    array[i][0] = line.substring(0, idx1).toInt();
    array[i][1] = line.substring(idx1 + 1, idx2).toInt();
    array[i][2] = line.substring(idx2 + 1).toInt();

    i++;
  }

  file.close();
  return true;
}

bool initializeStringArrayFromCSV(const char* filename, String array[][3], int rows) {
  File file = SD.open(filename);
  if (!file) {
    printMessage("ERROR", "error open: " + String(filename), false, false);
    return false;
  }

  int i = 0;
  while (file.available() && i < rows) {
    String line = file.readStringUntil('\n');
    line.trim();  // Rimuove \r, \n e spazi vuoti

    int idx1 = line.indexOf(',');
    int idx2 = line.lastIndexOf(',');

    if (idx1 == -1 || idx2 == -1 || idx1 == idx2) {
      printMessage("ERROR", "error parse: " + String(line), false, false);
      file.close();
      return false;
    }

    array[i][0] = line.substring(0, idx1);
    array[i][1] = line.substring(idx1 + 1, idx2);
    array[i][2] = line.substring(idx2 + 1);

    array[i][0].trim();
    array[i][1].trim();
    array[i][2].trim();

    i++;
  }

  file.close();
  return true;
}

bool initDefault(const char* filename) {
  File dataFile = SD.open(filename);
  if (!dataFile) {
    printMessage("ERROR", "error open: " + String(filename), false, false);
    return false;
  }

  int index = 0;
  while (dataFile.available() && index < 6) {
    String value = dataFile.readStringUntil('\n');
    value.trim();

    if (value.length() == 0) continue;

    if (index == 0) {
      DEFAULT_ACCELERATION_X = value.toInt();
      Serial.println("DEFAULT_ACC_X");
      Serial.println(DEFAULT_ACCELERATION_X);
    } else if (index == 1) {
      DEFAULT_ACCELERATION_Y = value.toInt();
      Serial.println("DEFAULT_ACC_Y");
      Serial.println(DEFAULT_ACCELERATION_Y);
    } else if (index == 2) {
      DEFAULT_ACCELERATION_Z = value.toInt();
      Serial.println("DEFAULT_ACC_Z");
      Serial.println(DEFAULT_ACCELERATION_Z);
    } else if (index >= 3 && index <= 5) {
      float floatValue = value.toInt();
      if (index == 3) {
        DEFAULT_DELAY_X = floatValue;
        Serial.println("DEFAULT_DELAY_X");
        Serial.println(DEFAULT_DELAY_X);
      } else if (index == 4) {
        DEFAULT_DELAY_Y = floatValue;
        Serial.println("DEFAULT_DELAY_Y");
        Serial.println(DEFAULT_DELAY_Y);
      } else if (index == 5) {
        DEFAULT_DELAY_Z = floatValue;
        Serial.println("DEFAULT_DELAY_Z");
        Serial.println(DEFAULT_DELAY_Z);
      }
    }

    index++;
  }

  dataFile.close();
  return true;
}

int loadWiFiCredentials(const char* filename, String wifi[][2]) {
  File file = SD.open(filename, FILE_READ);
  if (!file) {
    printMessage("ERROR", "error open: " + String(filename), false, false);
    return false;
  }

  int index = 0;
  while (file.available() && index < 99) {
    String line = file.readStringUntil('\n');
    line.trim();

    if (line.length() == 0) {
      continue;
    }

    int commaIndex = line.indexOf(',');
    if (commaIndex == -1) {
      printMessage("ERROR", "error row: " + String(line), false, false);
      continue;
    }

    String ssid = line.substring(0, commaIndex);
    String password = line.substring(commaIndex + 1);

    wifi[index][0] = ssid;
    wifi[index][1] = password;
    index++;
  }

  file.close();
  return index;
}

void csvConfig() {
  if (initializeIntArrayFromCSV("/WH1_INT.CSV", wh1, 24)) {
    printMessage("SETUP", "init wh1", false, false);
  } else {
    printMessage("ERROR", "error init wh1", false, false);
  }

  if (initializeIntArrayFromCSV("/WH2_INT.CSV", wh2, 24)) {
    printMessage("SETUP", "init wh2", false, false);
  } else {
    printMessage("ERROR", "error init wh2", false, false);
  }

  if (initializeStringArrayFromCSV("/WH1_STR.CSV", wh1_strings, 24)) {
    printMessage("SETUP", "init wh1_str", false, false);
  } else {
    printMessage("ERROR", "error init wh1_str", false, false);
  }

  if (initializeStringArrayFromCSV("/WH2_STR.CSV", wh2_strings, 24)) {
    printMessage("SETUP", "init wh2_str", false, false);
  } else {
    printMessage("ERROR", "error init wh2_str", false, false);
  }

  if (initDefault("/INIT_VAL.CSV")) {
  } else {
    printMessage("ERROR", "error init value", false, false);
  }
}

void netConfig() {
  numNets = loadWiFiCredentials("/WIFI.CSV", wifi);
  if (numNets == -1) {
    printMessage("ERROR", "error open: WIFI.CSV", false, false);
  }
}

void recreateFileWithDefaults(const char* filename) {
  if (SD.exists(filename)) {
    if (SD.remove(filename)) {
      printMessage("SAVE", "delete file: " + String(filename), true, false);
    } else {
      printMessage("ERROR", "error save: " + String(filename), true, false);
      return;
    }
  }

  File newFile = SD.open(filename, FILE_WRITE);
  if (newFile) {
    newFile.print(String(DEFAULT_ACCELERATION_X) + "\r");
    newFile.print(String(DEFAULT_ACCELERATION_Y) + "\r");
    newFile.print(String(DEFAULT_ACCELERATION_Z) + "\r");
    newFile.print(String(DEFAULT_DELAY_X, 3) + "\r");
    newFile.print(String(DEFAULT_DELAY_Y, 3) + "\r");
    newFile.print(String(DEFAULT_DELAY_Z, 3) + "\r");
    newFile.close();
    printMessage("SAVE", "success save: " + String(filename), true, false);
  } else {
    printMessage("ERROR", "error save: " + String(filename), true, false);
  }
}

void wifiConfig() {
  printMessage("SETUP", "Scanning WiFi...", false, false);
  int numNetworks = WiFi.scanNetworks();
  printMessage("SETUP", "Scanning END", false, false);

  File file = SD.open("/WIFI.CSV", FILE_READ);
  if (!file) {
    printMessage("ERROR", "Error opening WIFI.CSV", false, false);
    return;
  }

  bool connected = false;
  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    if (line.isEmpty()) continue;

    int separatorIndex = line.indexOf(',');
    if (separatorIndex == -1) continue;

    String ssid = line.substring(0, separatorIndex);
    String password = line.substring(separatorIndex + 1);
    ssid.trim();
    password.trim();

    if (password.endsWith(",")) {
      password = password.substring(0, password.length() - 1);
    }

    for (int i = 0; i < numNetworks; i++) {
      if (ssid == WiFi.SSID(i)) {
        printMessage("SETUP", "Connecting to " + ssid, false, false);

        // Reset connessione per evitare problemi
        //WiFi.disconnect(true);
        //WiFi.mode(WIFI_OFF);
        //delay(500);
        //WiFi.mode(WIFI_STA);
        WiFi.begin(ssid.c_str(), password.c_str());

        Serial.println("Trying: " + ssid);
        Serial.println("Password: " + password);

        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 30) {  // Timeout più lungo
          delay(1000);
          printMessage("SETUP", ".", false, false);
          attempts++;
        }

        if (WiFi.status() == WL_CONNECTED) {
          Serial.println("Connected to " + ssid);

          // Attendere un po' per garantire l'assegnazione dell'IP da DHCP
          delay(2000);
          Serial.print("IP Address: ");
          Serial.println(WiFi.localIP());

          if (WiFi.localIP().toString() == "0.0.0.0") {
            printMessage("ERROR", "DHCP failed, retrying...", false, false);
            WiFi.disconnect();
            delay(1000);
            WiFi.begin(ssid.c_str(), password.c_str());
            delay(5000);  // Aspetta il DHCP
            Serial.print("New IP: ");
            Serial.println(WiFi.localIP());
          }

          server.begin();
          if (mdns.begin(WiFi.localIP(), "vertistock")) {
            Serial.println("mDNS started: " + WiFi.localIP().toString());
            printMessage("SETUP", "mDNS started", false, false);
            mdns.addServiceRecord("_http._tcp", 80, MDNSServiceTCP);
            printMessage("WAIT", "Wait operation", false, false);
          } else {
            printMessage("ERROR", "Error starting mDNS", false, false);
          }

          connected = true;
          break;
        } else {
          printMessage("ERROR", "Connection failed", false, false);
        }
      }
    }
    if (connected) break;
  }

  file.close();

  if (!connected) {
    printMessage("ERROR", "WiFi not found", false, false);
  }
}

void restart() {
  printMessage("OPERATION", "restarting...", true, false);
  delay(1000);
  printMessage("OPERATION", "restarted", true, false);
  NVIC_SystemReset();
}

// firma corretta con riferimento
void sendResponse(WiFiClient& client, int statusCode, const char* contentType, const String& message) {
  client.print("HTTP/1.1 " + String(statusCode) + " OK\r\n");
  client.print("Content-Type: " + String(contentType) + "\r\n");
  client.print("Content-Length: " + String(message.length()) + "\r\n");
  // CORS
  client.print("Access-Control-Allow-Origin: *\r\n");
  client.print("Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n");
  client.print("Access-Control-Allow-Headers: Content-Type\r\n");
  client.print("Connection: close\r\n\r\n");
  client.print(message);
  client.stop();
}

void handleOptionsRequest(WiFiClient& client) {
  client.print("HTTP/1.1 200 OK\r\n");
  client.print("Access-Control-Allow-Origin: *\r\n");
  client.print("Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n");
  client.print("Access-Control-Allow-Headers: Content-Type\r\n");
  client.print("Connection: close\r\n\r\n");
  client.stop();
}

String headRequest(int contentLength) {
  String headString =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: application/json\r\n"
    "Content-Length: "
    + String(contentLength) + "\r\n"
                              "Connection: close\r\n"
                              "Access-Control-Allow-Origin: *\r\n"
                              "Access-Control-Allow-Credentials: true\r\n"
                              "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
                              "Access-Control-Allow-Headers: Content-Type, Authorization\r\n\r\n";

  return headString;
}

void status(WiFiClient& client) {
  StaticJsonDocument<4096> doc;
  doc["status"] = (WiFi.status() == WL_CONNECTED) ? "true" : "false";
  doc["rssi"] = WiFi.RSSI();
  size_t contentLength = measureJson(doc);
  client.print(headRequest(contentLength));
  serializeJson(doc, client);
  client.flush();
  doc.clear();
}

void getOp1(WiFiClient& client) {
  String response = "{";
  response += "\"xacc-read\":" + String(DEFAULT_ACCELERATION_X) + ",";
  response += "\"yacc-read\":" + String(DEFAULT_ACCELERATION_Y) + ",";
  response += "\"zacc-read\":" + String(DEFAULT_ACCELERATION_Z) + ",";
  response += "\"xdelay-read\":" + String(DEFAULT_DELAY_X) + ",";
  response += "\"ydelay-read\":" + String(DEFAULT_DELAY_Y) + ",";
  response += "\"zdelay-read\":" + String(DEFAULT_DELAY_Z) + ",";
  response += "\"xtime-read\":" + String(xtime) + ",";
  response += "\"ytime-read\":" + String(ytime) + ",";
  response += "\"ztime-read\":" + String(ztime) + ",";
  response += "\"xstep-read\":" + String(xstep) + ",";
  response += "\"ystep-read\":" + String(ystep) + ",";
  response += "\"zstep-read\":" + String(zstep) + ",";
  response += "\"xspeed-read\":" + String(calcSpeed(DEFAULT_DELAY_X, DEFAULT_ACCELERATION_X)) + ",";
  response += "\"yspeed-read\":" + String(calcSpeed(DEFAULT_DELAY_Y, DEFAULT_ACCELERATION_Y)) + ",";
  response += "\"zspeed-read\":" + String(calcSpeed(DEFAULT_DELAY_Z, DEFAULT_ACCELERATION_Z)) + ",";

  char buffer[10];

  response += "\"xdelay-read\":" + String(dtostrf(DEFAULT_DELAY_X, 1, 2, buffer)) + ",";
  response += "\"ydelay-read\":" + String(dtostrf(DEFAULT_DELAY_Y, 1, 2, buffer)) + ",";
  response += "\"zdelay-read\":" + String(dtostrf(DEFAULT_DELAY_Z, 1, 2, buffer));
  response += "}";

  size_t contentLength = response.length();
  client.print(headRequest(contentLength));
  client.print(response);
  client.flush();
}

void getOp2(WiFiClient& client) {
  String response = "{";
  response += "\"xacc-write\":" + String(DEFAULT_ACCELERATION_X) + ",";
  response += "\"yacc-write\":" + String(DEFAULT_ACCELERATION_Y) + ",";
  response += "\"zacc-write\":" + String(DEFAULT_ACCELERATION_Z) + ",";

  char buffer[10];

  response += "\"xdelay-write\":" + String(dtostrf(DEFAULT_DELAY_X, 1, 2, buffer)) + ",";
  response += "\"ydelay-write\":" + String(dtostrf(DEFAULT_DELAY_Y, 1, 2, buffer)) + ",";
  response += "\"zdelay-write\":" + String(dtostrf(DEFAULT_DELAY_Z, 1, 2, buffer)) + ",";

  response += "\"wifi\":[";
  for (int i = 0; i < numNets; i++) {
    if (wifi[i][0].length() > 0) {
      response += "{\"ssid\":\"" + wifi[i][0] + "\",";
      response += "\"pass\":\"" + wifi[i][1] + "\"},";
    }
  }

  if (response.endsWith(",")) {
    response.remove(response.length() - 1);
  }

  response += "]}";

  size_t contentLength = response.length();
  client.print(headRequest(contentLength));
  client.print(response);
  client.flush();
}

void getOp3wh1(WiFiClient& client) {
  String response = "{";

  response += "\"wh1\":[";
  for (int i = 0; i < sizewh1; i++) {
    response += "{\"x\":" + String(wh1[i][0]) + ",\"y\":" + String(wh1[i][1]) + ",\"z\":1,\"full\":" + String(wh1[i][2]);
    if (wh1[i][2] == 1) {
      response += ",\"name\":\"" + wh1_strings[i][0] + "\"";
      response += ",\"code\":\"" + wh1_strings[i][1] + "\"";
      response += ",\"count\":\"" + wh1_strings[i][2] + "\"";
    }
    response += "},";
  }
  response.remove(response.length() - 1);
  response += "]}";

  size_t contentLength = response.length();

  client.print(headRequest(contentLength));
  client.print(response);
  client.flush();
}

void getOp3wh2(WiFiClient& client) {
  String response = "{";

  response += "\"wh2\":[";
  for (int i = 0; i < sizewh2; i++) {
    response += "{\"x\":" + String(wh2[i][0]) + ",\"y\":" + String(wh2[i][1]) + ",\"z\":2,\"full\":" + String(wh2[i][2]);
    if (wh2[i][2] == 1) {
      response += ",\"name\":\"" + wh2_strings[i][0] + "\"";
      response += ",\"code\":\"" + wh2_strings[i][1] + "\"";
      response += ",\"count\":\"" + wh2_strings[i][2] + "\"";
    }
    response += "},";
  }
  response.remove(response.length() - 1);
  response += "]}";

  size_t contentLength = response.length();

  client.print(headRequest(contentLength));
  client.print(response);
  client.flush();
}

void postOp2(WiFiClient& client) {
  String request = "";
  while (client.connected() && client.available()) {
    char c = client.read();
    request += c;
  }

  int jsonStart = request.indexOf("{");
  int jsonEnd = request.lastIndexOf("}");
  if (jsonStart == -1 || jsonEnd == -1) {
    sendResponse(client, 400, "application/json", "{\"status\": \"error\", \"message\": \"JSON non trovato nella richiesta\"}");
    return;
  }

  String jsonString = request.substring(jsonStart, jsonEnd + 1);
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, jsonString);

  if (error) {
    sendResponse(client, 400, "application/json", "{\"status\": \"error\", \"message\": \"Errore di parsing JSON\"}");
    return;
  }

  const char* keys[] = { "xacc", "yacc", "zacc", "xdelay", "ydelay", "zdelay" };
  bool allKeysPresent = true;

  for (const char* key : keys) {
    if (!doc.containsKey(key)) {
      allKeysPresent = false;
    }
  }

  if (allKeysPresent) {
    DEFAULT_ACCELERATION_X = doc["xacc"].as<int>();
    DEFAULT_ACCELERATION_Y = doc["yacc"].as<int>();
    DEFAULT_ACCELERATION_Z = doc["zacc"].as<int>();
    DEFAULT_DELAY_X = doc["xdelay"].as<float>();
    DEFAULT_DELAY_Y = doc["ydelay"].as<float>();
    DEFAULT_DELAY_Z = doc["zdelay"].as<float>();
    recreateFileWithDefaults("/INIT_VAL.CSV");

    sendResponse(client, 200, "application/json", "{\"status\": \"success\", \"message\": \"Tutti i dati sono stati ricevuti\"}");
  } else {
    sendResponse(client, 400, "application/json", "{\"status\": \"error\", \"message\": \"Alcuni parametri sono mancanti\"}");
  }
}

void recreateFileWifi(const char* filename, const JsonDocument& doc, int count) {
  if (SD.exists(filename)) {
    SD.remove(filename);
  }

  File file = SD.open(filename, FILE_WRITE);
  if (!file) {
    printMessage("ERROR", "error open: WIFI.CSV", true, false);
    return;
  }

  for (int i = 0; i <= count - 1; i++) {
    String ssidKey = "ssid-" + String(i + 1);
    String passKey = "pass-" + String(i + 1);

    String riga = doc[ssidKey].as<String>() + "," + doc[passKey].as<String>() + "\n";
    file.print(riga);
  }

  file.close();
  printMessage("OPERATION", "file saved: WIFI.CSV", true, false);
}

void postWifi(WiFiClient& client) {
  String request = "";
  while (client.connected() && client.available()) {
    char c = client.read();
    request += c;
  }

  int jsonStart = request.indexOf("{");
  int jsonEnd = request.lastIndexOf("}");
  if (jsonStart == -1 || jsonEnd == -1) {
    sendResponse(client, 400, "application/json",
                 "{\"status\": \"error\", \"message\": \"JSON non trovato nella richiesta\"}");
    return;
  }

  String jsonString = request.substring(jsonStart, jsonEnd + 1);
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, jsonString);

  if (error) {
    sendResponse(client, 400, "application/json",
                 "{\"status\": \"error\", \"message\": \"Errore di parsing JSON\"}");
    return;
  }

  if (!doc.containsKey("count")) {
    sendResponse(client, 400, "application/json",
                 "{\"status\": \"error\", \"message\": \"Manca la chiave 'count'\"}");
    return;
  }

  int count = doc["count"].as<int>();
  if (count <= 0) {
    sendResponse(client, 400, "application/json",
                 "{\"status\": \"error\", \"message\": \"Valore di 'count' non valido\"}");
    return;
  }

  bool allKeysPresent = true;
  for (int i = 1; i <= count; i++) {
    String ssidKey = "ssid-" + String(i);
    String passKey = "pass-" + String(i);

    if (!doc.containsKey(ssidKey) || !doc.containsKey(passKey)) {
      allKeysPresent = false;
      break;
    }
  }

  if (!allKeysPresent) {
    sendResponse(client, 400, "application/json",
                 "{\"status\": \"error\", \"message\": \"Alcune chiavi ssid/pass mancanti\"}");
    return;
  }

  for (int i = 0; i < 99; i++) {
    wifi[i][0] = "";
    wifi[i][1] = "";
  }

  for (int i = 0; i <= count - 1 && i <= 99; i++) {
    String ssidKey = "ssid-" + String(i + 1);
    String passKey = "pass-" + String(i + 1);

    wifi[i][0] = doc[ssidKey].as<String>();  // SSID
    wifi[i][1] = doc[passKey].as<String>();  // Password
  }

  numNets = count;

  recreateFileWifi("/WIFI.CSV", doc, count);

  sendResponse(client, 200, "application/json",
               "{\"status\": \"success\", \"message\": \"Tutti i dati sono stati ricevuti e salvati\"}");
}

void postRestart(WiFiClient& client) {
  String request = "";
  while (client.connected() && client.available()) {
    char c = client.read();
    request += c;
  }

  if (request.indexOf("restart") != -1) {
    sendResponse(client, 200, "application/json", "{\"status\": \"success\", \"message\": \"Riavvio in corso\"}");
    client.stop();
    delay(100);
    restart();
  } else {
    sendResponse(client, 400, "application/json", "{\"status\": \"error\", \"message\": \"Comando non valido\"}");
  }
}

void writeFiles(const char* file1, const char* file2, int wh1[24][3], String wh1_strings[24][3]) {
  if (SD.exists(file1)) {
    SD.remove(file1);
    printMessage("OPERATION", "delete file: " + String(file1), true, false);
  }

  if (SD.exists(file2)) {
    SD.remove(file2);
    printMessage("OPERATION", "delete file: " + String(file2), true, false);
  }

  File dataFile1 = SD.open(file1, FILE_WRITE);
  if (dataFile1) {
    for (int i = 0; i < 24; i++) {
      dataFile1.print(wh1[i][0]);
      dataFile1.print(",");
      dataFile1.print(wh1[i][1]);
      dataFile1.print(",");
      dataFile1.println(wh1[i][2]);
    }
    dataFile1.close();
    printMessage("OPERATION", "saved file: " + String(file1), true, false);
  } else {
    printMessage("ERROR", "error save: " + String(file1), true, false);
  }

  File dataFile2 = SD.open(file2, FILE_WRITE);
  if (dataFile2) {
    for (int i = 0; i < 24; i++) {
      dataFile2.print(wh1_strings[i][0]);
      dataFile2.print(",");
      dataFile2.print(wh1_strings[i][1]);
      dataFile2.print(",");
      dataFile2.println(wh1_strings[i][2]);
    }
    dataFile2.close();
    printMessage("OPERATION", "saved file: " + String(file2), true, false);
  } else {
    printMessage("ERROR", "error save: " + String(file2), true, false);
  }
}

void sendPostRequest(const char* host, const char* path, const String& message) {
  WiFiClient client;

  while (true) {
    Serial.println("Connessione...");
    if (!client.connect(host, 80)) {
      Serial.println("Connessione fallita. Riprovo...");
      delay(1000);
      continue;
    }

    // Costruisce la richiesta POST con il path specificato
    String request = String("POST ") + path + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Content-Type: application/json\r\n" + "Content-Length: " + message.length() + "\r\n" + "Connection: close\r\n\r\n" + message;

    client.print(request);
    Serial.println("Richiesta inviata. Attendo risposta...");

    // Bloccante: legge la risposta
    String response = "";
    while (client.connected()) {
      while (client.available()) {
        char c = client.read();
        response += c;
      }
    }

    client.stop();

    Serial.println("Risposta ricevuta:");
    Serial.println(response);

    if (response.indexOf("received") != -1) {
      Serial.println("Conferma 'received' trovata. Fine.");
      break;
    } else {
      Serial.println("Risposta non valida. Riprovo...");
      delay(1000);
    }
  }
}

void sendGetRequest(const char* host, const char* path, const String& queryParams = "") {
  WiFiClient client;

  while (true) {
    Serial.println("Connessione GET a " + String(host) + "...");
    if (!client.connect(host, 80)) {
      Serial.println("Connessione fallita. Riprovo in 1s...");
      delay(1000);
      continue;
    }

    // Costruisco il path completo con eventuali parametri
    String fullPath = String(path);
    if (queryParams.length() > 0) {
      fullPath += "?" + queryParams;
    }

    // Richiesta GET (nota: non serve Content-Length né body)
    String request = String("GET ") + fullPath + " HTTP/1.1\r\n"
                     + "Host: " + host + "\r\n"
                     + "Connection: close\r\n\r\n";

    client.print(request);
    Serial.println("Richiesta GET inviata. Attendo risposta...");

    // Leggo la risposta
    String response = "";
    while (client.connected()) {
      while (client.available()) {
        response += char(client.read());
      }
    }
    client.stop();

    Serial.println("Risposta ricevuta:");
    Serial.println(response);

    if (path == "/op=maxstepx") {
      calculateMaxStep(response, "x");
    } else if (path == "/op=maxstepy") {
      calculateMaxStep(response, "y");
    } else if (path == "/op=maxstepz") {
      calculateMaxStep(response, "z");
    }

    break;
  }
}

void postOp3(WiFiClient& client) {
  String request = "";
  while (client.connected() && client.available()) {
    char c = client.read();
    request += c;
  }

  int jsonStart = request.indexOf("{");
  int jsonEnd = request.lastIndexOf("}");
  if (jsonStart == -1 || jsonEnd == -1) {
    sendResponse(client, 400, "application/json", "{\"status\": \"error\", \"message\": \"JSON non trovato nella richiesta\"}");
    return;
  }

  String jsonString = request.substring(jsonStart, jsonEnd + 1);
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, jsonString);

  if (error) {
    sendResponse(client, 400, "application/json", "{\"status\": \"error\", \"message\": \"Errore di parsing JSON\"}");
    return;
  }

  const char* keys[] = { "box", "z" };
  bool allKeysPresent = true;

  for (const char* key : keys) {
    if (!doc.containsKey(key)) {
      printMessage("ERROR", "key miss: " + String(key), true, false);
      allKeysPresent = false;
    }
  }

  int box = doc["box"].as<int>();
  if (box < 0 || box >= 24) {
    sendResponse(client, 400, "application/json", "{\"status\":\"error\",\"message\":\"Indice box fuori range\"}");
    return;
  }

  String x;
  String y;

  // 2) Prendi il valore x
  if (doc["z"].as<int>() == 1) {
    x = "{ position:" + String(wh1[box][0]) + ", delay:" + String(DEFAULT_DELAY_X) + "}";
  } else if (doc["z"].as<int>() == 2) {
    x = "{ position:" + String(wh2[box][0]) + ", delay:" + String(DEFAULT_DELAY_X) + "}";
  }

  if (doc["z"].as<int>() == 1) {
    y = "{ position:" + String(wh1[box][0]) + ", ydelay:" + String(DEFAULT_DELAY_Y) + ", zdelay:" + String(DEFAULT_DELAY_Z) + "}";
  } else if (doc["z"].as<int>() == 2) {
    y = "{ position:" + String(wh2[box][0]) + ", ydelay:" + String(DEFAULT_DELAY_Y) + ", zdelay:" + String(DEFAULT_DELAY_Z) + "}";
  }

  if (allKeysPresent) {
    sendPostRequest("m5stack-0-x.local", "/op=withdrawx", String(x));
    //sendPostRequest("m5stack-y-z.local", "/op=withdrawyz", String(y));

    //sendGetRequest("m5stack-y-z.local", "/op=returnyz", "");
    sendGetRequest("m5stack-0-x.local", "/op=returnx", "");

    sendResponse(client, 200, "application/json", "{\"status\": \"success\", \"message\": \"Tutti i dati sono stati ricevuti\"}");
    if (doc["z"].as<int>() == 1) {
      wh1[doc["box"].as<int>()][2] = 0;
      wh1_strings[doc["box"].as<int>()][0] = "";
      wh1_strings[doc["box"].as<int>()][1] = "";
      wh1_strings[doc["box"].as<int>()][2] = "";
      writeFiles("WH1_INT.CSV", "WH1_STR.CSV", wh1, wh1_strings);
    } else if (doc["z"].as<int>() == 2) {
      wh2[doc["box"].as<int>()][2] = 0;
      wh2_strings[doc["box"].as<int>()][0] = "";
      wh2_strings[doc["box"].as<int>()][1] = "";
      wh2_strings[doc["box"].as<int>()][2] = "";
      writeFiles("WH2_INT.CSV", "WH2_STR.CSV", wh2, wh2_strings);
    }
  } else {
    sendResponse(client, 400, "application/json", "{\"status\": \"error\", \"message\": \"Alcuni parametri sono mancanti\"}");
  }
}

void calculateMaxStep(String response, String type) {
  int receivedValue = response.toInt();

  if (type == "x") {
    xstep = receivedValue;
  } else if (type == "y") {
    ystep = receivedValue;
  } else if (type == "z") {
    zstep = receivedValue;
  } else {
    Serial.println("error type");
  }

  calcolateMaxTime(xstep, ystep, zstep);
}

void calcolateMaxTime(int xstep, int ystep, int zstep) {
  if (xstep != 0) {
    xtime = DEFAULT_DELAY_X * xstep;
  }

  if (ystep != 0) {
    ytime = DEFAULT_DELAY_Y * ystep;
  }

  if (zstep != 0) {
    ztime = DEFAULT_DELAY_Z * zstep;
  }
};

void startMotor(WiFiClient& client, String type) {
  String jsonResponse;
  StaticJsonDocument<200> doc;
  doc["status"] = "1";
  if (type == "x") {
    doc["xacc"] = DEFAULT_ACCELERATION_X;
    doc["xdelay"] = DEFAULT_DELAY_X;
  } else if (type == "y") {
    doc["yacc"] = DEFAULT_ACCELERATION_Y;
    doc["ydelay"] = DEFAULT_DELAY_Y;
  } else if (type == "z") {
    doc["zacc"] = DEFAULT_ACCELERATION_Z;
    doc["zdelay"] = DEFAULT_DELAY_Z;
  }
  serializeJson(doc, jsonResponse);
  client.print("HTTP/1.1 200 OK\r\n");
  client.print("Content-Type: application/json\r\n");
  client.print("Connection: close\r\n\r\n");
  client.print(jsonResponse);
  doc.clear();
}

void controlWarehouse(int whr, String name, String code, String count) {
  int(*wh)[3];
  String(*wh_str)[3];
  int size;
  String filenameInt;
  String filenameStr;

  int x, y;
  int z = whr;

  String sendX = "";
  String sendYZ = "";

  switch (whr) {
    case 1:
      wh = wh1;
      wh_str = wh1_strings;
      size = sizewh1;
      filenameInt = "/WH1_INT.CSV";
      filenameStr = "/WH1_STR.CSV";
      break;
    case 2:
      wh = wh2;
      wh_str = wh2_strings;
      size = sizewh2;
      filenameInt = "/WH2_INT.CSV";
      filenameStr = "/WH2_STR.CSV";
      break;
    default:
      Serial.println("valore whr errato");
      return;
  }

  for (int i = 0; i < size; i++) {
    if (wh[i][2] == 0) {
      x = wh[i][0];
      y = wh[i][1];

      wh[i][2] = 1;
      

      wh_str[i][0] = name;
      wh_str[i][1] = code;
      wh_str[i][2] = count;

      break;
    }
  }

  sendX = "{ position:" + String(x) + ", delay:" + String(DEFAULT_DELAY_X) + "}";
  //sendYZ = "{y:" + String(y) + ",z:" + String(z) + "}";

  //sendPostRequest("m5stack-y-z.local", "/op=p_p_y_z", send_y_z);
  sendPostRequest("m5stack-0-x.local", "/op=p_p_x", String(sendX));

  //sendGetRequest("m5stack-y-z.local", "/op=returnyz", "");
  sendGetRequest("m5stack-0-x.local", "/op=returnx", "");

  Serial.println("name");
  Serial.println(name);

  Serial.println("code");
  Serial.println(code);

  Serial.println("count");
  Serial.println(count);

  if (updateWHint(filenameInt.c_str())) {
    printMessage("SAVE", "success save: " + String(filenameInt), true, false);
    Serial.println("success save: " + String(filenameInt));
  } else {
    printMessage("ERROR", "error save: " + String(filenameInt), true, false);
    Serial.println("error save: " + String(filenameInt));
  }

  if (updateWHstr(filenameStr.c_str())) {
    printMessage("SAVE", "success save: " + String(filenameStr), true, false);
    Serial.println("success save: " + String(filenameStr));
  } else {
    printMessage("ERROR", "error save: " + String(filenameStr), true, false);
    Serial.println("error save: " + String(filenameStr));
  }
}

// --- Funzione di utilità per confrontare C-stringhe ---
bool matchFilename(const char* a, const char* b) {
  return strcmp(a, b) == 0;
}

bool updateWHint(const char* filename) {
  int(*wh)[3] = nullptr;
  int size = 0;

  if (matchFilename(filename, "/WH1_INT.CSV")) {
    wh = wh1;
    size = sizewh1;
  } else if (matchFilename(filename, "/WH2_INT.CSV")) {
    wh = wh2;
    size = sizewh2;
  } else {
    Serial.println("updateWHint: nome file non riconosciuto");
    return false;
  }

  // Se esiste, cancellalo
  if (SD.exists(filename)) {
    if (!SD.remove(filename)) {
      Serial.println("updateWHint: errore cancellazione " + String(filename));
      return false;
    }
    Serial.println("updateWHint: file cancellato " + String(filename));
  }

  // Crea e scrivi
  File newFile = SD.open(filename, FILE_WRITE);
  if (!newFile) {
    Serial.println("updateWHint: errore apertura " + String(filename));
    return false;
  }

  for (int i = 0; i < size; i++) {
    for (int j = 0; j < 3; j++) {
      newFile.print(String(wh[i][j]));
      newFile.print((j < 2) ? "," : "\r\n");
    }
  }
  newFile.close();
  return true;
}

bool updateWHstr(const char* filename) {
  String(*wh_str)[3] = nullptr;
  int size = 0;

  if (matchFilename(filename, "/WH1_STR.CSV")) {
    wh_str = wh1_strings;
    size = sizewh1;
  } else if (matchFilename(filename, "/WH2_STR.CSV")) {
    wh_str = wh2_strings;
    size = sizewh2;
  } else {
    Serial.println("updateWHstr: nome file non riconosciuto");
    return false;
  }

  // Se esiste, cancellalo
  if (SD.exists(filename)) {
    if (!SD.remove(filename)) {
      Serial.println("updateWHstr: errore cancellazione " + String(filename));
      return false;
    }
    Serial.println("updateWHstr: file cancellato " + String(filename));
  }

  // Crea e scrivi
  File newFile = SD.open(filename, FILE_WRITE);
  if (!newFile) {
    Serial.println("updateWHstr: errore apertura " + String(filename));
    return false;
  }

  for (int i = 0; i < size; i++) {
    for (int j = 0; j < 3; j++) {
      newFile.print(wh_str[i][j]);
      newFile.print((j < 2) ? "," : "\r\n");
    }
  }
  newFile.close();
  return true;
}

void receiveQr(WiFiClient& client, int whr) {
  String request = "";
  while (client.connected() && client.available()) {
    char c = client.read();
    request += c;
  }

  int jsonStart = request.indexOf("{");
  int jsonEnd = request.lastIndexOf("}");
  if (jsonStart == -1 || jsonEnd == -1) {
    sendResponse(client, 400, "application/json", "{\"status\": \"error\", \"message\": \"JSON non trovato nella richiesta\"}");
    return;
  }

  String jsonString = request.substring(jsonStart, jsonEnd + 1);

  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, jsonString);
  if (error) {
    sendResponse(client, 400, "application/json", "{\"status\": \"error\", \"message\": \"Errore di parsing JSON\"}");
    return;
  }

  const char* keys[] = { "name", "code", "count" };
  bool allKeysPresent = true;
  for (const char* key : keys) {
    if (!doc.containsKey(key)) {
      printMessage("ERROR", "Chiave mancante: " + String(key), true, false);
      allKeysPresent = false;
    }
  }

  if (!allKeysPresent) {
    sendResponse(client, 400, "application/json", "{\"status\": \"error\", \"message\": \"Alcuni parametri sono mancanti\"}");
    return;
  }

  controlWarehouse(whr, doc["name"].as<String>(), doc["code"].as<String>(), doc["count"].as<String>());

  sendResponse(client, 200, "application/json", "{\"status\":\"success\",\"message\":\"QR code ricevuto\"}");
  client.stop();
}

void handleClient(WiFiClient client) {
  String requestLine = client.readStringUntil('\r');
  client.readStringUntil('\n');

  // salto tutti gli header
  while (client.available() && client.read() != '\n')
    ;

  // PRE-FLIGHT CORS
  if (requestLine.startsWith("OPTIONS ")) {
    handleOptionsRequest(client);
    return;
  }

  if (requestLine.indexOf("POST /op=qr1") != -1) {
    Serial.println("received qr1");
    receiveQr(client, 1);
    printMessage("OPERATION", "POST /op=qr2", true, false);
  } else if (requestLine.indexOf("POST /op=qr2") != -1) {
    Serial.println("received qr2");
    receiveQr(client, 2);
    printMessage("OPERATION", "POST /op=qr2", true, false);
  } else if (requestLine.indexOf("POST /op=w_2") != -1) {
    printMessage("OPERATION", "POST /op=w_2", true, false);
    postOp2(client);
  } else if (requestLine.indexOf("POST /op=wifi") != -1) {
    printMessage("OPERATION", "POST /op=wifi", true, false);
    postWifi(client);
  } else if (requestLine.indexOf("POST /op=restart") != -1) {
    printMessage("OPERATION", "POST /op=restart", true, false);
    postRestart(client);
  } else if (requestLine.indexOf("POST /op=w_3") != -1) {
    printMessage("OPERATION", "POST /op=w_3", true, false);
    postOp3(client);
  } else if (requestLine.indexOf("GET /op=status") != -1) {
    status(client);
  } else if (requestLine.indexOf("GET /op=r_1") != -1) {
    printMessage("OPERATION", "GET /op=r_1", true, false);
    getOp1(client);
  } else if (requestLine.indexOf("GET /op=r_2") != -1) {
    printMessage("OPERATION", "GET /op=r_2", true, false);
    getOp2(client);
  } else if (requestLine.indexOf("GET /op=r_3_1") != -1) {
    printMessage("OPERATION", "GET /op=r_3_1", true, false);
    getOp3wh1(client);
  } else if (requestLine.indexOf("GET /op=r_3_2") != -1) {
    printMessage("OPERATION", "GET /op=r_3_2", true, false);
    getOp3wh2(client);
  } else {
    sendResponse(client, 404, "application/json", "{\"status\": \"error\", \"message\": \"Risorsa non trovata\"}");
  }

  printMessage("WAIT", "wait operation", false, false);
  client.stop();
}

void setup() {
  Serial.begin(115200);

  setupDisplay();

  if (!SD.begin(chipSelect)) {
    printMessage("ERROR", "error init SD", false, false);
  } else {
    printMessage("SETUP", "init SD", false, false);
  }

  csvConfig();

  netConfig();

  wifiConfig();

  /**richieste a m5stack**/
  sendPostRequest("m5stack-0-x.local", "/op=startx", "start");
  //sendPostRequest("m5stack-y-z.local", "/op=startyz", "start");

  sendGetRequest("m5stack-0-x.local", "/op=maxstepx", "");
  //sendGetRequest("m5stack-y-z.local", "/op=maxstepy", "");
  //sendGetRequest("m5stack-y-z.local", "/op=maxstepz", "");
}

void loop() {
  mdns.run();
  WiFiClient client = server.available();  // by-value
  if (client) {
    handleClient(client);
  }
}
