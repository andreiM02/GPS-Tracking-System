#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HardwareSerial.h>

// Dimensiunea ecranului OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32 // Sau 64 pentru ecranul 128x64

// Pinul pentru citirea tensiunii bateriei
#define VDET_2 35

// Creăm un obiect pentru ecran
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Variabile pentru tensiunea bateriei
int analogValueVDET_2;
float analogVoltsVDET_2;
int batteryPercentage;

// Tensiunea bateriei (configurabilă)
const float BATTERY_MIN_V = 3.0; // Tensiune minimă (baterie descărcată complet)
const float BATTERY_MAX_V = 4.2; // Tensiune maximă (baterie încărcată complet)

// Definim un obiect serial suplimentar pentru GPS
HardwareSerial mySerial(1); // UART1 pentru GPS

void setup() {
  // Inițializăm comunicația I2C pe pinii 21 (SDA) și 22 (SCL)
  Wire.begin(25, 26);

  // Inițializăm ecranul OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Adresa implicită I2C este 0x3C
    Serial.begin(115200);
    Serial.println(F("Eroare la inițializarea ecranului OLED!"));
    while (true);
  }

  // Ștergem ecranul
  display.clearDisplay();

  // Configurăm textul pentru mesajul inițial
  display.setTextSize(1);             // Dimensiunea textului
  display.setTextColor(SSD1306_WHITE); // Culoarea textului
  display.setCursor(0, 0);            // Poziția textului

  // 1. Mesajul de inițializare
  display.println(F("Tracker GPS"));
  display.println(F("v1.0.1"));
  display.display();                  // Afișăm pe ecran
  delay(2000); // Pauză de 2 secunde

  // Inițializăm portul serial pentru debugging
  Serial.begin(115200);

  // Configurăm rezoluția ADC la 12 biți
  analogReadResolution(12);

  // Inițializăm GPS pe UART1 (RX: 14, TX: 15)
  mySerial.begin(115200, SERIAL_8N1, 14, 15); // Pinii RX=14 și TX=15 pentru GPS
}

void loop() {
  // Citim valoarea brută de pe pinul analogic
  analogValueVDET_2 = analogRead(VDET_2);

  // Calculăm tensiunea bateriei în volți
  analogVoltsVDET_2 = analogValueVDET_2 * (3.43 / 1000.0); // Conversie la volți

  // Calculăm procentajul bateriei
  if (analogVoltsVDET_2 <= BATTERY_MIN_V) {
    batteryPercentage = 0; // Dacă tensiunea e mai mică decât minimul
  } else if (analogVoltsVDET_2 >= BATTERY_MAX_V) {
    batteryPercentage = 100; // Dacă tensiunea e mai mare decât maximul
  } else {
    // Calculăm procentajul pe baza intervalului de tensiune
    batteryPercentage = (int)((analogVoltsVDET_2 - BATTERY_MIN_V) / (BATTERY_MAX_V - BATTERY_MIN_V) * 100);
  }

  // Afișăm valorile pe monitorul serial
  Serial.printf("ADC analog VDET_2 value = %d\n", analogValueVDET_2);
  Serial.printf("Battery voltage value = %.2f V\n", analogVoltsVDET_2);
  Serial.printf("Battery percentage = %d%%\n", batteryPercentage);

  // Afișăm pe ecran procentajul bateriei
  display.clearDisplay();             // Ștergem ecranul
  display.setCursor(0, 0);            // Resetăm cursorul
  display.setTextSize(1);             // Setăm dimensiunea textului
  display.printf("Battery: %d%%\n", batteryPercentage); // Procentajul bateriei

  // Citim datele de la GPS și actualizăm statusul
  String gpsData = "";
  while (mySerial.available()) {
    gpsData += (char)mySerial.read(); // Citim datele de la GPS și le adăugăm la string-ul gpsData
  }

  if (gpsData.length() > 0) {
    // Dacă am primit date de la GPS, schimbăm statusul la "ACTIVE"
    display.println(F("GSM Status: OFF"));
    display.println(F("GPS Status: ACTIVE"));
    Serial.println("Data received from GPS:");
    Serial.println(gpsData); // Afișăm datele GPS în monitorul serial
  } else {
    // Dacă nu avem date de la GPS, statusul rămâne "OFF"
    display.println(F("GSM Status: OFF"));
    display.println(F("GPS Status: OFF"));
  }

  display.display(); // Afișăm pe ecran

  // Pauză de 10 secunde între citiri
  delay(1000);
}
