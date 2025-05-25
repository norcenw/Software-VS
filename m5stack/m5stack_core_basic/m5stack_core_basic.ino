#include <Arduino.h>
#include <M5Stack.h>
#include <stdlib.h>
#include <ESPmDNS.h>
//#include "FastLED.h"

#include "WiFi.h"
#include "utility/MPU9250.h"

extern const unsigned char gImage_logo[];
extern const unsigned char m5stack_startup_music[];

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

MPU9250 IMU;

WiFiServer server(80);


/*finecorsa*/
const int endRun1 = 21;
const int endRun2 = 22;

/* motori */
const int DIR_PIN = 2;
const int STEP_PIN = 5;
const int ENA_PIN = 17;

unsigned long COUNTER = 0;
unsigned long FINAL_COUNTER = 0;

int THIS_ZONE = 0;
int THIS_ZONE_STEP = 0;
int THIS_DELAY = 0;

int x = 0;

// Percentuale fissa per rampa (15%)
const unsigned int RAMP_PERCENT = 2;

void forwardMotor(unsigned long steps, unsigned long stepDelay) {
  // 1) protezione base
  if (steps < 2 || stepDelay < 1) return;

  // 2) calcolo rampSteps come 15% di steps
  unsigned long rampSteps = (steps * RAMP_PERCENT) / 100;
  // se 2*rampSteps supera il totale, riduco a metà
  if (rampSteps * 2 > steps) {
    rampSteps = steps / 2;
  }
  // almeno 1 passo
  rampSteps = max(1UL, rampSteps);

  // 3) passi centrali (plateau)
  unsigned long middleSteps = steps - 2 * rampSteps;

  // 4) prevenzione divisione per zero
  unsigned long rampCounter = stepDelay / rampSteps;
  if (rampCounter < 1) rampCounter = 1;

  // 5) delay iniziale doppio (massimo) che scenderà a stepDelay
  unsigned long rampDelay = stepDelay * 2;

  // 6) setup motore
  digitalWrite(ENA_PIN, LOW);
  digitalWrite(DIR_PIN, LOW);
  delay(20);

  // 7) accelerazione (primi rampSteps)
  for (unsigned long i = 0; i < rampSteps; i++) {
    if (digitalRead(endRun1) == LOW || digitalRead(endRun2) == LOW) {
      stopMotor();
      return;
    }
    COUNTER++;
    Serial.println(COUNTER);
    digitalWrite(STEP_PIN, HIGH);
    delay(rampDelay);
    digitalWrite(STEP_PIN, LOW);
    delay(rampDelay);
    // riduco rampDelay verso stepDelay
    rampDelay = (rampDelay > stepDelay + rampCounter)
                  ? rampDelay - rampCounter
                  : stepDelay;
  }

  // 8) plateau (middleSteps)
  for (unsigned long i = 0; i < middleSteps; i++) {
    if (digitalRead(endRun1) == LOW || digitalRead(endRun2) == LOW) {
      stopMotor();
      return;
    }
    COUNTER++;
    Serial.println(COUNTER);
    digitalWrite(STEP_PIN, HIGH);
    delay(stepDelay);
    digitalWrite(STEP_PIN, LOW);
    delay(stepDelay);
  }

  // 9) decelerazione (ultimi rampSteps)
  for (unsigned long i = 0; i < rampSteps; i++) {
    if (digitalRead(endRun1) == LOW || digitalRead(endRun2) == LOW) {
      stopMotor();
      return;
    }
    COUNTER++;
    Serial.println(COUNTER);
    digitalWrite(STEP_PIN, HIGH);
    delay(rampDelay);
    digitalWrite(STEP_PIN, LOW);
    delay(rampDelay);
    // aumento rampDelay verso 2×stepDelay
    rampDelay += rampCounter;
  }

  // 10) stop motore
  digitalWrite(ENA_PIN, HIGH);
  delay(1000);
}

