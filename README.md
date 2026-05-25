# IoT System for Urban Resilience: Real-Time Flood Monitoring

This repository contains the firmware and architecture design for an automated smart city flood monitoring device, developed as part of the academic curriculum at Universidade Presbiteriana Mackenzie.

The system uses an ESP32 microcontroller integrated with an ultrasonic sensor to monitor water levels in urban drainage systems (e.g., storm drains and canals), publishing real-time telemetry to a cloud infrastructure via the MQTT protocol.

## 🛠️ Hardware Architecture

The prototype is simulated using the Wokwi platform and consists of the following components:
* **Microcontroller:** ESP32-WROOM-32D (with native TCP/IP stack support).
* **Input Sensor:** HC-SR04 Ultrasonic Sensor (utilizing Time-of-Flight algorithm).
* **Local Actuator:** 5mm Red LED (for real-time visual alerts).
* **Circuit Protection:** 220 Ω Resistor (current limiting for GPIO stability).

### Pin Mapping
* `GPIO 5` -> HC-SR04 Trigger
* `GPIO 18` -> HC-SR04 Echo
* `GPIO 2` -> Red LED Anode

---

## 💻 Firmware Features & Resilience Layers

The C++ firmware implements advanced processing techniques to ensure data integrity and edge resilience:

1.  **Moving Average Filter (Filtro de Média Móvel):** Smooths raw sensor readings using a 10-sample buffer, eliminating transient noise caused by water turbulence.
2.  **Hysteresis Control (5% Margin):** Stabilizes the LED actuator state, preventing rapid oscillation ("bouncing") when the water level hovers exactly at the threshold line.
3.  **Last Will and Testament (LWT):** Built into the MQTT connection scheme to automatically notify the central server with an `Offline` status if the device suffers an abrupt network failure.

---

## 🚀 How to Run the Project

1. Copy the full source code from `sketch.ino`.
2. Open the circuit design on [Wokwi](https://wokwi.com/projects/465034181390321665).
3. Install the **PubSubClient** library via the Wokwi Library Manager.
4. Click **Play** to start the simulation.
5. Click on the HC-SR04 sensor component to interactively simulate water level changes.

* **Critical Threshold:** Triggered when the distance to the water is $\le 42\text{ cm}$.
* **Telemetry Payload Example:** `{"distancia": 31, "alerta": 1}`
