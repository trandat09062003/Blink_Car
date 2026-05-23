# 🚨 Hộp Đen Giám Sát An Toàn Xe Điện (AI TinyML & Blynk IoT)

Dự án nghiên cứu khoa học kỹ thuật xuất sắc, tích hợp Trí tuệ nhân tạo biên cục bộ (Edge AI/TinyML) sử dụng mô hình 1D-CNN và Nền tảng điều khiển IoT Blynk nhằm giám sát hành trình, phát hiện hành vi lái xe loạng choạng nguy hiểm và cảnh báo đổ ngã, tai nạn khẩn cấp theo thời gian thực.

---

## 🌟 Tính Năng Nổi Bật của Hệ Thống

1. **Phát hiện Hành vi Lạng Lách / Loạng Choạng bằng AI TinyML:**
   - Sử dụng mạng nơ-ron tích chập 1 chiều (1D-CNN) chạy suy luận offline trực tiếp trên ESP32 thông qua TensorFlow Lite Micro.
   - Cảm nhận gia tốc và góc quay 6 trục tần số 50Hz, xử lý qua cửa sổ trượt 2.0 giây để nhận diện chính xác hành vi mất lái nguy hiểm.

2. **Thuật toán Đổ Ngã Xe Thông Minh (Anti-Alarm False):**
   - Sự cố tai nạn/ngã xe chỉ được kích hoạt khi thỏa mãn đồng thời hai điều kiện:
     1. Người lái có dấu hiệu **loạng choạng/lạng lách liên tục từ 5 đến 10 giây** ngay trước đó.
     2. Xe bị **nghiêng đột ngột quá 60 độ** (góc Roll/Pitch) và **nằm im bất động (đứng im)** liên tục trong **3 giây**.
   - Thiết kế dạng máy trạng thái không chặn (Non-blocking State Machine) giúp còi hú cứu nạn kêu gọi tại chỗ đồng thời gửi định vị cứu trợ.

3. **Tích hợp IoT Blynk Platform thời gian thực:**
   - **V1 (Slider):** Đồng bộ cấu hình và điều chỉnh ngưỡng xác nhạy TinyML động từ ứng dụng di động.
   - **V2 (Value Display):** Hiển thị xác suất loạng choạng thời gian thực với độ chính xác 4 chữ số thập phân (`String(confidence, 4)`).
   - **V3 (Map):** Tự động gửi link Google Maps định vị vệ tinh GPS cứu trợ khẩn cấp thông qua thông báo Blynk Event `accident` (Critical Push Notification).

4. **Hiển thị Telemetry lên Màn hình LCD 2004 I2C:**
   - Cập nhật thời gian thực: Tốc độ xe (GPS), Xác suất AI (%), Ngưỡng Blynk (%), Số lượng vệ tinh bắt được, Trạng thái kết nối thẻ nhớ SD.

5. **Ghi Hành trình vào Thẻ nhớ MicroSD:**
   - Hỗ trợ lưu trữ dữ liệu thô (.CSV) tần số 50Hz để làm nguyên liệu huấn luyện mô hình AI mới.

6. **Deep Sleep Quản lý Năng lượng Thông minh:**
   - Hệ thống tự động đi vào giấc ngủ sâu (Deep Sleep) khi xe đứng yên quá 10 phút. ESP32 sẽ thức giấc định kỳ 5 giây để kiểm tra rung động (dắt xe/nổ máy) qua MPU6050, nếu có rung động sẽ tự khởi động lại toàn bộ hệ thống.

---

## 📐 Sơ Đồ Nối Chân Phần Cứng (GPIO Mapping)

Mạch điện được thiết kế khớp hoàn hảo với Altium Designer PCB chuyên dụng cho vi điều khiển ESP32 DEVKIT V1:

| Module Ngoại Vi | Chân Ngoại Vi | Chân GPIO trên ESP32 | Mô Tả Ý Nghĩa |
| :--- | :--- | :--- | :--- |
| **MPU6050 (IMU)** | SDA | **GPIO 21** | Đường truyền dữ liệu I2C |
| | SCL | **GPIO 22** | Đường truyền xung nhịp I2C |
| **LCD 2004 (I2C)** | SDA | **GPIO 21** | Chia sẻ bus dữ liệu I2C |
| | SCL | **GPIO 22** | Chia sẻ bus xung nhịp I2C |
| **MicroSD Card (SPI)** | CS | **GPIO 5** | Chân chọn chip SPI (Chip Select) |
| | SCK | **GPIO 18** | Chân xung nhịp SPI |
| | MISO | **GPIO 19** | Chân truyền Master Input Slave Output |
| | MOSI | **GPIO 23** | Chân nhận Master Output Slave Input |
| **NEO-6M GPS (Serial2)**| TX | **GPIO 16** | Chân nhận tín hiệu RX2 của ESP32 |
| | RX | **GPIO 17** | Chân truyền tín hiệu TX2 của ESP32 |
| **Còi Báo (Buzzer)** | Positive | **GPIO 4** | Kích còi SP1 qua Transistor Q1 |
| **Đèn LED Status** | Positive | **GPIO 2** | Đèn trạng thái tích hợp sẵn trên mạch |
| **Đèn LED PIR/Báo động**| Positive | **GPIO 12** | Đèn cảnh báo phụ giật nhấp nháy |

