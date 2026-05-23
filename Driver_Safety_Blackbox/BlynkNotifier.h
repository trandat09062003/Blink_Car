// BlynkNotifier.h – Blynk IoT integration for ESP32

#ifndef BLYNK_NOTIFIER_H
#define BLYNK_NOTIFIER_H

#include "Config.h"

class BlynkNotifier {
public:
    // Dynamic confidence threshold (updated via Blynk V1 slider)
    static float confidenceThreshold;

    // Connect to Wi-Fi and initialize Blynk
    static void init();

    // Blynk run handler (called in loop)
    static void run();

    // Send current TinyML confidence to Blynk V2
    static void sendConfidence(float confidence);

    // Send emergency alert and Google Maps link to Blynk
    static void sendEmergencyAlert(String mapLink);
};

#endif // BLYNK_NOTIFIER_H
