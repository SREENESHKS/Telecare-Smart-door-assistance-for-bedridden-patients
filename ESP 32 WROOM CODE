#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <mbedtls/base64.h>  // Base64 Encoding
#include <SPI.h>
#include <MFRC522.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define OLED_ADDR     0x3C  // Change to 0x3D if needed
#define BUTTON_PIN    4      // Push button connected to GPIO4
#define PLAYE_PIN     27     // ISD1820 PLAYE connected to GPIO27
#define VIBRATION_SENSOR_PIN 32  // SW-420 Vibration Sensor
#define BUZZER_PIN 25  // Buzzer for alarm
#define SS_PIN 5
#define RST_PIN 22
#define LOCK_PIN 2  // GPIO for solenoid lock

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
MFRC522 rfid(SS_PIN, RST_PIN);
WiFiClientSecure client;
String receivedCommand = "";

// **WiFi Credentials**
const char* ssid = "";
const char* password = "";

// **Twilio API Credentials**
const char* account_sid = "";
const char* auth_token = "";
const char* twilio_phone_number = "";
const char* caretaker_phone_number = "";

const int vibration_threshold = 1111;
int vibration_count = 0;

// Authorized UIDs
byte authorizedUIDs[][4] = {
    {0xC3, 0x14, 0x2F, 0xDA},
    {0xA6, 0x53, 0x8B, 0x3F}
};
const byte numAuthorizedUIDs = sizeof(authorizedUIDs) / sizeof(authorizedUIDs[0]);

#define PLAYE_PIN 27  // ISD1820 PLAYE connected to GPIO27
void showMessage(String message) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.println(message);
    display.display();
}
void printHex(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
    Serial.println();
}
bool isAuthorizedUID(byte *scannedUID) {
    for (byte i = 0; i < numAuthorizedUIDs; i++) {
        if (memcmp(scannedUID, authorizedUIDs[i], 4) == 0) {
            return true;  // Match found, UID is authorized
        }
    }
    return false;  // No match found
}


void setup() {
    Serial.begin(115200);
    Serial2.begin(115200, SERIAL_8N1, 16, 17);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(VIBRATION_SENSOR_PIN, INPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LOCK_PIN, OUTPUT);
    pinMode(PLAYE_PIN, OUTPUT);  // Set PLAYE as output
    digitalWrite(PLAYE_PIN, LOW); // Ensure playback is off initially

    digitalWrite(LOCK_PIN, LOW);  // Lock door initially

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    Serial.println("\nWiFi Connected!");
    client.setInsecure();

    SPI.begin();
    rfid.PCD_Init();

    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        Serial.println("SSD1306 allocation failed!");
        while (1);
    }
    showMessage("System Ready!");
    Serial.println("Unlocking...");
    digitalWrite(LOCK_PIN, LOW);
    delay(5000);
    Serial.println("Locking...");
    digitalWrite(LOCK_PIN, HIGH);   
}

void loop() {
    checkButton();
    checkSerialCommand();
    detectVibration();
    checkRFID();
    checkVoicePlayback();  // Function to handle ISD1820 playback
}

void checkVoicePlayback() {
    if (digitalRead(BUTTON_PIN) == LOW) {  // If button is pressed
        delay(5000);  // Wait for 5 seconds (5000 milliseconds)
        digitalWrite(PLAYE_PIN, HIGH);  // Trigger playback
        delay(500);  // Small delay to ensure playback starts
        digitalWrite(PLAYE_PIN, LOW);   // Stop playback trigger
    }
}


void checkButton() {
    int buttonState = digitalRead(BUTTON_PIN);
    Serial.println("Button State: " + String(buttonState));  // Debugging line

    if (buttonState == LOW) {  // If button is pressed
        Serial.println("Button Pressed!");  // Debugging
        showMessage("PHOTO IS CAPTURING\nSTAY NEAR CAMERA");
        delay(2000);
        showMessage("CHECKING TEMPERATURE...");
        delay(5000);
        showMessage("WAITING FOR APPROVAL...");
    }
}


void checkSerialCommand() {
    if (Serial2.available()) {
        receivedCommand = Serial2.readStringUntil('\n');
        receivedCommand.trim();
        Serial.println("Received Command: " + receivedCommand);
        
        if (receivedCommand == "/photo") {
            showMessage("RECAPTURING PHOTO,\nPLEASE STAND BEFORE CAMERA...");
            delay(2000);
        } else if (receivedCommand == "/temp") {
            showMessage("TEMPERATURE CHECKING...");
            delay(2000);
        }
        showMessage("WAITING FOR APPROVAL...");
    }
}

void detectVibration() {
    if (digitalRead(VIBRATION_SENSOR_PIN) == LOW) {
        vibration_count++;
        Serial.println("Vibration detected! Count: " + String(vibration_count));
        if (vibration_count >= vibration_threshold) {
            Serial.println("🚨 Tampering confirmed! Triggering alarm and call.");
            makeEmergencyCall();
            digitalWrite(BUZZER_PIN, HIGH);
            delay(3000);
            digitalWrite(BUZZER_PIN, LOW);
            vibration_count = 0;
        }
    } else {
        vibration_count = 0;
    }
    delay(500);
}

void makeEmergencyCall() {
    Serial.println("📞 Making Emergency Call via Twilio...");
    if (!client.connect("api.twilio.com", 443)) {
        Serial.println("❌ Twilio Connection Failed!");
        return;
    }
    String auth = String(account_sid) + ":" + String(auth_token);
    size_t output_length;
    unsigned char encoded[128];
    mbedtls_base64_encode(encoded, sizeof(encoded), &output_length, (const unsigned char*)auth.c_str(), auth.length());
    String encodedAuth = String((char*)encoded);
    
    String requestBody = "To=" + String(caretaker_phone_number) +
                         "&From=" + String(twilio_phone_number) +
                         "&Url=";
    
    client.println("POST /2010-04-01/Accounts/" + String(account_sid) + "/Calls.json HTTP/1.1");
    client.println("Host: api.twilio.com");
    client.println("Authorization: Basic " + encodedAuth);
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println("Content-Length: " + String(requestBody.length()));
    client.println();
    client.print(requestBody);
    Serial.println("✅ Emergency Call Triggered!");
}

void checkRFID() {
    if (!rfid.PICC_IsNewCardPresent()) {
        Serial.println("No new card present.");
        return;
    }

    if (!rfid.PICC_ReadCardSerial()) {
        Serial.println("Failed to read the card.");
        return;
    }

    Serial.print("Scanned UID: ");
    printHex(rfid.uid.uidByte, rfid.uid.size);

    if (isAuthorizedUID(rfid.uid.uidByte)) {
        Serial.println("Access Granted! Unlocking door...");
        showMessage("ACCESS GRANTED!\nDOOR OPENED!");
        digitalWrite(LOCK_PIN, HIGH);  // Unlock
        delay(5000);
        digitalWrite(LOCK_PIN, LOW);   // Lock
        showMessage("DOOR LOCKED!");
    } else {
        Serial.println("Access Denied!");
        showMessage("ACCESS DENIED!");
        delay(2000);
        showMessage("WAITING FOR APPROVAL...");
    }`

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
}
