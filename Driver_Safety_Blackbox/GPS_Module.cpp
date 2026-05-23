/**
 * ==================================================================================
 * GPS_Module.cpp - ĐỊNH NGHĨA PHƯƠNG THỨC XỬ LÝ ĐỊNH VỊ TOÀN CẦU (GPS)
 * ==================================================================================
 * Dự án: Hộp đen giám sát an toàn xe điện học sinh (GPS + AI)
 * Tác giả: Antigravity AI (Google DeepMind Team)
 * ==================================================================================
 */

#include "GPS_Module.h"

GPSModule::GPSModule(HardwareSerial& serialPort) : gpsSerial(serialPort) {
}

void GPSModule::begin() {
    // NEO-6M mặc định 9600 bps. RX=34 (GPS_TX module), TX=35 (GPS_RX module).
    gpsSerial.begin(9600, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX);
    Serial.print(F("Đã khởi tạo Module GPS (Baud: 9600, RX: "));
    Serial.print(PIN_GPS_RX);
    Serial.print(F(", TX: "));
    Serial.print(PIN_GPS_TX);
    Serial.println(F(")."));
}

void GPSModule::update() {
    // Đọc và phân tích dữ liệu luồng từ cổng Serial kết nối GPS
    while (gpsSerial.available() > 0) {
        gpsEngine.encode(gpsSerial.read());
    }
}

String GPSModule::getGoogleMapsLink() {
    if (gpsEngine.location.isValid()) {
        double lat = gpsEngine.location.lat();
        double lon = gpsEngine.location.lng();
        // Định dạng chuỗi liên kết vị trí chuẩn Google Maps
        return "https://maps.google.com/?q=" + String(lat, 6) + "," + String(lon, 6);
    }
    return "Khong co tin hieu ve tinh GPS.";
}
