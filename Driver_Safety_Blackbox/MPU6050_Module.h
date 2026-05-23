/**
 * ==================================================================================
 * MPU6050_Module.h - MODULE QUẢN LÝ CẢM BIẾN GIA TỐC VÀ GÓC NGHIÊNG
 * ==================================================================================
 * Dự án: Hộp đen giám sát an toàn xe điện học sinh (GPS + AI)
 * Tác giả: Antigravity AI (Google DeepMind Team)
 * Mô tả: Khai báo lớp điều khiển cảm biến MPU6050, thuật toán lọc góc,
 *        thuật toán lạng lách và phát hiện ngã đổ xe.
 * ==================================================================================
 */

#ifndef MPU6050_MODULE_H
#define MPU6050_MODULE_H

#include "Config.h"
#include <Wire.h>
#include <MPU6050_light.h>

class MPU6050Module {
private:
    MPU6050 mpuSensor;
    
    // Các biến phục vụ thuật toán Heuristic phát hiện Lạng lách
    float rollHistory[50];
    float pitchHistory[50];
    int historyIndex;
    
    // Các biến phục vụ thuật toán phát hiện Ngã xe
    float lastTotalAccelVec;
    unsigned long tiltStartMillis;
    float prevAx, prevAy, prevAz;

public:
    MPU6050Module();

    // Khởi tạo cảm biến MPU6050 và chạy hiệu chuẩn cân bằng ban đầu
    bool begin();

    // Đọc cập nhật dữ liệu liên tục từ cảm biến (gọi trong loop)
    void update();

    // Lấy các giá trị thô phục vụ Ghi thẻ SD / Truyền Edge Impulse
    float getAccX() { return mpuSensor.getAccX(); }
    float getAccY() { return mpuSensor.getAccY(); }
    float getAccZ() { return mpuSensor.getAccZ(); }
    float getGyroX() { return mpuSensor.getGyroX(); }
    float getGyroY() { return mpuSensor.getGyroY(); }
    float getGyroZ() { return mpuSensor.getGyroZ(); }

    // Lấy góc nghiêng vật lý (Roll - X, Pitch - Y) đã qua bộ lọc bù
    float getRoll() { return mpuSensor.getAngleX(); }
    float getPitch() { return mpuSensor.getAngleY(); }

    // Thuật toán Heuristic phát hiện hành vi lạng lách, đánh võng dồn dập
    bool detectSwerving(float speedKmh);

    // Thuật toán kiểm tra đổ ngã xe kết hợp tĩnh học
    bool checkFallAccident();
};

#endif // MPU6050_MODULE_H
