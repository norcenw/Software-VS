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

bool motorYOK = false;
bool motorZOK = false;
bool limitY1OK = false;
bool limitY2OK = false;
bool limitZ1OK = false;
bool limitZ2OK = false;

/*finecorsa*/
const int END_RUN_Y_1 = 21;
const int END_RUN_Y_2 = 22;
const int END_RUN_Z_1 = 34;
const int END_RUN_Z_2 = 35;

/* motori */
const int DIR_PIN_Y = 2;
const int STEP_PIN_Y = 5;
const int ENA_PIN_Y = 17;

const int DIR_PIN_Z = 25;
const int STEP_PIN_Z = 26;
const int ENA_PIN_Z = 33;

unsigned long COUNTER_Y = 0;
unsigned long FINAL_COUNTER_Y = 0;

unsigned long COUNTER_Z = 0;
unsigned long FINAL_COUNTER_Z = 0;

int THIS_ZONE_Y = 0;
int THIS_ZONE_STEP_Y = 0;
int THIS_DELAY_Y = 0;

int THIS_ZONE_Z = 0;
int THIS_ZONE_STEP_Z = 0;
int THIS_DELAY_Z = 0;

int x = 0;
int y = 0;
int z = 0;

bool controlAxis = true;

String wifi[99][2];

const unsigned int RAMP_PERCENT = 2;

void forwardMotorY(unsigned long steps, unsigned long stepDelay) {
  drawStatusGrid(true, motorZOK, limitY1OK, limitY2OK, limitZ1OK, limitZ2OK);
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
  digitalWrite(ENA_PIN_Y, LOW);
  digitalWrite(DIR_PIN_Y, LOW);
  delay(20);

  // 7) accelerazione (primi rampSteps)
  for (unsigned long i = 0; i < rampSteps; i++) {
    if (digitalRead(END_RUN_Y_1) == LOW || digitalRead(END_RUN_Y_2) == LOW) {
      stopMotorY();
      return;
    }
    COUNTER_Y++;
    Serial.println(COUNTER_Y);
    digitalWrite(STEP_PIN_Y, HIGH);
    delay(rampDelay);
    digitalWrite(STEP_PIN_Y, LOW);
    delay(rampDelay);
    // riduco rampDelay verso stepDelay
    rampDelay = (rampDelay > stepDelay + rampCounter)
                  ? rampDelay - rampCounter
                  : stepDelay;
  }

  // 8) plateau (middleSteps)
  for (unsigned long i = 0; i < middleSteps; i++) {
    if (digitalRead(END_RUN_Y_1) == LOW || digitalRead(END_RUN_Y_2) == LOW) {
      stopMotorY();
      return;
    }
    COUNTER_Y++;
    Serial.println(COUNTER_Y);
    digitalWrite(STEP_PIN_Y, HIGH);
    delay(stepDelay);
    digitalWrite(STEP_PIN_Y, LOW);
    delay(stepDelay);
  }

  // 9) decelerazione (ultimi rampSteps)
  for (unsigned long i = 0; i < rampSteps; i++) {
    if (digitalRead(END_RUN_Y_1) == LOW || digitalRead(END_RUN_Y_2) == LOW) {
      stopMotorY();
      return;
    }
    COUNTER_Y++;
    Serial.println(COUNTER_Y);
    digitalWrite(STEP_PIN_Y, HIGH);
    delay(rampDelay);
    digitalWrite(STEP_PIN_Y, LOW);
    delay(rampDelay);
    // aumento rampDelay verso 2×stepDelay
    rampDelay += rampCounter;
  }

  // 10) stop motore
  digitalWrite(ENA_PIN_Y, HIGH);
  drawStatusGrid(false, motorZOK, limitY1OK, limitY2OK, limitZ1OK, limitZ2OK);
  delay(1000);
}

