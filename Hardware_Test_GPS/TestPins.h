/**
 * TestPins.h - Chân GPIO khớp PCB Altium (dùng cho sketch test phần cứng)
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

#define PIN_GPS_RX       16    // RX2 (nhận từ GPS TX - cần câu dây)
#define PIN_GPS_TX       17    // TX2 (gửi sang GPS RX - cần câu dây)

#define PIN_SIM_RST      25    // RST SIM
#define PIN_SIM_RX       26    // RX1 (nhận từ SIM TX)
#define PIN_SIM_TX       27    // TX1 (gửi sang SIM RX)

#define GPS_BAUD         9600
#define SIM_BAUD         115200

#endif