void backMotor(unsigned long steps, unsigned long stepDelay) {
  // 1) protezione base
  if (steps < 2 || stepDelay < 1) return;

  // 2) calcolo rampSteps come 15% di steps
  unsigned long rampSteps = (steps * RAMP_PERCENT) / 100;
  if (rampSteps * 2 > steps) {
    rampSteps = steps / 2;
  }
  rampSteps = max(1UL, rampSteps);

  // 3) passi centrali (plateau)
  unsigned long middleSteps = steps - 2 * rampSteps;

  // 4) prevenzione divisione per zero
  unsigned long rampCounter = stepDelay / rampSteps;
  if (rampCounter < 1) rampCounter = 1;

  // 5) delay iniziale doppio (massimo)
  unsigned long rampDelay = stepDelay * 2;

  // 6) setup motore (inversione)
  digitalWrite(ENA_PIN, LOW);
  digitalWrite(DIR_PIN, HIGH);
  delay(20);

  // 7) accelerazione (primi rampSteps, decremento COUNTER)
  for (unsigned long i = 0; i < rampSteps; i++) {
    if (digitalRead(endRun1) == LOW || digitalRead(endRun2) == LOW) {
      stopMotor();
      return;
    }
    COUNTER--;
    Serial.println(COUNTER);
    digitalWrite(STEP_PIN, HIGH);
    delay(rampDelay);
    digitalWrite(STEP_PIN, LOW);
    delay(rampDelay);
    rampDelay = (rampDelay > stepDelay + rampCounter)
                  ? rampDelay - rampCounter
                  : stepDelay;
  }

  // 8) plateau (middleSteps)
  for (unsigned long i = 0; i < middleSteps; i++) {
    if (digitalRead(endRun1) == LOW || digitalRead(endRun2) == LOW) {
      stopMotor();
      return;
    }
    COUNTER--;
    Serial.println(COUNTER);
    digitalWrite(STEP_PIN, HIGH);
    delay(stepDelay);
    digitalWrite(STEP_PIN, LOW);
    delay(stepDelay);
  }

  // 9) decelerazione (ultimi rampSteps)
  for (unsigned long i = 0; i < rampSteps; i++) {
    if (digitalRead(endRun1) == LOW || digitalRead(endRun2) == LOW) {
      stopMotor();
      return;
    }
    COUNTER--;
    Serial.println(COUNTER);
    digitalWrite(STEP_PIN, HIGH);
    delay(rampDelay);
    digitalWrite(STEP_PIN, LOW);
    delay(rampDelay);
    rampDelay += rampCounter;
  }

  // 10) stop motore
  digitalWrite(ENA_PIN, HIGH);
  delay(1000);
}

/// Ferma il driver
void stopMotor() {
  // disabilita subito
  digitalWrite(ENA_PIN, HIGH);
}

void configStart(WiFiClient &client) {
  bool CYCLE = true;
  digitalWrite(ENA_PIN, LOW);  /* accende driver */
  digitalWrite(DIR_PIN, HIGH); /* gira all'indietro */
  delay(20);

  while (CYCLE) {
    if (digitalRead(endRun1) == LOW || digitalRead(endRun2) == LOW) {
      stopMotor();
      CYCLE = false;
    }
    digitalWrite(STEP_PIN, HIGH);
    delay(10);
    digitalWrite(STEP_PIN, LOW);
    delay(10);
  }

  delay(1000);

  CYCLE = true;
  digitalWrite(ENA_PIN, LOW); /* accende driver */
  digitalWrite(DIR_PIN, LOW); /* gira in avanti */
  delay(20);

  while (CYCLE) {
    if (digitalRead(endRun1) == LOW || digitalRead(endRun2) == LOW) {
      stopMotor();
      CYCLE = false;
    }
    COUNTER++;
    digitalWrite(STEP_PIN, HIGH);
    delay(10);
    digitalWrite(STEP_PIN, LOW);
    delay(10);
  }

  FINAL_COUNTER = COUNTER;
  Serial.print("lunghezza totale: ");
  Serial.println(FINAL_COUNTER);

  THIS_ZONE_STEP = calcolateDistance(0, false);
  delay(1000);

  backMotor(FINAL_COUNTER - THIS_ZONE_STEP, 10);
  client.print("received");
}

int calcolateDistance(int POSITION, bool TYPE) {
  int PARTIAL_DISTANCE = FINAL_COUNTER / 9;
  int CENTER_DISTANCE = PARTIAL_DISTANCE / 2;
  int DISTANCE = (PARTIAL_DISTANCE * POSITION);
  if (!TYPE) {
    DISTANCE = DISTANCE + CENTER_DISTANCE;
  }
  Serial.print("calcolate distance: ");
  Serial.println(DISTANCE);
  return DISTANCE;
}