void backMotorY(unsigned long steps, unsigned long stepDelay) {
  drawStatusGrid(true, motorZOK, limitY1OK, limitY2OK, limitZ1OK, limitZ2OK);
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
  digitalWrite(ENA_PIN_Y, LOW);
  digitalWrite(DIR_PIN_Y, HIGH);
  delay(20);

  // 7) accelerazione (primi rampSteps)
  for (unsigned long i = 0; i < rampSteps; i++) {
    if (digitalRead(END_RUN_Y_1) == LOW || digitalRead(END_RUN_Y_2) == LOW) {
      stopMotorY();
      return;
    }
    COUNTER_Y++;
    Serial.println(COUNTER_Y);
    digitalWrite(STEP_PIN_Y, HIGH);
    delay(rampDelay);
    digitalWrite(STEP_PIN_Y, LOW);
    delay(rampDelay);
    // riduco rampDelay verso stepDelay
    rampDelay = (rampDelay > stepDelay + rampCounter)
                  ? rampDelay - rampCounter
                  : stepDelay;
  }

  // 8) plateau (middleSteps)
  for (unsigned long i = 0; i < middleSteps; i++) {
    if (digitalRead(END_RUN_Y_1) == LOW || digitalRead(END_RUN_Y_2) == LOW) {
      stopMotorY();
      return;
    }
    COUNTER_Y++;
    Serial.println(COUNTER_Y);
    digitalWrite(STEP_PIN_Y, HIGH);
    delay(stepDelay);
    digitalWrite(STEP_PIN_Y, LOW);
    delay(stepDelay);
  }

  // 9) decelerazione (ultimi rampSteps)
  for (unsigned long i = 0; i < rampSteps; i++) {
    if (digitalRead(END_RUN_Y_1) == LOW || digitalRead(END_RUN_Y_2) == LOW) {
      stopMotorY();
      return;
    }
    COUNTER_Y++;
    Serial.println(COUNTER_Y);
    digitalWrite(STEP_PIN_Y, HIGH);
    delay(rampDelay);
    digitalWrite(STEP_PIN_Y, LOW);
    delay(rampDelay);
    // aumento rampDelay verso 2×stepDelay
    rampDelay += rampCounter;
  }

  // 10) stop motore
  digitalWrite(ENA_PIN_Y, HIGH);
  drawStatusGrid(false, motorZOK, limitY1OK, limitY2OK, limitZ1OK, limitZ2OK);
  delay(1000);
}

void forwardMotorZ(unsigned long steps, unsigned long stepDelay) {
  drawStatusGrid(motorYOK, true, limitY1OK, limitY2OK, limitZ1OK, limitZ2OK);
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
  digitalWrite(ENA_PIN_Z, LOW);
  digitalWrite(DIR_PIN_Z, LOW);
  delay(20);

  // 7) accelerazione (primi rampSteps)
  for (unsigned long i = 0; i < rampSteps; i++) {
    if (digitalRead(END_RUN_Z_1) == LOW || digitalRead(END_RUN_Z_2) == LOW) {
      stopMotorZ();
      return;
    }
    COUNTER_Z++;
    Serial.println(COUNTER_Z);
    digitalWrite(STEP_PIN_Z, HIGH);
    delay(rampDelay);
    digitalWrite(STEP_PIN_Z, LOW);
    delay(rampDelay);
    // riduco rampDelay verso stepDelay
    rampDelay = (rampDelay > stepDelay + rampCounter)
                  ? rampDelay - rampCounter
                  : stepDelay;
  }

  // 8) plateau (middleSteps)
  for (unsigned long i = 0; i < middleSteps; i++) {
    if (digitalRead(END_RUN_Z_1) == LOW || digitalRead(END_RUN_Z_2) == LOW) {
      stopMotorZ();
      return;
    }
    COUNTER_Z++;
    Serial.println(COUNTER_Z);
    digitalWrite(STEP_PIN_Z, HIGH);
    delay(stepDelay);
    digitalWrite(STEP_PIN_Z, LOW);
    delay(stepDelay);
  }

  // 9) decelerazione (ultimi rampSteps)
  for (unsigned long i = 0; i < rampSteps; i++) {
    if (digitalRead(END_RUN_Z_1) == LOW || digitalRead(END_RUN_Z_2) == LOW) {
      stopMotorZ();
      return;
    }
    COUNTER_Z++;
    Serial.println(COUNTER_Z);
    digitalWrite(STEP_PIN_Z, HIGH);
    delay(rampDelay);
    digitalWrite(STEP_PIN_Z, LOW);
    delay(rampDelay);
    // aumento rampDelay verso 2×stepDelay
    rampDelay += rampCounter;
  }

  // 10) stop motore
  digitalWrite(ENA_PIN_Z, HIGH);
  drawStatusGrid(motorYOK, false, limitY1OK, limitY2OK, limitZ1OK, limitZ2OK);
  delay(1000);
}

