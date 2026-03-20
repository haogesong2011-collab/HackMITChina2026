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

###  Requirements
- Arduino IDE
- ESP32 Board Package (installed via Board Manager)
- Two ESP32-C3 boards (one for radar, one for glasses)

---
### 2️⃣ Install ESP32 Board Package (via Arduino IDE)

1. Open the Arduino IDE software  
2. In the menu bar, click **Board → Boards Manager**  
3. In the search box at the top right, type `ESP32`  
4. Find **ESP32 by Espressif Systems**  
5. Click **Install** to install the latest version  
6. Once installed, you can select ESP32 boards under **Board**

###  Radar Module Setup (ESP32 for Radar)

1. Connect the ESP32-C3 (radar module) to your computer
2. Open Arduino IDE
3. Open the file: `firmware/radar/radar.ino`
4. Select the correct board: **ESP32-C3**
5. Select the correct port
6. Upload the code to the ESP32 connected to the radar (LD2451)

---

###  Glasses Module Setup (ESP32 for Glasses)

1. Connect the ESP32-C3 (glasses module) to your computer
2. Open Arduino IDE
3. Open the file: `firmware/glasses/glasses.ino`
4. Select the correct board: **ESP32-C3**
5. Select the correct port
6. Upload the code to the ESP32 installed on the glasses

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

##  Future Improvements

- **Integration of a rear bicycle warning light system**  
  A smart rear light will be added to the bicycle. When the radar detects a fast-approaching vehicle, the light will flash automatically to warn vehicles behind.

- **OLED display for smart glasses**  
  An OLED display will be integrated into the glasses, allowing users to view key information (e.g., alerts, speed, navigation) through a semi-transparent lens, forming a lightweight HUD system.

- **Mobile application development**  
  A companion mobile app will be developed with:
  - SOS emergency alerts  
  - Real-time navigation support  
  - System status monitoring  

- **Navigation-guided LED strip**  
  A blue LED strip will be added along the edge of the lens. It will be controlled by the navigation system to provide intuitive directional guidance (e.g., turning left/right).

##  Team
- Harry Song — Hardware
- Leo Xie — 3D Modeling
- Henry Liu — Software