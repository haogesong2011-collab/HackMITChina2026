# HackMITChina2026  Smart Cycling Glasses

##  Project Background
Cyclists often face safety risks from fast-approaching vehicles and changing light conditions. This project aims to improve cycling safety through smart wearable technology.

##  Features
- Automatic lens darkening based on light intensity
- Rear radar detection (LD2451) for approaching vehicles
- Real-time alerts via glasses display
- Red LED warning system

##  Hardware
- ESP32-C3
- LD2451 mmWave radar
- Flexible LCD Lens with UV sensor
- LED strip（(red)
- Battery
- Type-C charging module
##  Software
- Arduino (.ino)
- ESP-NOW communication

##  How to Run
1. Install Arduino IDE
2. Install ESP32 board package
3. Open `firmware/glasses/glasses.ion`or`firmware/radar/radar.ion`
4. Upload to ESP32

##  System Architecture

The system is based on a dual-ESP32 distributed architecture:

### 1. Radar Module (ESP32-C3)
- Connected to the LD2451 mmWave radar
- Detects approaching vehicles (distance, speed, direction)
- Processes sensor data locally
- Sends processed data wirelessly via ESP-NOW

### 2. Glasses Module (ESP32-C3)
- Receives data from the radar module
- Interprets risk level (e.g., fast-approaching vehicles)
- Triggers visual alerts:
  - LED warning signals

###  Data Flow
Radar (ESP32) → ESP-NOW → Glasses (ESP32) → User Alert

###  Key Design Advantages
- Low latency communication (ESP-NOW)
- Distributed processing (reduces load on a single device)
- Modular design (easy to upgrade each module independently)


##  Dependencies
- ESP32 Arduino Core
- ESP-NOW library

##  Team
- Harry Song — Hardware
- Leo Xie — 3D Modeling
- Henry Liu — Software