void backMotorZ(unsigned long steps, unsigned long stepDelay) {
  drawStatusGrid(motorYOK, true, limitY1OK, limitY2OK, limitZ1OK, limitZ2OK);
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
  digitalWrite(ENA_PIN_Z, LOW);
  digitalWrite(DIR_PIN_Z, HIGH);
  delay(20);

  // 7) accelerazione (primi rampSteps)
  for (unsigned long i = 0; i < rampSteps; i++) {
    if (digitalRead(END_RUN_Z_1) == LOW || digitalRead(END_RUN_Z_2) == LOW) {
      stopMotorZ();
      return;
    }
    COUNTER_Z++;
    Serial.println(COUNTER_Z);
    digitalWrite(STEP_PIN_Z, HIGH);
    delay(rampDelay);
    digitalWrite(STEP_PIN_Z, LOW);
    delay(rampDelay);
    // riduco rampDelay verso stepDelay
    rampDelay = (rampDelay > stepDelay + rampCounter)
                  ? rampDelay - rampCounter
                  : stepDelay;
  }

  // 8) plateau (middleSteps)
  for (unsigned long i = 0; i < middleSteps; i++) {
    if (digitalRead(END_RUN_Z_1) == LOW || digitalRead(END_RUN_Z_2) == LOW) {
      stopMotorZ();
      return;
    }
    COUNTER_Z++;
    Serial.println(COUNTER_Z);
    digitalWrite(STEP_PIN_Z, HIGH);
    delay(stepDelay);
    digitalWrite(STEP_PIN_Z, LOW);
    delay(stepDelay);
  }

  // 9) decelerazione (ultimi rampSteps)
  for (unsigned long i = 0; i < rampSteps; i++) {
    if (digitalRead(END_RUN_Z_1) == LOW || digitalRead(END_RUN_Z_2) == LOW) {
      stopMotorZ();
      return;
    }
    COUNTER_Z++;
    Serial.println(COUNTER_Z);
    digitalWrite(STEP_PIN_Z, HIGH);
    delay(rampDelay);
    digitalWrite(STEP_PIN_Z, LOW);
    delay(rampDelay);
    // aumento rampDelay verso 2×stepDelay
    rampDelay += rampCounter;
  }

  // 10) stop motore
  digitalWrite(ENA_PIN_Z, HIGH);
  drawStatusGrid(motorYOK, false, limitY1OK, limitY2OK, limitZ1OK, limitZ2OK);
  delay(1000);
}

/// Ferma il driver
void stopMotorY() {
  // disabilita subito
  digitalWrite(ENA_PIN_Y, HIGH);
  if (digitalRead(END_RUN_Y_1) == LOW) {
    drawStatusGrid(motorYOK, motorZOK, true, limitY2OK, limitZ1OK, limitZ2OK);
  } else if (digitalRead(END_RUN_Y_2) == LOW) {
    drawStatusGrid(motorYOK, motorZOK, limitY1OK, true, limitZ1OK, limitZ2OK);
  }
}

void stopMotorZ() {
  // disabilita subito
  digitalWrite(ENA_PIN_Z, HIGH);
  if (digitalRead(END_RUN_Y_1) == LOW) {
    drawStatusGrid(motorYOK, motorZOK, limitY1OK, limitY2OK, true, limitZ2OK);
  } else if (digitalRead(END_RUN_Y_2) == LOW) {
    drawStatusGrid(motorYOK, motorZOK, limitY1OK, limitY2OK, limitZ1OK, true);
  }
}

