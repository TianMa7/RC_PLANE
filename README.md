# [Project Name]: Fixed-Wing RC Aircraft

## ✈️ Executive Summary
**Mission Goal:** Design and manufacture a custom fixed-wing aircraft featuring fly-by-wire stabilization and high-efficiency glide performance.[cite: 1]

### 📋 Technical Specifications
| Category | Metric |
| :--- | :--- |
| **Wingspan** | 80cm[cite: 1] |
| **Airfoil** | NACA 2412[cite: 1] |
| **Weight (Target)** | 500-700g[cite: 1] |
| **Power System** | 2212 1400kv Motor |[cite: 1] |
| **Control Logic** | ESP32-S3 / C3 (ESP-IDF) |[cite: 1] |

---

### 🛠️ Design & Aerodynamics
* **Research & Development:** Selection of the NACA 2412 airfoil was driven by the need for a stable lift-to-drag ratio across a variety of Reynolds numbers suitable for small-scale RC flight.[cite: 1]
* **Simulation (CFD):** Aerodynamic performance was verified through Computational Fluid Dynamics to analyze stall characteristics and pressure distribution across the 80cm span.[cite: 1]
* **Mechanical (CAD):** The structural architecture was developed using a hybrid approach, combining precision 3D-printed components with lightweight XPS foam.[cite: 1]

### 💻 Systems & Integration
* **Avionics:** Utilizing the ESP32 microcontroller family to handle real-time sensor fusion and flight control logic.[cite: 1]
* **Control Logic:** Implementation of a cascaded PID control loop architecture for pitch and roll attitude command/hold.[cite: 1]
* **Electronics:** Integration of MPU-6050/LSM6DS3 IMUs for precise orientation data and logic-level shifting for high-performance ESC communication.[cite: 1]

### 🚀 Validation & Flight Results
* **Pre-Flight:** Success in static thrust testing (2212 motor) and structural verification of the wing spar under simulated G-loads.[cite: 1]
* **Maiden Flight:** [Insert Date][cite: 1]
* **Outcome:** The fly-by-wire system successfully translated pilot input into stabilized bank angles, achieving [X] mAh/km efficiency.[cite: 1]

---
*Developed by Tian Ma*[cite: 1]
