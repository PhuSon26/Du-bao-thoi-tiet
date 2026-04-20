#include <Arduino.h>
#include <LoRa.h>
#include <DHT.h>
#include <Adafruit_BMP280.h>
#include <BH1750.h>
#include <Wire.h> //thư viện i2c vì bh1750 truyền theo giao thức này

<<<<<<< HEAD
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
=======
#define DHTTYPE DHT22 // khai báo loại cảm biến là DHT22
#define DHTPIN 23 // Chân Data của DHT22 nối với GPIO 4
DHT dht(DHTPIN, DHTTYPE); //khởi tạo object dht gòm có DHTPIN VÀ DHTTYPE
BH1750 lightMeter; //khởi tạo cảm biến ánh sáng lightMeter
//----cấu hình LoRa UART (serial2)
#define RXD2 16
#define TXD2 17
#define LoRa_baudrate 9600
void setup() {
  Serial.begin(115200);
  //Serial2.begin(baudrate, config, RX_pin, TX_pin);
  //baudrate: tốc độ truyền dữ liệu giữa esp32 và lora
  // config: cấu hình khu dữ liệu. 8: 8 bit dữ liệu, N: không kiểm tra lỗi, 1: bit stop
  //RX_pin: esp32 nhận dữ liệu từ lora
  //TX-pin: esp32 gửi dữ liệu sang lora
  Serial2.begin(LoRa_baudrate,  SERIAL_8N1, RXD2, TXD2);
  Serial.println("\n--- Khoi dong Tram Phat---");
  Wire.begin(); //Khoi dong I2C
  dht.begin(); //DHT22
  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE); // Khoi dong DH1750 o che do do lien tuc (continuous) va do chinh xac cao (high_res)

}
//Temp,pressure,humid,lux,rain
//Node gửi qua gateway theo định dạng string này nha
//Ví dụ: 36,36.36,36.36,36,1
//Rain thì 0 nếu k mưa, 1 nếu mưa
void loop() {
  float humid = dht.readHumidity(); // đo độ ẩm không khí
  float temp = dht.readTemperature(); 
  uint16_t lux = lightMeter.readLightLevel();//vì ánh sáng ko âm nên uint16_t là số nguyên ko âm 16 bit
  if(isnan(humid) || isnan(temp)) //isnan(): kieemr tra gia tri bi loi
  {
    Serial.println("Loi doc DHT22!");
  }
  else
  {
    //
    String payload = String(temp, 1) + "," + String(pressure, 1) + "," + String(humid, 1) + "," +  String(lux) + "," + String(rain);
    Serial.print("Dang gui goi tin: ");
    Serial.println(payload);
    Serial2.println(payload); //gửi qua lora
  }
  delay(5000); // chờ 5 s rồi gửi gói tiếp theo
}
>>>>>>> DHT22_BH1750