void configStartY(WiFiClient &client) {
  digitalWrite(ENA_PIN_Y, LOW);  /* accende driver */
  digitalWrite(DIR_PIN_Y, HIGH); /* gira all'indietro */
  delay(20);

  drawStatusGrid(motorYOK, motorZOK, limitY1OK, limitY2OK, limitZ1OK, limitZ2OK);
  while (true) {
    // se uno dei due fine‐corsa è attivato (LOW)
    if (digitalRead(END_RUN_Y_1) == LOW || digitalRead(END_RUN_Y_2) == LOW) {
      // inverti la direzione
      digitalWrite(DIR_PIN_Y, LOW);
      delay(5);  // piccolo intervallo per la stabilità

      // continua a fare passi finché almeno un fine‐corsa resta LOW
      while (digitalRead(END_RUN_Y_1) == LOW || digitalRead(END_RUN_Y_2) == LOW) {
        digitalWrite(STEP_PIN_Y, HIGH);
        delay(10);
        digitalWrite(STEP_PIN_Y, LOW);
        delay(10);
      }

      // una volta tornati entrambi HIGH, ferma il motore e interrompi il ciclo
      stopMotorY();
      break;
    }

    // altrimenti continua nel verso “forward”
    digitalWrite(STEP_PIN_Y, HIGH);
    delay(10);
    digitalWrite(STEP_PIN_Y, LOW);
    delay(10);
  }

  delay(1000);

  digitalWrite(ENA_PIN_Y, LOW); /* accende driver */
  digitalWrite(DIR_PIN_Y, LOW); /* gira in avanti */
  delay(20);

  drawStatusGrid(motorYOK, motorZOK, limitY1OK, limitY2OK, limitZ1OK, limitZ2OK);
  while (true) {
    // se uno dei due fine‐corsa è attivato (LOW)
    if (digitalRead(END_RUN_Y_1) == LOW || digitalRead(END_RUN_Y_2) == LOW) {
      // inverti la direzione
      digitalWrite(DIR_PIN_Y, HIGH);
      delay(5);  // piccolo intervallo per la stabilità

      // continua a fare passi finché almeno un fine‐corsa resta LOW
      while (digitalRead(END_RUN_Y_1) == LOW || digitalRead(END_RUN_Y_2) == LOW) {
        digitalWrite(STEP_PIN_Y, HIGH);
        delay(10);
        digitalWrite(STEP_PIN_Y, LOW);
        delay(10);
      }

      // una volta tornati entrambi HIGH, ferma il motore e interrompi il ciclo
      stopMotorY();
      break;
    }

    COUNTER_Y++;
    // altrimenti continua nel verso “forward”
    digitalWrite(STEP_PIN_Y, HIGH);
    delay(10);
    digitalWrite(STEP_PIN_Y, LOW);
    delay(10);
  }

  FINAL_COUNTER_Y = COUNTER_Y;
  Serial.print("lunghezza totale: ");
  Serial.println(FINAL_COUNTER_Y);

  backMotorY(FINAL_COUNTER_Y, 10);
  drawStatusGrid(motorYOK, motorZOK, limitY1OK, limitY2OK, limitZ1OK, limitZ2OK);
  client.print("received");
}

void configStartZ(WiFiClient &client) {
  digitalWrite(ENA_PIN_Z, LOW);  /* accende driver */
  digitalWrite(DIR_PIN_Z, HIGH); /* gira all'indietro */
  delay(20);

  drawStatusGrid(motorYOK, motorZOK, limitY1OK, limitY2OK, limitZ1OK, limitZ2OK);
  while (true) {
    // se uno dei due fine‐corsa è attivato (LOW)
    if (digitalRead(END_RUN_Z_1) == LOW || digitalRead(END_RUN_Z_2) == LOW) {
      // inverti la direzione
      digitalWrite(DIR_PIN_Z, LOW);
      delay(5);  // piccolo intervallo per la stabilità

      // continua a fare passi finché almeno un fine‐corsa resta LOW
      while (digitalRead(END_RUN_Z_1) == LOW || digitalRead(END_RUN_Z_2) == LOW) {
        digitalWrite(STEP_PIN_Z, HIGH);
        delay(10);
        digitalWrite(STEP_PIN_Z, LOW);
        delay(10);
      }

      // una volta tornati entrambi HIGH, ferma il motore e interrompi il ciclo
      stopMotorZ();
      break;
    }

    // altrimenti continua nel verso “forward”
    digitalWrite(STEP_PIN_Z, HIGH);
    delay(10);
    digitalWrite(STEP_PIN_Z, LOW);
    delay(10);
  }

  delay(1000);

  digitalWrite(ENA_PIN_Z, LOW); /* accende driver */
  digitalWrite(DIR_PIN_Z, LOW); /* gira in avanti */
  delay(20);

  drawStatusGrid(motorYOK, motorZOK, limitY1OK, limitY2OK, limitZ1OK, limitZ2OK);
  while (true) {
    // se uno dei due fine‐corsa è attivato (LOW)
    if (digitalRead(END_RUN_Z_1) == LOW || digitalRead(END_RUN_Z_2) == LOW) {
      // inverti la direzione
      digitalWrite(DIR_PIN_Z, HIGH);
      delay(5);  // piccolo intervallo per la stabilità

      // continua a fare passi finché almeno un fine‐corsa resta LOW
      while (digitalRead(END_RUN_Z_1) == LOW || digitalRead(END_RUN_Z_2) == LOW) {
        digitalWrite(STEP_PIN_Z, HIGH);
        delay(10);
        digitalWrite(STEP_PIN_Z, LOW);
        delay(10);
      }

      // una volta tornati entrambi HIGH, ferma il motore e interrompi il ciclo
      stopMotorZ();
      break;
    }

    COUNTER_Z++;
    // altrimenti continua nel verso “forward”
    digitalWrite(STEP_PIN_Z, HIGH);
    delay(10);
    digitalWrite(STEP_PIN_Z, LOW);
    delay(10);
  }

  FINAL_COUNTER_Z = COUNTER_Z;
  Serial.print("lunghezza totale: ");
  Serial.println(FINAL_COUNTER_Z);

  backMotorZ(FINAL_COUNTER_Z / 2, 10);
  drawStatusGrid(motorYOK, motorZOK, limitY1OK, limitY2OK, limitZ1OK, limitZ2OK);
  client.print("received");
}

