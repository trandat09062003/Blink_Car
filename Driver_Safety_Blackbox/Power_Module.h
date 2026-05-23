/**
 * ==================================================================================
 * Power_Module.h - MODULE QUẢN LÝ TIẾT KIỆM NĂNG LƯỢNG (DEEP SLEEP)
 * ==================================================================================
 * Dự án: Hộp đen giám sát an toàn xe điện học sinh (GPS + AI)
 * Tác giả: Antigravity AI (Google DeepMind Team)
 * Mô tả: Khai báo lớp điều khiển chế độ ngủ sâu thông minh và tự động đánh thức.
 * ==================================================================================
 */

#ifndef POWER_MODULE_H
#define POWER_MODULE_H

#include "Config.h"

class PowerModule {
private:
    unsigned long lastActivityTime;
    float lastAccX, lastAccY, lastAccZ;

public:
    PowerModule();

    // Thiết lập thời gian hoạt động ban đầu
    void begin();

    // Cập nhật và kiểm tra xem có rung động chuyển động cơ học hay không
    void monitorMovement(float ax, float ay, float az);

    // Kiểm tra xem xe đã đứng im quá thời gian quy định (ví dụ 10 phút) hay chưa
    bool isTimeoutReached();

    // Ra lệnh đưa hệ thống ESP32 vào chế độ Deep Sleep định kỳ 5 giây đánh thức
    void enterDeepSleep();

    // Thực hiện cảnh báo rung nháy LED
    void flashStatusLED(int durationMs, int times);
};

#endif // POWER_MODULE_H
