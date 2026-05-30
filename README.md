# Du-bao-thoi-tiet
Bước 1: Khởi động Backend Server
Máy tính chạy server và ESP32 Gateway bắt buộc phải kết nối chung một mạng WiFi.

Cài đặt các thư viện Python cần thiết:
pip install fastapi uvicorn
Kiểm tra địa chỉ IPv4 của máy tính (Mở CMD gõ ipconfig trên Windows hoặc ifconfig trên Linux/macOS).
Khởi chạy Server:
python server.py
Server sẽ chạy tại http://<IP_MAY_TINH>:8000 và tự động tạo file weather.db.

Bước 2: Cấu hình và Nạp code cho Gateway (ESP32)
Mở file mã nguồn của Gateway trên Arduino IDE.

Cập nhật thông tin WiFi:

const char *ssid = "TEN_WIFI_CUA_BAN";
const char *password = "MAT_KHAU_WIFI";
Cập nhật địa chỉ Server (Thay IP_MAY_TINH bằng IPv4 lấy ở Bước 1):

C++

String server = "http://<IP_MAY_TINH>:8000/update.php";
Biên dịch và nạp code xuống ESP32 Gateway. Mở Serial Monitor (Baudrate 115200) để theo dõi quá trình kết nối WiFi.

Bước 3: Khởi động Trạm Phát (Node)
Kết nối đúng các chân cảm biến và LoRa với ESP32 Node theo mã nguồn.

Nạp code cho ESP32 Node.

Mở Serial Monitor để kiểm tra xem Node có đang đọc đúng cảm biến và phát chuỗi payload qua LoRa hay không.

Bước 4: Giám sát trên Dashboard
Khi Gateway nhận được dữ liệu từ Node và báo mã HTTP 200 trên Serial Monitor, bạn hãy mở trình duyệt web trên máy tính hoặc điện thoại (trong cùng mạng LAN) và truy cập vào:

http://<IP_MAY_TINH>:8000
Giao diện quản lý index.html sẽ hiện ra với dữ liệu thời gian thực được tự động cập nhật mỗi 3 giây.
