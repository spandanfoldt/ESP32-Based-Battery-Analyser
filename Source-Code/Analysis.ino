
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Preferences.h>
#include <string.h>

// =====================================================
// PIN CONFIGURATION
// =====================================================

#define SDA_PIN       21
#define SCL_PIN       22
#define TEMP_PIN      4
#define MOSFET_PIN    25

// =====================================================
// BATTERY CONFIGURATION
// =====================================================

const float RATED_CAPACITY_MAH = 2000.0;


const float STOP_VOLTAGE_V = 3.50;

const float MAX_TEMPERATURE_C = 45.0;
const float CURRENT_DEADBAND_MA = 0.5;

const unsigned long SAMPLE_INTERVAL_MS = 1000;

const bool FULLY_CHARGED_BEFORE_TEST = false;
const bool CUTOFF_CONFIRMED = false;

// =====================================================
// OBJECTS
// =====================================================

Adafruit_INA219 ina219;
OneWire oneWire(TEMP_PIN);
DallasTemperature sensors(&oneWire);
Preferences preferences;

// =====================================================
// MEASUREMENTS
// =====================================================

float batteryVoltage = 0.0;
float current_mA = 0.0;
float temperatureC = 0.0;

bool temperatureValid = false;

// =====================================================
// CAPACITY VARIABLES
// =====================================================

float dischargedCapacity_mAh = 0.0;

float previousCurrent_mA = 0.0;

unsigned long previousTime = 0;
unsigned long lastSampleTime = 0;

// =====================================================
// STORED BATTERY HEALTH
// =====================================================



float lastValidSOH = -1.0;
float lastMeasuredCapacity = -1.0;

// =====================================================
// TEST CONTROL
// =====================================================

bool testRunning = false;

// =====================================================
// READ SENSORS
// =====================================================

void readSensors() {

  float busVoltage = ina219.getBusVoltage_V();
  float shuntVoltage = ina219.getShuntVoltage_mV();

  batteryVoltage =
      busVoltage + (shuntVoltage / 1000.0);

  current_mA = ina219.getCurrent_mA();

  sensors.requestTemperatures();

  temperatureC = sensors.getTempCByIndex(0);

  temperatureValid =
      (temperatureC != DEVICE_DISCONNECTED_C &&
       temperatureC > -55.0 &&
       temperatureC < 125.0);
}

// =====================================================
// CAPACITY INTEGRATION
// =====================================================

void updateCapacity() {

  unsigned long now = millis();

  if (previousTime == 0) {

    previousTime = now;
    previousCurrent_mA = current_mA;

    return;
  }

  float elapsedHours =
      (now - previousTime) / 3600000.0;

  float currentNow =
      (current_mA > CURRENT_DEADBAND_MA)
      ? current_mA : 0.0;

  float currentPrevious =
      (previousCurrent_mA > CURRENT_DEADBAND_MA)
      ? previousCurrent_mA : 0.0;

  float averageCurrent =
      (currentNow + currentPrevious) / 2.0;

  if (testRunning) {

    dischargedCapacity_mAh +=
        averageCurrent * elapsedHours;
  }

  previousCurrent_mA = current_mA;
  previousTime = now;
}

// =====================================================
// START TEST
// =====================================================

void startTest() {

  if (testRunning) {
    Serial.println("Test already running.");
    return;
  }

  readSensors();

  if (!temperatureValid) {
    Serial.println("ERROR: Temperature sensor unavailable.");
    return;
  }

  if (batteryVoltage <= STOP_VOLTAGE_V) {
    Serial.println("ERROR: Battery voltage too low.");
    return;
  }

  if (temperatureC >= MAX_TEMPERATURE_C) {
    Serial.println("ERROR: Temperature too high.");
    return;
  }

  previousTime = 0;
  previousCurrent_mA = 0.0;

  digitalWrite(MOSFET_PIN, HIGH);

  testRunning = true;

  Serial.println("TEST STARTED");
}

// =====================================================
// STOP TEST
// =====================================================

void stopTest(const char *reason) {

  if (!testRunning) {
    return;
  }

  updateCapacity();

  digitalWrite(MOSFET_PIN, LOW);

  testRunning = false;

  Serial.print("TEST STOPPED: ");
  Serial.println(reason);

  // Save SOH only after a verified full capacity test.
  if (FULLY_CHARGED_BEFORE_TEST &&
      CUTOFF_CONFIRMED &&
      strcmp(reason, "VERIFIED CUTOFF") == 0) {

    lastMeasuredCapacity = dischargedCapacity_mAh;

    lastValidSOH =
        (lastMeasuredCapacity / RATED_CAPACITY_MAH) * 100.0;

    lastValidSOH = constrain(lastValidSOH, 0.0, 100.0);

    preferences.putFloat("soh", lastValidSOH);
    preferences.putFloat("capacity", lastMeasuredCapacity);

    Serial.println("VALID SOH SAVED");
  }
}

