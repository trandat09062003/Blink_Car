/**
 * ==================================================================================
 * Config.h - CẤU HÌNH HỆ THỐNG VÀ ĐỊNH NGHĨA CHÂN KẾT NỐI
 * ==================================================================================
 * Dự án: Hộp đen giám sát an toàn xe điện học sinh (GPS + AI)
 * Sơ đồ: Altium Designer PCB (ESP32 DEVKIT V1)
 * ==================================================================================
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==========================================
// 1. CẤU HÌNH CHẾ ĐỘ HOẠT ĐỘNG (CHỌN 1 TRONG 2)
// ==========================================
#define MODE_DATA_COLLECTION  0  // Chế độ 1: Thu thập dữ liệu thô (Raw Data) ghi thẻ nhớ SD / Serial
#define MODE_INFERENCE        1  // Chế độ 2: Nhận diện hành vi thực tế (TinyML AI & Cảnh báo)

// !!! HỌC SINH THAY ĐỔI CHẾ ĐỘ HOẠT ĐỘNG TẠI ĐÂY !!!
#define CURRENT_MODE          MODE_INFERENCE

// ==========================================
// 2. THAM SỐ TOÀN CỤC CẤU HÌNH HỆ THỐNG
// ==========================================

#define SPEED_LIMIT_KMH        40.0

#define DEEP_SLEEP_TIMEOUT_MS  600000

#define SAMPLING_PERIOD_MS     20

// ==========================================
// 3. SƠ ĐỒ KẾT NỐI PHẦN CỨNG (GPIO PIN MAPPING)
//    Khớp schematic Altium Sheet3 / ESP32 DEVKIT V1
// ==========================================

// --- LED & còi ---
#define PIN_STATUS_LED   2     // D2  STATUS  -> LED1 (Đúng theo PCB)
#define PIN_PIR_LED      12    // D12 Cần hàn dây câu từ U1-28 (BUZZER net) sang LED2-1 (PIR net) để sử dụng LED2
#define PIN_BUZZER       4     // D4  BIP     -> Còi SP1 qua Q1 (Thực tế nối vào GPIO4 trên PCB)

// --- I2C: MPU6050 (JP2) + LCD 2004 I2C (PCF8574) ---
// Module LCD2004 (nhãn trên board): GND | VCC | SDA | SCL
#define PIN_I2C_SDA      21    // D21 SDA (Đúng theo PCB)
#define PIN_I2C_SCL      22    // D22 SCL (Đúng theo PCB)

// --- SPI: MicroSD (header P2) ---
#define PIN_SD_CS        5     // D5  CS (Đúng theo PCB)
#define PIN_SD_SCK       18    // D18 SCK (Đúng theo PCB)
#define PIN_SD_MISO      19    // D19 MISO (Đúng theo PCB)
#define PIN_SD_MOSI      23    // D23 MOSI (Đúng theo PCB)

// --- GPS NEO-6M (JP1), Hardware Serial2 ---
// Cần sửa phần cứng:
// 1. Cắt đường mạch nối GPIO17 (U1-7) với net GPS_RX.
// 2. Hàn dây câu từ JP1 pin 2 (GPS_TX) vào GPIO16 (U1-6).
// 3. Hàn dây câu từ JP1 pin 3 (GPS_RX) vào GPIO17 (U1-7).
#define PIN_GPS_RX       16    // RX2 (Đọc dữ liệu từ GPS TX sau khi sửa phần cứng)
#define PIN_GPS_TX       17    // TX2 (Gửi cấu hình sang GPS RX sau khi sửa phần cứng)

// --- Wi-Fi Credentials for Blynk ---
#define WIFI_SSID        "VIETSET_TECH" // Thay bằng tên Wi-Fi nhà bạn
#define WIFI_PASS        "vs68686868"    // Thay bằng mật khẩu Wi-Fi nhà bạn

#endif // CONFIG_H