void configPin() {
  /* finecorsa */
  pinMode(endRun1, INPUT_PULLUP);
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.println("config pin limit switches 21");
  Serial.println("config pin limit switches 21");
  pinMode(endRun2, INPUT_PULLUP);
  M5.Lcd.setCursor(0, 30);
  M5.Lcd.println("config pin limit switches 22");
  Serial.println("config pin limit switches 22");

  /* motori */
  pinMode(DIR_PIN, OUTPUT);
  M5.Lcd.setCursor(0, 40);
  M5.Lcd.println("config pin DIR_PIN 2");
  Serial.println("config pin limit switches 2");
  pinMode(STEP_PIN, OUTPUT);
  M5.Lcd.setCursor(0, 50);
  M5.Lcd.println("config pin STEP_PIN 5");
  Serial.println("config pin STEP_PIN 5");
  M5.Lcd.setCursor(0, 60);
  pinMode(ENA_PIN, OUTPUT);
  M5.Lcd.println("config pin ENA_PIN 17");
  Serial.println("config pin ENA_PIN 17");
}

void startupLogo() {
  static uint8_t brightness, pre_brightness;
  uint32_t length = strlen((char *)m5stack_startup_music);
  M5.Lcd.setBrightness(0);
  M5.Lcd.pushImage(0, 0, 320, 240, (uint16_t *)gImage_logo);
  for (int i = 0; i < length; i++) {
    //dacWrite(SPEAKER_PIN, m5stack_startup_music[i] >> 2);
    delayMicroseconds(40);
    brightness = (i / 157);
    if (pre_brightness != brightness) {
      pre_brightness = brightness;
      M5.Lcd.setBrightness(brightness);
    }
  }

  /* for (int i = 255; i >= 0; i--) {
    M5.Lcd.setBrightness(i);
    if (i <= 32) {
      dacWrite(SPEAKER_PIN, i);
    }
    delay(10);
  } */

  M5.Lcd.fillScreen(BLACK);
  delay(800);
}

// TF card test
void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);
  M5.Lcd.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    M5.Lcd.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    M5.Lcd.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      M5.Lcd.print("  DIR : ");
      Serial.println(file.name());
      M5.Lcd.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      M5.Lcd.print("  FILE: ");
      Serial.print(file.name());
      M5.Lcd.print(file.name());
      Serial.print("  SIZE: ");
      M5.Lcd.print("  SIZE: ");
      Serial.println(file.size());
      M5.Lcd.println(file.size());
    }
    file = root.openNextFile();
  }
}

void readFile(fs::FS &fs, const char *path) {
  Serial.printf("Reading file: %s\n", path);
  M5.Lcd.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    M5.Lcd.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  M5.Lcd.print("Read from file: ");
  while (file.available()) {
    int ch = file.read();
    Serial.write(ch);
    M5.Lcd.write(ch);
  }
}

void writeFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Writing file: %s\n", path);
  M5.Lcd.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    M5.Lcd.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
    M5.Lcd.println("File written");
  } else {
    Serial.println("Write failed");
    M5.Lcd.println("Write failed");
  }
}

void buttons_test() {
  M5.update();
  if (M5.BtnA.isPressed()) {
    stopMotor();
    digitalWrite(ENA_PIN, LOW);
    digitalWrite(DIR_PIN, HIGH);
    delay(20);

    // Giro finché tieni premuto A
    while (M5.BtnA.isPressed()) {
      M5.update();  // IMPORTANTE!
      if (digitalRead(endRun1) == LOW || digitalRead(endRun2) == LOW) {
        break;
      }
      digitalWrite(STEP_PIN, HIGH);
      delay(10);
      digitalWrite(STEP_PIN, LOW);
      delay(10);
    }
    stopMotor();
    return;
  }

  // Pulsante B: stop istantaneo o dopo 1s
  M5.update();
  if (M5.BtnB.wasReleased() || M5.BtnB.pressedFor(1000, 200)) {
    stopMotor();
    return;
  }

  // Giro finché tieni premuto C
  M5.update();
  if (M5.BtnC.isPressed()) {
    stopMotor();
    stopMotor();
    digitalWrite(ENA_PIN, LOW);
    digitalWrite(DIR_PIN, LOW);
    delay(20);

    while (M5.BtnC.isPressed()) {
      M5.update();
      if (digitalRead(endRun1) == LOW || digitalRead(endRun2) == LOW) {
        break;
      }
      digitalWrite(STEP_PIN, HIGH);
      delay(10);
      digitalWrite(STEP_PIN, LOW);
      delay(10);
    }
    stopMotor();
    return;
  }
}

