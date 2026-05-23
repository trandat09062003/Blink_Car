// BlynkNotifier.cpp – Blynk IoT implementation for ESP32

// BLYNK configuration macros MUST be defined before including BlynkSimpleEsp32.h
#define BLYNK_TEMPLATE_ID   "TMPL6ED-9LyQA"
#define BLYNK_TEMPLATE_NAME "Driver Safety"
#define BLYNK_AUTH_TOKEN    "oFEdhfp3NWf79DyVUEeWgi75ZtLGc225"

#include <BlynkSimpleEsp32.h>
#include <WiFi.h>
#include "BlynkNotifier.h"

// Initialize global threshold (default to 0.45)
float BlynkNotifier::confidenceThreshold = 0.45f;

void BlynkNotifier::init() {
    Serial.println(F("[Blynk] Dang ket noi Wi-Fi..."));
    
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    
    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 10000) {
        delay(500);
        Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println(F("\n[Blynk] Da ket noi Wi-Fi thanh cong!"));
        Serial.print(F("[Blynk] IP Address: "));
        Serial.println(WiFi.localIP());
    } else {
        Serial.println(F("\n[Blynk] Ket noi Wi-Fi that bai! Thu ket noi lai trong nen..."));
    }

    // Initialize Blynk connection
    Blynk.config(BLYNK_AUTH_TOKEN);
    Blynk.connect();
    
    // Send initial configuration to Blynk (Sync V1 slider value to 45%)
    if (Blynk.connected()) {
        Blynk.virtualWrite(V1, (int)(confidenceThreshold * 100));
        Serial.println(F("[Blynk] Da dong bo gia tri nguong ban dau: 45%"));
    }
}

void BlynkNotifier::run() {
    Blynk.run();
}

void BlynkNotifier::sendConfidence(float confidence) {
    if (Blynk.connected()) {
        // Send confidence (0.0 to 1.0) to Virtual Pin V2 with 4 decimal places precision
        Blynk.virtualWrite(V2, String(confidence, 4));
    }
}

void BlynkNotifier::sendEmergencyAlert(String mapLink) {
    if (Blynk.connected()) {
        // Send map link to Virtual Pin V3 (Value Display/Text widget)
        Blynk.virtualWrite(V3, mapLink);
        
        // Log event to trigger push notification (Requires 'accident' event configured in Blynk Console)
        Blynk.logEvent("accident", "CANH BAO: Phat hien nga xe! Vi tri: " + mapLink);
        Serial.println(F("[Blynk] Da gui canh bao tai nan va vi tri len Blynk!"));
    } else {
        Serial.println(F("[Blynk] Khong the gui canh bao, Blynk chua ket noi!"));
    }
}

// BLYNK_WRITE is called when the value of Virtual Pin V1 changes on the Blynk App/Cloud (Slider widget)
BLYNK_WRITE(V1) {
    int sliderVal = param.asInt(); // Slider values should be 0 to 100
    BlynkNotifier::confidenceThreshold = (float)sliderVal / 100.0f;
    
    Serial.print(F("[Blynk] Cap nhat nguong tu Slider (V1): "));
    Serial.print(sliderVal);
    Serial.print(F("% (Gia tri: "));
    Serial.print(BlynkNotifier::confidenceThreshold);
    Serial.println(F(")"));
}
