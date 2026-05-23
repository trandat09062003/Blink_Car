/**
 * Hardware_Test_GPS.ino - Test module định vị GPS NEO-6M qua cổng Serial2
 *
 * Mở Serial Monitor 115200.
 *  - Sử dụng cổng Serial2 với chân RX=16 và TX=17.
 *  - Lưu ý: Trên mạch PCB Altium cũ, net GPS_TX bị đứt (floating), bạn cần hàn một sợi dây câu nhỏ 
 *    từ chân TX của module GPS sang chân GPIO 16 (RX2) của ESP32.
 *  - Chương trình sẽ liên tục đọc các byte thô từ GPS và hiển thị số lượng byte nhận được.
 *  - Đưa anten ra ngoài trời/cửa sổ để bắt được tín hiệu vệ tinh (khi led đỏ trên module GPS nhấp nháy).
 */

#include "TestPins.h"
#include <TinyGPSPlus.h>

TinyGPSPlus gps;
unsigned long lastPrintMs = 0;
unsigned long totalBytesReceived = 0;
bool hasSeenNmeaStart = false;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println(F("\n=== HỘP ĐEN - TEST MODULE GPS NEO-6M ==="));
  Serial.print(F("Chân RX2 (ESP32) nối với GPS TX: ")); Serial.println(PIN_GPS_RX);
  Serial.print(F("Chân TX2 (ESP32) nối với GPS RX: ")); Serial.println(PIN_GPS_TX);
  Serial.print(F("Tốc độ baud mặc định GPS:       ")); Serial.println(GPS_BAUD);
  Serial.println(F("Đang mở cổng Serial2..."));

  Serial2.begin(GPS_BAUD, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX);
  delay(200);
  Serial.println(F("Bắt đầu nghe tín hiệu từ GPS..."));
}

void loop() {
  // Đọc liên tục dữ liệu từ module GPS gửi về
  while (Serial2.available()) {
    char c = Serial2.read();
    totalBytesReceived++;
    
    if (c == '$') {
      hasSeenNmeaStart = true;
    }
    
    // Đẩy byte vào bộ giải mã TinyGPS++
    gps.encode(c);
  }

  // Định kỳ in thông tin mỗi 1 giây
  if (millis() - lastPrintMs >= 1000) {
    lastPrintMs = millis();

    Serial.print(F("[Thống kê] Số bytes đã nhận: "));
    Serial.print(totalBytesReceived);
    
    if (totalBytesReceived == 0) {
      Serial.println(F(" -> [FAIL] KHÔNG nhận được dữ liệu!"));
      Serial.println(F("   -> Kiểm tra nguồn 5V/3.3V cấp cho GPS, dây nối TX/RX."));
      Serial.println(F("   -> RẤT QUAN TRỌNG: Cần câu dây từ chân TX của GPS sang chân GPIO 16 trên ESP32."));
    } else if (!hasSeenNmeaStart) {
      Serial.println(F(" -> [WARN] Có nhận được byte nhưng chưa thấy ký tự '$' (NMEA start)."));
      Serial.println(F("   -> Có thể sai tốc độ baud (thử đổi baud trong TestPins.h từ 9600 sang 4800 hoặc 115200)."));
    } else {
      Serial.println(F(" -> [OK] Đang nhận luồng dữ liệu NMEA tốt."));
    }

    // Hiển thị kết quả giải mã
    if (gps.location.isValid()) {
      Serial.print(F("  - Vị trí: Lat = "));
      Serial.print(gps.location.lat(), 6);
      Serial.print(F(", Lng = "));
      Serial.println(gps.location.lng(), 6);
      Serial.print(F("  - Số vệ tinh kết nối: "));
      Serial.println(gps.satellites.value());
      Serial.print(F("  - Tốc độ di chuyển: "));
      Serial.print(gps.speed.kmph(), 1);
      Serial.println(F(" km/h"));
    } else {
      Serial.print(F("  - Trạng thái định vị: Đang chờ khóa tín hiệu vệ tinh (GPS Fix)..."));
      if (gps.satellites.value() > 0) {
        Serial.print(F(" (Số vệ tinh nhìn thấy: "));
        Serial.print(gps.satellites.value());
        Serial.print(F(")"));
      }
      Serial.println();
    }
    Serial.println(F("--------------------------------------------------"));
  }
}