static byte c1;

byte utf8ascii(byte ascii) {
  if (ascii < 128)  // Standard ASCII-set 0..0x7F handling
  {
    c1 = 0;
    return (ascii);
  }

  // get previous input
  byte last = c1;  // get last char
  c1 = ascii;      // remember actual character

  switch (last)  // conversion depending on first UTF8-character
  {
    case 0xC2:
      return (ascii);
      break;
    case 0xC3:
      return (ascii | 0xC0);
      break;
    case 0x82:
      if (ascii == 0xAC) return (0x80);  // special case Euro-symbol
  }

  return (0);  // otherwise: return zero, if character has to be ignored
}

String utf8ascii(String s) {
  String r = "";
  char c;
  for (int i = 0; i < s.length(); i++) {
    c = utf8ascii(s.charAt(i));
    if (c != 0) r += c;
  }
  return r;
}

/*************************************/
/*************************************/
/*************************************/
/* connesione al wifi all'accensione */
/*************************************/
/*************************************/
/*************************************/

void wifi_scan() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.println("scan start");
  M5.Lcd.println("scan start");

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  M5.Lcd.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
    M5.Lcd.println("no networks found");
  } else {
    Serial.print(n);
    M5.Lcd.print(n);
    Serial.println(" networks found");
    M5.Lcd.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      M5.Lcd.print(i + 1);
      Serial.print(": ");
      M5.Lcd.print(": ");
      Serial.print(WiFi.SSID(i).c_str());
      M5.Lcd.print(utf8ascii(WiFi.SSID(i).c_str()));
      Serial.print(" (");
      M5.Lcd.print(" (");
      Serial.print(WiFi.RSSI(i));
      M5.Lcd.print(WiFi.RSSI(i));
      Serial.print(")");
      M5.Lcd.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " "
                                                                : "*");
      M5.Lcd.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " "
                                                                : "*");
      delay(5);
    }
  }
  Serial.println("");
  M5.Lcd.println("");
}

void wifi_config(fs::FS &fs) {
  int numNetworks = WiFi.scanNetworks();

  File file = fs.open("/wifi.csv", FILE_READ);
  if (!file) {
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
        WiFi.begin(ssid.c_str(), password.c_str());

        Serial.println("Trying: " + ssid);
        Serial.println("Password: " + password);

        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 30) {
          delay(1000);
          attempts++;
        }

        if (WiFi.status() == WL_CONNECTED) {
          delay(2000);  // Tempo per DHCP

          IPAddress localIP = WiFi.localIP();
          Serial.print("Connected. IP: ");
          Serial.println(localIP);

          // Se IP è ancora 0.0.0.0, riprova DHCP
          if (localIP.toString() == "0.0.0.0") {
            WiFi.disconnect();
            delay(1000);
            WiFi.begin(ssid.c_str(), password.c_str());
            delay(5000);
            localIP = WiFi.localIP();
            Serial.print("New IP: ");
            Serial.println(localIP);
          }

          // Imposta DNS (non hostname .local ma risolutore DNS)
          IPAddress dnsIP(8, 8, 8, 8);  // DNS Google come esempio
          WiFi.config(WiFi.localIP(), WiFi.gatewayIP(), WiFi.subnetMask(), dnsIP);

          // Avvia server e mDNS
          server.begin();

          String hostname = "m5stack-0-x";  // Fisso come richiesto
          if (MDNS.begin(hostname.c_str())) {
            Serial.println("mDNS avviato come " + hostname);
            MDNS.addService("http", "tcp", 80);
          }

          connected = true;
          break;
        }
      }
    }

    if (connected) break;
  }

  file.close();
}

void wifi_setup() {
  wifi_scan();
  wifi_config(SD);
}

/*******************************/
/*******************************/
/*******************************/
/* test schermo all'accensione */
/*******************************/
/*******************************/
/*******************************/

