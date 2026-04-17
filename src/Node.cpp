#include <Arduino.h>
#include <LoRa.h>
#include <DHT.h>
#include <Adafruit_BMP280.h>
#include <BH1750.h>

Adafruit_BMP280 bmp;
const int RainSensorPin = A0;

void setup()
{
  Serial.begin(9600);
  pinMode(RainSensorPin, INPUT);
  if (!bmp.begin(0x76))
  {
    Serial.println("Khong tim thay cam bien BMP280!");
    while (1);
  }
}

void loop()
{
  int RainSensor_Value = analogRead(RainSensorPin);
  Serial.println(bmp.readTemperature()); //đo nhiệt độ
  
  Serial.println(bmp.readPressure()); //đo áp suất
  
  Serial.println(RainSensor_Value); //đo độ ướt (mưa)

  delay(2000);
}