#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define RXD2 16
#define TXD2 17
#define LoRa_baudrate 115200

const char *ssid = "TEN_WIFI";
const char *password = "MAT_KHAU_WIFI";

// link web/server để nhận dữ liệu
String server = "http://your-server.com/update.php";

void setup()
{
    Serial.begin(115200);

    // Serial2 dùng để nhận dữ liệu từ module LoRa
    Serial2.begin(LoRa_baudrate, SERIAL_8N1, RXD2, TXD2);

    Serial.println("Gateway start");

    // kết nối wifi
    WiFi.begin(ssid, password);
    Serial.print("Connecting wifi");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("Wifi connected");
    Serial.println(WiFi.localIP());
}

void loop()
{
    // nếu LoRa gửi dữ liệu sang thì gateway đọc
    if (Serial2.available())
    {
        String s = Serial2.readStringUntil('\n');
        s.trim();

        Serial.println(s);

        // dữ liệu nhận dạng: temp,pressure,hum,lux,rain
        int p1 = s.indexOf(',');
        int p2 = s.indexOf(',', p1 + 1);
        int p3 = s.indexOf(',', p2 + 1);
        int p4 = s.indexOf(',', p3 + 1);

        // nếu thiếu dấu phẩy thì dữ liệu bị lỗi
        if (p1 < 0 || p2 < 0 || p3 < 0 || p4 < 0)
        {
            Serial.println("loi du lieu");
            return;
        }

        // tách từng giá trị cảm biến từ chuỗi nhận được
        float temp = s.substring(0, p1).toFloat();
        float pressure = s.substring(p1 + 1, p2).toFloat();
        float hum = s.substring(p2 + 1, p3).toFloat();
        int lux = s.substring(p3 + 1, p4).toInt();
        int rain = s.substring(p4 + 1).toInt();

        String mua = "";

        // phân loại mưa theo giá trị analog
        if (rain >= 0 && rain < 400)
        {
            mua = "mua_to";
        }
        else if (rain >= 400 && rain < 800)
        {
            mua = "mua_nhe";
        }
        else
        {
            mua = "khong_mua";
        }

        Serial.print("nhiet do: ");
        Serial.println(temp);
        Serial.print("ap suat: ");
        Serial.println(pressure);
        Serial.print("do am: ");
        Serial.println(hum);
        Serial.print("anh sang: ");
        Serial.println(lux);
        Serial.print("mua: ");
        Serial.print(rain);
        Serial.print(" - ");
        Serial.println(mua);

        // nếu còn wifi thì gửi dữ liệu lên web
        if (WiFi.status() == WL_CONNECTED)
        {
            HTTPClient http;

            // ghép dữ liệu vào link dạng GET
            String link = server + "?temp=" + String(temp);
            link += "&pressure=" + String(pressure);
            link += "&hum=" + String(hum);
            link += "&lux=" + String(lux);
            link += "&rain=" + String(rain);
            link += "&status=" + mua;

            http.begin(link);
            int code = http.GET();

            Serial.print("http: ");
            Serial.println(code);

            http.end();
        }
    }
}