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
    // Kalibrierungsdaten lesen
    Wire.beginTransmission(0x76);
    Wire.write(0x88);  // Start der Kalibrierungsdaten
    Wire.endTransmission();
    
    Wire.requestFrom(0x76, 6);  // Nur Temperatur-Kalibrierung zunächst
    if (Wire.available() >= 6) {
        dig_T1 = Wire.read() | (Wire.read() << 8);
        dig_T2 = Wire.read() | (Wire.read() << 8);
        dig_T3 = Wire.read() | (Wire.read() << 8);
        
        Serial.println("Calibration data:");
        Serial.print("T1: "); Serial.println(dig_T1);
        Serial.print("T2: "); Serial.println(dig_T2);
        Serial.print("T3: "); Serial.println(dig_T3);
    }
    
    // Sensor konfigurieren
    writeReg(0xF2, 0x01);    // humidity oversampling x1
    writeReg(0xF4, 0x27);    // temp/pressure oversampling x1, normal mode
    delay(100);
}

void BME280::writeReg(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(BME280_ADDRESS);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
}

void BME280::readData() {
    // Temperatur-Register direkt lesen (0xFA-0xFC)
    Wire.beginTransmission(BME280_ADDRESS);
    Wire.write(0xFA);  // Temperatur MSB
    Wire.endTransmission();
    Wire.requestFrom(BME280_ADDRESS, 3);
    data[3] = Wire.read();  // MSB
    data[4] = Wire.read();  // LSB
    data[5] = Wire.read();  // XLSB
    
    // Debug für Temperatur-Bytes
    Serial.println("Temperature raw bytes:");
    Serial.print("MSB (data[3]): 0x"); Serial.println(data[3], HEX);
    Serial.print("LSB (data[4]): 0x"); Serial.println(data[4], HEX);
    Serial.print("XLSB (data[5]): 0x"); Serial.println(data[5], HEX);
    
    // Druck-Register direkt lesen (0xF7-0xF9)
    Wire.beginTransmission(BME280_ADDRESS);
    Wire.write(0xF7);  // Druck MSB
    Wire.endTransmission();
    Wire.requestFrom(BME280_ADDRESS, 3);
    data[0] = Wire.read();  // MSB
    data[1] = Wire.read();  // LSB
    data[2] = Wire.read();  // XLSB
    
    // Feuchtigkeits-Register direkt lesen (0xFD-0xFE)
    Wire.beginTransmission(BME280_ADDRESS);
    Wire.write(0xFD);  // Feuchtigkeit MSB
    Wire.endTransmission();
    Wire.requestFrom(BME280_ADDRESS, 2);
    data[6] = Wire.read();  // MSB
    data[7] = Wire.read();  // LSB
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
    // Druckregister lesen (0xF7-0xF9)
    Wire.beginTransmission(0x76);
    Wire.write(0xF7);
    Wire.endTransmission();
    
    Wire.requestFrom(0x76, 3);
    if (Wire.available() >= 3) {
        uint32_t msb = Wire.read();
        uint32_t lsb = Wire.read();
        uint32_t xlsb = Wire.read();
        
        int32_t adc_P = (msb << 12) | (lsb << 4) | (xlsb >> 4);
        
        Serial.print("Raw pressure: 0x");
        Serial.println(adc_P, HEX);
        
        // Vorläufig zur Überprüfung der Rohdaten
        return adc_P / 100.0;
    }
    return 0;
}

float BME280::readHumidity() {
    readData();
    
    int32_t adc_H = ((uint32_t)data[6] << 8) | data[7];
    
    int32_t v_x1_u32r = (t_fine - ((int32_t)76800));
    v_x1_u32r = (((((adc_H << 14) - (((int32_t)dig_H4) << 20) - (((int32_t)dig_H5) * v_x1_u32r)) +
                   ((int32_t)16384)) >> 15) * (((((((v_x1_u32r * ((int32_t)dig_H6)) >> 10) * (((v_x1_u32r *
                   ((int32_t)dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) *
                   ((int32_t)dig_H2) + 8192) >> 14));
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)dig_H1)) >> 4));
    v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
    v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;
    
    return (float)(v_x1_u32r >> 12) / 1024.0;
}

// Neue Hilfsfunktion zum Lesen eines Registers
uint8_t BME280::readReg(uint8_t reg) {
    Wire.beginTransmission(BME280_ADDRESS);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom(BME280_ADDRESS, 1);
    return Wire.read();
} 