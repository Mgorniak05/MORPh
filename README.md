# MORPh — Modular Optical Rover Platform

**MORPh** is a modular 2D space-mapping robot built with **Lego Technic** and powered by an **ESP32-S3** microcontroller. Equipped with a rotating **VL53L1X Time-of-Flight (ToF)** laser distance sensor and a real-time **WebSocket** control dashboard, the rover is designed for indoor obstacle detection, telemetry streaming, and 2D spatial mapping.

---

## Hardware Architecture

* **Microcontroller:** ESP32-S3 DevKit
* **Optical Distance Sensor:** VL53L1X ToF Laser
* **Drive System:** 2× N20 DC Micro Gear Motors
* **Motor Driver:** H-Bridge (TB6612FNG)
* **Scanner Turret:** 360° Continuous Rotation Servo
* **Power System:** 2× 18650 Li-Ion Battery Pack (7.4V nominal / 8.4V max) + DC-DC Step-Down Converter
* **Chassis:** Modular Lego Technic frame

---

## ESP32-S3 Pinout

| Peripheral / Module | Module Pin | ESP32-S3 Pin | Function |
| :--- | :--- | :--- | :--- |
| **H-Bridge (Motor A - Left)** | `PWMA` | **GPIO 5** | PWM Speed Control (Channel A) |
| **H-Bridge (Motor A - Left)** | `AIN1` | **GPIO 4** | Direction Control Input 1 |
| **H-Bridge (Motor A - Left)** | `AIN2` | **GPIO 7** | Direction Control Input 2 |
| **H-Bridge (Motor B - Right)** | `PWMB` | **GPIO 17** | PWM Speed Control (Channel B) |
| **H-Bridge (Motor B - Right)** | `BIN1` | **GPIO 15** | Direction Control Input 1 |
| **H-Bridge (Motor B - Right)** | `BIN2` | **GPIO 16** | Direction Control Input 2 |
| **H-Bridge (Control)** | `STBY` | **3V3** | Driver Enable (pulled HIGH) |
| **360° Scanner Servo** | `PWM` | **GPIO 14** | 50 Hz Servo Control Signal |
| **VL53L1X ToF Sensor** | `SDA` | **GPIO 2** | I2C Data Line |
| **VL53L1X ToF Sensor** | `SCL` | **GPIO 1** | I2C Clock Line |
| **VL53L1X ToF Sensor** | `VIN` / `GND` | **3V3** / **GND** | Sensor Power Supply |

---

## WebSocket Dashboard & Controls

The ESP32-S3 hosts a WebSocket server on **port `81`**, streaming telemetry packets (`Action,Distance`) every `150 ms` to `index.html`.

* **Rover Driving:**
  * `[W]` — Move Forward
  * `[S]` — Move Backward
  * `[A]` — Turn Left
  * `[D]` — Turn Right
* **Scanner Turret:**
  * `[Q]` — Step / Rotate Left
  * `[E]` — Step / Rotate Right

---

## Getting Started

1. **Clone the repository:**
   ```bash
   git clone [https://github.com/Mgorniak05/MORPh.git](https://github.com/Mgorniak05/MORPh.git)
   cd MORPh

2. **Configure Wi-Fi credentials:**
   Create an `include/secrets.h` file (ignored by Git) with your network details:
   ```cpp
   #pragma once
   #define WIFI_SSID "YOUR_WIFI_SSID"
   #define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
   ```

3. **Build and upload firmware:**
   Open the project in **VS Code + PlatformIO**, connect the ESP32-S3 via the **COM (UART)** port, and upload the firmware.

4. **Launch the control panel:**
   Check the assigned IP address in the Serial Monitor (`115200 baud`), update `RobotIP` in `index.html` if needed, and open `index.html` in your browser.

---

## Roadmap

- [x] Differential drive control via H-Bridge & WebSockets
- [x] Non-blocking 360° servo stepping control
- [x] VL53L1X ToF integration with narrowed 15° ROI (`4x4`)
- [ ] Real-time 2D radar visualization in the web dashboard
- [ ] Automated 360° room sweep & 2D obstacle point-cloud mapping