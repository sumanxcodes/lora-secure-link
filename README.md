# LoRa Secure Communication
COS60011 Technology Design Project

# Securing LoRa-Based Networking for Sensor Data Transmission

This project focuses on securing LoRa-based networking for sensor data transmission from a prototype Flux Tower. It is part of an effort to reduce the cost of measuring greenhouse gas uptake in vegetation by using low-cost components.

## Background: Why This Project Matters

### Greenhouse Gas and Climate Change
- There is a critical need to reduce CO₂ emissions and also withdraw excess CO₂ from the atmosphere.
- One way to do this is by studying how different vegetation types absorb CO₂.

### Measuring CO₂ Absorption
- Certain crops, plants, and land types absorb CO₂ better than others.
- Flux Towers are used to measure this absorption.

### Flux Towers
- These are tall sensor towers that gather microclimate data to monitor CO₂ uptake.
- **The problem?** They are very expensive (~$70K-$100K each).

### Project Goal
- Reduce costs by replacing expensive parts (e.g., dataloggers, cellular links) with low-cost components (e.g., Raspberry Pi, LoRa).
- Implement a LoRa-based networking system for these towers.

## What is LoRa?

LoRa (Long Range) is a low-cost, low-power, wireless communication technology. It is designed for Internet of Things (IoT) applications with long battery life.

### Key Features:
- **Low power consumption** – Can operate for years on small batteries.
- **Long range** – Can cover 1+ km even with basic antennas.
- **Does not support TCP/IP** – Requires custom network implementation.
- **Low data rate** – Max ~20 kbps, suitable for transmitting sensor data.

## My Role as a Cybersecurity Student

### Main Task
- Implement security for the LoRa network.

## Minimum Deliverables (Pass/Credit Level)
- Develop a basic LoRa-based communication system.
- Encrypt messages and authenticate nodes using hardcoded keys.

## Stretch Goals (Higher Marks)
- Implement **Diffie-Hellman hybrid key exchange** to establish session keys.
- Use **RSA** for authenticating nodes.
- Develop a **user-friendly management interface** for security monitoring.

## Advanced Stretch Goals
- Implement a **multi-hop relay system** (flooding messages with TTL strategy).
- Deploy the solution on an **industry-grade STM32 microcontroller**.
