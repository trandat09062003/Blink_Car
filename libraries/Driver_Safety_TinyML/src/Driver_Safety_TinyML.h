/**
 * ==================================================================================
 * Driver_Safety_TinyML.h - THƯ VIỆN NHÚNG MÔ HÌNH AI TINYML TRÊN ESP32
 * ==================================================================================
 * Dự án: Hộp đen giám sát an toàn xe điện học sinh (GPS + AI)
 * Tác giả: Antigravity AI (Google DeepMind Team)
 * Mô tả: Định nghĩa bộ đệm vòng (circular buffer) và giao tiếp suy luận TFLite
 *        sử dụng EloquentTinyML để phân loại hành vi lái xe.
 * ==================================================================================
 */

#ifndef DRIVER_SAFETY_TINYML_H
#define DRIVER_SAFETY_TINYML_H

#include <Arduino.h>

class DriverSafetyTinyML {
private:
    float inputBuffer[600];       // Bộ đệm phẳng chứa 100 mẫu gần nhất * 6 trục = 600 phần tử
    int currentSampleCount;       // Số lượng mẫu hiện tại đã thu thập được
    bool modelInitialized;        // Trạng thái khởi tạo của mô hình AI

public:
    DriverSafetyTinyML();

    // Khởi tạo mô hình AI TinyML với dữ liệu byte array
    // Trả về true nếu khởi tạo thành công
    bool begin();

    // Thêm một mẫu cảm biến mới đọc được từ MPU6050 vào cuối bộ đệm trượt
    // Tự động đẩy mẫu cũ nhất ra ngoài (FIFO) nếu bộ đệm đã đầy 100 mẫu
    void addSample(float ax, float ay, float az, float gx, float gy, float gz);

    // Kiểm tra xem bộ đệm đã thu thập đủ 100 mẫu (2 giây dữ liệu ở tần số 50Hz) để chạy AI chưa
    bool isReady() const;

    // Reset lại số lượng mẫu trong bộ đệm về 0
    void reset();

    // Suy luận và trả về độ tin cậy cảnh báo lạng lách (giá trị từ 0.0 đến 1.0)
    // Nếu chưa đủ 100 mẫu hoặc lỗi khởi tạo, trả về 0.0
    float predictSwerving();

    // Trả về số lượng mẫu hiện tại trong bộ đệm
    int getSampleCount() const { return currentSampleCount; }
};

#endif // DRIVER_SAFETY_TINYML_H
