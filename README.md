# Smart Door System for Bedridden and Differently Abled Individuals

This project presents a **Smart Door System** designed to enhance **home security**, **remote visitor management**, and **emergency response**, tailored specifically for bedridden and differently abled individuals. With a blend of IoT, automation, and real-time communication, the system promotes accessibility, safety, and independence.

## Features

- **ESP32-CAM Integration**  
  Captures real-time photos of visitors and sends instant notifications through a Telegram Bot.

- **RFID Authentication**  
  Ensures secure access for caregivers and authorized individuals using RFID cards.

- **MLX90614 Temperature Sensor**  
  Measures visitor body temperature as a health safety check before allowing entry.

- **ISD1820 Voice Module**  
  Delivers voice-based feedback at the door to guide or inform visitors.

- **Solenoid Lock Control**  
  Enables remote door locking/unlocking via authenticated commands from the user’s smartphone.

- **Tamper Detection System**  
  Detects physical tampering and triggers emergency alerts via Telegram and Twilio voice call API.

- **Power Backup System**  
  Operates on a 12V Li-ion battery with regulated voltage to maintain function during power outages.

- **User-Friendly Interface**  
  Controlled entirely via a smartphone app, minimizing the need for physical interaction.

## Objectives

- Prioritize **patient safety** and **inclusivity** in smart home automation.
- Offer a **cost-effective** and **easy-to-install** solution.
- Provide caregivers with reliable tools for monitoring and secure access.

## Hardware Components

- ESP32-CAM Module
- MLX90614 Infrared Temperature Sensor
- RFID Reader and Tags
- ISD1820 Voice Module
- Solenoid Lock
- 12V Li-ion Battery + Voltage Regulator
- Custom PCB/Prototyping Board
- Jumper Wires, Resistors, and other standard electronics

## Software Stack

- **Arduino IDE** for firmware development
- **Telegram Bot API** for real-time visitor notifications
- **Twilio API** for automated emergency voice calls
- **Smartphone App** (via Telegram interface or custom-built UI)

## System Architecture

```mermaid
graph TD
  A[Visitor Arrives] --> B[ESP32-CAM Captures Photo]
  B --> C[Telegram Notification to User]
  A --> D[RFID Verification]
  D -->|Authorized| E[Unlock Door]
  D -->|Unauthorized| F[Voice Feedback via ISD1820]
  A --> G[Temperature Check via MLX90614]
  G --> H[Log or Deny Access]
  I[Tamper Detected] --> J[Telegram Alert + Twilio Call]
  K[User Smartphone] --> E



