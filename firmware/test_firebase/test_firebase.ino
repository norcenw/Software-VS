#include <WiFiS3.h>
#include <Firebase.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Configurazione WiFi
#define WIFI_SSID "FASTWEB-XP3E4k"
#define WIFI_PASSWORD "7tjM4BU5JR"

// Configurazione Firebase
#define API_KEY "AIzaSyC7qZzqC9ET-SbYqDmhLa_yMEYtaVRb8ro"
#define DATABASE_URL "https://vertistock-a7be2-default-rtdb.europe-west1.firebasedatabase.app/"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void setupFirebase() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
}

void writeToFirebase(const char* path, int value) {
    if (Firebase.RTDB.setInt(&fbdo, path, value)) {
        Serial.println("Dati scritti con successo");
    } else {
        Serial.println("Errore di scrittura: " + fbdo.errorReason());
    }
}

int readFromFirebase(const char* path) {
    if (Firebase.RTDB.getInt(&fbdo, path)) {
        return fbdo.to<int>();
    } else {
        Serial.println("Errore di lettura: " + fbdo.errorReason());
        return -1;
    }
}

void printMessage(const String& title, const String& text, bool clientConnected, bool m5Connected) {
    display.clearDisplay();

    int topMargin = 10;
    display.setTextSize(1);
    display.setTextColor(WHITE);

    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((SCREEN_WIDTH - w) / 2, topMargin);
    display.println(title);

    int text_y = topMargin + h + 4;
    String line = "";
    int maxWidth = SCREEN_WIDTH - 4;

    for (int i = 0; i < text.length(); i++) {
        char c = text.charAt(i);
        String testLine = line;
        testLine += c;
        
        display.getTextBounds(testLine, 0, 0, &x1, &y1, &w, &h);
        if (w > maxWidth || c == '\n') {
            display.setCursor((SCREEN_WIDTH - w) / 2, text_y);
            display.println(line);
            line = (c == '\n') ? "" : String(c);
            text_y += h + 2;
        } else {
            line = testLine;
        }
    }
    if (line.length() > 0) {
        display.getTextBounds(line, 0, 0, &x1, &y1, &w, &h);
        display.setCursor((SCREEN_WIDTH - w) / 2, text_y);
        display.println(line);
    }
    display.display();
}

void setup() {
    Serial.begin(115200);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("Errore nell'inizializzazione del display");
        while (true);
    }
    display.clearDisplay();
    setupFirebase();
    writeToFirebase("/test/value", 42);
    int value = readFromFirebase("/test/value");
    Serial.println("Valore letto: " + String(value));
}

void loop() {
}
