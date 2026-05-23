/**
 * Hardware_Test_MPU6050.ino - Test cảm biến gia tốc và góc nghiêng MPU6050
 *
 * Mở Serial Monitor với tốc độ 115200.
 *  - Giữ mạch đứng yên trong 2-3 giây đầu tiên để thực hiện quá trình hiệu chuẩn (calibration).
 *  - Nghiêng mạch sang trái, phải, trước, sau để xem sự thay đổi góc độ.
 */

#include "TestPins.h"
#include <Wire.h>
#include <MPU6050_light.h>

MPU6050 mpu(Wire);
unsigned long lastUpdateMs = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println(F("\n=== HỘP ĐEN - TEST CẢM BIẾN MPU6050 ==="));

  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  delay(100);

  byte status = mpu.begin();
  if (status != 0) {
    Serial.print(F("[FAIL] Không kết nối được với MPU6050. Mã lỗi: "));
    Serial.println(status);
    Serial.println(F("-> Vui lòng kiểm tra lại chân SDA (21) và SCL (22), nguồn cấp (3.3V) và địa chỉ của chip."));
    while (1) {
      delay(1000);
    }
  }

  Serial.println(F("[PASS] Tìm thấy cảm biến MPU6050."));
  Serial.println(F("Đang tiến hành hiệu chuẩn nhanh (VUI LÒNG ĐỂ THIẾT BỊ ĐỨNG YÊN HOÀN TOÀN)..."));
  
  // Nháy nhẹ led báo hiệu hiệu chuẩn
  pinMode(PIN_STATUS_LED, OUTPUT);
  digitalWrite(PIN_STATUS_LED, HIGH);
  mpu.calcOffsets(true, true);
  digitalWrite(PIN_STATUS_LED, LOW);

  Serial.println(F("Hiệu chuẩn XONG! Bắt đầu đọc dữ liệu:"));
  Serial.println(F("-----------------------------------------------------------------"));
  Serial.println(F("Gia tốc Accel (X, Y, Z) [g]   |   Góc xoay Angle (Roll, Pitch) [độ]"));
  Serial.println(F("-----------------------------------------------------------------"));
}

void loop() {
  mpu.update();

  if (millis() - lastUpdateMs >= 300) { // Cập nhật thông số lên màn hình mỗi 300ms
    lastUpdateMs = millis();

    Serial.print(F("Acc: "));
    Serial.print(mpu.getAccX(), 2);   Serial.print(F(", "));
    Serial.print(mpu.getAccY(), 2);   Serial.print(F(", "));
    Serial.print(mpu.getAccZ(), 2);   Serial.print(F(" \t| Ang: "));
    Serial.print(mpu.getAngleX(), 1); Serial.print(F(", "));
    Serial.println(mpu.getAngleY(), 1);
  }
}
