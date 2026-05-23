#  Hệ Thống Quan Trắc Kho Lạnh Dược Phẩm (Cold Storage Monitoring)

![ESP32-S3](https://img.shields.io/badge/Platform-ESP32--S3-blue?logo=espressif)
![FreeRTOS](https://img.shields.io/badge/RTOS-FreeRTOS-green?logo=freertos)
![TinyML](https://img.shields.io/badge/AI-TinyML%20%7C%20TFLite-orange?logo=tensorflow)
![CoreIoT](https://img.shields.io/badge/Cloud-CoreIoT%20%7C%20ThingsBoard-00569e)

Hệ thống cung cấp giải pháp giám sát nhiệt độ, độ ẩm cho chuỗi cung ứng lạnh dược phẩm. Điểm nhấn của dự án là việc tích hợp Trí tuệ nhân tạo tại biên (**Edge AI / TinyML**) và cơ chế dự phòng mất mạng tự động qua **ESP-NOW**.

---

## ✨ Tính năng nổi bật

*  **Phân loại rủi ro bằng TinyML:** Không sử dụng các ngưỡng `if-else` cứng nhắc, hệ thống chạy một mô hình Mạng nơ-ron (TensorFlow Lite Micro) trực tiếp trên ESP32-S3 để phân tích 7 đặc trưng thời gian thực (nhiệt độ, độ ẩm, độ trễ, trung bình trượt...). Từ đó phân loại chính xác 3 trạng thái: *An toàn (Optimal)*, *Nguy cơ vừa (Moderate Risk)*, và *Nguy cơ cao (High Risk)*.
*  **Đa nhiệm thời gian thực (FreeRTOS):** Kiến trúc phần mềm được chia thành 7 tasks độc lập chạy trên 2 nhân của ESP32-S3. Các tác vụ nặng về mạng (Webserver, MQTT) được cách ly khỏi các tác vụ thu thập cảm biến và suy luận AI, ngăn chặn triệt để hiện tượng nghẽn luồng.
*  **Cơ chế cứu hộ Gateway (ESP-NOW):** Khi một thiết bị trong kho bị mất kết nối Wi-Fi, nó sẽ tự động phát quảng bá dữ liệu (broadcast) qua sóng ESP-NOW. Các thiết bị lân cận sẽ đóng vai trò Gateway để chuyển tiếp dữ liệu lên đám mây, đảm bảo luồng dữ liệu không bao giờ bị gián đoạn.
*  **Cấu hình Zero-Code (Captive Portal):** Người dùng không cần biên dịch lại code khi đổi Wi-Fi. Ở lần đầu khởi động, thiết bị tự phát Wi-Fi (AP mode) để người dùng truy cập trang `192.168.4.1` cài đặt thông tin mạng và token CoreIoT. 
*  **Giám sát Kép (Local & Cloud):**
    * **Local Webserver:** Giao diện web mượt mà cập nhật dữ liệu realtime qua WebSocket.
    * **CoreIoT (ThingsBoard):** Gửi telemetry (nhiệt độ, độ ẩm, nhãn rủi ro) và nhận lệnh điều khiển thiết bị (RPC) từ xa.
*  **Cảnh báo trực quan:**
    * Màn hình LCD I2C 16x2 hiển thị trạng thái và độ tin cậy của AI.
    * Tần số chớp nháy của LED đỏ thay đổi tỉ lệ thuận với nhiệt độ thực tế.
    * Vòng LED NeoPixel thay đổi dải màu theo trạng thái rủi ro hoặc theo độ ẩm.

---

## 🛠️ Yêu cầu phần cứng

1.  **Mạch điều khiển:** Yolo Uno (Vi điều khiển ESP32-S3-WROOM-1, Dual-core 240MHz)
2.  **Cảm biến:** DHT20 (Giao tiếp I2C)
3.  **Hiển thị:** Màn hình LCD 16x2 kèm module I2C
4.  **Chỉ thị:** Đèn LED đỏ (GPIO 48) và NeoPixel WS2812 (GPIO 45) tích hợp sẵn trên mạch.

---

## 🏗️ Kiến trúc Phần mềm

Mã nguồn được viết bằng **C++** trên nền tảng **PlatformIO**. Hệ thống tận dụng sức mạnh của FreeRTOS với việc phân chia tài nguyên hợp lý qua các Hàng đợi (Queue) và Cờ hiệu (Semaphore):

* **Core 1 (Application):** Chuyên xử lý việc đọc cảm biến (`TempHumiMonitor`), phân tích AI (`TinyML`), điều khiển LED (`LedBlinky`, `NeoBlinky`), và giám sát nút nhấn phần cứng (`ToggleBoot`).
* **Core 0 (Networking):** Xử lý giao thức mạng bao gồm `WebserverStream` (AsyncTCP, WebSockets) và `Task_Core_IoT` (MQTT, RPC).

Dữ liệu được trung chuyển qua các hàng đợi dạng *overwrite* (chỉ lấy giá trị mới nhất) giúp hệ thống phản hồi với độ trễ cực thấp.

---

## 🚀 Hướng dẫn cài đặt

### 1. Nạp Firmware
1. Clone repository này về máy tính.
2. Mở dự án bằng **VS Code** với extension **PlatformIO**.
3. Kết nối board Yolo Uno vào máy tính.
4. Bấm **Build** và **Upload** firmware. Mở Serial Monitor (baud rate `115200`).

### 2. Cấu hình thiết bị lần đầu
1. Lần đầu khởi động, board sẽ phát ra một mạng Wi-Fi (Ví dụ: `ESP32 K2`).
2. Dùng điện thoại/laptop kết nối vào Wi-Fi này.
3. Trình duyệt sẽ tự động mở (hoặc truy cập thủ công vào `http://192.168.4.1`).
4. Điền thông tin Wi-Fi nhà xưởng, Token CoreIoT và lưu cấu hình. 
5. Thiết bị sẽ tự khởi động lại và kết nối vào hệ thống giám sát.

>  **Note (Factory Reset):** Nhấn giữ nút `BOOT` trên mạch quá 2 giây, thiết bị sẽ xoá toàn bộ cấu hình mạng cũ và quay về trạng thái thiết lập ban đầu.

---

## 👥 Contributors

* **Huỳnh Trung Kiên (2311730):** Kiến trúc hệ thống, ESP-NOW Gateway, CoreIoT, Quản lý tác vụ FreeRTOS.
* **Trần Đăng Khoa (2311645):** Xây dựng mô hình TinyML, Local Async Webserver, WebSocket, Giao diện điều khiển.