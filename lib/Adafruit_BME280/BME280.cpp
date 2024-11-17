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
    // Sensor aus dem Sleep-Mode holen
    writeReg(0xF4, 0x00);  // Sleep mode
    delay(50);  // Warten auf Reset
    
    // Lese Kalibrierungsdaten
    Wire.beginTransmission(BME280_ADDRESS);
    Wire.write(0x88);  // Start der Kalibrierungsdaten
    Wire.endTransmission();
    Wire.requestFrom(BME280_ADDRESS, 26);
    
    dig_T1 = (Wire.read() | (uint16_t)Wire.read() << 8);
    dig_T2 = (Wire.read() | (uint16_t)Wire.read() << 8);
    dig_T3 = (Wire.read() | (uint16_t)Wire.read() << 8);
    
    // Debug-Ausgaben für Temperatur-Kalibrierung
    Serial.println("Temperature calibration data:");
    Serial.print("dig_T1: "); Serial.println(dig_T1);
    Serial.print("dig_T2: "); Serial.println(dig_T2);
    Serial.print("dig_T3: "); Serial.println(dig_T3);
    
    dig_P1 = (Wire.read() | (uint16_t)Wire.read() << 8);
    dig_P2 = (Wire.read() | (uint16_t)Wire.read() << 8);
    dig_P3 = (Wire.read() | (uint16_t)Wire.read() << 8);
    dig_P4 = (Wire.read() | (uint16_t)Wire.read() << 8);
    dig_P5 = (Wire.read() | (uint16_t)Wire.read() << 8);
    dig_P6 = (Wire.read() | (uint16_t)Wire.read() << 8);
    dig_P7 = (Wire.read() | (uint16_t)Wire.read() << 8);
    dig_P8 = (Wire.read() | (uint16_t)Wire.read() << 8);
    dig_P9 = (Wire.read() | (uint16_t)Wire.read() << 8);
    
    Wire.beginTransmission(BME280_ADDRESS);
    Wire.write(0xA1);  // dig_H1
    Wire.endTransmission();
    Wire.requestFrom(BME280_ADDRESS, 1);
    dig_H1 = Wire.read();
    
    Wire.beginTransmission(BME280_ADDRESS);
    Wire.write(0xE1);  // dig_H2 bis dig_H6
    Wire.endTransmission();
    Wire.requestFrom(BME280_ADDRESS, 7);
    dig_H2 = (Wire.read() | (uint16_t)Wire.read() << 8);
    dig_H3 = Wire.read();
    dig_H4 = ((int8_t)Wire.read() << 4) | (Wire.read() & 0xF);
    dig_H5 = ((int8_t)Wire.read() << 4) | (Wire.read() >> 4);
    dig_H6 = (int8_t)Wire.read();

    // Debug-Ausgaben für Rohdaten
    readData();
    Serial.println("Raw sensor data:");
    for(int i = 0; i < 8; i++) {
        Serial.print("data["); Serial.print(i); Serial.print("]: ");
        Serial.println(data[i], HEX);
    }

    // Sensor-Konfiguration
    writeReg(0xF2, 0x01);  // humidity oversampling x1
    delay(50);  // Warten auf Konfiguration
    writeReg(0xF4, 0x27);  // temp/pressure oversampling x1, normal mode
    delay(50);  // Warten auf Moduswechsel
    
    // Debug: Konfiguration überprüfen
    Serial.println("Sensor configuration:");
    uint8_t config = readReg(0xF4);
    Serial.print("Control register (0xF4): 0x"); Serial.println(config, HEX);
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
    readData();
    
    int32_t adc_T = ((uint32_t)data[3] << 12) | ((uint32_t)data[4] << 4) | ((data[5] >> 4) & 0x0F);
    
    // Debug-Ausgabe für ADC-Wert
    Serial.print("adc_T: "); Serial.println(adc_T);
    
    int32_t var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    int32_t var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
    
    // Debug-Ausgaben für Zwischenberechnungen
    Serial.print("var1: "); Serial.println(var1);
    Serial.print("var2: "); Serial.println(var2);
    
    t_fine = var1 + var2;
    float T = (t_fine * 5 + 128) >> 8;
    return T / 100;
}

float BME280::readPressure() {
    readData();
    
    int32_t adc_P = ((uint32_t)data[0] << 12) | ((uint32_t)data[1] << 4) | ((data[2] >> 4) & 0x0F);
    
    int64_t var1 = ((int64_t)t_fine) - 128000;
    int64_t var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
    var2 = var2 + (((int64_t)dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;
    
    if (var1 == 0) {
        return 0;
    }
    
    int64_t p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;
    
    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);
    return (float)p / 256.0;
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