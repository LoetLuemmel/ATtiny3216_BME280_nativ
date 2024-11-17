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
    Wire.begin();  // Standard I2C ohne swap
    Wire.setClock(100000);  // Standard 100kHz
    delay(100);
    
    // Nur die valide Adresse testen
    Serial.println("Testing BME280 at 0x76...");
    byte available = Wire.requestFrom(0x76, 1);
    Serial.print("Response: ");
    Serial.println(available);
    
    Serial.println("Setup complete");
}

void loop() {
    delay(1000);
    Serial.println("Loop");
}
