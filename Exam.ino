// ▒▒▒▒▒ BLYNK TEMPLATE INFO ▒▒▒▒▒
#define BLYNK_TEMPLATE_ID "user9@wyns.it"
#define BLYNK_TEMPLATE_NAME "user9"
#define BLYNK_PRINT Serial  // Laat Blynk-berichten zien in de seriële monitor

// ▒▒▒▒▒ BIBLIOTHEKEN INLADEN ▒▒▒▒▒
#include <WiFi.h>                   // WiFi-functionaliteit
#include <BlynkSimpleEsp32.h>       // Blynk voor ESP32
#include <Adafruit_SSD1306.h>       // OLED display library
#include <Adafruit_Sensor.h>        // Sensor abstractielaag
#include <DHT.h>                    // DHT11 sensor
#include <Wire.h>                   // I2C communicatie

// ▒▒▒▒▒ BLYNK EN WIFI ▒▒▒▒▒
char auth[] = "KUJ0ofb3dM95MZpvWppjP3RXn8ePkp5o";  // Auth Token van jouw Blynk device
const char* ssid = "telenet-6584584";              // Naam van je WiFi netwerk
const char* password = "fRkrA3nubkrx";             // Wachtwoord van je WiFi netwerk

// ▒▒▒▒▒ DHT SENSOR CONFIG ▒▒▒▒▒
#define DHTPIN 4            // De data pin van de DHT11 is verbonden met GPIO4
#define DHTTYPE DHT11       // Je gebruikt een DHT11 sensor
DHT dht(DHTPIN, DHTTYPE);   // Maak een DHT object aan

// ▒▒▒▒▒ OLED DISPLAY CONFIG ▒▒▒▒▒
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);  // OLED via I2C

// ▒▒▒▒▒ GPIO-TOEWIJZINGEN ▒▒▒▒▒
#define LED_GREEN 16
#define LED_YELLOW 17
#define LED_RED 5
#define BUZZER 27

// ▒▒▒▒▒ SETUP FUNCTIE ▒▒▒▒▒
void setup() {
  Serial.begin(115200);  // Start seriële communicatie

  // Verbind met Blynk via lokale server (poort 8081)
  Blynk.begin(auth, ssid, password, "server.wyns.it", 8081);

  dht.begin();  // Start DHT sensor

  // OLED display initialiseren
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED niet gevonden!");
    while (true); // Stop als OLED niet gevonden is
  }

  // Initialiseer OLED display
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  // Stel GPIO-pinnen in als uitgang
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
}

// ▒▒▒▒▒ HOOFDLUS ▒▒▒▒▒
void loop() {
  Blynk.run();  // Laat Blynk actief blijven

  // Lees temperatuur en luchtvochtigheid uit de DHT11
  float temp = dht.readTemperature();  // in graden Celsius
  float hum = dht.readHumidity();      // in procent

  // Check of de sensorwaarden geldig zijn
  if (isnan(temp) || isnan(hum)) {
    Serial.println("Sensorfout!");
    return;  // Stop deze loop als meting faalt
  }

  // ▒▒▒ Comfortscore bepalen ▒▒▒
  int comfortScore = 0;

  // Slechte luchtkwaliteit → rood + buzzer
  if (temp > 30 || hum > 70) {
    comfortScore = 2;
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(BUZZER, HIGH);
  }
  // Licht oncomfortabel → geel
  else if (temp > 25 || hum < 40 || hum > 60) {
    comfortScore = 1;
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, HIGH);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(BUZZER, LOW);
  }
  // Comfortabel → groen
  else {
    comfortScore = 0;
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(BUZZER, LOW);
  }

  // ▒▒▒ Waarden versturen naar Blynk App ▒▒▒
  Blynk.virtualWrite(V0, temp);         // Temperatuur naar V0
  Blynk.virtualWrite(V1, hum);          // Vochtigheid naar V1
  Blynk.virtualWrite(V2, comfortScore); // Comfortscore naar V2

  // ▒▒▒ Toon waarden op OLED scherm ▒▒▒
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Temp: ");
  display.print(temp);
  display.println(" C");

  display.print("Vocht: ");
  display.print(hum);
  display.println(" %");

  display.display(); // Toon alles op het scherm

  delay(3000); // Wacht 3 seconden voor volgende meting
}
