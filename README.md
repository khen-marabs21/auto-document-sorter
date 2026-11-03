# Auto Document Sorter using OCR & ESP32 / Arduino

An IoT project that uses a smartphone camera, Tesseract.js browser-based OCR, an ESP32 web server, and an Arduino to automatically sort documents (such as Official Registration Forms / ORF and Summary of Grades / SOG) via a servo motor.

## Project Structure
- `/arduino_sorter` - Arduino microcontroller code (handles LCD, LEDs, and Servo motor based on serial commands).
- `/esp32_web_server` - ESP32 microcontroller code (hosts the Web UI, handles Wi-Fi, and sends UART commands to Arduino).

## Wiring Overview
- **ESP32 TX2 (GPIO 17)** -> **Arduino RX (Pin 2 via SoftwareSerial)**
- **ESP32 RX2 (GPIO 16)** -> **Arduino TX (Pin 3 via SoftwareSerial)**
- **Arduino Servo** -> **Pin 9**
- **I2C LCD Display** -> **SDA / SCL pins**
