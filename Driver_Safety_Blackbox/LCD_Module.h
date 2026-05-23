// LCD_Module.h – Control LCD 2004 I2C display for telemetry data

#ifndef LCD_MODULE_H
#define LCD_MODULE_H

#include "Config.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

class LCDModule {
private:
    LiquidCrystal_I2C *lcd;
    byte lcdAddr;
    bool lcdFound;
    
    // Scan I2C bus to find LCD address (typically 0x27 or 0x3F)
    byte scanLCDAddress();

public:
    LCDModule();
    ~LCDModule();

    // Initialize LCD screen
    void begin();

    // Update the 20x4 display with live telemetry data
    void update(float speed, float aiConfidence, float threshold, bool gpsConnected, bool sdConnected);

    // Display crash accident screen
    void displayCrash();
};

#endif // LCD_MODULE_H