unsigned long testLines(uint16_t color) {
  unsigned long start, t;
  int x1, y1, x2, y2, w = M5.Lcd.width(), h = M5.Lcd.height();

  M5.Lcd.fillScreen(TFT_BLACK);

  x1 = y1 = 0;
  y2 = h - 1;
  start = micros();
  for (x2 = 0; x2 < w; x2 += 6) M5.Lcd.drawLine(x1, y1, x2, y2, color);
  x2 = w - 1;
  for (y2 = 0; y2 < h; y2 += 6) M5.Lcd.drawLine(x1, y1, x2, y2, color);
  t = micros() - start;  // fillScreen doesn't count against timing

  M5.Lcd.fillScreen(TFT_BLACK);

  x1 = w - 1;
  y1 = 0;
  y2 = h - 1;
  start = micros();
  for (x2 = 0; x2 < w; x2 += 6) M5.Lcd.drawLine(x1, y1, x2, y2, color);
  x2 = 0;
  for (y2 = 0; y2 < h; y2 += 6) M5.Lcd.drawLine(x1, y1, x2, y2, color);
  t += micros() - start;

  M5.Lcd.fillScreen(TFT_BLACK);

  x1 = 0;
  y1 = h - 1;
  y2 = 0;
  start = micros();
  for (x2 = 0; x2 < w; x2 += 6) M5.Lcd.drawLine(x1, y1, x2, y2, color);
  x2 = w - 1;
  for (y2 = 0; y2 < h; y2 += 6) M5.Lcd.drawLine(x1, y1, x2, y2, color);
  t += micros() - start;

  M5.Lcd.fillScreen(TFT_BLACK);

  x1 = w - 1;
  y1 = h - 1;
  y2 = 0;
  start = micros();
  for (x2 = 0; x2 < w; x2 += 6) M5.Lcd.drawLine(x1, y1, x2, y2, color);
  x2 = 0;
  for (y2 = 0; y2 < h; y2 += 6) M5.Lcd.drawLine(x1, y1, x2, y2, color);

  return micros() - start;
}

unsigned long testFastLines(uint16_t color1, uint16_t color2) {
  unsigned long start;
  int x, y, w = M5.Lcd.width(), h = M5.Lcd.height();

  M5.Lcd.fillScreen(TFT_BLACK);
  start = micros();
  for (y = 0; y < h; y += 5) M5.Lcd.drawFastHLine(0, y, w, color1);
  for (x = 0; x < w; x += 5) M5.Lcd.drawFastVLine(x, 0, h, color2);

  return micros() - start;
}

unsigned long testRects(uint16_t color) {
  unsigned long start;
  int n, i, i2, cx = M5.Lcd.width() / 2, cy = M5.Lcd.height() / 2;

  M5.Lcd.fillScreen(TFT_BLACK);
  n = min(M5.Lcd.width(), M5.Lcd.height());
  start = micros();
  for (i = 2; i < n; i += 6) {
    i2 = i / 2;
    M5.Lcd.drawRect(cx - i2, cy - i2, i, i, color);
  }

  return micros() - start;
}

unsigned long testFilledRects(uint16_t color1, uint16_t color2) {
  unsigned long start, t = 0;
  int n, i, i2, cx = M5.Lcd.width() / 2 - 1, cy = M5.Lcd.height() / 2 - 1;

  M5.Lcd.fillScreen(TFT_BLACK);
  n = min(M5.Lcd.width(), M5.Lcd.height());
  for (i = n - 1; i > 0; i -= 6) {
    i2 = i / 2;
    start = micros();
    M5.Lcd.fillRect(cx - i2, cy - i2, i, i, color1);
    t += micros() - start;
    // Outlines are not included in timing results
    M5.Lcd.drawRect(cx - i2, cy - i2, i, i, color2);
  }

  return t;
}

unsigned long testFilledCircles(uint8_t radius, uint16_t color) {
  unsigned long start;
  int x, y, w = M5.Lcd.width(), h = M5.Lcd.height(), r2 = radius * 2;

  M5.Lcd.fillScreen(TFT_BLACK);
  start = micros();
  for (x = radius; x < w; x += r2) {
    for (y = radius; y < h; y += r2) {
      M5.Lcd.fillCircle(x, y, radius, color);
    }
  }

  return micros() - start;
}

