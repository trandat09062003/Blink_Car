/**
 * TestPins.h - Định nghĩa chân GPIO khớp với PCB Altium (dùng cho dự án thu thập dữ liệu)
 */
#ifndef TEST_PINS_H
#define TEST_PINS_H

#define PIN_STATUS_LED   2     // D2 (STATUS)
#define PIN_PIR_LED      12    // D12 (Cần hàn dây câu sang net PIR trên PCB)
#define PIN_BUZZER       4     // D4 (BIP loa)

#define PIN_I2C_SDA      21    // SDA MPU/LCD
#define PIN_I2C_SCL      22    // SCL MPU/LCD

#define PIN_SD_CS        5     // CS thẻ SD
#define PIN_SD_SCK       18    // SCK thẻ SD
#define PIN_SD_MISO      19    // MISO thẻ SD
#define PIN_SD_MOSI      23    // MOSI thẻ SD

#define PIN_BOOT_BUTTON  0     // Phím BOOT tích hợp trên mạch ESP32

#endif
