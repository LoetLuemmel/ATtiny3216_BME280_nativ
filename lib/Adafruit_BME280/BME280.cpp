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
    
    // Druck-Kalibrierungsdaten lesen
    Wire.beginTransmission(0x76);
    Wire.write(0x8E);  // Start der Druck-Kalibrierungsdaten
    Wire.endTransmission();
    
    Wire.requestFrom(0x76, 18);  // 9 Werte × 2 Bytes
    if (Wire.available() >= 18) {
        dig_P1 = Wire.read() | (Wire.read() << 8);
        dig_P2 = Wire.read() | (Wire.read() << 8);
        dig_P3 = Wire.read() | (Wire.read() << 8);
        dig_P4 = Wire.read() | (Wire.read() << 8);
        dig_P5 = Wire.read() | (Wire.read() << 8);
        dig_P6 = Wire.read() | (Wire.read() << 8);
        dig_P7 = Wire.read() | (Wire.read() << 8);
        dig_P8 = Wire.read() | (Wire.read() << 8);
        dig_P9 = Wire.read() | (Wire.read() << 8);
        
        Serial.println("Pressure calibration data:");
        Serial.print("P1: "); Serial.println(dig_P1);
        Serial.print("P2: "); Serial.println(dig_P2);
    }
    
    // Humidity calibration mit Rohdaten-Debug
    Wire.beginTransmission(0x76);
    Wire.write(0xA1);
    Wire.endTransmission();
    Wire.requestFrom(0x76, 1);
    uint8_t h1 = Wire.read();
    
    Wire.beginTransmission(0x76);
    Wire.write(0xE1);
    Wire.endTransmission();
    Wire.requestFrom(0x76, 7);
    
    if (Wire.available() >= 7) {
        uint8_t raw[7];
        for(int i=0; i<7; i++) {
            raw[i] = Wire.read();
        }
        
        Serial.println("\nRaw calibration bytes:");
        for(int i=0; i<7; i++) {
            Serial.print("0x");
            if(raw[i] < 0x10) Serial.print("0");
            Serial.print(raw[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
        
        // Zusammensetzen der Werte
        dig_H1 = h1;
        dig_H2 = (int16_t)(raw[1] << 8 | raw[0]);
        dig_H3 = raw[2];
        dig_H4 = (int16_t)((raw[3] << 4) | (raw[4] & 0x0F));
        dig_H5 = (int16_t)((raw[4] >> 4) | (raw[5] << 4));
        dig_H6 = (int8_t)raw[6];

        Serial.println("\nCalibration data:");
        Serial.print("H1: "); Serial.println(dig_H1);
        Serial.print("H2: "); Serial.println(dig_H2);
        Serial.print("H3: "); Serial.println(dig_H3);
        Serial.print("H4: "); Serial.println(dig_H4);
        Serial.print("H5: "); Serial.println(dig_H5);
        Serial.print("H6: "); Serial.println(dig_H6);
    }
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
    readTemperature();
    
    Wire.beginTransmission(0x76);
    Wire.write(0xFD);
    Wire.endTransmission();
    
    Wire.requestFrom(0x76, 2);
    if (Wire.available() >= 2) {
        uint32_t adc_H = Wire.read();
        adc_H <<= 8;
        adc_H |= Wire.read();
        
        Serial.println("\nHumidity calculation steps:");
        Serial.print("ADC_H: "); Serial.println(adc_H);
        Serial.print("dig_H1-H6: "); 
        Serial.print(dig_H1); Serial.print(", ");
        Serial.print(dig_H2); Serial.print(", ");
        Serial.print(dig_H3); Serial.print(", ");
        Serial.print(dig_H4); Serial.print(", ");
        Serial.print(dig_H5); Serial.print(", ");
        Serial.println(dig_H6);
        
        int32_t v_x1_u32r = t_fine - ((int32_t)76800);
        Serial.print("v_x1_u32r: "); Serial.println(v_x1_u32r);
        
        // Erste Phase
        int32_t v_x1_u32 = adc_H << 14;
        v_x1_u32 -= ((int32_t)dig_H4) << 20;
        v_x1_u32 -= ((int32_t)dig_H5) * v_x1_u32r;
        v_x1_u32 += 16384;
        v_x1_u32 >>= 15;
        
        Serial.print("After phase 1: "); Serial.println(v_x1_u32);
        
        // Zweite Phase
        int32_t v_x2_u32 = v_x1_u32r * ((int32_t)dig_H6);
        v_x2_u32 >>= 10;
        int32_t v_x3_u32 = v_x1_u32r * ((int32_t)dig_H3);
        v_x3_u32 >>= 11;
        v_x3_u32 += 32768;
        v_x2_u32 *= v_x3_u32;
        v_x2_u32 >>= 10;
        v_x2_u32 += 2097152;
        v_x2_u32 *= ((int32_t)dig_H2);
        v_x2_u32 += 8192;
        v_x2_u32 >>= 14;
        
        Serial.print("After phase 2: "); Serial.println(v_x2_u32);
        
        // Finale Berechnung
        v_x1_u32 *= v_x2_u32;
        v_x1_u32 >>= 15;
        
        Serial.print("Before H1 comp: "); Serial.println(v_x1_u32);
        
        int32_t h1_comp = (v_x1_u32 >> 7) * (v_x1_u32 >> 7);
        h1_comp >>= 8;
        h1_comp *= ((int32_t)dig_H1);
        h1_comp >>= 4;
        
        v_x1_u32 -= h1_comp;
        v_x1_u32 = (v_x1_u32 < 0) ? 0 : v_x1_u32;
        v_x1_u32 = (v_x1_u32 > 419430400) ? 419430400 : v_x1_u32;
        
        float h = (float)(v_x1_u32 >> 12) / 1024.0f;
        
        Serial.print("Final humidity: "); Serial.println(h);
        return h;
    }
    return 0;
}

// Neue Hilfsfunktion zum Lesen eines Registers
uint8_t BME280::readReg(uint8_t reg) {
    Wire.beginTransmission(BME280_ADDRESS);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom(BME280_ADDRESS, 1);
    return Wire.read();
} 