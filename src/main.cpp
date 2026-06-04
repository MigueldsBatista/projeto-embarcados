#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

// Pins are set via platformio.ini build_flags, but keep safe defaults here.
#ifndef RC522_SCK_PIN
#define RC522_SCK_PIN 18
#endif
#ifndef RC522_MOSI_PIN
#define RC522_MOSI_PIN 23
#endif
#ifndef RC522_MISO_PIN
#define RC522_MISO_PIN 19
#endif
#ifndef RC522_SS_PIN
#define RC522_SS_PIN 5
#endif
#ifndef RC522_RST_PIN
#define RC522_RST_PIN 22
#endif

// RGB LED pins (GPIO numbers based on user description)
#ifndef LED_RED_PIN
#define LED_RED_PIN 4   // D4
#endif
#ifndef LED_GREEN_PIN
#define LED_GREEN_PIN 2 // D2
#endif
#ifndef LED_BLUE_PIN
#define LED_BLUE_PIN 15 // D15
#endif

MFRC522 rfid(RC522_SS_PIN, RC522_RST_PIN);

static String uidToString(const MFRC522::Uid &uid) {
  String out;
  for (byte i = 0; i < uid.size; i++) {
    if (i) out += ':';
    if (uid.uidByte[i] < 0x10) out += '0';
    out += String(uid.uidByte[i], HEX);
  }
  out.toUpperCase();
  return out;
}

static bool printCardInfo(const String &uidStr) {
  // TODO: Replace / extend this mapping with your real badge registry.
  struct Entry {
    const char *uid;
    const char *nome;
    const char *cargo;
  };

  static const Entry entries[] = {
      {"4D:C2:27:06", "Rafael Barros", "Admin"},
      {"4C:82:28:06", "Miguel Batista", "Admin"},
  };

  for (const auto &e : entries) {
    if (uidStr == e.uid) {
      Serial.printf("UID: %s\n", e.uid);
      Serial.printf("Nome: %s\n", e.nome);
      Serial.printf("Cargo: %s\n", e.cargo);
      return true;
    }
  }

  Serial.printf("UID: %s\n", uidStr.c_str());
  Serial.println("Cracha nao cadastrado");
  return false;
}

void runDiagnostics();

void setup() {
  // Initialize RGB LED pins
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_BLUE_PIN, OUTPUT);
  // Start with all LEDs off
  digitalWrite(LED_RED_PIN, LOW);
  digitalWrite(LED_GREEN_PIN, LOW);
  digitalWrite(LED_BLUE_PIN, LOW);

  Serial.begin(9600);
  delay(200);

  // Use explicit SPI pins to avoid board-variant surprises.
  SPI.begin(RC522_SCK_PIN, RC522_MISO_PIN, RC522_MOSI_PIN, RC522_SS_PIN);
  rfid.PCD_Init();

  // Connectivity hint: if this reads 0x00/0xFF, wiring/pins/power is wrong.
  byte v = rfid.PCD_ReadRegister(MFRC522::VersionReg);

  Serial.println("RC522 pronto. Aproxime o cracha...");
  Serial.printf(
      "SPI: SCK=%d MISO=%d MOSI=%d SS(SDA)=%d | RST=%d\n",
      RC522_SCK_PIN,
      RC522_MISO_PIN,
      RC522_MOSI_PIN,
      RC522_SS_PIN,
      RC522_RST_PIN);
  Serial.printf("LED: R=%d G=%d B=%d\n", LED_RED_PIN, LED_GREEN_PIN, LED_BLUE_PIN);
  Serial.printf("RC522 VersionReg: 0x%02X\n", v);

  // Run a quick pin/LED diagnostics
  runDiagnostics();
}

void runDiagnostics() {
  Serial.println("--- Diagnostic start ---");
  // Verify SPI version register again
  byte v = rfid.PCD_ReadRegister(MFRC522::VersionReg);
  Serial.printf("VersionReg: 0x%02X\n", v);
  // Blink each LED to confirm pin wiring
  Serial.println("Testing RED LED");
  digitalWrite(LED_RED_PIN, LOW);
  delay(200);
  digitalWrite(LED_RED_PIN, HIGH);
  delay(200);
  Serial.println("Testing GREEN LED");
  digitalWrite(LED_GREEN_PIN, LOW);
  delay(200);
  digitalWrite(LED_GREEN_PIN, HIGH);
  delay(200);
  Serial.println("Testing BLUE LED");
  digitalWrite(LED_BLUE_PIN, LOW);
  delay(200);
  digitalWrite(LED_BLUE_PIN, HIGH);
  delay(200);
  Serial.println("--- Diagnostic end ---");
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent()) {
    delay(10);
    return;
  }
  if (!rfid.PICC_ReadCardSerial()) {
    delay(10);
    return;
  }

  String uidStr = uidToString(rfid.uid);
  bool known = printCardInfo(uidStr);
  // LED feedback
  if (known) {
    // Green blink for known card
    digitalWrite(LED_RED_PIN, LOW);
    digitalWrite(LED_GREEN_PIN, HIGH);
    delay(500); // brief period
    digitalWrite(LED_GREEN_PIN, LOW);
    digitalWrite(LED_RED_PIN, HIGH);
  } else {
    // Unknown card: flash red three times
    for (int i = 0; i < 3; ++i) {
      digitalWrite(LED_RED_PIN, LOW);
      delay(200);
      digitalWrite(LED_RED_PIN, HIGH);
      delay(200);
    }
  }
  Serial.println("---");

  // Stop reading this card until it leaves the field.
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(500);
}
