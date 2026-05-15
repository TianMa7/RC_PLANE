# Fixed-Wing RC Aircraft

## ✈️ Summary and takeaways
**Mission goal:** Design and fly an RC aircraft capable of fly-by-wire controls and stable flight. Learn the nuances of designing systems that can withstand the complex dynamics of flight.

### 📋 Technical specifications
| Category | Metric |
| :--- | :--- |
| **Wingspan** | 99.25 cm |
| **Airfoil** | NACA 2412 |
| **Weight** | 837 g |
| **Motor** | 2212 1400 KV BLDC motor |
| **Battery** | 3S 2200 mAh LiPo |
| **Material cost** | $157 CAD |
| **Materials list** | [Excel sheet](HARDWARE/RC%20Plane%20-%20Major%20Item%20List.xlsx) |

---

### 🛠️ Design & aerodynamics
- **Research & development:** Selection of the NACA 2412 airfoil was driven by the need for a stable lift-to-drag ratio across a variety of Reynolds numbers suitable for small-scale RC flight. Pul[...]
- **Simulation (CFD):** STAR-CCM+ CFD was used to validate design choices and identify potential changes. It was also used to calculate the aerodynamic balance point to determine weight distribu[...]
- **Mechanical (CAD):** SolidWorks was used to model the aircraft and internal components. Designs were originally considered to be a combination of laser-cut balsa wood, composite construction, a[...]

### 💻 Systems & integration
- **Scope:** Development of electronics and control software capable of producing stable, controllable flight for the plane. Controls are fly-by-wire for pitch and roll, and raw input for yaw. Communi[...]
- **Control logic:** RTOS implementation of a PID-based control loop taking input from blended IMU data with a Mahony filter, as well as user input from the RC controller. Uses a PID loop to maintain stabi[...]
- **Electronics:** Protoboard containing ESP32-C3, LSM6DS3, and other components. Controlled a BLDC motor as well as three MG90S servos. A full material list and breakdown can be found [HERE](HARDWARE/RC%20Plane%20-%20Major%20Item%20List.xlsx).

### 🚀 Validation & flight results
- **Pre-flight:** Aircraft was 16% below maximum calculated weight. Static thrust and controls testing validated structural integrity of the body as well as the electronics.
- **Maiden flight:** [Insert date][cite: 1]
- **Outcome:** EMF interference with unshielded electronic devices (servo, IMU) is theorized to be the reason behind unpredictable and unlevel flight/crash. Issue was not flagged as interference o[...]

---

*Developed by Tian Ma, Hunter Liu, Andrew Poon*
