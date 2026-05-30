#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define RXD2 16
#define TXD2 17
#define LORA_BAUD 9600

// WIFI
const char *ssid = "Le Phu Son";
const char *password = "02062006";

// SERVER PYTHON
String server = "http://192.168.1.6:8000/update.php";

void setup()
{
    Serial.begin(9600);

    // UART LoRa
    Serial2.begin(LORA_BAUD, SERIAL_8N1, RXD2, TXD2);

    Serial.println();
    Serial.println("===== GATEWAY START =====");

    // WIFI
    WiFi.begin(ssid, password);

    Serial.print("Dang ket noi WiFi");

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("WiFi connected");

    Serial.print("IP Gateway: ");
    Serial.println(WiFi.localIP());

    Serial.println("Dang cho du lieu...");
}

void loop()
{
    // có dữ liệu LoRa gửi tới
    if (Serial2.available())
    {
        String s = Serial2.readStringUntil('\n');

        s.trim();

        // bỏ qua chuỗi rỗng
        if (s.length() == 0)
        {
            return;
        }

        Serial.println();
        Serial.println("===== DATA =====");
        Serial.println(s);

        // format:
        // temp,pressure,humidity,lux,rain

        int p1 = s.indexOf(',');
        int p2 = s.indexOf(',', p1 + 1);
        int p3 = s.indexOf(',', p2 + 1);
        int p4 = s.indexOf(',', p3 + 1);

        // lỗi format
        if (p1 < 0 || p2 < 0 || p3 < 0 || p4 < 0)
        {
            Serial.println("LOI DU LIEU");
            return;
        }

        // tách dữ liệu
        float temp = s.substring(0, p1).toFloat();

        float pressure =
            s.substring(p1 + 1, p2).toFloat();

        float hum =
            s.substring(p2 + 1, p3).toFloat();

        int lux =
            s.substring(p3 + 1, p4).toInt();

        int rain =
            s.substring(p4 + 1).toInt();

        // xác định trạng thái mưa
        String mua;

        if (rain < 400)
        {
            mua = "mua_to";
        }
        else if (rain < 1500)
        {
            mua = "mua_nhe";
        }
        else
        {
            mua = "khong_mua";
        }

        // in ra serial monitor
        Serial.println("------ SENSOR ------");

        Serial.print("Nhiet do: ");
        Serial.println(temp);

        Serial.print("Ap suat: ");
        Serial.println(pressure);

        Serial.print("Do am: ");
        Serial.println(hum);

        Serial.print("Anh sang: ");
        Serial.println(lux);

        Serial.print("Mua: ");
        Serial.print(rain);
        Serial.print(" -> ");
        Serial.println(mua);

        // gửi web
        if (WiFi.status() == WL_CONNECTED)
        {
            HTTPClient http;

            String url = server;

            url += "?temp=" + String(temp, 1);
            url += "&pressure=" + String(pressure, 1);
            url += "&hum=" + String(hum, 1);
            url += "&lux=" + String(lux);
            url += "&rain=" + String(rain);
            url += "&status=" + mua;

            Serial.println();
            Serial.println("GUI SERVER:");
            Serial.println(url);

            http.begin(url);

            int httpCode = http.GET();

            Serial.print("HTTP CODE: ");
            Serial.println(httpCode);

            if (httpCode > 0)
            {
                String response = http.getString();

                Serial.println("SERVER RESP:");
                Serial.println(response);
            }
            else
            {
                Serial.println("GUI THAT BAI");
            }

            http.end();
        }
        else
        {
            Serial.println("MAT KET NOI WIFI");
        }
    }
    else
    {
        Serial.println("KHONG CO DU LIEU");
        delay(1000);
    }
}