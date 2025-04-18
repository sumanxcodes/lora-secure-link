# LoRa Secure Communication
**COS60011 Technology Design Project**

---

# 🔐 Securing LoRa-Based Networking for Sensor Data Transmission

This project focuses on securing LoRa-based networking for sensor data transmission from a prototype Flux Tower. It is part of an effort to reduce the cost of measuring greenhouse gas uptake in vegetation by using low-cost components.

---

## 📚 Navigation
- [Why This Project Matters](#background-why-this-project-matters)
- [What is LoRa?](#what-is-lora)
- [Project Structure](#project-structure)
- [Project Phases](#project-phases)
- [Dependencies](#dependencies)
- [PlatformIO Setup](#platformio-setup)
- [Test Setup](#test-setup)

---

## 🔍 Background: Why This Project Matters

### 🌍 Greenhouse Gas and Climate Change
- There is a critical need to reduce CO₂ emissions and also withdraw excess CO₂ from the atmosphere.
- One way to do this is by studying how different vegetation types absorb CO₂.

### 🌿 Measuring CO₂ Absorption
- Certain crops, plants, and land types absorb CO₂ better than others.
- Flux Towers are used to measure this absorption.

### 🗼 Flux Towers
- These are tall sensor towers that gather microclimate data to monitor CO₂ uptake.
- **The problem?** They are very expensive (~$70K-$100K each).

### 🎯 Project Goal
- Reduce costs by replacing expensive parts (e.g., dataloggers, cellular links) with low-cost components (e.g., Raspberry Pi, LoRa).
- Implement a LoRa-based networking system for these towers.

---

## 📡 What is LoRa?

LoRa (Long Range) is a low-cost, low-power, wireless communication technology. It is designed for Internet of Things (IoT) applications with long battery life.

### 🔧 LoRa Features:
- **Low power consumption** – Can operate for years on small batteries.
- **Long range** – Can cover 1+ km even with basic antennas.
- **Does not support TCP/IP** – Requires custom network implementation.
- **Low data rate** – Max ~20 kbps, suitable for transmitting sensor data.

---

## 📦 Project Type
Embedded Communication / Security / IoT

## 📍 PlatformIO Board
Arduino Uno / ESP32 / STM32 (adjust as needed)

## 📡 Radio Module
SX1278 (LoRa) using RadioHead or LoRa.h

---

## 📁 Project Structure
```
src/
├── main.cpp                   → Entry point with setup/loop
├── DH_Exchange.{h,cpp}        → Handles Diffie-Hellman key exchange
├── NodeManager.{h,cpp}        → Tracks node states and IDs
├── MessageUtils.{h,cpp}       → Message creation and parsing helpers
├── Challenge_Response.{h,cpp} → Challenge-response authentication
└── Message_Handler.{h,cpp}    → Handles encrypted messages

include/
└── (same headers)

lib/
└── (optional shared libraries)
```

---

## 🔑 Project Phases
1. **DH Key Exchange:** Share public keys, compute shared session key  
2. **Challenge-Response:** Authenticate peers using shared session key  
3. **Encrypted Messaging:** Secure data transmission using session key

---

## 🔧 Dependencies
- [Arduino LoRa library](https://platformio.org/lib/show/5003/LoRa) (`sandeepmistry/LoRa`)
- SPI library (built-in)
- PlatformIO default toolchain for your selected board

---

## 🛠 PlatformIO Setup
Add this to your `platformio.ini`:

```ini
[env:your_board]
platform = atmelavr  ; or espressif32
board = uno          ; or esp32dev
framework = arduino
lib_deps =
    sandeepmistry/LoRa
```

---

## 🧪 Test Setup
- Two LoRa-enabled nodes with unique `DEVICE_ID`s
- Serial monitor at 9600 baud
- Power via USB or battery

---
