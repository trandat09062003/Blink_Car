/**
 * Driver_Safety_Data_Collection.ino - Chương trình thu thập dữ liệu thô (Giai đoạn 1)
 *
 * Dự án: Hộp đen giám sát an toàn xe điện học sinh (GPS + AI)
 * Tác giả: Antigravity AI (Google DeepMind Team)
 *
 * Chức năng:
 *  - Đọc dữ liệu thô 6 trục (Accel X/Y/Z, Gyro X/Y/Z) từ cảm biến MPU6050 ở tần số 50Hz (mỗi 20ms).
 *  - Ghi dữ liệu dạng CSV vào thẻ nhớ MicroSD qua giao tiếp SPI (mỗi lần khởi động tạo một file mới: data_0.csv, data_1.csv, ...).
 *  - In dữ liệu CSV ra cổng Serial để đồng bộ với Edge Impulse Data Forwarder.
 *  - Nhấp nháy LED Status (GPIO 2) báo hiệu đang ghi dữ liệu.
 */

#include "TestPins.h"
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <MPU6050_light.h>

MPU6050 mpu(Wire);
bool isSDReady = false;
String logFilename = "";
File myLogFile;
unsigned long lastSampleMicros = 0;
unsigned long sampleCount = 0;

void setup() {
  // Khởi tạo Serial Debug với PC
  Serial.begin(115200);
  delay(1000);
  Serial.println(F("\n=== HỘP ĐEN GIÁM SÁT AN TOÀN - THU THẬP DỮ LIỆU (GIAI ĐOẠN 1) ==="));

  // Cấu hình chân đèn LED Status
  pinMode(PIN_STATUS_LED, OUTPUT);
  digitalWrite(PIN_STATUS_LED, LOW);

  // Cấu hình còi còi báo hiệu (Buzzer)
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);

  // Cấu hình phím nhấn BOOT trên mạch ESP32
  pinMode(PIN_BOOT_BUTTON, INPUT_PULLUP);

  // 1. Khởi tạo đường truyền I2C
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  delay(100);

  // 2. Khởi tạo cảm biến MPU6050
  Serial.print(F("Khởi tạo MPU6050... "));
  byte mpuStatus = mpu.begin();
  if (mpuStatus != 0) {
    Serial.print(F("THẤT BẠI! Mã lỗi: "));
    Serial.println(mpuStatus);
    Serial.println(F("-> Vui lòng kiểm tra lại chân SDA (21), SCL (22) và nguồn cấp MPU6050."));
    
    // Nhấp nháy đèn LED liên tục báo lỗi MPU6050
    while (1) {
      digitalWrite(PIN_STATUS_LED, HIGH);
      delay(100);
      digitalWrite(PIN_STATUS_LED, LOW);
      delay(100);
    }
  }
  Serial.println(F("THÀNH CÔNG!"));

  // Thực hiện hiệu chuẩn MPU6050 (Yêu cầu giữ mạch thẳng đứng và đứng yên hoàn toàn)
  Serial.println(F("Đang hiệu chuẩn MPU6050... Vui lòng ĐỂ THIẾT BỊ ĐỨNG YÊN HOÀN TOÀN."));
  
  // Bíp còi báo hiệu bắt đầu hiệu chuẩn
  digitalWrite(PIN_BUZZER, HIGH);
  delay(200);
  digitalWrite(PIN_BUZZER, LOW);
  
  mpu.calcOffsets(true, true);
  
  // Bíp 2 tiếng báo hiệu hiệu chuẩn hoàn tất
  digitalWrite(PIN_BUZZER, HIGH);
  delay(100);
  digitalWrite(PIN_BUZZER, LOW);
  delay(100);
  digitalWrite(PIN_BUZZER, HIGH);
  delay(100);
  digitalWrite(PIN_BUZZER, LOW);
  Serial.println(F("Hiệu chuẩn cảm biến MPU6050 thành công!"));

  // 3. Khởi tạo cổng truyền thông SPI và Thẻ nhớ MicroSD
  pinMode(PIN_SD_CS, OUTPUT);
  digitalWrite(PIN_SD_CS, HIGH); // Kéo CS lên cao trước

  // Khởi tạo SPI không truyền CS tránh xung đột
  SPI.begin(PIN_SD_SCK, PIN_SD_MISO, PIN_SD_MOSI);
  delay(100);

  Serial.println(F("Đang kết nối thẻ nhớ MicroSD..."));
  uint32_t speedTest[] = {4000000, 1000000, 400000, 8000000};
  int numSpeeds = sizeof(speedTest) / sizeof(speedTest[0]);

  for (int i = 0; i < numSpeeds; i++) {
    Serial.print(F("Thử kết nối ở tốc độ: "));
    Serial.print(speedTest[i] / 1000);
    Serial.println(F(" kHz..."));

    SD.end();
    delay(100);

    if (SD.begin(PIN_SD_CS, SPI, speedTest[i])) {
      isSDReady = true;
      Serial.print(F("THÀNH CÔNG tại tần số: "));
      Serial.print(speedTest[i] / 1000);
      Serial.println(F(" kHz!"));
      break;
    }
    delay(100);
  }

  if (!isSDReady) {
    Serial.println(F("THẤT BẠI! Không thể kết nối thẻ nhớ."));
    Serial.println(F("-> Vui lòng kiểm tra thẻ nhớ đã cắm chưa, định dạng FAT32 chưa, và các chân SPI (CS=5, SCK=18, MISO=19, MOSI=23)."));
    
    // Nháy LED và kêu còi cảnh báo lỗi phần cứng thẻ nhớ liên tục
    for (int i = 0; i < 5; i++) {
      digitalWrite(PIN_STATUS_LED, HIGH);
      digitalWrite(PIN_BUZZER, HIGH);
      delay(150);
      digitalWrite(PIN_STATUS_LED, LOW);
      digitalWrite(PIN_BUZZER, LOW);
      delay(100);
    }
  } else {
    // Tự động tạo tệp tin mới tuần tự dạng data_0.csv, data_1.csv, ... để tránh ghi đè
    int fileIndex = 0;
    char nameBuf[32];
    do {
      sprintf(nameBuf, "/data_%d.csv", fileIndex++);
    } while (SD.exists(nameBuf));
    logFilename = String(nameBuf);

    Serial.print(F("Tệp tin đang ghi dữ liệu: "));
    Serial.println(logFilename);

    // Mở tệp tin và giữ tệp luôn mở để ghi dữ liệu lâu dài
    myLogFile = SD.open(logFilename, FILE_WRITE);
    if (myLogFile) {
      myLogFile.println("Timestamp_ms,AccX,AccY,AccZ,GyroX,GyroY,GyroZ");
      myLogFile.flush(); // Đẩy tiêu đề xuống thẻ nhớ ngay lập tức
      Serial.println(F("Khởi tạo tiêu đề tệp dữ liệu CSV và giữ tệp mở thành công!"));
      
      // Bíp 1 tiếng dài báo hiệu khởi tạo SD và tạo file thành công
      digitalWrite(PIN_BUZZER, HIGH);
      delay(400);
      digitalWrite(PIN_BUZZER, LOW);
    } else {
      Serial.println(F("[ERROR] Không thể tạo tệp tin ghi trên thẻ SD!"));
      isSDReady = false;
      
      // Nhấp nháy còi/LED báo lỗi tạo file
      for (int i = 0; i < 3; i++) {
        digitalWrite(PIN_STATUS_LED, HIGH);
        digitalWrite(PIN_BUZZER, HIGH);
        delay(200);
        digitalWrite(PIN_STATUS_LED, LOW);
        digitalWrite(PIN_BUZZER, LOW);
        delay(100);
      }
    }
  }

  Serial.println(F("\nBắt đầu lấy mẫu cảm biến (Tần số: 50Hz - Chu kỳ: 20ms)..."));
  Serial.println(F("Định dạng dòng dữ liệu:"));
  Serial.println(F("Timestamp_ms,AccX_g,AccY_g,AccZ_g,GyroX_ds,GyroY_ds,GyroZ_ds"));
  Serial.println(F("================================================================"));

  lastSampleMicros = micros();
}

