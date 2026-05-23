/**
 * ==================================================================================
 * Driver_Safety_TinyML.cpp - ĐỊNH NGHĨA CÁC PHƯƠNG THỨC SUY LUẬN AI TINYML
 * ==================================================================================
 * Dự án: Hộp đen giám sát an toàn xe điện học sinh (GPS + AI)
 * Tác giả: Antigravity AI (Google DeepMind Team)
 * ==================================================================================
 */

#include "Driver_Safety_TinyML.h"
#include "model_data.h"
#include <tflm_esp32.h>
#include <eloquent_tinyml.h>

// Định nghĩa con trỏ tĩnh đến đối tượng EloquentTinyML để tránh khởi tạo tĩnh gây crash
static Eloquent::TF::Sequential<30, 64 * 1024>* ml = nullptr;

DriverSafetyTinyML::DriverSafetyTinyML() {
    currentSampleCount = 0;
    modelInitialized = false;
    memset(inputBuffer, 0, sizeof(inputBuffer));
}

bool DriverSafetyTinyML::begin() {
    // Kiểm tra xem g_model có phải là placeholder trống không
    if (g_model_len <= 10) {
        Serial.println(F("[DriverSafetyTinyML] CANH BAO: Dang su dung mo hinh TFLite trong!"));
        Serial.println(F("[DriverSafetyTinyML] Vui long chay train_model.py de xuat mo hinh thuc te."));
        modelInitialized = false;
        return false;
    }

    if (ml == nullptr) {
        Serial.println(F("[DriverSafetyTinyML] Dang khoi tao interpreter TensorFlow Lite Micro..."));
        ml = new Eloquent::TF::Sequential<30, 64 * 1024>();
        ml->setNumInputs(600);
        ml->setNumOutputs(2);
        
        // Đăng ký thủ công các toán tử cần thiết cho mô hình 1D-CNN (được TFLite trích xuất dạng 2D Conv)
        ml->resolver.AddExpandDims();
        ml->resolver.AddSqueeze();
        ml->resolver.AddConv2D();
        ml->resolver.AddMaxPool2D();
        ml->resolver.AddReshape();
        ml->resolver.AddFullyConnected();
        ml->resolver.AddSoftmax();
        ml->resolver.AddRelu();
        ml->resolver.AddAdd();
        ml->resolver.AddMul();
        ml->resolver.AddShape();
        ml->resolver.AddStridedSlice();
        ml->resolver.AddPack();
        ml->resolver.AddUnpack();

        auto& exception = ml->begin((unsigned char*) g_model);
        if (!exception.isOk()) {
            Serial.print(F("[DriverSafetyTinyML] LOI KHOI TAO TINYML: "));
            Serial.println(exception.toString());
            modelInitialized = false;
            return false;
        }
    }
    
    modelInitialized = true;
    Serial.println(F("[DriverSafetyTinyML] Khoi tao mo hinh AI TinyML thanh cong!"));
    return true;
}

void DriverSafetyTinyML::addSample(float ax, float ay, float az, float gx, float gy, float gz) {
    if (currentSampleCount < 100) {
        // Neu chua du 100 mau, ghi tiep vao vi tri tiep theo
        int idx = currentSampleCount * 6;
        inputBuffer[idx]     = ax * 0.5f; // Normalize accelerometer (approx +/-2g) to ~[-1,1]
        inputBuffer[idx + 1] = ay * 0.5f;
        inputBuffer[idx + 2] = az * 0.5f;
        inputBuffer[idx + 3] = gx * 0.01f; // Normalize gyro (approx +/-250 dps) to ~[-2.5,2.5]
        inputBuffer[idx + 4] = gy * 0.01f;
        inputBuffer[idx + 5] = gz * 0.01f;
        currentSampleCount++;
    } else {
        // Dich trai 6 phan tu (tuong duong day 1 mau cu ra ngoai)
        memmove(inputBuffer, inputBuffer + 6, 594 * sizeof(float));
        
        // Them mau moi nhat vao cuoi bo dem (scaled)
        inputBuffer[594] = ax * 0.5f;
        inputBuffer[595] = ay * 0.5f;
        inputBuffer[596] = az * 0.5f;
        inputBuffer[597] = gx * 0.01f;
        inputBuffer[598] = gy * 0.01f;
        inputBuffer[599] = gz * 0.01f;
    }
}

bool DriverSafetyTinyML::isReady() const {
    return currentSampleCount >= 100;
}

void DriverSafetyTinyML::reset() {
    currentSampleCount = 0;
    memset(inputBuffer, 0, sizeof(inputBuffer));
}

float DriverSafetyTinyML::predictSwerving() {
    if (!modelInitialized || !isReady() || ml == nullptr) {
        return 0.0;
    }

        ml->predict(inputBuffer);
    // Retrieve the probability of the "Swerving" class (index 1)
    float probSwerving = ml->output(1);
    return probSwerving;
}
