# Fixed-Wing RC Aircraft

<table>
  <tr>
    <td width="50%"><img src="https://github.com/TianMa7/RC_PLANE/raw/master/R%26D/Photos%20%26%20Videos/Full_Plane_WSS.png" width="100%"></td>
    <td width="50%"><img src="https://github.com/TianMa7/RC_PLANE/raw/master/R%26D/Photos%20%26%20Videos/solidworks360.gif" width="100%"></td>
  </tr>
</table>

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
<table>
  <tr>
    <td width="50%">
      <img src="https://github.com/TianMa7/RC_PLANE/raw/master/R%26D/Photos%20%26%20Videos/vorticity%20sweep.gif" width="100%">
      <br><em>Vorticity X Sweep</em>
    </td>
    <td width="50%">
      <img src="https://github.com/TianMa7/RC_PLANE/raw/master/R%26D/Photos%20%26%20Videos/velocity%20sweep.gif" width="100%">
      <br><em>Velocity Y Sweep</em>
    </td>
  </tr>
</table>

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

### 📷 Media Gallery

<table>
  <tr>
    <td width="33%"><img src="R&D/Photos%20&%20Videos/Andrew_Plane.jpeg" width="100%" alt="Andrew Plane"><br><em>Andrew Plane</em></td>
    <td width="33%"><img src="R&D/Photos%20&%20Videos/Crashed_Plane.JPG" width="100%" alt="Crashed Plane"><br><em>Crashed Plane</em></td>
    <td width="33%"><img src="R&D/Photos%20&%20Videos/Finished_Plane.jpeg" width="100%" alt="Finished Plane"><br><em>Finished Plane</em></td>
  </tr>
  <tr>
    <td width="33%"><img src="R&D/Photos%20&%20Videos/Group_Plane.jpeg" width="100%" alt="Group Plane"><br><em>Group Plane</em></td>
    <td width="33%"><img src="R&D/Photos%20&%20Videos/Hunter_Plane.jpeg" width="100%" alt="Hunter Plane"><br><em>Hunter Plane</em></td>
    <td width="33%"><img src="R&D/Photos%20&%20Videos/ProtoBoard.jpeg" width="100%" alt="Protoboard"><br><em>Protoboard</em></td>
  </tr>
  <tr>
    <td width="33%"><img src="R&D/Photos%20&%20Videos/Servo_Mount.jpeg" width="100%" alt="Servo Mount"><br><em>Servo Mount</em></td>
    <td width="33%"><img src="R&D/Photos%20&%20Videos/Tian_Plane.jpeg" width="100%" alt="Tian Plane"><br><em>Tian Plane</em></td>
    <td width="33%">
      <img src="https://github.com/TianMa7/RC_PLANE/raw/master/R%26D/Photos%20%26%20Videos/control%20surface%20test.gif" width="100%">
      <br><em>Ailerons Test</em>
    </td>
  </tr>
  <tr>
    <td width="33%">
      <img src="https://github.com/TianMa7/RC_PLANE/raw/master/R%26D/Photos%20%26%20Videos/pid%20test.gif" width="100%">
      <br><em>Auto Balancing Test</em>
    </td>
    <td width="33%">
      <img src="https://github.com/TianMa7/RC_PLANE/raw/master/R%26D/Photos%20%26%20Videos/flight%20test.gif" width="100%">
      <br><em>Flight Test</em>
    </td>
    <td width="33%">
      <img src="https://github.com/TianMa7/RC_PLANE/raw/master/R%26D/Photos%20%26%20Videos/360%20overview.gif" width="100%">
      <br><em>360 Overview</em>
    </td>
  </tr>
</table>

---
*Developed by Tian Ma, Hunter Liu, Andrew Poon*
