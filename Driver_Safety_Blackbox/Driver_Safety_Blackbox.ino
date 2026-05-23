/**
 * ==================================================================================
 * Driver_Safety_Blackbox.ino - SKETCH CHÍNH ĐIỀU KHIỂN HỘP ĐEN GIÁM SÁT AN TOÀN
 * ==================================================================================
 * Dự án: Hộp đen giám sát an toàn xe điện học sinh (GPS + AI)
 * Thiết kế phần cứng: Altium Designer PCB
 * Ngôn ngữ lập trình: C/C++ (Arduino framework)
 * Tác giả: Antigravity AI (Google DeepMind Team)
 * 
 * Mô tả: Phân rã luồng chương trình một cách khoa học:
 *        - Đọc và lọc dữ liệu MPU6050 ở tần số 50Hz.
 *        - Ghi log CSV vào thẻ nhớ SD (Chế độ thu thập).
 *        - Xử lý luồng định vị vệ tinh GPS NEO-6M thời gian thực.
 *        - Tích hợp mạng nơ-ron AI TinyML phát hiện hành vi lạng lách nguy hiểm.
 *        - Gửi SMS báo cứu hộ khẩn cấp chứa link Google Maps khi ngã xe.
 *        - Cơ chế quản lý điện năng thông minh (Deep Sleep) tự thức giấc.
 * ==================================================================================
 */

#include "Config.h"
#include "MPU6050_Module.h"
#include "GPS_Module.h"
#include "SD_Module.h"
#include "Power_Module.h"
#include "BlynkNotifier.h"
#include "LCD_Module.h"

// ==================================================================================
// LỰA CHỌN PHƯƠNG ÁN AI / THUẬT TOÁN PHÁT HIỆN LẠNG LÁCH
// ==================================================================================
// 0: Sử dụng thuật toán Heuristic (Góc nghiêng Roll & Độ lệch chuẩn)
// 1: Sử dụng AI TinyML cục bộ (Mô hình 1D-CNN TensorFlow Lite, xuất từ train_model.py)
// 2: Sử dụng AI Edge Impulse (Thư viện Arduino xuất dạng ZIP)
#define AI_SWERVING_METHOD    1

#if AI_SWERVING_METHOD == 1
#include <Driver_Safety_TinyML.h>
DriverSafetyTinyML tinyML;
#elif AI_SWERVING_METHOD == 2
#include <Driver_Safety_AI_inferencing.h>

// Hàm suy luận TinyML từ Edge Impulse thời gian thực
void runTinyMLInference(float ax, float ay, float az, float gx, float gy, float gz) {
    float features[6] = { ax, ay, az, gx, gy, gz };

    signal_t signal;
    int err = numpy::signal_from_buffer(features, sizeof(features) / sizeof(features[0]), &signal);
    if (err != 0) {
        Serial.println("[AI EI Error]: Khong the chuyen doi tin hieu!");
        return;
    }

    ei_impulse_result_t result = { 0 };
    err = run_classifier(&signal, &result, false);
    if (err != 0) {
        Serial.println("[AI EI Error]: Suy luan that bai!");
        return;
    }

    float confidenceLangLach = result.classification[1].value;
    if (confidenceLangLach > 0.80) {
        Serial.print(">>> [CANH BAO AI EDGE IMPULSE]: PHAT HIEN LANG LACH! Conf: ");
        Serial.println(confidenceLangLach, 2);
        
        digitalWrite(PIN_PIR_LED, HIGH);
        digitalWrite(PIN_BUZZER, HIGH);
        delay(100);
        digitalWrite(PIN_PIR_LED, LOW);
        digitalWrite(PIN_BUZZER, LOW);
        
        sdModule.logSystemEvent("EDGE IMPULSE SWERVING DETECTED! Confidence: " + String(confidenceLangLach, 2));
    }
}
#endif

