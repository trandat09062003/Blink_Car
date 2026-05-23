/**
 * ==================================================================================
 * MPU6050_Module.cpp - ĐỊNH NGHĨA CÁC PHƯƠNG THỨC ĐIỀU KHIỂN CẢM BIẾN MPU6050
 * ==================================================================================
 * Dự án: Hộp đen giám sát an toàn xe điện học sinh (GPS + AI)
 * Tác giả: Antigravity AI (Google DeepMind Team)
 * ==================================================================================
 */

#include "MPU6050_Module.h"

MPU6050Module::MPU6050Module() : mpuSensor(Wire) {
    historyIndex = 0;
    lastTotalAccelVec = 1.0;
    memset(rollHistory, 0, sizeof(rollHistory));
    memset(pitchHistory, 0, sizeof(pitchHistory));
    tiltStartMillis = 0;
    prevAx = 0.0;
    prevAy = 0.0;
    prevAz = 0.0;
}

bool MPU6050Module::begin() {
    byte status = mpuSensor.begin();
    if (status != 0) {
        Serial.println(F("[ERROR]: Không thể tìm thấy cảm biến MPU6050 qua I2C!"));
        return false;
    }

    Serial.println(F("Đang hiệu chuẩn cảm biến MPU6050... Vui lòng đặt xe thẳng đứng và đứng yên hoàn toàn."));
    
    // Bíp còi + nháy LED PIR báo hiệu bắt đầu hiệu chuẩn
    digitalWrite(PIN_PIR_LED, HIGH);
    digitalWrite(PIN_BUZZER, HIGH);
    delay(200);
    digitalWrite(PIN_PIR_LED, LOW);
    digitalWrite(PIN_BUZZER, LOW);

    // Tính toán offsets của Accelerometer và Gyroscope
    mpuSensor.calcOffsets(true, true);
    Serial.println(F("Hiệu chuẩn MPU6050 THÀNH CÔNG!"));
    return true;
}

void MPU6050Module::update() {
    mpuSensor.update();
}

bool MPU6050Module::detectSwerving(float speedKmh) {
    float roll = mpuSensor.getAngleX();
    float pitch = mpuSensor.getAngleY();

    // Lưu vào bộ đệm xoay vòng lịch sử góc nghiêng
    rollHistory[historyIndex] = roll;
    pitchHistory[historyIndex] = pitch;
    historyIndex = (historyIndex + 1) % 50;

    // Chỉ kiểm tra lạng lách nếu xe đang di chuyển (tốc độ > 10 km/h) để tránh báo sai khi dắt xe
    if (speedKmh <= 10.0) {
        return false;
    }

    // Tính toán độ lệch chuẩn (Standard Deviation) của góc nghiêng Roll để xem xe có lắc liên tục
    float rollSum = 0;
    for (int i = 0; i < 50; i++) {
        rollSum += rollHistory[i];
    }
    float rollMean = rollSum / 50.0;
    
    float rollVariance = 0;
    for (int i = 0; i < 50; i++) {
        rollVariance += sq(rollHistory[i] - rollMean);
    }
    float rollStdDev = sqrt(rollVariance / 50.0);

    // Nếu độ biến thiên góc nghiêng lớn hơn 15 độ (nghiêng lắc trái phải liên tục tần số cao)
    if (rollStdDev > 15.0) {
        return true;
    }
    return false;
}

bool MPU6050Module::checkFallAccident() {
    float roll = abs(mpuSensor.getAngleX());
    float pitch = abs(mpuSensor.getAngleY());

    float ax = mpuSensor.getAccX();
    float ay = mpuSensor.getAccY();
    float az = mpuSensor.getAccZ();

    // Tính độ biến thiên gia tốc ba trục so với chu kỳ trước
    float deltaAx = abs(ax - prevAx);
    float deltaAy = abs(ay - prevAy);
    float deltaAz = abs(az - prevAz);

    prevAx = ax;
    prevAy = ay;
    prevAz = az;

    // Xem xe có đứng im hay không (biến động gia tốc tức thời cực nhỏ)
    bool isStationary = (deltaAx < 0.05 && deltaAy < 0.05 && deltaAz < 0.05);
    bool isTilted = (roll > 60.0 || pitch > 60.0);

    if (isTilted && isStationary) {
        if (tiltStartMillis == 0) {
            tiltStartMillis = millis(); // Bắt đầu bấm giờ
        }
        // Nếu duy trì liên tục trạng thái nghiêng + đứng im trên 3 giây (3000ms)
        if (millis() - tiltStartMillis >= 3000) {
            return true;
        }
    } else {
        tiltStartMillis = 0; // Reset nếu xe chuyển động lại hoặc thẳng đứng dậy
    }
    return false;
}