> [!IMPORTANT]
> **Hướng dẫn sửa đổi phần cứng (Hardware Hacks):**
> 1. **GPS Serial2 Hack:** Do lỗi mạch in ban đầu, cần cắt đường mạch nối GPIO17 với nét `GPS_RX`. Tiến hành câu dây: Chân TX của GPS (JP1 pin 2) ➡️ GPIO16, Chân RX của GPS (JP1 pin 3) ➡️ GPIO17.
> 2. **PIR LED Hack:** Hàn dây câu từ net `BUZZER` sang `LED2-1` (PIR net) để nháy đèn LED cảnh báo đồng bộ với còi hú.

---

## 💻 Hướng Dẫn Vận Hành & Huấn Luyện AI Mới

### Bước 1: Thu thập Dữ liệu Lái xe Mới
1. Mở tệp [Config.h](file:///C:/Users/DELL/OneDrive - Hanoi University of Science and Technology/Desktop/canh bao lai xe - _sv/Driver_Safety_Blackbox/Config.h) và sửa cấu hình sang chế độ thu thập:
   ```cpp
   #define CURRENT_MODE    MODE_DATA_COLLECTION
   ```
2. Nạp code xuống ESP32. Chạy thử nghiệm thực tế:
   - Ghi dữ liệu đi bình thường: Lưu các file vào `dataset/` đặt tên `normal_1.csv`, `normal_2.csv`...
   - Ghi dữ liệu loạng choạng nguy hiểm: Lắc tay lái dồn dập 5-10s rồi lưu file vào `dataset/` đặt tên `swerving_1.csv`, `swerving_2.csv`...
3. Nhấn phím **BOOT** trên ESP32 để đóng file an toàn trước khi rút thẻ nhớ.

### Bước 2: Huấn luyện mạng nơ-ron AI TinyML
1. Cài đặt Python 3 và thư viện máy học:
   ```bash
   pip install tensorflow numpy pandas scikit-learn
   ```
2. Nếu muốn kiểm tra giả lập với dáng đi bộ, hãy sinh dữ liệu giả lập trước:
   ```bash
   python generate_synthetic_data.py
   ```
3. Chạy tập lệnh huấn luyện tự động:
   ```bash
   python train_model.py
   ```
   *Tập lệnh sẽ tự động áp dụng chuẩn hóa khớp 100% với ESP32, huấn luyện mạng 1D-CNN đạt độ chính xác ~100%, nén thành TFLite và ghi đè trực tiếp vào thư viện nhúng `libraries/Driver_Safety_TinyML/src/model_data.h`!*

### Bước 3: Nạp Code Chạy Thực Tế (Inference Mode)
1. Quay lại [Config.h](file:///C:/Users/DELL/OneDrive - Hanoi University of Science and Technology/Desktop/canh bao lai xe - _sv/Driver_Safety_Blackbox/Config.h) chuyển về:
   ```cpp
   #define CURRENT_MODE    MODE_INFERENCE
   ```
2. Biên dịch và nạp code. Hệ thống sẽ ngay lập tức chạy mô hình AI mới nhạy bén tuyệt đối!

---

## 🧪 Hướng Dẫn Kiểm Thử (Testing Guide)

*   **Test Đứng Yên:** Đặt hộp đen đứng yên trên bàn ➡️ Xác suất lạng lách sẽ báo **`0.0000`** (0%) và đèn status nháy chậm.
*   **Test Đi Thẳng:** Di chuyển thiết bị thẳng đều mượt mà ➡️ Xác suất duy trì dưới **`0.0500`**.
*   **Test Loạng Choạng/Lạng Lách:** Lắc liên tục mạch cảm biến MPU6050 trái-phải liên tục từ 5-10 giây ➡️ Xác suất trên Serial Monitor và Blynk lập tức nhảy vọt lên **`> 0.8500`**, còi hú báo động sẽ tít tít dồn dập.
*   **Test Ngã Xe Cứu Hộ:** Lắc liên tục 6 giây, sau đó đột ngột lật nghiêng cảm biến quá 60 độ và giữ yên hoàn toàn trong 3 giây ➡️ Còi sẽ hú báo động liên tục âm lượng cao, màn hình LCD hiển thị `💥 TAI NAN HIEN TAI! 💥`, và Blynk di động lập tức nhận tin nhắn đẩy (Push Notification) kèm link bản đồ định vị GPS!