void loop() {
  // 1. Kiểm tra lệnh dừng từ Serial Monitor (tiện lợi khi test trên bàn)
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    if (cmd == 's' || cmd == 'S') {
      if (isSDReady) {
        isSDReady = false;
        myLogFile.close(); // Đóng file sạch sẽ và cập nhật FAT
        Serial.println(F("\n=== [DA DUNG GHI] Nguoi dung yeu cau dung ghi. Da luu file an toan! ==="));
        // Kêu còi 1 tiếng dài báo hiệu đã ngừng ghi và lưu file
        digitalWrite(PIN_BUZZER, HIGH);
        delay(600);
        digitalWrite(PIN_BUZZER, LOW);
      }
    }
  }

  // 2. Kiểm tra phím BOOT vật lý trên mạch ESP32 để thoát an toàn (khi đang lái xe)
  if (digitalRead(PIN_BOOT_BUTTON) == LOW) {
    delay(50); // Chống rung phím
    if (digitalRead(PIN_BOOT_BUTTON) == LOW) {
      if (isSDReady) {
        isSDReady = false;
        myLogFile.close(); // Đóng file sạch sẽ và cập nhật FAT
        Serial.println(F("\n=== [DA DUNG GHI] Phim BOOT da duoc nhan. Da dong file an toan va luu du lieu! ==="));
        
        // Kêu còi 3 tiếng bíp ngắn liên hồi báo hiệu đã dừng ghi và lưu file thành công
        for (int i = 0; i < 3; i++) {
          digitalWrite(PIN_BUZZER, HIGH);
          delay(150);
          digitalWrite(PIN_BUZZER, LOW);
          delay(100);
        }
      }
      
      // Chờ nhả phím BOOT ra để tránh lặp lại
      while (digitalRead(PIN_BOOT_BUTTON) == LOW) {
        delay(10);
      }
    }
  }

  // Đọc dữ liệu thô liên tục từ cảm biến
  mpu.update();

  unsigned long currentMicros = micros();

  // Định thì lấy mẫu chính xác 20ms (tương ứng tần số 50Hz)
  if (currentMicros - lastSampleMicros >= 20000) {
    lastSampleMicros += 20000; // Tránh trôi thời gian tích lũy
    sampleCount++;

    // Lấy thông số trục cảm biến
    float ax = mpu.getAccX();
    float ay = mpu.getAccY();
    float az = mpu.getAccZ();
    float gx = mpu.getGyroX();
    float gy = mpu.getGyroY();
    float gz = mpu.getGyroZ();
    unsigned long timestampMs = millis();

    // Định dạng CSV dòng dữ liệu
    String csvRow = String(timestampMs) + "," +
                    String(ax, 4) + "," + String(ay, 4) + "," + String(az, 4) + "," +
                    String(gx, 2) + "," + String(gy, 2) + "," + String(gz, 2);

    // 2. In ra cổng Serial Debug (Chỉ in khi SD đang ghi để tránh trôi luồng khi đã dừng)
    if (isSDReady) {
      Serial.println(csvRow);

      // 3. Ghi trực tiếp vào file lưu trên thẻ nhớ SD (đã được mở sẵn từ setup)
      if (myLogFile) {
        myLogFile.println(csvRow);
        
        // Định kỳ flush (mỗi 50 mẫu ~ 1 giây) để ghi đệm xuống thẻ nhớ, tránh mất dữ liệu khi sập nguồn
        if (sampleCount % 50 == 0) {
          myLogFile.flush();
        }
      } else {
        isSDReady = false;
        Serial.println(F("\n[ERROR] File chua duoc mo hoac bi loi the nho!"));
      }

      // Nhấp nháy đèn LED Status báo hiệu đang hoạt động (mỗi 10 chu kỳ lấy mẫu ~ 200ms)
      if (sampleCount % 10 == 0) {
        digitalWrite(PIN_STATUS_LED, !digitalRead(PIN_STATUS_LED));
      }
    } else {
      // Khi đã dừng ghi, tắt LED Status
      digitalWrite(PIN_STATUS_LED, LOW);
    }
  }
}