// ==========================================
// 1. KHỞI TẠO CÁC ĐỐI TƯỢNG MODULE LỚP
// ==========================================
MPU6050Module mpuModule;
GPSModule     gpsModule(Serial2);  // GPS NEO-6M kết nối cổng Serial2
SDModule      sdModule;
PowerModule   powerModule;
LCDModule     lcdModule;

// Khai báo biến thời gian lấy mẫu
unsigned long lastSamplingTime = 0;

// Khai báo biến quản lý còi báo động non-blocking (kêu tít tít trong 3 giây)
unsigned long alarmEndTime = 0;
unsigned long lastAlarmToggleTime = 0;
bool alarmState = false;


// ==========================================
// 3. HÀM SETUP KHỞI ĐỘNG HỆ THỐNG
// ==========================================
void setup() {
    // Khởi tạo các module cơ bản
    Serial.begin(115200);
    delay(1000);
    Serial.println(F("\n=== HOP DEN GIAM SAT AN TOAN XE DIEN - KHOI DONG ==="));

    // Khởi tạo Blynk
    BlynkNotifier::init();

    // Thiết lập ngõ ra LED và còi (theo PCB)
    pinMode(PIN_STATUS_LED, OUTPUT);
    pinMode(PIN_PIR_LED, OUTPUT);
    pinMode(PIN_BUZZER, OUTPUT);
    
    digitalWrite(PIN_PIR_LED, LOW);
    digitalWrite(PIN_BUZZER, LOW);
    digitalWrite(PIN_STATUS_LED, LOW);

    // ==========================================
    // KIỂM TRA ĐÁNH THỨC TỪ DEEP SLEEP (HẠN CHẾ DÒNG TĨNH)
    // ==========================================
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
        Serial.println(F("[Sleep Mode]: Đánh thức định kỳ 5s để quét rung động xe..."));
        
        // Khởi động nhanh I2C và MPU6050
        Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
        MPU6050 quickMpu(Wire);
        if (quickMpu.begin() == 0) {
            quickMpu.calcOffsets(true, true);
            float startAx = quickMpu.getAccX();
            float startAy = quickMpu.getAccY();
            float startAz = quickMpu.getAccZ();
            bool hasMovement = false;
            
            // Đo rung động trong 200ms
            for (int i = 0; i < 10; i++) {
                delay(20);
                quickMpu.update();
                float delta = abs(quickMpu.getAccX() - startAx) + abs(quickMpu.getAccY() - startAy) + abs(quickMpu.getAccZ() - startAz);
                if (delta > 0.08) {
                    hasMovement = true;
                    break;
                }
            }

            if (!hasMovement) {
                Serial.println(F("[Sleep Mode]: Xe vẫn đứng yên hoàn toàn. Tiếp tục ngủ sâu..."));
                esp_sleep_enable_timer_wakeup(5000000);
                esp_deep_sleep_start();
            }
            Serial.println(F("[Sleep Mode]: Phát hiện rung động dắt xe/nổ máy! Thức giấc hoàn toàn!"));
        }
    }

    // ==========================================
    // KHỞI TẠO CÁC KHỐI CHỨC NĂNG NGOẠI VI
    // ==========================================
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    lcdModule.begin();
    
    powerModule.begin();
    
    if (!mpuModule.begin()) {
        Serial.println(F("Lỗi hệ thống: MPU6050 initialization failed!"));
    }
    
    sdModule.begin();
    if (CURRENT_MODE == MODE_INFERENCE) {
        gpsModule.begin();
#if AI_SWERVING_METHOD == 1
        tinyML.begin();
#endif
    }

    // Nháy LED báo khởi động thành công
    powerModule.flashStatusLED(300, 3);
    sdModule.logSystemEvent("System initialized successfully in mode: " + String(CURRENT_MODE));
    // Menu removed; system proceeds directly without user interaction.
    
    lastSamplingTime = millis();
}

