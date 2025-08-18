# 📝 FRAM Data Logger

A compact datalogger that stores sensor data in **FRAM memory**, using an **RTC** and **ScioSense ENS16x** , **ENS210** and  **ENS220** sensors. Designed for mobile, low-power applications with timestamped logging.

<img src="./pic/FRAM_DATA_Logger_V01.png" width="800" alt="FRAM Data Logger V01">

## 📦 Project Overview

This example demonstrates how to use **FRAM memory** with an **RTC** and **ScioSense ENS16x EVKit** to build a reliable datalogger.

- **FRAM Advantages:**  
  - Low power consumption  
  - Virtually unlimited write cycles  
- **Memory Capacity:** 32.768 kB  
- **Dataset Format:** depending on setting (includes timestamp)  
- **Setup Version:** `V02`  
- **Max Datasets:** 32760 / size of dataset -20 

---

## 🔋 Power Supply

Designed for mobile use. Recommended power options:

- USB power bank<br>
  or<br>
- Dedicated battery + voltage converter

---

## ⚠️ Precautions

- **Baud Rate:** Set to **115200 Baud** for serial communication

---

## 📖 Menu Interface

Interactive menu available via serial monitor. Commands can be set during compilation or runtime.

```arduino
****************************
	FRAM Data Logger
C:\Users\myPath\FRAMLogger_V02\FRAMLogger_V02.ino
18:31:48  Aug 17 2025

Menue:
------

?   this info
d   set date and time
s   START/STOP logging
r   dump logging
m   show/hide measurement
t   set time between measurements (0-255sec)
c   toggle continuous/rollover mode
u   unified memory dump


Memory size (kB): 32768
possible dataset to store:     1819
time between measurements (s): 1
number of stored datasets:     18
```


## 🔄 Changelog
V1.0  Initial version
V2.0  - added ENS220 support
      - selectable parameters
