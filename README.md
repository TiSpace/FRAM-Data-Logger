# 📝 FRAM Data Logger

A compact datalogger that stores sensor data in **FRAM memory**, using an **RTC** and **ScioSense ENS16x** and **ENS210** sensors. Designed for mobile, low-power applications with timestamped logging.

<img src="./pic/FRAM_DATA_Logger_V01.png" width="800" alt="FRAM Data Logger V01">

## 📦 Project Overview

This example demonstrates how to use **FRAM memory** with an **RTC** and **ScioSense ENS16x EVKit** to build a reliable datalogger.

- **FRAM Advantages:**  
  - Low power consumption  
  - Virtually unlimited write cycles  
- **Memory Capacity:** 32.768 kB  
- **Dataset Format:** 10 bytes per entry (includes timestamp)  
- **Setup Version:** `V01`  
- **Max Datasets:** 3,276

---

## 🔋 Power Supply

Designed for mobile use. Recommended power options:

- USB power bank<br>
  or<br>
- Dedicated battery + voltage converter

---

## ⚠️ Precautions

- **Baud Rate:** Set to **9600 Baud** for serial communication

---

## 📖 Menu Interface

Interactive menu available via serial monitor. Commands can be set during compilation or runtime.

```arduino
****************************
    FRAM Data Logger
C:\Users\myPath\saveToFRAM_V01.ino
19:05:35  Aug  1 2025

Menu:
------
?   this info
d   set date and time
s   START/STOP logging
r   dump logging
m   show/hide measurement
t   set time between measurements (0–255 sec)

Memory size (kB):              32768
Possible datasets to store:    3274
Time between measurements (s): 7
Number of stored datasets:     6
