# -*- coding: utf-8 -*-
"""
train_model.py - Kịch bản huấn luyện mô hình TinyML AI (Phân loại hành vi lái xe)
Dự án: Hộp đen giám sát an toàn xe điện học sinh (GPS + AI)

Kịch bản này thực hiện:
  1. Đọc dữ liệu CSV từ thư mục 'dataset/' (các file dạng 'normal_*.csv' và 'swerving_*.csv')
  2. Phân nhỏ dữ liệu chuỗi thời gian thành các cửa sổ (windows) kích thước 100 mẫu (2 giây ở tần số 50Hz)
  3. Xây dựng và huấn luyện mô hình mạng nơ-ron tích chập 1 chiều (1D-CNN) bằng TensorFlow / Keras
  4. Chuyển đổi mô hình đã huấn luyện sang định dạng TensorFlow Lite (TFLite) tối ưu hóa kích thước
  5. Xuất mô hình thành mảng byte C++ để nhúng trực tiếp vào ESP32 dưới dạng thư viện

Yêu cầu cài đặt thư viện trước khi chạy:
  pip install tensorflow numpy pandas scikit-learn
"""

import os
import sys
import glob
import numpy as np
import pandas as pd
from sklearn.model_selection import train_test_split
import tensorflow as tf
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Dense, Flatten, Conv1D, MaxPooling1D, Dropout

# Đảm bảo mã hóa UTF-8 khi xuất log tiếng Việt trên Windows
if hasattr(sys.stdout, 'reconfigure'):
    sys.stdout.reconfigure(encoding='utf-8')

# ==================================================================================
# 1. CẤU HÌNH THAM SỐ
# ==================================================================================
WINDOW_SIZE = 100  # 100 mẫu = 2 giây (tại tần số lấy mẫu 50Hz của MPU6050)
STEP_SIZE = 20     # Bước dịch cửa sổ = 20 mẫu (chồng lấp 80% để tăng lượng dữ liệu)
NUM_FEATURES = 6   # 6 trục cảm biến: AccX, AccY, AccZ, GyroX, GyroY, GyroZ
DATA_DIR = "dataset" # Thư mục chứa các tệp CSV dữ liệu đã thu thập

# ==================================================================================
# 2. HÀM TẢI VÀ TIỀN XỬ LÝ DỮ LIỆU
# ==================================================================================
def load_and_preprocess_data():
    X = []
    y = []
    
    # Tìm tất cả các file CSV trong thư mục dataset
    normal_files = glob.glob(os.path.join(DATA_DIR, "normal_*.csv"))
    swerving_files = glob.glob(os.path.join(DATA_DIR, "swerving_*.csv"))
    
    print(f"Tìm thấy: {len(normal_files)} file dữ liệu Bình thường (Normal)")
    print(f"Tìm thấy: {len(swerving_files)} file dữ liệu Lạng lách (Swerving)")
    
    if len(normal_files) == 0 and len(swerving_files) == 0:
        print("[ERROR] Không tìm thấy file CSV nào trong thư mục dataset/!")
        print("Vui lòng tạo thư mục 'dataset/' và bỏ các file CSV đã thu thập vào đó.")
        return None, None, None, None
        
    # Xử lý các file dữ liệu Bình thường (Nhãn 0)
    for file_path in normal_files:
        df = pd.read_csv(file_path)
        # Bỏ cột Timestamp nếu có
        if 'Timestamp_ms' in df.columns:
            data = df[['AccX', 'AccY', 'AccZ', 'GyroX', 'GyroY', 'GyroZ']].values
        else:
            data = df.iloc[:, 1:7].values # Giả định bỏ cột đầu tiên
            
        # Áp dụng chuẩn hóa đồng nhất với ESP32: Acc * 0.5, Gyro * 0.01
        data = data.copy()
        data[:, 0:3] = data[:, 0:3] * 0.5
        data[:, 3:6] = data[:, 3:6] * 0.01

        # Chia nhỏ dữ liệu thành các cửa sổ trượt
        for i in range(0, len(data) - WINDOW_SIZE, STEP_SIZE):
            window = data[i:i + WINDOW_SIZE]
            X.append(window)
            y.append(0) # Nhãn 0: Bình thường
            
    # Xử lý các file dữ liệu Lạng lách (Nhãn 1)
    for file_path in swerving_files:
        df = pd.read_csv(file_path)
        if 'Timestamp_ms' in df.columns:
            data = df[['AccX', 'AccY', 'AccZ', 'GyroX', 'GyroY', 'GyroZ']].values
        else:
            data = df.iloc[:, 1:7].values
            
        # Áp dụng chuẩn hóa đồng nhất với ESP32: Acc * 0.5, Gyro * 0.01
        data = data.copy()
        data[:, 0:3] = data[:, 0:3] * 0.5
        data[:, 3:6] = data[:, 3:6] * 0.01

        for i in range(0, len(data) - WINDOW_SIZE, STEP_SIZE):
            window = data[i:i + WINDOW_SIZE]
            X.append(window)
            y.append(1) # Nhãn 1: Lạng lách

    X = np.array(X)
    y = np.array(y)
    
    print(f"Tổng số cửa sổ dữ liệu thu được: {X.shape[0]}")
    print(f"Kích thước X: {X.shape} (Số cửa sổ, Độ dài cửa sổ, Số trục cảm biến)")
    print(f"Kích thước y: {y.shape} (Số lượng nhãn)")
    
    # Chia tập dữ liệu thành Train (80%) và Test (20%)
    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42, stratify=y)
    
    return X_train, X_test, y_train, y_test

