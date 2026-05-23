/**
 * ==================================================================================
 * GPS_Module.h - MODULE GIÁM SÁT ĐỊNH VỊ TOÀN CẦU (GPS)
 * ==================================================================================
 * Dự án: Hộp đen giám sát an toàn xe điện học sinh (GPS + AI)
 * Tác giả: Antigravity AI (Google DeepMind Team)
 * Mô tả: Khai báo lớp điều khiển và xử lý tín hiệu từ module GPS NEO-6M.
 * ==================================================================================
 */

#ifndef GPS_MODULE_H
#define GPS_MODULE_H

#include "Config.h"
#include <TinyGPSPlus.h>

class GPSModule {
private:
    TinyGPSPlus gpsEngine;
    HardwareSerial& gpsSerial;

public:
    GPSModule(HardwareSerial& serialPort);

    // Khởi tạo cổng Serial kết nối GPS
    void begin();

    // Nhận và phân tích dữ liệu NMEA (gọi liên tục trong loop)
    void update();

    // Kiểm tra xem GPS đã định vị thành công vệ tinh hay chưa
    bool isLocationValid() { return gpsEngine.location.isValid(); }

    // Lấy tọa độ Vĩ độ (Latitude)
    double getLatitude() { return gpsEngine.location.lat(); }

    // Lấy tọa độ Kinh độ (Longitude)
    double getLongitude() { return gpsEngine.location.lng(); }

    // Lấy tốc độ xe thực tế (km/h)
    float getSpeedKmh() { 
        return gpsEngine.speed.isValid() ? gpsEngine.speed.kmph() : 0.0; 
    }

    // Trả về liên kết vị trí bản đồ Google Maps
    String getGoogleMapsLink();
};

#endif // GPS_MODULE_H
