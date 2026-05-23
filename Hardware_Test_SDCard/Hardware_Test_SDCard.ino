/**
 * Hardware_Test_SDCard.ino - Test khe cắm thẻ nhớ MicroSD qua giao tiếp SPI
 *
 * Mở Serial Monitor 115200.
 *  - Cắm thẻ nhớ đã định dạng FAT32 vào khe đọc.
 *  - Chương trình sẽ thử kết nối qua SPI bằng các dải tốc độ khác nhau để tránh nhiễu phần cứng,
 *    đọc dung lượng thẻ nhớ, sau đó thử ghi file test và đọc lại nội dung file đó.
 */

#include "TestPins.h"
#include <SPI.h>
#include <SD.h>

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println(F("\n=== HỘP ĐEN - TEST THẺ NHỚ MICROSD (SPI) ==="));
  Serial.print(F("Chân CS:   ")); Serial.println(PIN_SD_CS);
  Serial.print(F("Chân SCK:  ")); Serial.println(PIN_SD_SCK);
  Serial.print(F("Chân MISO: ")); Serial.println(PIN_SD_MISO);
  Serial.print(F("Chân MOSI: ")); Serial.println(PIN_SD_MOSI);

  // 1. Đảm bảo chân CS được cấu hình là OUTPUT và đặt lên mức HIGH trước 
  // để tránh việc các thiết bị SPI khác chiếm quyền điều khiển đường truyền.
  pinMode(PIN_SD_CS, OUTPUT);
  digitalWrite(PIN_SD_CS, HIGH);

  // 2. Khởi tạo SPI.
  // Không truyền PIN_SD_CS vào SPI.begin(...) để tránh xung đột quyền điều khiển chân CS
  // giữa Driver SPI phần cứng của ESP32 và thư viện phần mềm SD.
  SPI.begin(PIN_SD_SCK, PIN_SD_MISO, PIN_SD_MOSI);
  delay(100);

  // 3. Thử mount thẻ nhớ với các tốc độ SPI khác nhau từ thấp đến cao.
  // (Module thẻ nhớ giá rẻ hoặc dây nối dài thường bị méo tín hiệu ở tần số cao >8MHz).
  bool mountSuccess = false;
  uint32_t speedTest[] = {4000000, 1000000, 400000, 8000000};
  int numSpeeds = sizeof(speedTest) / sizeof(speedTest[0]);

  for (int i = 0; i < numSpeeds; i++) {
    Serial.print(F("Đang thử mount thẻ nhớ ở tần số: "));
    Serial.print(speedTest[i] / 1000);
    Serial.println(F(" kHz..."));

    // Giải phóng bộ nhớ SD trước khi thử lại ở tần số khác
    SD.end();
    delay(100);

    if (SD.begin(PIN_SD_CS, SPI, speedTest[i])) {
      mountSuccess = true;
      Serial.print(F("[PASS] Mount thẻ nhớ THÀNH CÔNG ở tần số: "));
      Serial.print(speedTest[i] / 1000);
      Serial.println(F(" kHz."));
      break;
    }
    delay(200);
  }

  if (!mountSuccess) {
    Serial.println(F("\n[FAIL] Không thể mount thẻ nhớ sau nhiều lần thử ở các tốc độ khác nhau!"));
    Serial.println(F("=========================================================================="));
    Serial.println(F("CÁC BƯỚC KHẮC PHỤC LỖI PHẦN CỨNG THẺ NHỚ:"));
    Serial.println(F(" 1. Định dạng thẻ nhớ:"));
    Serial.println(F("    - Thẻ nhớ phải có dung lượng <= 32GB và định dạng FAT32."));
    Serial.println(F("    - Hệ thống định dạng exFAT (cho thẻ > 64GB) hoặc NTFS sẽ KHÔNG hoạt động."));
    Serial.println(F(" 2. Kiểm tra tiếp tiếp xúc:"));
    Serial.println(F("    - Rút thẻ nhớ ra, lau sạch bề mặt tiếp xúc bằng cồn/khăn khô rồi cắm lại chặt."));
    Serial.println(F(" 3. Nhiễu & sụt nguồn:"));
    Serial.println(F("    - Quá trình đọc/ghi thẻ nhớ tiêu thụ dòng điện tức thời lớn (lên tới 150mA)."));
    Serial.println(F("    - Đảm bảo nguồn cấp VCC 5V hoặc 3.3V cấp cho thẻ ổn định."));
    Serial.println(F("    - Nên hàn thêm tụ lọc nguồn 10uF - 100uF song song với chân nguồn của module thẻ."));
    Serial.println(F(" 4. Đường truyền SPI:"));
    Serial.println(F("    - Đảm bảo các mối hàn chân CS(5), SCK(18), MISO(19), MOSI(23) không bị chập hay đứt đường."));
    Serial.println(F("=========================================================================="));
    return;
  }

  // Đọc loại và dung lượng thẻ
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.print(F("Dung lượng thẻ: "));
  Serial.print((uint32_t)cardSize);
  Serial.println(F(" MB"));

  // Thử ghi file
  Serial.println(F("Đang thử ghi vào file /hw_test.txt..."));
  File file = SD.open("/hw_test.txt", FILE_WRITE);
  if (!file) {
    Serial.println(F("[FAIL] Không thể mở/tạo file để ghi!"));
    return;
  }
  
  file.println("=== KIỂM TRA THẺ NHỚ THÀNH CÔNG ===");
  file.print("Thực hiện lúc millis: ");
  file.println(millis());
  file.close();
  Serial.println(F("[PASS] Đã ghi file hw_test.txt thành công."));

  // Thử đọc lại file
  Serial.println(F("Đang thử đọc lại nội dung file /hw_test.txt:"));
  file = SD.open("/hw_test.txt", FILE_READ);
  if (!file) {
    Serial.println(F("[FAIL] Không thể mở file để đọc!"));
    return;
  }

  Serial.println(F("---------------------------------------------"));
  while (file.available()) {
    Serial.write(file.read());
  }
  Serial.println(F("---------------------------------------------"));
  file.close();
  Serial.println(F("[PASS] Đã đọc lại thành công. Thẻ nhớ hoạt động tốt!"));
}

void loop() {
  // Không làm gì trong vòng lặp
}