// =====================================================
// RESET SESSION
// =====================================================

void resetTest() {

  if (testRunning) {
    stopTest("MANUAL RESET");
  }

  dischargedCapacity_mAh = 0.0;

  previousTime = 0;
  previousCurrent_mA = 0.0;

  Serial.println("SESSION CAPACITY RESET");
}

// =====================================================
// SAFETY CHECKS
// =====================================================

void checkSafety() {

  if (!testRunning) {
    return;
  }

  if (batteryVoltage <= STOP_VOLTAGE_V) {
    stopTest("PROVISIONAL VOLTAGE LIMIT");
    return;
  }

  if (!temperatureValid) {
    stopTest("TEMPERATURE SENSOR ERROR");
    return;
  }

  if (temperatureC >= MAX_TEMPERATURE_C) {
    stopTest("TEMPERATURE LIMIT");
    return;
  }
}

// =====================================================
// SERIAL PLOTTER OUTPUT
// =====================================================

void printMeasurements() {

  // Discharged percentage
  float dischargedPercent =
      (dischargedCapacity_mAh / RATED_CAPACITY_MAH) * 100.0;

  dischargedPercent =
      constrain(dischargedPercent, 0.0, 100.0);

  // Display last validated SOH, or provisional 100%.
  float soh = (lastValidSOH >= 0.0)
              ? lastValidSOH
              : 100.0;


  float referenceCapacity =
      (lastMeasuredCapacity >= 0.0)
      ? lastMeasuredCapacity
      : RATED_CAPACITY_MAH;

  float remainingCapacity_mAh =
      referenceCapacity - dischargedCapacity_mAh;

  remainingCapacity_mAh =
      constrain(remainingCapacity_mAh, 0.0, referenceCapacity);

  float remainingPercent =
      (remainingCapacity_mAh / referenceCapacity) * 100.0;

  // Output
  Serial.print("Voltage:");
  Serial.print(batteryVoltage, 3);

  Serial.print("\tCurrent:");
  Serial.print(current_mA, 2);

  Serial.print("\tTemperature:");
  Serial.print(temperatureValid ? temperatureC : -1.0, 2);

  Serial.print("\tCapacity:");
  Serial.print(dischargedCapacity_mAh, 3);

  Serial.print("\tDischarged_Percent:");
  Serial.print(dischargedPercent, 2);

  Serial.print("\tSOH:");
  Serial.print(soh, 2);

  Serial.print("\tRemaining_Capacity:");
  Serial.print(remainingCapacity_mAh, 2);

  Serial.print("\tRemaining_Percent:");
  Serial.println(remainingPercent, 2);
}

// =====================================================
// SERIAL COMMANDS
// =====================================================

void processCommands() {

  while (Serial.available()) {

    char command = Serial.read();

    if (command == 'S' || command == 's') {
      startTest();
    }

    else if (command == 'X' || command == 'x') {
      stopTest("MANUAL STOP");
    }

    else if (command == 'R' || command == 'r') {
      resetTest();
    }
  }
}

// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);

  pinMode(MOSFET_PIN, OUTPUT);
  digitalWrite(MOSFET_PIN, LOW);

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!ina219.begin()) {

    Serial.println("ERROR: INA219 NOT FOUND");

    while (1) {
      digitalWrite(MOSFET_PIN, LOW);
      delay(1000);
    }
  }

  ina219.setCalibration_32V_2A();

  sensors.begin();

  preferences.begin("battery", false);

  lastValidSOH =
      preferences.getFloat("soh", -1.0);

  lastMeasuredCapacity =
      preferences.getFloat("capacity", -1.0);

  readSensors();

  Serial.println();
  Serial.println("=================================");
  Serial.println("     BATTERY HEALTH ANALYSER");
  Serial.println("=================================");

  Serial.println("INA219 initialized");
  Serial.println("DS18B20 initialized");

  Serial.println("S = Start discharge");
  Serial.println("X = Stop discharge");
  Serial.println("R = Reset session");

  Serial.println("=================================");
}

// =====================================================
// MAIN LOOP
// =====================================================

void loop() {

  processCommands();

  unsigned long now = millis();

  if (now - lastSampleTime >= SAMPLE_INTERVAL_MS) {

    lastSampleTime = now;

    readSensors();

    if (testRunning) {

      updateCapacity();

      checkSafety();
    }

    printMeasurements();
  }
}
