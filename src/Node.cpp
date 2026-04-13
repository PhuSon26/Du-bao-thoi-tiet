#include <Arduino.h>
#include <LoRa.h>
#include <DHT.h>
#include <Adafruit_BMP280.h>
#include <BH1750.h>

Adafruit_BMP280 bmp;

void setup()
{
  Serial.begin(9600);
  if (!bmp.begin(0x76))
  {
    Serial.println("Khong tim thay cam bien BMP280!");
    while (1);
  }
}

void loop()
{
  Serial.println(bmp.readTemperature()); //đo nhiệt độ

  Serial.println(bmp.readPressure()); //đo áp suất
  
  delay(2000);
}