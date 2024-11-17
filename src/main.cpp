#include <Arduino.h>
#include <Wire.h>
#include "BME280.h"

#define I2C_SDA PIN_PB1
#define I2C_SCL PIN_PB0

BME280 bme;

void setup() {
    Serial.begin(9600);
    delay(1000);
    Serial.println("Start");
    
    Serial.println("Init I2C...");
    Wire.begin();
    Wire.setClock(100000);
    delay(100);
    
    Serial.println("Reading BME280 ID...");
    Wire.beginTransmission(0x76);
    Wire.write(0xD0);  // ID Register
    Wire.endTransmission();
    
    if (Wire.requestFrom(0x76, 1)) {
        byte chipId = Wire.read();
        Serial.print("Chip ID: 0x");
        Serial.println(chipId, HEX);
        // Sollte 0x60 sein für BME280
    }
    
    Serial.println("Setup complete");
}

void loop() {
    delay(1000);
    Serial.println("Loop");
}