# ==================================================================================
# 3. XÂY DỰNG MÔ HÌNH MẠNG NƠ-RON TÍCH CHẬP 1 CHIỀU (1D-CNN)
# ==================================================================================
def create_model():
    model = Sequential([
        # Lớp tích chập 1 chiều để trích xuất đặc trưng không gian-thời gian
        Conv1D(16, kernel_size=3, activation='relu', input_shape=(WINDOW_SIZE, NUM_FEATURES)),
        MaxPooling1D(pool_size=2),
        Dropout(0.1),
        
        Conv1D(32, kernel_size=3, activation='relu'),
        MaxPooling1D(pool_size=2),
        Dropout(0.1),
        
        Flatten(),
        # Lớp ẩn Dense kết nối đầy đủ
        Dense(32, activation='relu'),
        Dropout(0.2),
        # Lớp đầu ra (Softmax phân loại 2 lớp: 0 - Bình thường, 1 - Lạng lách)
        Dense(2, activation='softmax')
    ])
    
    model.compile(optimizer='adam',
                  loss='sparse_categorical_crossentropy',
                  metrics=['accuracy'])
    return model

# ==================================================================================
# 4. CHUYỂN ĐỔI SANG ĐỊNH DẠNG TFLITE C++ HEADER
# ==================================================================================
def convert_to_tflite_cpp(model, filename="model_data.h"):
    # 1. Chuyển đổi mô hình Keras sang định dạng TFLite FlatBuffer
    converter = tf.lite.TFLiteConverter.from_keras_model(model)
    
    # Cấu hình tối ưu hóa kích thước mô hình (Quantization) cho vi điều khiển
    # converter.optimizations = [tf.lite.Optimize.DEFAULT]  // disabled for float model
    tflite_model = converter.convert()
    
    # Lưu file .tflite nhị phân để kiểm tra
    tflite_filename = filename.replace(".h", ".tflite")
    with open(tflite_filename, "wb") as f:
        f.write(tflite_model)
    print(f"\n[OK] Đã lưu mô hình TFLite tại: {tflite_filename} (Dung lượng: {len(tflite_model) / 1024:.2f} KB)")
    
    # 2. Tạo nội dung tệp tin C++ Header (.h)
    hex_lines = []
    # Biến đổi dữ liệu nhị phân thành mảng hex (định dạng C++)
    for i, byte in enumerate(tflite_model):
        if i % 12 == 0:
            hex_lines.append("\n  ")
        hex_lines.append(f"0x{byte:02x}, ")
        
    cpp_array = "".join(hex_lines)
    
    header_content = f"""/**
 * {filename} - Mô hình TinyML TFLite nhúng cho ESP32
 * Tự động tạo bởi kịch bản train_model.py
 */

#ifndef MODEL_DATA_H
#define MODEL_DATA_H

// Độ dài mảng mô hình (bytes)
const unsigned int g_model_len = {len(tflite_model)};

// Mảng chứa mô hình TFLite đã nén (Quantized)
const unsigned char g_model[] = {{{cpp_array}
}};

#endif // MODEL_DATA_H
"""
    # Ghi file header
    with open(filename, "w", encoding="utf-8") as f:
        f.write(header_content)
    print(f"[OK] Đã xuất mô hình C++ Header thành công tại: {filename}")
    
    # Tự động ghi vào thư viện Driver_Safety_TinyML nếu có trong workspace
    lib_dir = os.path.join("libraries", "Driver_Safety_TinyML", "src")
    if os.path.exists(lib_dir):
        lib_filename = os.path.join(lib_dir, filename)
        with open(lib_filename, "w", encoding="utf-8") as f:
            f.write(header_content)
        print(f"[OK] Đã tự động cập nhật mô hình vào thư viện tại: {lib_filename}")
    else:
        print("Gợi ý: Sao chép file này vào thư mục libraries/Driver_Safety_TinyML/src/ để chạy suy luận.")

# ==================================================================================
# MAIN EXECUTION
# ==================================================================================
if __name__ == "__main__":
    # Tạo thư mục dataset nếu chưa có để người dùng bỏ file vào
    if not os.path.exists(DATA_DIR):
        os.makedirs(DATA_DIR)
        print(f"Đã tạo thư mục '{DATA_DIR}/'. Vui lòng lưu các file CSV của bạn vào đây:")
        print(" - Các file lái xe bình thường đặt tên: normal_1.csv, normal_2.csv, ...")
        print(" - Các file lạng lách đặt tên: swerving_1.csv, swerving_2.csv, ...")
        print("Sau đó chạy lại file script này để bắt đầu train.")
    else:
        # Load dữ liệu
        X_train, X_test, y_train, y_test = load_and_preprocess_data()
        
        if X_train is not None:
            # Khởi tạo mô hình
            model = create_model()
            model.summary()
            
            # Huấn luyện mô hình
            print("\n--- BẮT ĐẦU HUẤN LUYỆN MẠNG NƠ-RON ---")
            history = model.fit(
                X_train, y_train,
                epochs=30,
                batch_size=32,
                validation_split=0.1,
                verbose=1
            )
            
            # Đánh giá mô hình trên tập test độc lập
            print("\n--- ĐÁNH GIÁ MÔ HÌNH TRÊN TẬP TEST THỰC TẾ ---")
            loss, accuracy = model.evaluate(X_test, y_test, verbose=0)
            print(f"Độ chính xác (Accuracy): {accuracy * 100:.2f}%")
            print(f"Giá trị Loss: {loss:.4f}")
            
            # Chuyển đổi và xuất file C++ header
            convert_to_tflite_cpp(model)