int calcolateDistanceY(int POSITION, bool TYPE) {
  int PARTIAL_DISTANCE = FINAL_COUNTER_Y / 3;
  int DISTANCE = (PARTIAL_DISTANCE * POSITION);
  Serial.print("calcolate distance: ");
  Serial.println(DISTANCE);
  return DISTANCE;
}

void configPin() {
  /* finecorsa */
  pinMode(END_RUN_Y_1, INPUT_PULLUP);
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.println("config pin limit switches 21 Y");
  Serial.println("config pin limit switches 21 Y");
  pinMode(END_RUN_Y_2, INPUT_PULLUP);
  M5.Lcd.setCursor(0, 30);
  M5.Lcd.println("config pin limit switches 22 Y");
  Serial.println("config pin limit switches 22 Y");

  pinMode(END_RUN_Z_1, INPUT_PULLUP);
  M5.Lcd.setCursor(0, 40);
  M5.Lcd.println("config pin limit switches 34 Z");
  Serial.println("config pin limit switches 34 Z");
  pinMode(END_RUN_Z_2, INPUT_PULLUP);
  M5.Lcd.setCursor(0, 50);
  M5.Lcd.println("config pin limit switches 35 Z");
  Serial.println("config pin limit switches 35 Z");

  /* motori */
  pinMode(DIR_PIN_Y, OUTPUT);
  M5.Lcd.setCursor(0, 60);
  M5.Lcd.println("config pin DIR_PIN_Y 2");
  Serial.println("config pin DIR_PIN_Y 2");
  pinMode(STEP_PIN_Y, OUTPUT);
  M5.Lcd.setCursor(0, 70);
  M5.Lcd.println("config pin STEP_PIN_Y 5");
  Serial.println("config pin STEP_PIN_Y 5");
  M5.Lcd.setCursor(0, 80);
  pinMode(ENA_PIN_Y, OUTPUT);
  M5.Lcd.println("config pin ENA_PIN_Y 17");
  Serial.println("config pin ENA_PIN_Y 17");

  pinMode(DIR_PIN_Z, OUTPUT);
  M5.Lcd.setCursor(0, 90);
  M5.Lcd.println("config pin DIR_PIN_Z 25");
  Serial.println("config pin DIR_PIN_Z 25");
  pinMode(STEP_PIN_Z, OUTPUT);
  M5.Lcd.setCursor(0, 100);
  M5.Lcd.println("config pin STEP_PIN_Z 26");
  Serial.println("config pin STEP_PIN_Z 26");
  M5.Lcd.setCursor(0, 110);
  pinMode(ENA_PIN_Z, OUTPUT);
  M5.Lcd.println("config pin ENA_PIN_Z 33");
  Serial.println("config pin ENA_PIN_Z 33");
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

void drawStatusGrid(bool motorYOK, bool motorZOK, bool limitY1OK, bool limitY2OK, bool limitZ1OK, bool limitZ2OK) {
  // 1) Sfondo nero
  M5.Lcd.fillScreen(TFT_BLACK);

  // 2) Parametri di layout
  const uint8_t txtSize = 2;
  const int16_t col1 = 10;                                           // x prima colonna
  const int16_t col2 = M5.Lcd.width() - ((M5.Lcd.width() / 3) / 2);  // x seconda colonna
  const int16_t rowHeight = 20;                                      // distanza verticale tra le righe
  int16_t y = 10;                                                    // offset verticale iniziale

  M5.Lcd.setTextSize(txtSize);

  // Labels e flags in array per comodità
  const char *labels[6] = {
    "MOTOR_Y",
    "MOTOR_Z",
    "LIMIT_SWITCH_Y_1",
    "LIMIT_SWITCH_Y_2",
    "LIMIT_SWITCH_Z_1",
    "LIMIT_SWITCH_Z_2"
  };
  bool states[6] = {
    motorYOK,
    motorZOK,
    limitY1OK,
    limitY2OK,
    limitZ1OK,
    limitZ2OK
  };

  // 3) Disegno delle 6 righe
  for (int i = 0; i < 6; ++i) {
    // prima colonna: etichetta in blu
    M5.Lcd.setTextColor(TFT_BLUE);
    M5.Lcd.setCursor(col1, y + i * rowHeight);
    M5.Lcd.print(labels[i]);

    // seconda colonna: V o X in base allo stato
    M5.Lcd.setCursor(col2, y + i * rowHeight);
    M5.Lcd.setTextColor(states[i] ? TFT_GREEN : TFT_RED);
    M5.Lcd.print(states[i] ? "V" : "X");
  }
}

// TF card test
bool fileExists(fs::FS &fs, const char *path) {
  bool exists = fs.exists(path);
  Serial.printf("Checking if %s exists: %s\n", path, exists ? "YES" : "NO");
  return exists;
}

bool deleteFile(fs::FS &fs, const char *path) {
  if (!fileExists(fs, path)) {
    Serial.printf("deleteFile: %s non trovato\n", path);
    return false;
  }
  bool ok = fs.remove(path);
  Serial.printf("deleteFile %s: %s\n", path, ok ? "OK" : "FAIL");
  return ok;
}

bool writeNewFile(fs::FS &fs, const char *path, const char *message) {
  // se esiste, lo rimuovo
  if (fileExists(fs, path)) {
    if (!deleteFile(fs, path)) {
      Serial.println("writeNewFile: impossibile cancellare vecchio file");
      return false;
    }
  }
  // ora creo e scrivo
  Serial.printf("Creating and writing %s\n", path);
  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("writeNewFile: apertura fallita");
    return false;
  }
  if (file.print(message)) {
    Serial.println("writeNewFile: scrittura OK");
    file.close();
    return true;
  } else {
    Serial.println("writeNewFile: scrittura FALLITA");
    file.close();
    return false;
  }
}

