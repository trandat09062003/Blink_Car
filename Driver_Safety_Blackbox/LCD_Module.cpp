// LCD_Module.cpp – Control LCD 2004 I2C display for telemetry data

#include "LCD_Module.h"

LCDModule::LCDModule() : lcd(nullptr), lcdAddr(0), lcdFound(false) {
}

LCDModule::~LCDModule() {
    if (lcd != nullptr) {
        delete lcd;
    }
}

byte LCDModule::scanLCDAddress() {
    byte targetAddresses[] = {
        0x27, 0x3F, // Two most common default addresses
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 
        0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E
    };
    int numAddresses = sizeof(targetAddresses) / sizeof(targetAddresses[0]);

    for (int i = 0; i < numAddresses; i++) {
        byte addr = targetAddresses[i];
        Wire.beginTransmission(addr);
        byte error = Wire.endTransmission();

        if (error == 0) {
            return addr;
        }
    }
    return 0; // Not found
}

void LCDModule::begin() {
    Serial.println(F("[LCD] Dang quet tim man hinh LCD qua bus I2C..."));
    lcdAddr = scanLCDAddress();
    
    if (lcdAddr != 0) {
        lcdFound = true;
        Serial.print(F("[LCD] Tim thay LCD o dia chi: 0x"));
        Serial.println(lcdAddr, HEX);
        
        lcd = new LiquidCrystal_I2C(lcdAddr, 20, 4);
        lcd->init();
        lcd->backlight();
        lcd->clear();
        
        lcd->setCursor(0, 0);
        lcd->print("== HOP DEN AN TOAN ==");
        lcd->setCursor(0, 1);
        lcd->print("He thong: KHOI DONG");
        lcd->setCursor(0, 2);
        lcd->print("Blynk: DANG KET NOI");
        lcd->setCursor(0, 3);
        lcd->print("Giao tiep I2C... OK");
        delay(1500);
        lcd->clear();
    } else {
        Serial.println(F("[LCD] CANH BAO: Khong tim thay man hinh LCD 2004!"));
    }
}

void LCDModule::update(float speed, float aiConfidence, float threshold, bool gpsConnected, bool sdConnected) {
    if (!lcdFound || lcd == nullptr) return;

    // Line 1: Header
    lcd->setCursor(0, 0);
    lcd->print("== HOP DEN GIAM SAT =");

    // Line 2: Speed
    char speedBuf[21];
    snprintf(speedBuf, sizeof(speedBuf), "Toc do: %4.1f km/h  ", speed);
    lcd->setCursor(0, 1);
    lcd->print(speedBuf);

    // Line 3: AI Confidence and threshold
    char aiBuf[21];
    snprintf(aiBuf, sizeof(aiBuf), "AI: %3d%% (Nguong:%2d%%)", (int)(aiConfidence * 100), (int)(threshold * 100));
    lcd->setCursor(0, 2);
    lcd->print(aiBuf);

    // Line 4: System connections status
    char statBuf[21];
    snprintf(statBuf, sizeof(statBuf), "GPS: %-3s | SD: %-3s    ", gpsConnected ? "OK" : "ERR", sdConnected ? "OK" : "ERR");
    lcd->setCursor(0, 3);
    lcd->print(statBuf);
}

void LCDModule::displayCrash() {
    if (!lcdFound || lcd == nullptr) return;
    
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->print("🚨🚨 CANH BAO TAI NAN");
    lcd->setCursor(0, 1);
    lcd->print("  PHAT HIEN XE DO NGA ");
    lcd->setCursor(0, 2);
    lcd->print(" He thong dang phat  ");
    lcd->setCursor(0, 3);
    lcd->print(" coi bao dong cuu ho ");
}
