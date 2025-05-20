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

## 📡 LoRa Secure Communication with FSM, Diffie-Hellman & Challenge-Response

This project implements a secure peer-to-peer communication system over LoRa using:
- Finite State Machine (FSM) logic
- Diffie-Hellman key exchange
- Challenge-Response authentication
- Stream cipher encryption

It supports multi-node communication using broadcast and peer-specific messaging, ideal for secure IoT communication.

---

## 📁 Project Structure
```
/src
  ├── lora_rx_node.cpp       // RX node logic
  ├── lora_tx_node.cpp       // TX node logic

/lib
  ├── ChallengeAuth/         // Challenge-response auth
  ├── DHExchange/            // Diffie-Hellman key exchange
  ├── Encryption/            // Stream cipher encryption
  ├── MessageUtils/          // Message creation/parsing
  ├── NodeManager/           // Peer state tracking
  ├── EEPROMReader/          // Load device config from EEPROM
  ├── LoRaConfig/LoRaSetup.h // LoRa setup helpers
```

---

## 🔐 How It Works

### 1. **Device Startup**
- Each node reads its ID and seed from EEPROM.
- A `CLEAR` broadcast resets all peer states.

---

### 2. **Discovery via PING**
- RX node sends a `PING` to `ALL`.
- TX nodes respond with `PONG`.

---

### 3. **Diffie-Hellman Key Exchange**
- Upon receiving `PONG`, RX sends its public key (`PK`).
- TX responds with its own public key.
- Both nodes compute the same shared secret (session key) using DH.

---

### 4. **Challenge-Response Authentication**
- RX sends a random nonce in a `CHAL` message encrypted with the session key.
- TX decrypts the challenge, re-encrypts it, and sends back a `RESP`.
- RX verifies the response using the shared key.
- If valid, both nodes transition to `SECURE_COMM` state.

---

### 5. **Encrypted Communication**
- TX periodically reads from a light sensor and encrypts the data using a stream cipher (PRNG seeded by the session key + message count).
- Encrypted messages (`MSG`) are sent to the RX.
- RX decrypts them using the same PRNG setup.

---

### 6. **ACK and Retry**
- RX sends `ACK` to confirm secure communication.
- If `ACK` is not received, TX retries until successful.

---

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

## 🛠️ Setup Instructions

1. Flash `lora_rx_node.cpp` to one Arduino (e.g., RX node)
2. Flash `lora_tx_node.cpp` to another Arduino (e.g., TX node)
3. Power on both nodes; ensure they share the same frequency and spreading factor
4. Use Serial Monitor to view status and logs

---

## 📎 Example Message Format

```
MSG:<sender>:<receiver>:<ttl>:<msgCount>:<payload>
CHAL:<sender>:<receiver>:<ttl>:<msgCount>:<encryptedNonce>
RESP:<sender>:<receiver>:<ttl>:<msgCount>:<encryptedResponse>
```

---
