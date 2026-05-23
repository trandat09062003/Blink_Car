/**
 * ==================================================================================
 * SD_Module.h - MODULE QUẢN LÝ LƯU TRỮ THẺ NHỚ SD (SPI)
 * ==================================================================================
 * Dự án: Hộp đen giám sát an toàn xe điện học sinh (GPS + AI)
 * Tác giả: Antigravity AI (Google DeepMind Team)
 * Mô tả: Khai báo lớp quản lý đọc/ghi tệp tin trên thẻ nhớ MicroSD.
 * ==================================================================================
 */

#ifndef SD_MODULE_H
#define SD_MODULE_H

#include "Config.h"
#include <SPI.h>
#include <SD.h>

class SDModule {
private:
    bool isConnected;

public:
    SDModule();

    // Khởi tạo thẻ nhớ SD, tạo tệp CSV chứa tiêu đề (header) nếu chưa có
    bool begin();

    // Ghi một dòng dữ liệu thô cảm biến (CSV format) vào thẻ nhớ
    bool logSensorDataRow(String dataRow);

    // Ghi nhật ký hành trình hoặc sự cố (lỗi nguồn, tai nạn, quá tốc độ)
    void logSystemEvent(String eventMessage);

    // Kiểm tra trạng thái kết nối thẻ nhớ hiện tại
    bool isSDConnected() { return isConnected; }
};

#endif // SD_MODULE_H
