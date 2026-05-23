# Kế hoạch thực hiện: Hộp đen giám sát an toàn xe điện học sinh (GPS + AI)

Tài liệu này mô tả firmware ESP32 và **ánh xạ chân GPIO khớp schematic Altium** (ESP32 DEVKIT V1).

---

## 1. Sơ đồ kết nối phần cứng (Pin Mapping)

| Tên net / thiết bị | GPIO ESP32 | Mô tả |
|---|---|---|
| **STATUS** (LED1) | **2** | LED trạng thái |
| **PIR** (LED2) | **4** | LED báo hiệu (không phải cảm biến PIR rời) |
| **BUZZER** (SP1) | **12** | Loa qua transistor Q1 |
| **SDA / SCL** | **21 / 22** | I2C: MPU6050 + LCD 2004 |
| **CS / SCK / MOSI / MISO** | **5 / 18 / 23 / 19** | SPI MicroSD (P2) |
| **GPS_RX / GPS_TX** | **34 / 35** | Serial2 — GPS TX→34, GPS RX←35 |
| **RST_SIM / RX / TX** | **26 / 27 / 32** | Serial1 — RST, nhận từ TX module, gửi tới RX module |

### LCD 2004 (module I2C PCF8574)

| Nhãn module | Kết nối |
|---|---|
| GND | GND |
| VCC | VCC |
| SDA | GPIO **21** |
| SCL | GPIO **22** |

---

## 2. Kiến trúc firmware

- **Chế độ 1** (`MODE_DATA_COLLECTION`): Ghi CSV MPU6050 → SD + Serial, 50 Hz.
- **Chế độ 2** (`MODE_INFERENCE`): Ngã xe → SMS; quá tốc độ / lạng lách → còi GPIO **12**; LED **4** hỗ trợ báo trực quan; Deep Sleep sau 10 phút đứng yên.

Chuyển chế độ trong `Config.h`: `CURRENT_MODE`.

---

## 3. Thư viện

1. `Wire.h` / `SPI.h`
2. `SD.h`
3. `TinyGPSPlus.h`
4. `MPU6050_light.h`
5. `LiquidCrystal_I2C.h` (tùy chọn hiển thị LCD)

---

## 4. Các bước triển khai

1. Nạp `Test_LCD2004` xác nhận I2C **21/22**.
2. Nạp `Driver_Safety_Blackbox`, kiểm tra Serial Monitor.
3. Thu thập dữ liệu Edge Impulse (nếu dùng TinyML).
4. Kiểm tra GPS/SIM trên PCB thật; xử lý GPIO35 nếu GPS TX không hoạt động.

---

*Cập nhật theo schematic Altium — đồng bộ với `Config.h`.*