// ==========================================
// 4. VÒNG LẶP CHƯƠNG TRÌNH LOOP CHÍNH
// ==========================================
void loop() {
    // Blynk background processing
    BlynkNotifier::run();

    // Chỉ cập nhật dữ liệu định vị GPS ở chế độ chạy thực tế (Giai đoạn 2)
    // GPS cần được quét liên tục không giới hạn thời gian để tránh tràn bộ đệm Serial
    if (CURRENT_MODE == MODE_INFERENCE) {
        gpsModule.update();
    }

    // Lấy mẫu cảm biến MPU6050 chính xác ở tần số 50Hz (mỗi 20ms)
    if (millis() - lastSamplingTime >= SAMPLING_PERIOD_MS) {
        lastSamplingTime = millis();

        // Cập nhật dữ liệu góc nghiêng và gia tốc từ cảm biến IMU
        mpuModule.update();

        // Đọc các giá trị thô từ cảm biến
        float ax = mpuModule.getAccX();
        float ay = mpuModule.getAccY();
        float az = mpuModule.getAccZ();
        float gx = mpuModule.getGyroX();
        float gy = mpuModule.getGyroY();
        float gz = mpuModule.getGyroZ();
        
        float speed = 0.0;
        if (CURRENT_MODE == MODE_INFERENCE) {
            speed = gpsModule.getSpeedKmh();
        }

        // ==================================================================================
        // CHẾ ĐỘ 1: THU THẬP DỮ LIỆU CẢM BIẾN THÔ (DATA COLLECTION MODE)
        // ==================================================================================
        if (CURRENT_MODE == MODE_DATA_COLLECTION) {
            // Định dạng dòng CSV: Thời gian, AccX, AccY, AccZ, GyroX, GyroY, GyroZ
            String csvRow = String(lastSamplingTime) + "," +
                            String(ax, 4) + "," + String(ay, 4) + "," + String(az, 4) + "," +
                            String(gx, 2) + "," + String(gy, 2) + "," + String(gz, 2);

            // 1. In ra cổng Serial Debug (Dành cho Edge Impulse Data Forwarder qua cáp)
            Serial.println(csvRow);

            // 2. Ghi trực tiếp vào thẻ nhớ MicroSD (Dành cho việc chạy thực tế lưu trữ)
            if (sdModule.isSDConnected()) {
                sdModule.logSensorDataRow(csvRow);
            }

            // Nhấp nháy nhanh LED Status báo hiệu đang ghi dữ liệu
            digitalWrite(PIN_STATUS_LED, !digitalRead(PIN_STATUS_LED));
        }
        
        // ==================================================================================
        // CHẾ ĐỘ 2: NHẬN DIỆN THỰC TẾ & KHỞI CHẠY AI (INFERENCE MODE)
        // ==================================================================================
        else if (CURRENT_MODE == MODE_INFERENCE) {
            // Giám sát dung lượng rung động của xe phục vụ Deep Sleep
            static unsigned long swerveStartMillis = 0;
            static unsigned long lastSwerveMillis = 0;
            powerModule.monitorMovement(ax, ay, az);
            
            // Nháy LED chậm định kỳ 1 giây báo trạng thái hoạt động bình thường
            if (millis() % 1000 < 50) {
                digitalWrite(PIN_STATUS_LED, HIGH);
            } else {
                digitalWrite(PIN_STATUS_LED, LOW);
            }

            // ==========================================
            // TÁC VỤ 1: KIỂM TRA ĐỔ NGÃ XE & GỬI SMS CỨU HỘ
            // ==========================================
            // ==========================================
            // TÁC VỤ 1: KIỂM TRA ĐỔ NGÃ XE (YÊU CẦU LẠNG LÁCH 5-10S TRƯỚC ĐÓ VÀ ĐỨNG IM)
            // ==========================================
            bool isTilted = (abs(mpuModule.getRoll()) > 60.0 || abs(mpuModule.getPitch()) > 60.0);
            bool isFall = mpuModule.checkFallAccident();
            bool recentlySwervedSustained = (swerveStartMillis != 0 && 
                                             (lastSwerveMillis - swerveStartMillis >= 5000));
            
            if (isFall && recentlySwervedSustained) {
                Serial.println(F("🚨🚨🚨 [CẢNH BÁO TAI NẠN]: PHÁT HIỆN XE ĐỔ NGÃ VÀ NẰM IM!"));
                sdModule.logSystemEvent("CRASH ACCIDENT DETECTED!");

                // 1. Hú còi + nháy LED PIR kêu gọi cứu nạn + hien thi LCD
                digitalWrite(PIN_PIR_LED, HIGH);
                digitalWrite(PIN_BUZZER, HIGH);
                lcdModule.displayCrash();

                // 2. Trích xuất vị trí bản đồ Google Maps từ GPS vệ tinh
                String mapLink = gpsModule.getGoogleMapsLink();
                Serial.print(F("Vị trí tai nạn: "));
                Serial.println(mapLink);
                // 3. Gửi cảnh báo tai nạn qua Blynk
                BlynkNotifier::sendEmergencyAlert(mapLink);
                sdModule.logSystemEvent("Emergency alert sent via Blynk. Map: " + mapLink);

                // 4. Khóa chương trình trong vòng lặp còi hú liên hồi báo động cứu nạn tại chỗ
                // Phát còi liên tục khi phát hiện tai nạn
                tone(PIN_BUZZER, 2000); // 2kHz
                while (true) {
                    digitalWrite(PIN_PIR_LED, HIGH);
                    // Buzzer already sounding via tone()
                    delay(300);
                    digitalWrite(PIN_PIR_LED, LOW);
                    delay(300);
                }
                // Khi thoát (không bao giờ tới), tắt âm thanh
                noTone(PIN_BUZZER);
            }
        

// ==========================================
            // TÁC VỤ 2: KIỂM TRA CẢNH BÁO QUÁ TỐC ĐỘ GIỚI HẠN
            // ==========================================
            if (speed > SPEED_LIMIT_KMH) {
                Serial.print(F("⚠️ [Tốc độ]: Chạy quá giới hạn! Tốc độ hiện tại: "));
                Serial.print(speed);
                Serial.println(F(" km/h"));

                // Kích hoạt còi báo động kêu trong 3 giây
                alarmEndTime = millis() + 3000;
                
                static unsigned long lastOverspeedLog = 0;
                if (millis() - lastOverspeedLog > 10000) { // Log sự cố quá tốc độ mỗi 10 giây
                    sdModule.logSystemEvent("OVERSPEED DETECTED! Speed: " + String(speed, 1) + " km/h");
                    lastOverspeedLog = millis();
                }
            }

            // ==========================================
            // TÁC VỤ 3: NHẬN DIỆN HÀNH VI LẠNG LÁCH ĐÁNH VÕNG
            // ==========================================
            bool swervingDetected = false;

            #if AI_SWERVING_METHOD == 0
            // PHƯƠNG ÁN A: SỬ DỤNG THUẬT TOÁN BỔ TRỢ HẸN GIỜ (HEURISTIC)
            if (mpuModule.detectSwerving(speed)) {
                swervingDetected = true;
                Serial.println(F("⚠️⚠️⚠️ [CẢNH BÁO HEURISTIC]: PHÁT HIỆN LẠNG LÁCH DỒN DẬP!"));
                sdModule.logSystemEvent("HEURISTIC SWERVING DETECTED! Speed: " + String(speed, 1));
            }

            #elif AI_SWERVING_METHOD == 1
            // PHƯƠNG ÁN B: SỬ DỤNG AI TINYML CỤC BỘ (TFLITE MICRO)
            static float lastPredictConfidence = 0.0;
            tinyML.addSample(ax, ay, az, gx, gy, gz);
            if (tinyML.isReady()) {
                float confidence = tinyML.predictSwerving();
                lastPredictConfidence = confidence;
                // Gửi confidence lên Blynk để hiển thị
                BlynkNotifier::sendConfidence(confidence);
                
                // Hạn chế in Serial liên tục, chỉ in mỗi 1 giây để dễ quan sát
                static unsigned long lastInferencePrint = 0;
                if (millis() - lastInferencePrint > 1000) {
                    Serial.print(F("AI TinyML Swerving Probability: "));
                    Serial.println(confidence, 4);
                    lastInferencePrint = millis();
                }

                if (confidence > BlynkNotifier::confidenceThreshold) { // So sánh với ngưỡng cài đặt từ Blynk
                    swervingDetected = true;
                    Serial.print(F("[AI]: PHAT HIEN LANG LACH NGUY HIEM! Conf: "));
                    Serial.println(confidence, 2);
                    sdModule.logSystemEvent("AI TINYML SWERVING DETECTED! Confidence: " + String(confidence, 2));
                }
            }

            #elif AI_SWERVING_METHOD == 2
            // PHƯƠNG ÁN C: SỬ DỤNG AI TINYML EDGE IMPULSE
            runTinyMLInference(ax, ay, az, gx, gy, gz);
            #endif


            if (swervingDetected) {
                if (swerveStartMillis == 0) {
                    swerveStartMillis = millis();
                }
                lastSwerveMillis = millis();
                alarmEndTime = millis() + 3000;

                Serial.print(F("[AI/Heuristic]: Dang lang lach! Thoi gian lien tuc: "));
                Serial.print((lastSwerveMillis - swerveStartMillis) / 1000.0);
                Serial.println(F("s"));
            } else {
                // Nếu quá 15 giây không phát hiện lạng lách và xe KHÔNG bị nghiêng đổ, reset bộ đếm liên tục
                // Điều này cho phép lưu lại bộ nhớ lạng lách trước khi xe ngã nằm im (vốn mất ít nhất 3 giây)
                if (!isTilted && swerveStartMillis != 0 && millis() - lastSwerveMillis > 15000) {
                    swerveStartMillis = 0;
                }
            }

            // ==========================================
            // QUẢN LÝ CÒI BÁO ĐỘNG NON-BLOCKING (TÍT TÍT 3 GIÂY)
            // ==========================================
            if (millis() < alarmEndTime) {
                // Sử dụng tone() để phát âm thanh còi liên tục trong thời gian alarm
                if (millis() - lastAlarmToggleTime >= 150) { // 150ms mỗi lần kiểm tra
                    lastAlarmToggleTime = millis();
                    // Bật hoặc tắt âm thanh tùy trạng thái alarm
                    if (millis() < alarmEndTime) {
                        tone(PIN_BUZZER, 2000); // 2kHz tone
                    } else {
                        noTone(PIN_BUZZER);
                    }
                    // Cùng lúc bật/tắt đèn báo
                    digitalWrite(PIN_PIR_LED, (millis() < alarmEndTime) ? HIGH : LOW);
                    alarmState = (millis() < alarmEndTime);
                }
            } else {
                if (alarmEndTime != 0) {
                    noTone(PIN_BUZZER);
                    digitalWrite(PIN_PIR_LED, LOW);
                    alarmEndTime = 0;
                }
            }

            // ==========================================
            // TÁC VỤ 5: CẬP NHẬT MÀN HÌNH LCD 2004 (MỖI 1 GIÂY)
            // ==========================================
            static unsigned long lastLCDUpdateTime = 0;
            if (millis() - lastLCDUpdateTime >= 1000) {
                lastLCDUpdateTime = millis();
                lcdModule.update(speed, lastPredictConfidence, BlynkNotifier::confidenceThreshold, gpsModule.isLocationValid(), sdModule.isSDConnected());
            }

            // ==========================================
            // TÁC VỤ 4: QUẢN LÝ NĂNG LƯỢNG TIẾT KIỆM (DEEP SLEEP)
            // ==========================================
            if (powerModule.isTimeoutReached()) {
                sdModule.logSystemEvent("System going to Deep Sleep due to inactivity.");
                powerModule.enterDeepSleep();
            }
        }
    }
}

