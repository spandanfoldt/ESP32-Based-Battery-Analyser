# ESP32-Based LiPo Battery Analyser

An ESP32-based battery monitoring system designed to analyse the performance and health of a rechargeable Lithium-Polymer (LiPo) battery by measuring voltage, current, and temperature.

## 1. Project Overview

This project implements a battery analyser for a **3.7V, 2000mAh LiPo battery**. It uses an ESP32 microcontroller to acquire electrical and thermal measurements and estimate the battery's discharge characteristics.

The system monitors:

- Battery voltage (V)
- Discharge current (mA)
- Battery temperature (°C)
- Discharged capacity (mAh)
- Discharged percentage (%)
- Remaining capacity (mAh)
- State of Health (SOH) (%)

An INA219 sensor is used for voltage and current measurement, while a DS18B20 temperature sensor monitors the battery temperature. A MOSFET-controlled resistive load enables controlled battery discharge through a 100Ω resistor.

## 2. Project Objectives

- Measure the voltage and current of a LiPo battery.
- Monitor battery temperature during discharge.
- Calculate discharged capacity using current integration.
- Estimate the remaining battery capacity.
- Evaluate battery State of Health (SOH).
- Implement a microcontroller-based battery monitoring system.

## 3. Hardware Components

| Component | Purpose |
|---|---|
| ESP32 DevKit | Main controller and data processing |
| INA219 | Voltage and current measurement |
| DS18B20 | Temperature measurement |
| 3.7V, 2000mAh LiPo battery | Battery under test |
| N-channel MOSFET | Electronic load switching |
| 100Ω resistor | Resistive discharge load |
| 100Ω resistor | MOSFET gate resistor |
| 10kΩ resistor | MOSFET gate pull-down |
| 4.7kΩ resistor | DS18B20 data pull-up |

## 4. Software and Libraries

- Arduino IDE
- C++ (Arduino Framework)
- Adafruit INA219 Library
- OneWire Library
- DallasTemperature Library

## 5. Circuit Connections

### A. ESP32 – INA219

| INA219 Pin | ESP32 Connection |
|---|---|
| VCC | 5V |
| GND | GND |
| SDA | GPIO 21 |
| SCL | GPIO 22 |

The battery positive terminal is connected to INA219 VIN+, while VIN− connects to the discharge load.

### B. ESP32 – DS18B20

| DS18B20 Pin | Connection |
|---|---|
| VDD | 5V |
| GND | GND |
| DATA | GPIO 4 |

A 4.7kΩ pull-up resistor is connected between DATA and 3.3V.

### C. MOSFET-Controlled Discharge Circuit

| Connection | Wiring |
|---|---|
| ESP32 GPIO 25 | MOSFET Gate through 100Ω |
| MOSFET Gate | GND through 10kΩ |
| MOSFET Drain | Load resistor |
| MOSFET Source | Battery negative |
| Load resistor | INA219 VIN− to MOSFET Drain |

The battery negative terminal and ESP32 GND share a common reference.

## 6. Working Principle

The system operates through the following stages:

1. **Voltage Measurement:** The INA219 measures the battery voltage through its high-side voltage sensing arrangement.

2. **Current Measurement:** The INA219 measures the current flowing through the discharge circuit.

3. **Temperature Monitoring:** The DS18B20 continuously monitors the battery temperature.

4. **Controlled Discharge:** The ESP32 controls the MOSFET to switch the 100Ω resistive load ON or OFF.

5. **Capacity Calculation:** The discharged capacity is calculated by integrating the measured current over time.

6. **Battery Health Estimation:** The system uses the calculated discharge capacity to estimate remaining capacity and State of Health.

## 7. Mathematical Calculations

### A. Discharged Capacity

The discharged capacity is calculated using current integration:

\[
Q_{\text{discharged}} =
\frac{1}{3600}\int I(t)\,dt
\]

For discrete measurements:

\[
Q_{\text{discharged}} =
\sum_i \frac{I_i \times \Delta t_i}{3600}
\]

Where:

- \(I_i\) = Measured current in mA
- \(\Delta t_i\) = Time interval in seconds
- \(Q\) = Capacity in mAh

### B. Remaining Capacity

\[
Q_{\text{remaining}} =
Q_{\text{rated}}-Q_{\text{discharged}}
\]

For the 2000mAh battery:

\[
Q_{\text{remaining}} =
2000-Q_{\text{discharged}}
\]

### C. Discharged Percentage

\[
\text{Discharged Percentage} =
\frac{Q_{\text{discharged}}}{Q_{\text{rated}}}
\times 100
\]

### D. State of Health (SOH)

\[
SOH =
\frac{Q_{\text{measured full discharge}}}
{Q_{\text{rated}}}\times 100
\]

SOH is determined by comparing the actual measured capacity from a complete controlled discharge cycle with the rated battery capacity.

A partial discharge measurement alone cannot establish the actual SOH of the battery.

## 8. Output Parameters

The system displays the following parameters through the Arduino Serial Monitor:

| Parameter | Unit |
|---|---|
| Battery Voltage | V |
| Discharge Current | mA |
| Temperature | °C |
| Discharged Capacity | mAh |
| Discharged Percentage | % |
| Remaining Capacity | mAh |
| State of Health | % |

## 9. Applications

- Rechargeable battery performance monitoring
- LiPo battery testing and analysis
- Embedded battery monitoring systems
- Battery capacity estimation
- Educational experiments in electrical measurements and instrumentation

## 10. Future Improvements

- OLED display for real-time monitoring.
- Data logging for discharge characteristics.
- Battery voltage versus time plotting.
- Automatic cutoff at the battery's specified minimum voltage.
- Improved SOH estimation using repeated controlled charge-discharge cycles.
- Wireless monitoring through ESP32 Wi-Fi or Bluetooth.

## 11. Safety Considerations

- Avoid discharging the LiPo battery below its manufacturer's specified minimum voltage.
- Monitor battery temperature throughout the discharge process.
- Use a suitable protection circuit and a logic-level MOSFET.
- Ensure the 100Ω load resistor has an adequate power rating.
- Never leave a LiPo battery undergoing discharge unattended.
- Verify all connections before powering the circuit.

## 12. Conclusion

This project demonstrates the implementation of an ESP32-based LiPo battery analyser using electrical and temperature measurements. By combining voltage sensing, current integration, and temperature monitoring, the system provides useful information about battery discharge behaviour, remaining capacity, and estimated State of Health.

---

**Course:** Measurement and Electronic Instrumentation (EE3L010 )

**Platform:** ESP32

**Battery:** 3.7V, 2000mAh LiPo

**Load:** 100Ω resistive load

**Sensors:** INA219 and DS18B20

**Programming Language:** C++ (Arduino Framework)
