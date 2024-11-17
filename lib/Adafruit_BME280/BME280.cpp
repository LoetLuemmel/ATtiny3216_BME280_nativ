#include "BME280.h"

#define BME280_ADDRESS 0x76

#define BME280_REGISTER_DIG_T1 0x88
#define BME280_REGISTER_DIG_T2 0x8A
#define BME280_REGISTER_DIG_T3 0x8C
#define BME280_REGISTER_DIG_P1 0x8E
#define BME280_REGISTER_DIG_P2 0x90
#define BME280_REGISTER_DIG_P3 0x92
#define BME280_REGISTER_DIG_P4 0x94
#define BME280_REGISTER_DIG_P5 0x96
#define BME280_REGISTER_DIG_P6 0x98
#define BME280_REGISTER_DIG_P7 0x9A
#define BME280_REGISTER_DIG_P8 0x9C
#define BME280_REGISTER_DIG_P9 0x9E
#define BME280_REGISTER_DIG_H1 0xA1
#define BME280_REGISTER_DIG_H2 0xE1
#define BME280_REGISTER_DIG_H3 0xE3
#define BME280_REGISTER_DIG_H4 0xE4
#define BME280_REGISTER_DIG_H5 0xE5
#define BME280_REGISTER_DIG_H6 0xE7
#define BME280_REGISTER_CHIPID 0xD0
#define BME280_REGISTER_CONTROLHUMID 0xF2
#define BME280_REGISTER_CONTROL 0xF4
#define BME280_REGISTER_DATA 0xF7

BME280::BME280() {
}

void BME280::begin() {
    // Reset
    Wire.beginTransmission(0x76);
    Wire.write(0xE0);
    Wire.write(0xB6);
    Wire.endTransmission();
    delay(10);
    
    // Check ID
    Wire.beginTransmission(0x76);
    Wire.write(0xD0);
    Wire.endTransmission();
    Wire.requestFrom(0x76, 1);
    uint8_t chipID = Wire.read();
    
    if (chipID != 0x60) {
        Serial.println("BME280 not found!");
        return;
    }
    Serial.println("BME280 found! (ID: 0x60)");
    
    // Read calibration data
    readCalibrationData();
    
    // Configure sensor
    Wire.beginTransmission(0x76);
    Wire.write(0xF2);  // ctrl_hum
    Wire.write(0x05);  // 16x oversampling
    Wire.endTransmission();
    
    Wire.beginTransmission(0x76);
    Wire.write(0xF4);  // ctrl_meas
    Wire.write(0xB7);  // 16x oversampling, normal mode
    Wire.endTransmission();
    
    Serial.println("BME280 initialization complete");
}

void BME280::readCalibrationData() {
    // Temperature & Pressure (0x88-0xA1)
    Wire.beginTransmission(0x76);
    Wire.write(0x88);
    Wire.endTransmission();
    Wire.requestFrom(0x76, 26);
    
    dig_T1 = (Wire.read() | (uint16_t)Wire.read() << 8);
    dig_T2 = (Wire.read() | (uint16_t)Wire.read() << 8);
    dig_T3 = (Wire.read() | (uint16_t)Wire.read() << 8);
    
    dig_P1 = (Wire.read() | (uint16_t)Wire.read() << 8);
    dig_P2 = (Wire.read() | (uint16_t)Wire.read() << 8);
    dig_P3 = (Wire.read() | (uint16_t)Wire.read() << 8);
    dig_P4 = (Wire.read() | (uint16_t)Wire.read() << 8);
    dig_P5 = (Wire.read() | (uint16_t)Wire.read() << 8);
    dig_P6 = (Wire.read() | (uint16_t)Wire.read() << 8);
    dig_P7 = (Wire.read() | (uint16_t)Wire.read() << 8);
    dig_P8 = (Wire.read() | (uint16_t)Wire.read() << 8);
    dig_P9 = (Wire.read() | (uint16_t)Wire.read() << 8);
    
    // Read humidity calibration (0xE1-0xE7)
    Wire.beginTransmission(0x76);
    Wire.write(0xE1);
    Wire.endTransmission();
    Wire.requestFrom(0x76, 7);
    
    uint8_t h2_lsb = Wire.read();  // 0xE1
    uint8_t h2_msb = Wire.read();  // 0xE2
    dig_H2 = (int16_t)((h2_msb << 8) | h2_lsb);
    
    dig_H3 = Wire.read();          // 0xE3
    
    uint8_t h4_msb = Wire.read();  // 0xE4
    uint8_t h4_lsb = Wire.read();  // 0xE5[3:0]
    dig_H4 = (int16_t)((h4_msb << 4) | (h4_lsb & 0x0F));
    
    uint8_t h5_msb = Wire.read();  // 0xE6
    dig_H5 = (int16_t)((h5_msb << 4) | (h4_lsb >> 4));  // h4_lsb[7:4]
    
    dig_H6 = (int8_t)Wire.read();  // 0xE7
    
    // Read H1 from 0xA1
    Wire.beginTransmission(0x76);
    Wire.write(0xA1);
    Wire.endTransmission();
    Wire.requestFrom(0x76, 1);
    dig_H1 = Wire.read();
}