unsigned long testCircles(uint8_t radius, uint16_t color) {
  unsigned long start;
  int x, y, r2 = radius * 2, w = M5.Lcd.width() + radius,
            h = M5.Lcd.height() + radius;

  // Screen is not cleared for this one -- this is
  // intentional and does not affect the reported time.
  start = micros();
  for (x = 0; x < w; x += r2) {
    for (y = 0; y < h; y += r2) {
      M5.Lcd.drawCircle(x, y, radius, color);
    }
  }

  return micros() - start;
}

unsigned long testTriangles() {
  unsigned long start;
  int n, i, cx = M5.Lcd.width() / 2 - 1, cy = M5.Lcd.height() / 2 - 1;

  M5.Lcd.fillScreen(TFT_BLACK);
  n = min(cx, cy);
  start = micros();
  for (i = 0; i < n; i += 5) {
    M5.Lcd.drawTriangle(cx, cy - i,      // peak
                        cx - i, cy + i,  // bottom left
                        cx + i, cy + i,  // bottom right
                        M5.Lcd.color565(0, 0, i));
  }

  return micros() - start;
}

unsigned long testFilledTriangles() {
  unsigned long start, t = 0;
  int i, cx = M5.Lcd.width() / 2 - 1, cy = M5.Lcd.height() / 2 - 1;

  M5.Lcd.fillScreen(TFT_BLACK);
  start = micros();
  for (i = min(cx, cy); i > 10; i -= 5) {
    start = micros();
    M5.Lcd.fillTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i,
                        M5.Lcd.color565(0, i, i));
    t += micros() - start;
    M5.Lcd.drawTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i,
                        M5.Lcd.color565(i, i, 0));
  }

  return t;
}

unsigned long testRoundRects() {
  unsigned long start;
  int w, i, i2, cx = M5.Lcd.width() / 2 - 1, cy = M5.Lcd.height() / 2 - 1;

  M5.Lcd.fillScreen(TFT_BLACK);
  w = min(M5.Lcd.width(), M5.Lcd.height());
  start = micros();
  for (i = 0; i < w; i += 6) {
    i2 = i / 2;
    M5.Lcd.drawRoundRect(cx - i2, cy - i2, i, i, i / 8,
                         M5.Lcd.color565(i, 0, 0));
  }

  return micros() - start;
}

unsigned long testFilledRoundRects() {
  unsigned long start;
  int i, i2, cx = M5.Lcd.width() / 2 - 1, cy = M5.Lcd.height() / 2 - 1;

  M5.Lcd.fillScreen(TFT_BLACK);
  start = micros();
  for (i = min(M5.Lcd.width(), M5.Lcd.height()); i > 20; i -= 6) {
    i2 = i / 2;
    M5.Lcd.fillRoundRect(cx - i2, cy - i2, i, i, i / 8,
                         M5.Lcd.color565(0, i, 0));
  }

  return micros() - start;
}

/*******************************/
/*******************************/
/*******************************/
/*       handle request        */
/*******************************/
/*******************************/
/*******************************/
void sendResponse(WiFiClient &client, int statusCode, String contentType, String message) {
  client.println("HTTP/1.1 " + String(statusCode) + " OK");
  client.println("Content-Type: " + contentType);
  client.println("Access-Control-Allow-Origin: *");
  client.println("Access-Control-Allow-Methods: GET, POST, OPTIONS");
  client.println("Access-Control-Allow-Headers: Content-Type");
  client.println();
  client.println(message);
}

void withdraw(WiFiClient &client, String body) {
  extract(body);
  THIS_ZONE_STEP = calcolateDistance(THIS_ZONE, true);
  delay(1000);
  forwardMotor(THIS_ZONE_STEP, THIS_DELAY);
  client.print("received");
}

