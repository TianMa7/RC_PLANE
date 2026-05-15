# Fixed-Wing RC Aircraft

## ✈️ Summary and Takeaways
**Mission Goal:** Design and fly an RC Aircraft capable of fly-by-wire controls and stable flight. Learn about nuances of designing systems capable of withstanding the complex dynamics of flight. 

### 📋 Technical Specifications
| Category | Metric |
| :--- | :--- |
| **Wingspan** | 99.25cm |
| **Airfoil** | NACA 2412 |
| **Weight** | 837g |
| **Motor** | 2212 1400kv BLDC Motor |
| **Battery** | 3S 2200 mAh LIPO |
| **Material Cost** | $157 CAD |
| **Materials List** | [Download Excel Sheet](https://github.com/TianMa7/RC_PLANE/raw/master/HARDWARE/RC%20Plane%20-%20Major%20Item%20List.xlsx)

---

### 🛠️ Design & Aerodynamics
* **Research & Development:** Selection of the NACA 2412 airfoil was driven by the need for a stable lift-to-drag ratio across a variety of Reynolds numbers suitable for small-scale RC flight. Pull-Pull control systems were implemented to reduce and optimize weight distribution.
* **Simulation (CFD):** Star-CCM+ CFD was used to validate design choices and identify potential changes. It was also used to calculate the aerodynamic balance point to determine weight distribution.
* **Mechanical (CAD):** SolidWorks was used to model the aircraft and internal components. Designs were originally considered to be a combination of laser cut balsa wood, composite construction, and 3D printing. Project was gradually descoped to 3D printing and composite components to save manufacturing time and complexity.

### 💻 Systems & Integration
* **Scope:** Development of electronics and control software capable of producing stable, controllable flight for plane. Controls are fly-by-wire for pitch and roll, and raw input for yaw. Communication between radio rx/tx was beyond the scope of the project.
* **Control Logic:** RTOS Implementation of a PID-based control loop taking input from blended IMU data with Mahony filter as well as user input from rc controller. Uses PID loop to maintain stability when external forces are acting upon it.
* **Electronics:** Protoboard containing esp32c3, lsm6ds3, and other components. Controlled a BLDC motor as well as 3 MG90s Servos. A full material list and breakdown can be found [HERE](https://github.com/TianMa7/RC_PLANE/raw/master/HARDWARE/RC%20Plane%20-%20Major%20Item%20List.xlsx).

### 🚀 Validation & Flight Results
* **Pre-Flight:** Aircraft was 16% below maximum calculated weight. Static thrust and controls testing validated structural integrity of the body as well as the electronics. 
* **Maiden Flight:** [Insert Date]
* **Outcome:** EMF interference towards unshielded electronic devices (servo, imu) theorized to be the reason behind unpredictable and unlevel flight/crash. Issue was not flagged as interference only became an issue when motor was facing load from propeller. Bench testing was primarily not conducted with propeller on for safety. Future designs should explore methods of reducing such interference, as well as creating more robust control surfaces and improving software design. Mechanical design will focus on improving aerodynamic efficiency as well as shifting CoG forward for better stability.

---

### 📷 Photos & Videos

<table>
  <tr>
    <td width="33%"><img src="R&D/Photos%20&%20Videos/IMG_6453.JPG" width="100%" alt="Plane Photo 1"></td>
    <td width="33%"><img src="R&D/Photos%20&%20Videos/IMG_6507.JPG" width="100%" alt="Plane Photo 2"></td>
    <td width="33%"><img src="R&D/Photos%20&%20Videos/IMG_6516.JPG" width="100%" alt="Plane Photo 3"></td>
  </tr>
  <tr>
    <td width="33%">
      <video src="https://github.com/TianMa7/RC_PLANE/raw/master/R&D/Photos%20&%20Videos/Flight%20Test.mov" width="100%" controls></video>
      <br><em>Flight Test</em>
    </td>
    <td width="33%">
      <video src="https://github.com/TianMa7/RC_PLANE/raw/master/R&D/Photos%20&%20Videos/Auto_Balancing_Test.mov" width="100%" controls></video>
      <br><em>Auto Balancing Test</em>
    </td>
    <td width="33%">
      <video src="https://github.com/TianMa7/RC_PLANE/raw/master/R&D/Photos%20&%20Videos/Ailerons_Test.mov" width="100%" controls></video>
      <br><em>Ailerons Test</em>
    </td>
  </tr>
</table>

---
*Developed by Tian Ma, Hunter Liu, Andrew Poon*
