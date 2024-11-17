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

  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(100000);
  delay(100);

  bme.begin();
}

void loop() {
  Serial.print("Temperature = ");
  Serial.print(bme.readTemperature());
  Serial.println(" *C");

  Serial.print("Pressure = ");
  Serial.print(bme.readPressure() / 100.0); // Convert to hPa
  Serial.println(" hPa");

  Serial.print("Humidity = ");
  Serial.print(bme.readHumidity());
  Serial.println(" %");

  Serial.println();
  delay(2000);
}