void drawAxis() {
  const int size = 3;       // ingrandimento del testo
  const int w = 8 * size;   // larghezza approssimativa di un carattere
  const int h = 12 * size;  // altezza approssimativa

  int x = (M5.Lcd.width() - w) / 2;
  int y = (M5.Lcd.height() - h) - 5;

  // cancella l’area dello schermo
  M5.Lcd.fillRect(x, y, w, h, TFT_BLACK);

  // disegna la lettera
  M5.Lcd.setTextSize(size);
  M5.Lcd.setTextColor(TFT_RED);
  M5.Lcd.setCursor(x, y);
  M5.Lcd.print(controlAxis ? "Y" : "Z");
}

void buttons_test() {
  M5.update();

  // --- stato statico per gestione doppio‐click B ---
  static bool waitingSingle = false;  // flag per primo click B
  static unsigned long tLast = 0;     // timestamp dell’ultimo rilascio B
  const unsigned long dblT = 500;     // max ms per doppio‐click
  unsigned long now = millis();

  // --- 1) Rilascio di B: potenziale doppio‐click ---
  if (M5.BtnB.wasReleased()) {
    if (waitingSingle && (now - tLast) <= dblT) {
      // secondo click entro timeout → cambio asse
      controlAxis = !controlAxis;
      waitingSingle = false;
      drawAxis();  // aggiorna la lettera Y/Z sul display
      return;
    } else {
      // primo click: entro in attesa
      waitingSingle = true;
      tLast = now;
      return;
    }
  }

  // --- 2) Timeout sul primo click → stop normale ---
  if (waitingSingle && (now - tLast) > dblT) {
    waitingSingle = false;
    if (controlAxis) stopMotorY();
    else stopMotorZ();
    return;
  }

  // --- 3) Long-press su B (>1s) → stop immediato ---
  if (M5.BtnB.pressedFor(1000, 200)) {
    waitingSingle = false;
    if (controlAxis) stopMotorY();
    else stopMotorZ();
    return;
  }

  // --- 4) Pulsante A: indietro finché tieni premuto ---
  if (M5.BtnA.isPressed()) {
    // setup
    if (controlAxis) {
      stopMotorY();
      digitalWrite(ENA_PIN_Y, LOW);
      digitalWrite(DIR_PIN_Y, HIGH);
    } else {
      stopMotorZ();
      digitalWrite(ENA_PIN_Z, LOW);
      digitalWrite(DIR_PIN_Z, HIGH);
    }
    delay(20);

    // ciclo di stepping
    while (M5.BtnA.isPressed()) {
      M5.update();
      bool end1 = controlAxis
                    ? (digitalRead(END_RUN_Y_1) == LOW)
                    : (digitalRead(END_RUN_Z_1) == LOW);
      bool end2 = controlAxis
                    ? (digitalRead(END_RUN_Y_2) == LOW)
                    : (digitalRead(END_RUN_Z_2) == LOW);
      if (end1 || end2) break;

      if (controlAxis) {
        digitalWrite(STEP_PIN_Y, HIGH);
        delay(10);
        digitalWrite(STEP_PIN_Y, LOW);
      } else {
        digitalWrite(STEP_PIN_Z, HIGH);
        delay(10);
        digitalWrite(STEP_PIN_Z, LOW);
      }
      delay(10);
    }

    // stop fine corsa
    if (controlAxis) stopMotorY();
    else stopMotorZ();
    return;
  }

  // --- 5) Pulsante C: avanti finché tieni premuto ---
  if (M5.BtnC.isPressed()) {
    // setup
    if (controlAxis) {
      stopMotorY();
      digitalWrite(ENA_PIN_Y, LOW);
      digitalWrite(DIR_PIN_Y, LOW);
    } else {
      stopMotorZ();
      digitalWrite(ENA_PIN_Z, LOW);
      digitalWrite(DIR_PIN_Z, LOW);
    }
    delay(20);

    // ciclo di stepping
    while (M5.BtnC.isPressed()) {
      M5.update();
      bool end1 = controlAxis
                    ? (digitalRead(END_RUN_Y_1) == LOW)
                    : (digitalRead(END_RUN_Z_1) == LOW);
      bool end2 = controlAxis
                    ? (digitalRead(END_RUN_Y_2) == LOW)
                    : (digitalRead(END_RUN_Z_2) == LOW);
      if (end1 || end2) break;

      if (controlAxis) {
        digitalWrite(STEP_PIN_Y, HIGH);
        delay(10);
        digitalWrite(STEP_PIN_Y, LOW);
      } else {
        digitalWrite(STEP_PIN_Z, HIGH);
        delay(10);
        digitalWrite(STEP_PIN_Z, LOW);
      }
      delay(10);
    }

    // stop fine corsa
    if (controlAxis) stopMotorY();
    else stopMotorZ();
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

          String hostname = "m5stack-y-z";  // Fisso come richiesto
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

void extract(const String &body) {
  int values[4] = { 0, 0, 0, 0 };  // [zoneY, delayY, zoneZ, delayZ]
  int idx = 0;                     // indice del valore corrente (0..3)
  bool inNumber = false;           // true se siamo dentro a una sequenza di cifre

  // Scorro la stringa fino a trovare 4 numeri oppure alla fine
  for (int i = 0; i < body.length() && idx < 4; ++i) {
    char c = body.charAt(i);
    if (isDigit(c)) {
      // accumulo la cifra
      inNumber = true;
      values[idx] = values[idx] * 10 + (c - '0');
    } else if (inNumber) {
      // terminatore del numero corrente → passo al prossimo
      inNumber = false;
      idx++;
    }
  }

  // Se la stringa finisce con un numero senza delimitatore, incremento ancora idx
  if (inNumber && idx < 4) {
    idx++;
  }

  // Assegno ai globali
  THIS_ZONE_Y = values[0];
  THIS_DELAY_Y = values[1];
  THIS_ZONE_Z = values[2];
  THIS_DELAY_Z = values[3];

  // Stampo su Serial in formato leggibile
  Serial.print("THIS_ZONE_Y: ");
  Serial.println(THIS_ZONE_Y);
  Serial.print("THIS_DELAY_Y: ");
  Serial.println(THIS_DELAY_Y);
  Serial.print("THIS_ZONE_Z: ");
  Serial.println(THIS_ZONE_Z);
  Serial.print("THIS_DELAY_Z: ");
  Serial.println(THIS_DELAY_Z);
}

void withdrawYZ(WiFiClient &client, String body) {
  extract(body);
  THIS_ZONE_STEP_Y = calcolateDistanceY(THIS_ZONE_Y, true);
  delay(1000);
  forwardMotorY(THIS_ZONE_STEP_Y, THIS_DELAY_Y);

  if (THIS_ZONE_Z == 1) {
    forwardMotorZ(FINAL_COUNTER_Z / 2, THIS_DELAY_Z);
  } else {
    backMotorZ(FINAL_COUNTER_Z / 2, THIS_DELAY_Z);
  }

  forwardMotorY(30, THIS_DELAY_Y);

  if (THIS_ZONE_Z == 1) {
    backMotorZ(FINAL_COUNTER_Z / 2, THIS_DELAY_Z);
  } else {
    forwardMotorZ(FINAL_COUNTER_Z / 2, THIS_DELAY_Z);
  }

  backMotorY(30, THIS_DELAY_Y);

  client.print("received");
}

void withdrawZ(WiFiClient &client) {
  if (THIS_ZONE_Z == 1) {
    forwardMotorZ(FINAL_COUNTER_Z / 2, THIS_DELAY_Z);
  } else {
    backMotorZ(FINAL_COUNTER_Z / 2, THIS_DELAY_Z);
  }

  if (THIS_ZONE_Z == 1) {
    backMotorZ(FINAL_COUNTER_Z / 2, THIS_DELAY_Z);
  } else {
    forwardMotorZ(FINAL_COUNTER_Z / 2, THIS_DELAY_Z);
  }

  client.print("received");
}

void pickAndPlaceYZ(WiFiClient &client, String body) {
  extract(body);

  if (THIS_ZONE_Z == 1) {
    forwardMotorZ(FINAL_COUNTER_Z / 2, THIS_DELAY_Z);
  } else {
    backMotorZ(FINAL_COUNTER_Z / 2, THIS_DELAY_Z);
  }

  forwardMotorY(30, THIS_DELAY_Y);

  if (THIS_ZONE_Z == 1) {
    backMotorZ(FINAL_COUNTER_Z / 2, THIS_DELAY_Z);
  } else {
    forwardMotorZ(FINAL_COUNTER_Z / 2, THIS_DELAY_Z);
  }

  THIS_ZONE_STEP_Y = calcolateDistanceY(THIS_ZONE_Y, true);
  forwardMotorY(THIS_ZONE_STEP_Y - 30, THIS_DELAY_Y);
  client.print("received");
}

void pickAndPlaceZ(WiFiClient &client) {
  if (THIS_ZONE_Z == 1) {
    forwardMotorZ(FINAL_COUNTER_Z / 2, THIS_DELAY_Z);
  } else {
    backMotorZ(FINAL_COUNTER_Z / 2, THIS_DELAY_Z);
  }

  if (THIS_ZONE_Z == 1) {
    backMotorZ(FINAL_COUNTER_Z / 2, THIS_DELAY_Z);
  } else {
    forwardMotorZ(FINAL_COUNTER_Z / 2, THIS_DELAY_Z);
  }

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

  if (requestLine.indexOf("POST /op=starty") != -1) {
    Serial.println("configStart");
    configStartY(client);
  } else if (requestLine.indexOf("POST /op=startz") != -1) {
    Serial.println("configStart");
    configStartZ(client);
  } else if (requestLine.indexOf("GET /op=maxstepy") != -1) {
    client.print(FINAL_COUNTER_Y);
  } else if (requestLine.indexOf("GET /op=maxstepz") != -1) {
    client.print(FINAL_COUNTER_Z);
  } else if (requestLine.indexOf("POST /op=withdrawyz") != -1) {
    Serial.println("withdraw Y Z");
    withdrawYZ(client, body);
  } else if (requestLine.indexOf("GET /op=withdrawz") != -1) {
    Serial.println("withdraw Z");
    withdrawZ(client);
  } else if (requestLine.indexOf("POST /op=p_p_y_z") != -1) {
    Serial.println("pick and place");
    pickAndPlaceYZ(client, body);
  } else if (requestLine.indexOf("GET /op=p_p_z") != -1) {
    Serial.println("pick and place");
    pickAndPlaceZ(client);
  } else if (requestLine.indexOf("GET /op=returnyz") != -1) {
    Serial.println("return");
    backMotorY(THIS_ZONE_STEP_Y, THIS_DELAY_Y);
    client.print("received");
  } else if (requestLine.indexOf("POST /op=wifi") != -1) {
    Serial.println("wifi");

    if (writeNewFile(SD, "/WIFI.CSV", body.c_str())) {
      Serial.println("File aggiornato con successo");
    } else {
      Serial.println("Errore nell'aggiornamento file");
    }

    client.print("received");
  } else {
    sendResponse(client, 404, "application/json", "{\"status\":\"error\",\"message\":\"Risorsa non trovata\"}");
  }

  client.stop();
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
}

// the setup routine runs once when M5Stack starts up
void setup() {
  start();
  configPin();
  drawStatusGrid(motorYOK, motorZOK, limitY1OK, limitY2OK, limitZ1OK, limitZ2OK);
  drawAxis();
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