#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "esp_camera.h"
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

Adafruit_MLX90614 mlx = Adafruit_MLX90614();  // MLX90614 sensor

const char* ssid = "";
const char* password = "";
String BOTtoken = "";
String CHAT_ID = "";
String CARETAKER_CHAT_ID = ""; 

WiFiClientSecure clientTCP;
UniversalTelegramBot bot(BOTtoken, clientTCP);

#define FLASH_LED_PIN 4
#define BUTTON_PIN 13
#define BUZZER_PIN 12
bool flashState = LOW;
String latestTemp = "N/A"; 

// Camera Pins
#define PWDN_GPIO_NUM  32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM   0
#define SIOD_GPIO_NUM  26
#define SIOC_GPIO_NUM  27
#define Y9_GPIO_NUM    35
#define Y8_GPIO_NUM    34
#define Y7_GPIO_NUM    39
#define Y6_GPIO_NUM    36
#define Y5_GPIO_NUM    21
#define Y4_GPIO_NUM    19
#define Y3_GPIO_NUM    18
#define Y2_GPIO_NUM     5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM  23
#define PCLK_GPIO_NUM  22

void configInitCamera() {
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 8;
    config.fb_count = 1;

    if (esp_camera_init(&config) != ESP_OK) {
        Serial.println("Camera init failed");
        ESP.restart();
    }
}

void flushCameraBuffer() {
    for (int i = 0; i < 3; i++) {
        camera_fb_t *fb = esp_camera_fb_get();
        if (fb) esp_camera_fb_return(fb);
    }
}

String sendPhotoTelegram(String chat_id) {
    flushCameraBuffer();
    delay(200);
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        return "Camera capture failed";
    }

    if (clientTCP.connect("api.telegram.org", 443)) {
        String head = "--boundary\r\nContent-Disposition: form-data; name=\"chat_id\";\r\n\r\n" + chat_id + 
                      "\r\n--boundary\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"photo.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
        String tail = "\r\n--boundary--\r\n";

        clientTCP.println("POST /bot" + BOTtoken + "/sendPhoto HTTP/1.1");
        clientTCP.println("Host: api.telegram.org");
        clientTCP.println("Content-Length: " + String(fb->len + head.length() + tail.length()));
        clientTCP.println("Content-Type: multipart/form-data; boundary=boundary");
        clientTCP.println();
        clientTCP.print(head);
        clientTCP.write(fb->buf, fb->len);
        clientTCP.print(tail);
        esp_camera_fb_return(fb);
        
        bot.sendMessage(chat_id, "\U0001F321 Temp: " + latestTemp + "°C", ""); 
    }
    return "Photo sent!";
}
void handleNewMessages(int numNewMessages) {
    Serial.println("Processing " + String(numNewMessages) + " new messages");

    for (int i = 0; i < numNewMessages; i++) {
        String chat_id = bot.messages[i].chat_id;
        String text = bot.messages[i].text;
        Serial.println("Received: " + text);

        if (text == "/start") {
            bot.sendMessage(chat_id, "Welcome! Use these commands:\n"
                                     "/photo - Capture image\n"
                                     "/temp - Get temperature\n"
                                     "/flash - Turn flash ON\n"
                                     "/unflash - Turn flash OFF\n"
                                     "/unlock - Unlock door", "");
        } 
        else if (text == "/photo") {
            bot.sendMessage(chat_id, "Capturing image...", "");
            sendPhotoTelegram(chat_id);
        }
        else if (text == "/temp") {
            bot.sendMessage(chat_id, "\U0001F321 Temp: " + latestTemp + "°C", "");
        }
        else if (text == "/flash") {
            digitalWrite(FLASH_LED_PIN, HIGH);
            flashState = HIGH;
            bot.sendMessage(chat_id, "\U0001F4A1 Flash turned ON", "");
        }
        else if (text == "/unflash") {
            digitalWrite(FLASH_LED_PIN, LOW);
            flashState = LOW;
            bot.sendMessage(chat_id, "\U0001F4A1 Flash turned OFF", "");
        }
        else if (text == "/unlock") {
            bot.sendMessage(chat_id, "\U0001F6AA Door unlocked!", "");
            // Add unlock logic here
        }
    }
}


void setup() {
    Serial.begin(115200);
    pinMode(FLASH_LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(FLASH_LED_PIN, LOW);
    digitalWrite(BUZZER_PIN,LOW);
    configInitCamera();
    WiFi.begin(ssid, password);
    clientTCP.setCACert(TELEGRAM_CERTIFICATE_ROOT);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    Serial.println(WiFi.localIP());

    Wire.begin(15, 14);  // SDA = 15, SCL = 14
    if (!mlx.begin()) {
        Serial.println("MLX90614 not detected!");
    }
}

void loop() {
    static bool buttonPressed = false;
    static unsigned long lastMessageTime = 0;
    static bool caretakerNotified = false;

    latestTemp = String(mlx.readObjectTempC(), 1);  // Read MLX90614 temp

    // Check if the button is pressed and debounce it
    if (digitalRead(BUTTON_PIN) == LOW && !buttonPressed) {
        delay(50);  // Debounce delay
        if (digitalRead(BUTTON_PIN) == LOW) {  // Confirm button press
            buttonPressed = true;
            digitalWrite(BUZZER_PIN, HIGH);  // Turn ON buzzer
            
            // Send message to user
            bot.sendMessage(CHAT_ID, "\U0001F6AA Someone is at the door! Capturing image...", "");
            sendPhotoTelegram(CHAT_ID);
            lastMessageTime = millis();
            caretakerNotified = false;
        }
    }

    // Check if the button is released
    if (digitalRead(BUTTON_PIN) == HIGH && buttonPressed) {
        buttonPressed = false;
        digitalWrite(BUZZER_PIN, LOW);  // Turn OFF buzzer
    }

    // Check if 4 minutes have passed without response
    if (!caretakerNotified && millis() - lastMessageTime > 240000) {
        bot.sendMessage(CARETAKER_CHAT_ID, "⚠️ User has not responded in 4 minutes! Please check the door.", "");
        sendPhotoTelegram(CARETAKER_CHAT_ID);
        caretakerNotified = true;  // Prevent multiple messages
    }

    // Process new Telegram messages
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
        handleNewMessages(numNewMessages);
        lastMessageTime = millis();  // Reset timer after user response
        caretakerNotified = false;
        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
}

