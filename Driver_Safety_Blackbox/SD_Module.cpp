/**
 * ==================================================================================
 * SD_Module.cpp - ĐỊNH NGHĨA PHƯƠNG THỨC GHI THẺ NHỚ SD (SPI)
 * ==================================================================================
 * Dự án: Hộp đen giám sát an toàn xe điện học sinh (GPS + AI)
 * Tác giả: Antigravity AI (Google DeepMind Team)
 * ==================================================================================
 */

#include "SD_Module.h"

SDModule::SDModule() {
    isConnected = false;
}

bool SDModule::begin() {
    Serial.println(F("Đang tiến hành kết nối thẻ MicroSD qua SPI..."));
    pinMode(PIN_SD_CS, OUTPUT);
    digitalWrite(PIN_SD_CS, HIGH); // Kéo CS lên cao trước

    // Khởi tạo SPI không truyền CS tránh xung đột
    SPI.begin(PIN_SD_SCK, PIN_SD_MISO, PIN_SD_MOSI);
    delay(100);

    bool mountSuccess = false;
    uint32_t speedTest[] = {4000000, 1000000, 400000, 8000000};
    int numSpeeds = sizeof(speedTest) / sizeof(speedTest[0]);

    for (int i = 0; i < numSpeeds; i++) {
        Serial.print(F("  Thử mount ở tần số: "));
        Serial.print(speedTest[i] / 1000);
        Serial.println(F(" kHz..."));

        SD.end();
        delay(100);

        if (SD.begin(PIN_SD_CS, SPI, speedTest[i])) {
            mountSuccess = true;
            Serial.print(F("  -> KẾT NỐI THÀNH CÔNG tại tần số: "));
            Serial.print(speedTest[i] / 1000);
            Serial.println(F(" kHz!"));
            break;
        }
        delay(100);
    }

    if (!mountSuccess) {
        Serial.println(F("  -> THẤT BẠI! Hãy kiểm tra thẻ nhớ đã cắm đúng hay chưa."));
        isConnected = false;
        return false;
    }

    isConnected = true;

    // Tạo file dữ liệu CSV thô thu thập nếu chưa tồn tại trên thẻ nhớ
    if (CURRENT_MODE == MODE_DATA_COLLECTION) {
        if (!SD.exists("/sensor_data.csv")) {
            File dataFile = SD.open("/sensor_data.csv", FILE_WRITE);
            if (dataFile) {
                dataFile.println("Timestamp_ms,AccX_g,AccY_g,AccZ_g,GyroX_ds,GyroY_ds,GyroZ_ds");
                dataFile.close();
                Serial.println(F("Đã tạo tệp tin tiêu đề CSV mới: 'sensor_data.csv'"));
            }
        }
    }

    // Tạo file nhật ký sự kiện hệ thống nếu chưa có
    if (!SD.exists("/system_logs.txt")) {
        File logFile = SD.open("/system_logs.txt", FILE_WRITE);
        if (logFile) {
            logFile.println("=== THIẾT BỊ HỘP ĐEN GHI LOG KHỞI ĐỘNG ===");
            logFile.close();
        }
    }
    return true;
}

bool SDModule::logSensorDataRow(String dataRow) {
    if (!isConnected) return false;

    File dataFile = SD.open("/sensor_data.csv", FILE_WRITE);
    if (dataFile) {
        dataFile.println(dataRow);
        dataFile.close();
        return true;
    }
    
    // Ghi thất bại đồng nghĩa thẻ nhớ có thể đã bị lỏng hoặc rút ra đột ngột
    isConnected = false;
    return false;
}

void SDModule::logSystemEvent(String eventMessage) {
    if (!isConnected) return;

    File logFile = SD.open("/system_logs.txt", FILE_WRITE);
    if (logFile) {
        unsigned long timestamp = millis();
        logFile.print("[");
        logFile.print(timestamp);
        logFile.print(" ms]: ");
        logFile.println(eventMessage);
        logFile.close();
    }
}
