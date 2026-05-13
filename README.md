# Fixed-Wing RC Aircraft

## ✈️ Summary and Takeaways
**Mission Goal:** Design and fly an RC Aircraft capable of fly-by-wire controls and stable flight. Learn about nuances of designing systems capable of withstanding the complex dynamics of flight. 

### 📋 Technical Specifications
| Category | Metric |
| :--- | :--- |
| **Wingspan** | 99.25cm |
| **Airfoil** | HUNTA SAY THIS |
| **Weight** | 837g |
| **Motor** | 2212 1400kv BLDC Motor |
| **Battery** | 3S 2200 mAh LIPO |
| **Material Cost** | $157 CAD |
| **Materials List** | [Excel Sheet](HARDWARE/RC Plane - Major Item List.xlsx)

---

### 🛠️ Design & Aerodynamics
* **Research & Development:** Selection of the NACA 2412 airfoil was driven by the need for a stable lift-to-drag ratio across a variety of Reynolds numbers suitable for small-scale RC flight.[cite: 1]
* **Simulation (CFD):** Aerodynamic performance was verified through Computational Fluid Dynamics to analyze stall characteristics and pressure distribution across the 80cm span.[cite: 1]
* **Mechanical (CAD):** The structural architecture was developed using a hybrid approach, combining precision 3D-printed components with lightweight XPS foam.[cite: 1]
* **Fabrication:**

### 💻 Systems & Integration
* **Scope:** Development of electronics and control software capable of producing stable, controllable flight for plane. Controls are fly-by-wire for pitch and roll, and raw input for raw. Communication between radio rx/tx was beyond the scope of the project.
* **Control Logic:** RTOS Implementation of a PID-based control loop taking input from blended IMU data with Mahony filter as well as user input from rc controller. Uses PID loop to maintain stability when external forces are acting upon it
* **Electronics:** Protoboard containing esp32c3, lsm6ds3, and other components. Controlled a BLDC motor as well as 3 MG90s Servos. A full material list and breakdown can be found [HERE](HARDWARE/RC Plane - Major Item List.xlsx).

### 🚀 Validation & Flight Results
* **Pre-Flight:** Aircraft was 16% below maximum calculated weight. Static thrust and controls testing validated structural integrity of the body as well as the electronics. 
* **Maiden Flight:** [Insert Date][cite: 1]
* **Outcome:** EMF interference towards unshielded electronic devices (servo, imu) theorized to be the reason behind unpredictable and unlevel flight/crash. Issue was not flagged as interference only became an issue when motor was facing load from propellor. bench testing was primarily not conducted with propellor on for safety. Future designs should explore methods of reducing such interference, as well as creating more robust control surfaces and improving software design. Mechanical design will focus on improving aerodynamic efficiency as well as shifting CoG forward for better stability.

---
*Developed by Tian Ma, Hunter Liu, Andrew Poon*
