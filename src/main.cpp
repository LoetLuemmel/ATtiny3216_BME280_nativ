#include <Arduino.h>
#include <Wire.h>
#include "BME280.h"

#define I2C_SDA PIN_PB1
#define I2C_SCL PIN_PB0

BME280 bme;

void setup() {
    Serial.begin(9600);
    while (!Serial);
    Serial.println(F("BME280 test"));

    // I2C Initialisierung mit Debug
    Wire.swap(1);
    Wire.begin();
    Wire.setClock(100000);
    delay(100);

    // Test der I2C Kommunikation
    Serial.println("Testing I2C communication...");
    
    Wire.beginTransmission(0x76);
    byte error = Wire.endTransmission();
    
    Serial.print("Basic I2C test error code: ");
    Serial.println(error);
    
    if (error == 0) {
        Serial.println("I2C device responded!");
    } else {
        Serial.println("No I2C device response");
    }
    
    delay(100);
    bme.begin();
}

void loop() {
    Serial.print("Temperature = ");
    Serial.print(bme.readTemperature());
    Serial.println(" *C");

    Serial.print("Pressure = ");
    Serial.print(bme.readPressure());
    Serial.println(" hPa");

    Serial.print("Humidity = ");
    Serial.print(bme.readHumidity());
    Serial.println(" %");

    Serial.println();
    delay(2000);
}
