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
    // Read calibration data
    Wire.beginTransmission(BME280_ADDRESS);
    Wire.write(BME280_REGISTER_DIG_T1);
    Wire.endTransmission();
    Wire.requestFrom(BME280_ADDRESS, 24);
    dig_T1 = Wire.read() | (Wire.read() << 8);
    dig_T2 = Wire.read() | (Wire.read() << 8);
    dig_T3 = Wire.read() | (Wire.read() << 8);
    dig_P1 = Wire.read() | (Wire.read() << 8);
    dig_P2 = Wire.read() | (Wire.read() << 8);
    dig_P3 = Wire.read() | (Wire.read() << 8);
    dig_P4 = Wire.read() | (Wire.read() << 8);
    dig_P5 = Wire.read() | (Wire.read() << 8);
    dig_P6 = Wire.read() | (Wire.read() << 8);
    dig_P7 = Wire.read() | (Wire.read() << 8);
    dig_P8 = Wire.read() | (Wire.read() << 8);
    dig_P9 = Wire.read() | (Wire.read() << 8);
    dig_H1 = Wire.read();
    
    Wire.beginTransmission(BME280_ADDRESS);
    Wire.write(BME280_REGISTER_DIG_H2);
    Wire.endTransmission();
    Wire.requestFrom(BME280_ADDRESS, 7);
    dig_H2 = Wire.read() | (Wire.read() << 8);
    dig_H3 = Wire.read();
    dig_H4 = (Wire.read() << 4) | (Wire.read() & 0xF);
    dig_H5 = ((Wire.read() & 0xF0) >> 4) | (Wire.read() << 4);
    dig_H6 = (int8_t)Wire.read();

    // Set sampling
    writeReg(BME280_REGISTER_CONTROLHUMID, 0x01);    // humidity oversampling x1
    writeReg(BME280_REGISTER_CONTROL, 0x27);         // temperature and pressure oversampling x1, mode normal
}

void BME280::writeReg(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(BME280_ADDRESS);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
}

void BME280::readData() {
    Wire.beginTransmission(BME280_ADDRESS);
    Wire.write(0xF7);
    Wire.endTransmission();
    Wire.requestFrom(BME280_ADDRESS, 8);
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
    readData();
    
    int32_t adc_T = ((uint32_t)data[3] << 12) | ((uint32_t)data[4] << 4) | ((data[5] >> 4) & 0x0F);
    
    int32_t var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    int32_t var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
    
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