void handleClient(WiFiClient client) {
  if (!client || !client.connected()) return;
  String requestLine = client.readStringUntil('\r');
  client.readStringUntil('\n');
  int contentLength = 0;
  while (client.available()) {
    String headerLine = client.readStringUntil('\n');
    headerLine.trim(); 
    if (headerLine.startsWith("Content-Length:")) {
      contentLength = headerLine.substring(headerLine.indexOf(':') + 1).toInt();
    }
    if (headerLine.length() == 0) {
      break;
    }
  }

  String body = "";
  if (contentLength > 0) {
    const int BUF_SIZE = 256;
    int toRead = min(contentLength, BUF_SIZE - 1);
    char buf[BUF_SIZE];
    client.readBytes(buf, toRead);
    buf[toRead] = '\0';
    body = String(buf);
  }

  if (requestLine.indexOf("POST /op=startx") != -1) {
    Serial.println("configStart");
    configStart(client);
  } else if (requestLine.indexOf("GET /op=maxstepx") != -1) {
    client.print(FINAL_COUNTER);
  } else if (requestLine.indexOf("POST /op=withdrawx") != -1) {
    Serial.println("withdraw");
    withdraw(client, body);
  } else if (requestLine.indexOf("GET /op=returnx") != -1) {
    Serial.println("return");
    backMotor(FINAL_COUNTER - THIS_ZONE_STEP, THIS_DELAY);
    client.print("received");
  } else {
    sendResponse(client,404, "application/json","{\"status\":\"error\",\"message\":\"Risorsa non trovata\"}");
  }

  client.stop();
}

void extract(const String &body) {
  int values[2] = { 0, 0 };  // temporanei per position e delay
  int idx = 0;               // indice del valore che stiamo riempiendo (0 o 1)
  bool inNumber = false;     // true se siamo in mezzo a una sequenza di cifre

  for (int i = 0; i < body.length() && idx < 2; ++i) {
    char c = body.charAt(i);
    if (isDigit(c)) {
      // accumulo la cifra
      inNumber = true;
      values[idx] = values[idx] * 10 + (c - '0');
    } else if (inNumber) {
      // ho finito il numero corrente, passo al prossimo
      inNumber = false;
      idx++;
    }
  }

  // assegno ai globali
  THIS_ZONE = values[0];
  THIS_DELAY = values[1];
}

/*******************************/
/*******************************/
/*******************************/
/*         setup e loop        */
/*******************************/
/*******************************/
/*******************************/