void BME280::writeReg(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(BME280_ADDRESS);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
}

void BME280::readData() {
    Wire.beginTransmission(0x76);
    Wire.write(0xF7);
    Wire.endTransmission();
    Wire.requestFrom(0x76, 8);
    
    data[0] = Wire.read();
    data[1] = Wire.read();
    data[2] = Wire.read();
    data[3] = Wire.read();
    data[4] = Wire.read();
    data[5] = Wire.read();
    data[6] = Wire.read();
    data[7] = Wire.read();
}

float BME280::readTemperature() {
    Wire.beginTransmission(0x76);
    Wire.write(0xFA);
    Wire.endTransmission();
    
    Wire.requestFrom(0x76, 3);
    if (Wire.available() >= 3) {
        uint32_t msb = Wire.read();
        uint32_t lsb = Wire.read();
        uint32_t xlsb = Wire.read();
        
        int32_t adc_T = (msb << 12) | (lsb << 4) | (xlsb >> 4);
        
        // Temperaturberechnung mit Kalibrierungsdaten
        int32_t var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
        int32_t var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
        
        t_fine = var1 + var2;
        float T = (t_fine * 5 + 128) >> 8;
        
        return T / 100.0;
    }
    return 0;
}

float BME280::readPressure() {
    Wire.beginTransmission(0x76);
    Wire.write(0xF7);
    Wire.endTransmission();
    
    Wire.requestFrom(0x76, 3);
    if (Wire.available() >= 3) {
        uint32_t msb = Wire.read();
        uint32_t lsb = Wire.read();
        uint32_t xlsb = Wire.read();
        
        int32_t adc_P = (msb << 12) | (lsb << 4) | (xlsb >> 4);
        
        // Druckberechnung mit Kalibrierung
        int64_t var1, var2, p;
        var1 = ((int64_t)t_fine) - 128000;
        var2 = var1 * var1 * (int64_t)dig_P6;
        var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
        var2 = var2 + (((int64_t)dig_P4) << 35);
        var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
        var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;
        
        if (var1 == 0) {
            return 0; // Vermeidung von Division durch 0
        }
        
        p = 1048576 - adc_P;
        p = (((p << 31) - var2) * 3125) / var1;
        var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
        var2 = (((int64_t)dig_P8) * p) >> 19;
        p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);
        
        return (float)p / 256.0 / 100.0; // Umrechnung in hPa
    }
    return 0;
}

float BME280::readHumidity() {
    readData();
    
    // Get raw humidity (check invalid value)
    uint32_t adc_h = ((uint32_t)data[6] << 8) | data[7];
    if (adc_h > 0x8000) {
        return 0;
    }
    
    // Temperature compensation term
    int32_t v_x1_u32r = t_fine - ((int32_t)76800);
    
    // Humidity compensation formula from Bosch API
    int32_t v_x1 = (((((adc_h << 14) - (((int32_t)dig_H4) << 20) - 
                      (((int32_t)dig_H5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) * 
                   (((((((v_x1_u32r * ((int32_t)dig_H6)) >> 10) * 
                      (((v_x1_u32r * ((int32_t)dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + 
                      ((int32_t)2097152)) * ((int32_t)dig_H2) + 8192) >> 14));
    
    v_x1 = (v_x1 - (((((v_x1 >> 15) * (v_x1 >> 15)) >> 7) * ((int32_t)dig_H1)) >> 4));
    
    if (v_x1 < 0) {
        v_x1 = 0;
    }
    if (v_x1 > 419430400) {
        v_x1 = 419430400;
    }
    
    return ((float)v_x1) / 1024.0 / 1024.0 * 100.0 / 1000.0;  // Zusätzliche Division durch 1000
}

// Neue Hilfsfunktion zum Lesen eines Registers
uint8_t BME280::readReg(uint8_t reg) {
    Wire.beginTransmission(BME280_ADDRESS);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom(BME280_ADDRESS, 1);
    return Wire.read();
}
  