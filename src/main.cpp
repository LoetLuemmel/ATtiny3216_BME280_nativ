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
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <GxEPD2_3C.h>
// Die I2C-Adresse des BME680 ist 0x77 (siehe BME680_I2C_ADDR)

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1028)

#define BME680_I2C_ADDR 0x76  // oder 0x76, je nachdem wie der SDO Pin beschaltet ist

#define ENABLE_GxEPD2_GFX 0
#define MAX_HEIGHT_EPD 16    
#define MAX_DISPLAY_BUFFER_SIZE 128

#define EPD_BUSY    PIN_PA2
#define EPD_RESET   PIN_PA3
#define EPD_DC      PIN_PA4
#define EPD_CS      PIN_PA5
#define EPD_SCK     PIN_PA6
#define EPD_MOSI    PIN_PA7

// Minimale Variante des Display-Treibers
using Display = GxEPD2_154_Z90c;
GxEPD2_3C<Display, Display::HEIGHT / 8> display(Display(EPD_CS, EPD_DC, EPD_RESET, EPD_BUSY));

Adafruit_BME680 bme(&Wire); // I2C
//Adafruit_BME680 bme(&Wire1); // example of I2C on another bus
//Adafruit_BME680 bme(BME_CS); // hardware SPI
//Adafruit_BME680 bme(BME_CS, BME_MOSI, BME_MISO,  BME_SCK);

// Add I2C pin definitions
#define I2C_SDA PIN_PB1
#define I2C_SCL PIN_PB0

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println(F("BME680 test"));

  // Initialize I2C first
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(100000); // Set to 100kHz to ensure stable communication
  delay(100); // Give I2C bus time to stabilize
  
  // Initialize BME680 before SPI devices
  if (!bme.begin(0x76)) {
    Serial.println("Could not find BME680 sensor at 0x76!");
    while (1);
  }
  Serial.println("BME680 sensor found!");

  // Set up BME680 parameters
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150);

  // Initialize SPI and display after I2C is stable
  SPI.begin();
  display.init(0, false);
  display.setRotation(0);
}

void loop() {
  if (! bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }
  Serial.print("Temperature = ");
  Serial.print(bme.temperature);
  Serial.println(" *C");

  Serial.print("Pressure = ");
  Serial.print(bme.pressure / 100.0);
  Serial.println(" hPa");

  Serial.print("Humidity = ");
  Serial.print(bme.humidity);
  Serial.println(" %");

  Serial.print("Gas = ");
  Serial.print(bme.gas_resistance / 1000.0);
  Serial.println(" KOhms");

  Serial.print("Approx. Altitude = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");

  Serial.println();
  delay(2000);

  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.drawRect(10, 10, 50, 50, GxEPD_BLACK);
    display.drawRect(70, 10, 50, 50, GxEPD_RED);
  }
  while (display.nextPage());
  
  delay(5000);
}