void start() {
  // gpio test
  // pinMode(BUTTON_A_PIN, INPUT_PULLUP);
  // if(digitalRead(BUTTON_A_PIN) == 0) {
  //     gpio_test_flg = 1;
  // }

  // if (gpio_test_flg) {
  //     GPIO_test();
  // }

  // initialize the M5Stack object
  M5.begin();

  /*
      Power chip connected to gpio21, gpio22, I2C device
      Set battery charging voltage and current
      If used battery, please call this function in your project
    */
  M5.Power.begin();

  // dac test
  // if (gpio_test_flg)
  // {
  //     adc_test();
  // }
  startupLogo();
  // ledBar();
  Wire.begin();

  // Lcd display
  M5.Lcd.setBrightness(100);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(1);
  M5.Lcd.printf("Display Test!");
  delay(300);

  M5.Lcd.fillScreen(WHITE);
  delay(150);
  M5.Lcd.fillScreen(RED);
  delay(150);
  M5.Lcd.fillScreen(GREEN);
  delay(150);
  M5.Lcd.fillScreen(BLUE);
  delay(150);
  M5.Lcd.fillScreen(BLACK);
  delay(150);

  // draw graphic
  yield();
  Serial.print(F("Lines                    "));
  yield();
  Serial.println(testLines(TFT_CYAN));
  // total+=testLines(TFT_CYAN);
  // delay(500);

  yield();
  Serial.print(F("Horiz/Vert Lines         "));
  yield();
  Serial.println(testFastLines(TFT_RED, TFT_BLUE));
  // total+=testFastLines(TFT_RED, TFT_BLUE);
  // delay(500);

  yield();
  Serial.print(F("Rectangles (outline)     "));
  yield();
  Serial.println(testRects(TFT_GREEN));
  // total+=testRects(TFT_GREEN);
  // delay(500);

  yield();
  Serial.print(F("Rectangles (filled)      "));
  yield();
  Serial.println(testFilledRects(TFT_YELLOW, TFT_MAGENTA));
  // total+=testFilledRects(TFT_YELLOW, TFT_MAGENTA);
  // delay(500);

  yield();
  Serial.print(F("Circles (filled)         "));
  yield();
  Serial.println(testFilledCircles(10, TFT_MAGENTA));
  // total+= testFilledCircles(10, TFT_MAGENTA);

  yield();
  Serial.print(F("Circles (outline)        "));
  yield();
  Serial.println(testCircles(10, TFT_WHITE));
  // total+=testCircles(10, TFT_WHITE);
  // delay(500);

  yield();
  Serial.print(F("Triangles (outline)      "));
  yield();
  Serial.println(testTriangles());
  // total+=testTriangles();
  // delay(500);

  yield();
  Serial.print(F("Triangles (filled)       "));
  yield();
  Serial.println(testFilledTriangles());
  // total += testFilledTriangles();
  // delay(500);

  yield();
  Serial.print(F("Rounded rects (outline)  "));
  yield();
  Serial.println(testRoundRects());
  // total+=testRoundRects();
  // delay(500);

  yield();
  Serial.print(F("Rounded rects (filled)   "));
  yield();
  Serial.println(testFilledRoundRects());
  // total+=testFilledRoundRects();
  // delay(500);

  yield();
  Serial.println(F("Done!"));
  yield();

  // rand draw
  int i = 250;
  while (--i) {
    M5.Lcd.fillTriangle(
      random(M5.Lcd.width() - 1), random(M5.Lcd.height() - 1),
      random(M5.Lcd.width() - 1), random(M5.Lcd.height() - 1),
      random(M5.Lcd.width() - 1), random(M5.Lcd.height() - 1),
      random(0xfffe));
  }
  for (int i = 255; i >= 0; i--) {
    M5.Lcd.setBrightness(i);
    delay(2);
  }

  // wifi test
  M5.Lcd.setCursor(0, 10);
  M5.Lcd.fillScreen(BLACK);
  for (int i = 0; i < 200; i++) {
    M5.Lcd.setBrightness(i);
    delay(2);
  }

  byte c = IMU.readByte(MPU9250_ADDRESS, WHO_AM_I_MPU9250);
  Serial.print("MPU9250 ");
  Serial.print("I AM ");
  Serial.print(c, HEX);
  Serial.print(" I should be ");
  Serial.println(0x71, HEX);
  Serial.println("");
  M5.Lcd.setCursor(20, 0);
  M5.Lcd.print("MPU9250");
  M5.Lcd.setCursor(0, 10);
  M5.Lcd.print("I AM");
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.print(c, HEX);
  M5.Lcd.setCursor(0, 30);
  M5.Lcd.print("I Should Be");
  M5.Lcd.setCursor(0, 40);
  M5.Lcd.println(0x71, HEX);
  M5.Lcd.println();
  delay(100);

  IMU.initMPU9250();
  // Initialize device for active mode read of acclerometer, gyroscope, and
  // temperature
  Serial.println("MPU9250 initialized for active data mode....");

  // Read the WHO_AM_I register of the magnetometer, this is a good test of
  // communication
  byte d = IMU.readByte(AK8963_ADDRESS, WHO_AM_I_AK8963);
  Serial.print("AK8963 ");
  Serial.print("I AM ");
  Serial.print(d, HEX);
  Serial.print(" I should be ");
  Serial.println(0x48, HEX);

  // M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(20, 100);
  M5.Lcd.print("AK8963");
  M5.Lcd.setCursor(0, 110);
  M5.Lcd.print("I AM");
  M5.Lcd.setCursor(0, 120);
  M5.Lcd.print(d, HEX);
  M5.Lcd.setCursor(0, 130);
  M5.Lcd.print("I Should Be");
  M5.Lcd.setCursor(0, 140);
  M5.Lcd.print(0x48, HEX);
  delay(1000);

  M5.Lcd.setCursor(0, 0);
  M5.Lcd.println("wifi test:");
  M5.Lcd.fillScreen(BLACK);
  wifi_setup();
  delay(2000);

  // TF card test
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 10);
  //M5.Lcd.printf("TF card test:\r\n");
  // digitalWrite(TFT_CS, 1);
  /* listDir(SD, "/", 0);
  writeFile(SD, "/hello.txt", "Hello world");
  readFile(SD, "/hello.txt"); */

  //configuration Pin
}

// the setup routine runs once when M5Stack starts up
void setup() {
  start();
  configPin();
}

// the loop routine runs over and over again forever
void loop() {
  buttons_test();
  M5.update();
  WiFiClient client = server.available();
  if (client) {
    handleClient(client);
  }
}