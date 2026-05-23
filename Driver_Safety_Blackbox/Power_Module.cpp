/**
 * ==================================================================================
 * Power_Module.cpp - ĐỊNH NGHĨA PHƯƠNG THỨC TIẾT KIỆM NĂNG LƯỢNG (DEEP SLEEP)
 * ==================================================================================
 * Dự án: Hộp đen giám sát an toàn xe điện học sinh (GPS + AI)
 * Tác giả: Antigravity AI (Google DeepMind Team)
 * ==================================================================================
 */

#include "Power_Module.h"

PowerModule::PowerModule() {
    lastActivityTime = 0;
    lastAccX = 0.0;
    lastAccY = 0.0;
    lastAccZ = 1.0;
}

void PowerModule::begin() {
    lastActivityTime = millis();
    pinMode(PIN_STATUS_LED, OUTPUT);
    digitalWrite(PIN_STATUS_LED, LOW);
}

void PowerModule::monitorMovement(float ax, float ay, float az) {
    // Tính toán mức chênh lệch gia tốc động cơ học ở 3 trục
    float delta = abs(ax - lastAccX) + abs(ay - lastAccY) + abs(az - lastAccZ);

    // Cập nhật giá trị lịch sử
    lastAccX = ax;
    lastAccY = ay;
    lastAccZ = az;

    // Nếu mức rung lắc động học vượt quá ngưỡng dắt xe dập dềnh hoặc đi xe (0.08G)
    if (delta > 0.08) {
        lastActivityTime = millis(); // Cập nhật thời điểm hoạt động cuối cùng
    }
}

bool PowerModule::isTimeoutReached() {
    return (millis() - lastActivityTime > DEEP_SLEEP_TIMEOUT_MS);
}

void PowerModule::enterDeepSleep() {
    Serial.println(F("Xe dung yen qua lau! Hop den chuan bi di vao che do ngu sau (Deep Sleep)..."));
    
    // Kêu còi + LED PIR trước khi Deep Sleep
    digitalWrite(PIN_PIR_LED, HIGH);
    digitalWrite(PIN_BUZZER, HIGH);
    delay(1500);
    digitalWrite(PIN_PIR_LED, LOW);
    digitalWrite(PIN_BUZZER, LOW);
    digitalWrite(PIN_STATUS_LED, LOW);

    // Cấu hình đánh thức định kỳ sau mỗi 5 giây bằng đồng hồ RTC nội bộ
    // 5 giây = 5.000.000 microseconds
    esp_sleep_enable_timer_wakeup(5000000);

    Serial.println(F("ESP32 bat dau ngu. Tam biet!"));
    delay(100);
    esp_deep_sleep_start();
}

void PowerModule::flashStatusLED(int durationMs, int times) {
    for (int i = 0; i < times; i++) {
        digitalWrite(PIN_STATUS_LED, HIGH);
        delay(durationMs / 2);
        digitalWrite(PIN_STATUS_LED, LOW);
        delay(durationMs / 2);
    }
}
