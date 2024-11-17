/***************************************************************************
  This is a library for the BME680 gas, humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BME680 Breakout
  ----> http://www.adafruit.com/products/3660

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

#include <Arduino.h>
#include <Wire.h>
#include "BME280.h"  // Lokale Bibliothek statt Adafruit

#define I2C_SDA PIN_PB1
#define I2C_SCL PIN_PB0

BME280 bme;  // Verwendung der einfacheren BME280 Klasse

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println(F("BME280 test"));

  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(100000);
  delay(100);

  // Debug: I2C Scanner
  Serial.println("Scanning I2C bus...");
  byte error, address;
  int nDevices = 0;
  for(address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      nDevices++;
    }
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found");
  }

  bme.begin();  // Einfachere Initialisierung
}

void loop() {
  float temp = bme.readTemperature();
  float pressure = bme.readPressure();
  float humidity = bme.readHumidity();

  Serial.print("Temperature = ");
  Serial.print(temp);
  Serial.println(" *C");

  Serial.print("Pressure = ");
  Serial.print(pressure);
  Serial.println(" hPa");

  Serial.print("Humidity = ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.println();
  delay(2000);
}
