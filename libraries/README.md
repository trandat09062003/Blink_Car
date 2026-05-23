# Thư viện trong dự án (`libraries/`)

Arduino IDE / arduino-cli tự nhận thư mục này khi mở sketch ở thư mục gốc dự án.

| Thư viện | Dùng cho | Sketch |
| :--- | :--- | :--- |
| **MPU6050_light** | Cảm biến gia tốc MPU6050 (I2C) | `Hardware_Test`, `Driver_Safety_Blackbox` |
| **TinyGPSPlus** | Parse NMEA GPS | `Hardware_Test`, `Driver_Safety_Blackbox` |
| **LiquidCrystal_I2C** | LCD 2004 qua PCF8574 (I2C) | `Hardware_Test`, `Test_LCD2004` |

Thư viện chuẩn board ESP32 (không cần copy): `Wire`, `SPI`, `SD`.

## Cài thêm (nếu IDE không thấy `libraries/`)

```bash
arduino-cli lib install "MPU6050_light" "TinyGPSPlus" "LiquidCrystal_I2C"
```

Hoặc: **Sketch → Include Library → Add .ZIP Library** (chỉ khi thiếu).

## Test phần cứng

Mở `Hardware_Test/Hardware_Test.ino` → chọn board **ESP32 Dev Module** → Upload → Serial Monitor **115200**.
