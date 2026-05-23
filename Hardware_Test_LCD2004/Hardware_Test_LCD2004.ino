/**
 * Hardware_Test_LCD2004.ino - Chương trình kiểm tra màn hình LCD 2004 (I2C)
 * 
 * Chức năng:
 *  1. Quét bus I2C chỉ trong dải địa chỉ của chip chuyển đổi PCF8574 (0x20 - 0x27)
 *     và PCF8574A (0x38 - 0x3F) để tránh nhận nhầm địa chỉ 0x68 của MPU6050.
 *  2. Tự động khởi tạo màn hình LCD 2004 (20 cột, 4 dòng) với địa chỉ tìm được.
 *  3. Phát tiếng bíp còi buzzer (GPIO4) báo hiệu khởi động thành công.
 *  4. Hiển thị thông báo "  HARDWARE TEST OK  " cùng các thông số phần cứng lên màn hình.
 *  5. Chớp tắt đèn nền LCD và nhấp nháy LED Status (GPIO2) theo chu kỳ.
 */

#include "TestPins.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C *lcd = nullptr;
byte lcdAddr = 0;
bool lcdFound = false;

// Hàm phát tiếng bíp còi buzzer
void beep(int count, int delayMs) {
  for (int i = 0; i < count; i++) {
    digitalWrite(PIN_BUZZER, HIGH);
    delay(delayMs);
    digitalWrite(PIN_BUZZER, LOW);
    if (i < count - 1) {
      delay(delayMs);
    }
  }
}

void setup() {
  // Cấu hình chân LED và Còi
  pinMode(PIN_STATUS_LED, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_STATUS_LED, LOW);
  digitalWrite(PIN_BUZZER, LOW);

  // Khởi tạo Serial
  Serial.begin(115200);
  delay(1000);
  Serial.println(F("\n=== THỬ NGHIỆM PHẦN CỨNG: MÀN HÌNH LCD 2004 (20x4) ==="));
  Serial.print(F("Cấu hình I2C: SDA = ")); Serial.print(PIN_I2C_SDA);
  Serial.print(F(", SCL = ")); Serial.println(PIN_I2C_SCL);

  // Khởi tạo bus I2C
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  delay(500);

  // 1. Quét tìm địa chỉ LCD (PCF8574/A)
  Serial.println(F("Đang quét tìm màn hình LCD qua bus I2C..."));
  
  // Dải địa chỉ PCF8574 (0x20-0x27) và PCF8574A (0x38-0x3F)
  byte targetAddresses[] = {
    0x27, 0x3F, // Hai địa chỉ mặc định phổ biến nhất
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E
  };
  int numAddresses = sizeof(targetAddresses) / sizeof(targetAddresses[0]);

  for (int i = 0; i < numAddresses; i++) {
    byte addr = targetAddresses[i];
    Wire.beginTransmission(addr);
    byte error = Wire.endTransmission();

    if (error == 0) {
      lcdAddr = addr;
      lcdFound = true;
      Serial.print(F("-> Tìm thấy LCD tại địa chỉ: 0x"));
      Serial.println(addr, HEX);
      break;
    }
  }

  if (lcdFound) {
    // 2. Khởi tạo LCD 20x4
    lcd = new LiquidCrystal_I2C(lcdAddr, 20, 4);
    lcd->init();
    lcd->backlight();
    lcd->clear();

    // Hiển thị thông tin lên LCD
    lcd->setCursor(0, 0);
    lcd->print("== HARDWARE TEST ==");
    
    lcd->setCursor(0, 1);
    lcd->print("  HARDWARE TEST OK  ");
    
    lcd->setCursor(0, 2);
    lcd->print("LCD: 2004 (I2C:0x");
    lcd->print(lcdAddr < 16 ? "0" : "");
    lcd->print(lcdAddr, HEX);
    lcd->print(")");
    
    lcd->setCursor(0, 3);
    lcd->print("SDA:21 SCL:22 OK");

    Serial.println(F("[OK]: Đã khởi tạo thành công LCD 2004!"));
    
    // Còi bíp 2 tiếng chào mừng thành công
    beep(2, 80);
  } else {
    Serial.println(F("[ERROR]: Không tìm thấy LCD 2004 qua I2C!"));
    Serial.println(F("-> Hãy kiểm tra lại dây cáp nguồn, SDA/SCL và biến trở điều chỉnh độ tương phản."));
    
    // Còi bíp 1 tiếng dài báo lỗi
    beep(1, 400);
  }
}

void loop() {
  if (lcdFound) {
    // Nhấp nháy LED Status báo hiệu ESP32 đang chạy
    digitalWrite(PIN_STATUS_LED, HIGH);
    delay(500);
    digitalWrite(PIN_STATUS_LED, LOW);
    delay(500);
  } else {
    // Nhấp nháy LED nhanh báo lỗi không tìm thấy LCD
    digitalWrite(PIN_STATUS_LED, HIGH);
    delay(100);
    digitalWrite(PIN_STATUS_LED, LOW);
    delay(100);
  }
}
