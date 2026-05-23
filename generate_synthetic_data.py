# -*- coding: utf-8 -*-
"""
generate_synthetic_data.py - Creating synthetic MPU6050 walking and falling data
"""

import os
import sys
import numpy as np
import pandas as pd

# Reconfigure stdout to UTF-8 if possible
if hasattr(sys.stdout, 'reconfigure'):
    sys.stdout.reconfigure(encoding='utf-8')

DATA_DIR = "dataset"
if not os.path.exists(DATA_DIR):
    os.makedirs(DATA_DIR)

# Physical parameters
FS = 50.0  # 50Hz Sampling Rate
DT = 1.0 / FS

def generate_normal_walking(duration_sec=15, file_id=1):
    N = int(duration_sec * FS)
    t = np.arange(N) * DT
    
    # Walking step frequency
    f_step = 1.8 + np.random.uniform(-0.1, 0.1)
    
    # Accelerometer (G)
    acc_z = 1.0 + 0.12 * np.sin(2 * np.pi * f_step * t) + np.random.normal(0, 0.03, N)
    acc_y = 0.04 * np.cos(2 * np.pi * f_step * t) + np.random.normal(0, 0.02, N)
    acc_x = 0.08 * np.sin(2 * np.pi * (f_step / 2.0) * t) + np.random.normal(0, 0.02, N)
    
    # Gyroscope (deg/s)
    gyro_x = 8.0 * np.cos(2 * np.pi * (f_step / 2.0) * t) + np.random.normal(0, 1.5, N)
    gyro_y = 4.0 * np.sin(2 * np.pi * f_step * t) + np.random.normal(0, 1.0, N)
    gyro_z = 6.0 * np.sin(2 * np.pi * (f_step / 2.0) * t) + np.random.normal(0, 1.2, N)
    
    # Timestamp
    timestamp = np.array([int(x * 1000) for x in t]) + 1000
    
    df = pd.DataFrame({
        'Timestamp_ms': timestamp,
        'AccX': acc_x,
        'AccY': acc_y,
        'AccZ': acc_z,
        'GyroX': gyro_x,
        'GyroY': gyro_y,
        'GyroZ': gyro_z
    })
    
    file_path = os.path.join(DATA_DIR, f"normal_{file_id}.csv")
    df.to_csv(file_path, index=False)
    print(f"[OK] Generated normal walking file: {file_path}")

def generate_swerving_and_fall(duration_sec=16, fall_time=9.0, file_id=1):
    N = int(duration_sec * FS)
    t = np.arange(N) * DT
    
    acc_x = np.zeros(N)
    acc_y = np.zeros(N)
    acc_z = np.zeros(N)
    gyro_x = np.zeros(N)
    gyro_y = np.zeros(N)
    gyro_z = np.zeros(N)
    
    for i in range(N):
        curr_t = t[i]
        if curr_t < fall_time:
            # PHASE 1: STUMBLING / UNSTABLE WALKING (0 -> fall_time)
            f_swerve = 1.4 + 0.3 * np.sin(0.5 * curr_t)
            
            acc_x[i] = 0.35 * np.sin(2 * np.pi * f_swerve * curr_t) + np.random.normal(0, 0.05)
            acc_y[i] = 0.12 * np.cos(2 * np.pi * f_swerve * 1.5 * curr_t) + np.random.normal(0, 0.04)
            acc_z[i] = 1.0 + 0.25 * np.sin(2 * np.pi * f_swerve * curr_t) + np.random.normal(0, 0.06)
            
            gyro_x[i] = 80.0 * np.cos(2 * np.pi * f_swerve * curr_t) + np.random.normal(0, 5.0)
            gyro_y[i] = 30.0 * np.sin(2 * np.pi * f_swerve * curr_t) + np.random.normal(0, 4.0)
            gyro_z[i] = 110.0 * np.sin(2 * np.pi * (f_swerve / 2.0) * curr_t) + np.random.normal(0, 6.0)
            
        elif curr_t >= fall_time and curr_t < (fall_time + 0.5):
            # PHASE 2: FALL IMPACT (fall_time -> fall_time + 0.5s)
            ratio = (curr_t - fall_time) / 0.5
            acc_x[i] = 1.8 * np.sin(np.pi * ratio) + np.random.normal(0, 0.1)
            acc_y[i] = -1.2 * np.sin(np.pi * ratio) + np.random.normal(0, 0.1)
            acc_z[i] = 2.4 * np.sin(np.pi * ratio) + np.random.normal(0, 0.1)
            
            gyro_x[i] = 220.0 * (1.0 - ratio) + np.random.normal(0, 10.0)
            gyro_y[i] = 90.0 * (1.0 - ratio) + np.random.normal(0, 5.0)
            gyro_z[i] = 150.0 * (1.0 - ratio) + np.random.normal(0, 8.0)
            
        else:
            # PHASE 3: LYING COMPLETELY STATIONARY (fall_time + 0.5s -> end)
            # Tilt angle is > 60 degrees (AccX ~ 0.88G)
            acc_x[i] = 0.88 + np.random.normal(0, 0.003) 
            acc_y[i] = 0.10 + np.random.normal(0, 0.003)
            acc_z[i] = 0.45 + np.random.normal(0, 0.003)
            
            gyro_x[i] = 0.0 + np.random.normal(0, 0.05)
            gyro_y[i] = 0.0 + np.random.normal(0, 0.05)
            gyro_z[i] = 0.0 + np.random.normal(0, 0.05)
            
    timestamp = np.array([int(x * 1000) for x in t]) + 1000
    
    df = pd.DataFrame({
        'Timestamp_ms': timestamp,
        'AccX': acc_x,
        'AccY': acc_y,
        'AccZ': acc_z,
        'GyroX': gyro_x,
        'GyroY': gyro_y,
        'GyroZ': gyro_z
    })
    
    file_path = os.path.join(DATA_DIR, f"swerving_{file_id}.csv")
    df.to_csv(file_path, index=False)
    print(f"[OK] Generated swerving/fall file: {file_path}")

# Generate 5 files for each class
for i in range(1, 6):
    generate_normal_walking(duration_sec=20, file_id=i)
    generate_swerving_and_fall(duration_sec=20, fall_time=10.0, file_id=i)

print("\n=== SYNTHETIC WALKING DATASET COMPLETED ===")
