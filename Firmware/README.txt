DỰ ÁN: ĐO NHIỆT ĐỘ MÔI TRƯỜNG ỨNG DỤNG TRONG GIÁM SÁT NHIỆT ĐỘ NHÀ KÍNH
Với dự án này, nhóm sinh viên thực hiện triển khai một hệ thống mạng cảm biến không dây giám sát nhiệt độ môi trong nhà kính sử dụng công nghệ truyền tin không dây Lora

YÊU CẦU TRIỂN KHAI
Để có thể triển khai được dự án này, cần cài đặt những phần mềm sau:
- Visual Studio Code
- IoT Development Framework chính thức của Espressif dành cho ESP32 (ESP - IDF) (Hướng dẫn: https://www.youtube.com/watch?v=OvMsayjTvjE&list=PL0LsX_xUaMBOf8jiiByV5mDIzsPB5P19j)
- STM32CubeIDE (phiên bản 1.11.0 trở lên)
- Esptouch trên điện thoại (có thể tìm trên App Store hoặc CH Play)
Phần cứng được chế tạo kèm theo dự án

TỔ CHỨC CHƯƠNG TRÌNH
Dự án bao gồm 3 folder chương trình:
1. Gateway: Folder folder chương trình gateway cho module esp32
2. Node_Sensor_1: folder chương trình node cảm biến thứ nhât
3. Node_Sensor_2: folder chương trình node cảm biến thứ hai

CÁCH CÀI ĐẶT VÀ SỬ DỤNG
Bước 1: Mở terminal ESP-IDF, trỏ đến thư mục chứ folder Gateway
Bước 2: Xóa flash của module ESP32 bằng lệnh: idf.py -p PORT erase_flash
với PORT là cổng COM tương ứng của thiết bị (xem trong Devices Manager)
Bước 3: Build và flash chương trình xuống module ESP32 đóng vai trò là gateway bằng lệnh: idf.py -p PORT flash monitor
với PORT là cổng COM tương ứng của thiết bị (xem trong Devices Manager), để thoát cửa sổ nhấn tổ hợp Ctrl + C
Bước 4: Sử dụng Esptouch để kết nối Wi-Fi cho module ESP32 thông qua SmartConfig 
Ví dụ terminal kết nối Wi-Fi thành công:
I (78114) esp_netif_handlers: sta ip: 192.168.0.115, mask: 255.255.255.0, gw: 192.168.0.1
I (78114) smartconfig_example: got ip:192.168.0.115
I (81194) smartconfig_example: smartconfig over
I (81194) app_mqtt: [APP] Free memory: 235068 bytes
I (81194) app_http_server: Starting server on port: '80'
I (81194) app_mqtt: Other event id:7
I (81214) app_http_server: Registering URI handlers

Bước 5: Mở folder Node_Sensor_1 bằng STM32CubeIDE
Bước 6: Vào file main.c, nạp chương trình xuống phần cứng node cảm biến thứ nhất
Bước 7: Mở folder Node_Sensor_2 bằng STM32CubeIDE
Bước 8: Vào file main.c, nạp chương trình xuống phần cứng node cảm biến thứ hai
Bước 9: Đăng nhập vào tài khoản Ubidots để sử dụng giao diện người dùng
Username: tungpham3i.hust
Password: tung8102001
Sau khi thực hiện xong các bước trên, người dùng có thể giám sát được thông số nhiệt độ gửi về của từng node và trạng thái node có đang kết nối trong mạng không