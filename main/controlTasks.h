#pragma once
/* =============================================================
 * GENERAL HEADERS
 * ============================================================= */
#include <algorithm>
#include <cmath>

/* =============================================================
 * ESP32S3 SPECIFIC HEADERS
 * ============================================================= */
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <driver/uart.h>

/* =============================================================
 * SENSOR AND CONTROL HEADERS
 * ============================================================= */
#include "driver/i2c_master.h"
#include "lsm6ds3.h"
#include "mahony_filter.h"
#include "iot_servo.h"
#include "pid.h"

/* configurations */
#include "flightCfg.h"

extern "C"
{
#include "ibus/ibus.h"
}

/* --------- SHARED RESOURCES ---------- */
struct FlightData
{
 //Current Orientation - IMU
 float pitchCurrent = 0;
 float rollCurrent = 0;

 //Target Orientation - IBUS
 float pitchTarget = 0;
 float rollTarget = 0;

 // Conservative baseline PID gains for first flight tuning.
 float KpAileron = 1.500f;
 float KiAileron = 0.008f;
 float KdAileron = 0.060f;

 float KpElevator = 3.000f;
 float KiElevator = 0.010f;
 float KdElevator = 0.075f;

 //Trim tuning values
 float elevatorTrim = 90;
 float aileronTrim = 90;

 // Allowed servo travel around trim (servo-deg).
 float aileronUpTravel = 80;
 float aileronDownTravel = 80;
 float elevatorUpTravel = 80;
 float elevatorDownTravel = 80;

 //command values
 float cmdAileron = aileronTrim;
 float cmdElevator = elevatorTrim;

 //Failsafe flag
 uint32_t last_ibus_timestamp_ms = 0;
 uint32_t last_imu_timestamp_ms = 0;
 int failsafeTimeout_ms = 150;
 bool is_ibus_failsafe_active = true;
 bool is_imu_failsafe_active = true;

 //dT ----> WHAT DOES THIS DO??????
 float flightControlTaskPeriod_ms = 10;
 float flightControlTaskPeriod_s = flightControlTaskPeriod_ms / (1000.0f); //20ms loop period, convert to seconds for PID calculations
};//stores global flight-related data

class CONTROLTASKS
{
 public:
 CONTROLTASKS(lsm6ds3_handle_t &imu);
 ~CONTROLTASKS();

 static void flightControlTask(void * pvParameters);
 static void logTask(void * pvParameters);
 /* IBUS channel config */
 static void channel_handler(ibus_channel_t *channels, void *cookie);

 private:

 //helper functions
 static void joystickMap (uint16_t control_in, float max_up, float max_down, float &angle);
 static void IBUSfailsafe();
 static void readIMU();
 static void moveServos();

 //objects
 static SemaphoreHandle_t FlightDataMutex; //initialize a mutex to prevent multiple access to planeData
 static FlightData planeData; //initialize a struct containing data for plane
 static MahonyFilter *mahony; //mahony filter object
 static lsm6ds3_handle_t imu_handle;//handle for imu task
 static PID *aileronPID; //pid object for aileron control
 static PID *elevatorPID;//pid object for elevator control



 //move shit from the flightData Struct into here

};

