#include "controlTasks.h"
SemaphoreHandle_t CONTROLTASKS::FlightDataMutex = nullptr;
FlightData CONTROLTASKS::planeData = {};
MahonyFilter* CONTROLTASKS::mahony = nullptr;
lsm6ds3_handle_t CONTROLTASKS::imu_handle = nullptr;
PID* CONTROLTASKS::aileronPID = nullptr;
PID* CONTROLTASKS::elevatorPID = nullptr;

CONTROLTASKS::CONTROLTASKS(lsm6ds3_handle_t &imu)
{
        /* Create mutex to lock flight data */
        FlightDataMutex = xSemaphoreCreateMutex();
        if (FlightDataMutex == nullptr)
        {
                ESP_LOGE(TAG, "Failed to create mutex: flightDataMutex");
                return;
        }

        /* init imu and mahony filter */
        imu_handle = imu;

        // Baseline Mahony gains from loop rate for fixed-wing bench bring-up.
        // Formula: kp ~= 2 / settle_time, ki ~= kp / loop_hz.
        const float loop_hz = 1000.0f / planeData.flightControlTaskPeriod_ms;
        const float settle_time_s = 1.2f;
        const float mahony_kp = std::clamp(2.0f / settle_time_s, 0.8f, 2.5f);
        const float mahony_ki = std::clamp(mahony_kp / loop_hz, 0.001f, 0.05f);

        mahony = new MahonyFilter(mahony_kp, mahony_ki);

        if (imu_handle == nullptr || mahony == nullptr)
        {
                ESP_LOGE(TAG, "Failed to create MahonyFilter");
                return;
        }

        ESP_LOGI(TAG, "Mahony gains: Kp=%.3f Ki=%.3f (loop %.1f Hz)",
                 mahony_kp, mahony_ki, loop_hz);

        /* end imu and manhony filter init */
}

CONTROLTASKS::~CONTROLTASKS()
{
        vSemaphoreDelete(FlightDataMutex);
        FlightDataMutex = nullptr;
}

//maps joystick movement to desired output (pwm signal to restricted angle)
inline void CONTROLTASKS::joystickMap(uint16_t control_in, float max_up, float max_down, float& angle)
{
        const int midpoint = 1500; //PWM signal midpoint
        const int deadband = 50; //deadband to prevent jittering

        float inputDiff = midpoint - control_in;
        if (abs(inputDiff) < deadband)
        {
                // Joystick is in the deadband, treat as neutral
                angle = 0;
        }
        else
        {
                // Joystick is pushed down
                angle = inputDiff / 500.0f * max_up; // Scale to max_up_up
        }
}

/* IBUS channel config */
void CONTROLTASKS::channel_handler(ibus_channel_t *channels, void *cookie)
{
        //check if mutex can be taken
        if (xSemaphoreTake(FlightDataMutex, 0) == pdTRUE)
        {
                //updates joystick inputs for pitch and roll
                joystickMap(channels[0].value, 30, 30, planeData.rollTarget);
                joystickMap(channels[1].value, 30, 30, planeData.pitchTarget);

                // joystickMap(channels[4].value, 180, 0, planeData.aileronTrim);
                // joystickMap(channels[5].value, 180, 0, planeData.elevatorTrim);

                /* ADD CONNECTIONS TO CHANNEL 5-6 AS WELL LATER FOR AUX USE */
                planeData.last_ibus_timestamp_ms = pdTICKS_TO_MS(xTaskGetTickCount());
                planeData.is_ibus_failsafe_active = false;

                //return mutex
                xSemaphoreGive(FlightDataMutex);
        }
}

//freertos task for flight control
void CONTROLTASKS::flightControlTask(void* pvParameters)
{
        //init all flight control specific tasks
        TickType_t lastWake = xTaskGetTickCount();

        //init aileron and elevator pid controls
        if (xSemaphoreTake(FlightDataMutex, pdMS_TO_TICKS(1000)) == pdFALSE)
        {
                ESP_LOGE(TAG, "Failed to take mutex for PID setup");
                return;
        }
        aileronPID = new PID(planeData.flightControlTaskPeriod_s,
                             planeData.aileronUpTravel,
                             -planeData.aileronDownTravel,
                             planeData.KpAileron, planeData.KiAileron,
                             planeData.KdAileron, 0.0f);
        elevatorPID = new PID(planeData.flightControlTaskPeriod_s,
                              planeData.elevatorUpTravel,
                              -planeData.elevatorDownTravel,
                              planeData.KpElevator, planeData.KiElevator,
                              planeData.KdElevator, 0.0f);
        float period = planeData.flightControlTaskPeriod_ms;

        xSemaphoreGive(FlightDataMutex);

        if (aileronPID == nullptr || elevatorPID == nullptr)
        {
                ESP_LOGE(TAG, "Failed to create PID objects");
                return;
        }
        // //end pid init

        //main control loop
        while (true)
        {
                IBUSfailsafe();
                readIMU();
                moveServos();

                vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(period));
        }
}

inline void CONTROLTASKS::IBUSfailsafe()
{
        uint32_t now = pdTICKS_TO_MS(xTaskGetTickCount());

        if (xSemaphoreTake(FlightDataMutex, 0) == pdTRUE)
        {
                if (now - planeData.last_ibus_timestamp_ms > planeData.failsafeTimeout_ms)
                {
                        planeData.pitchTarget = 0;
                        planeData.rollTarget = 0;
                        planeData.is_ibus_failsafe_active = true;
                }
                xSemaphoreGive(FlightDataMutex);
        }
}

inline void CONTROLTASKS::readIMU()
{
        const uint32_t now = pdTICKS_TO_MS(xTaskGetTickCount());

        LSM6DS3_ScaledData scaledData = {};

        esp_err_t retIMU = lsm6ds3_read_scaled(imu_handle, &scaledData);
        if (xSemaphoreTake(FlightDataMutex, 0) == pdTRUE)
        {
                if (retIMU == ESP_OK)//imu read successful
                {

                        planeData.is_imu_failsafe_active = false;
                        planeData.last_imu_timestamp_ms = now;
                        float period_s = planeData.flightControlTaskPeriod_s;

                        xSemaphoreGive(FlightDataMutex);

                        //remaps accel and gyro data to match plane's orientation and convert gyro to radians/s
                        const float ax = -scaledData.accel_x;
                        const float ay = -scaledData.accel_y;
                        const float az = scaledData.accel_z;
                        const float gx = -scaledData.gyro_x * DEG_TO_RAD;
                        const float gy = -scaledData.gyro_y * DEG_TO_RAD;
                        const float gz = scaledData.gyro_z * DEG_TO_RAD;

                        //runs mahony filter on data
                        mahony->update(
                        ax, ay, az,
                        gx, gy, gz,
                        period_s);
                        //finds euler angles
                        MahonyEuler euler = mahony->getEuler();

                        // writes euler angles to struct
                        if (xSemaphoreTake(FlightDataMutex, 0) == pdTRUE)
                        {
                                planeData.pitchCurrent = euler.pitch;
                                planeData.rollCurrent = euler.roll;
                                xSemaphoreGive(FlightDataMutex);
                        }
                }// imu determined to be off
                else if (now - planeData.last_imu_timestamp_ms > planeData.failsafeTimeout_ms)
                {
                        planeData.is_imu_failsafe_active = true;
                        planeData.cmdAileron = planeData.aileronTrim;
                        planeData.cmdElevator = planeData.elevatorTrim;

                        xSemaphoreGive(FlightDataMutex);
                }// imu read failed but failsafe not yet active
                else
                {
                        xSemaphoreGive(FlightDataMutex);
                }
        }
}

inline void CONTROLTASKS::moveServos()
{
        if (xSemaphoreTake(FlightDataMutex, 0) == pdTRUE)
        {
                if (planeData.is_imu_failsafe_active|| !aileronPID || !elevatorPID)
                {
                        planeData.cmdAileron = planeData.aileronTrim;
                        planeData.cmdElevator = planeData.elevatorTrim;
                }
                else
                {
                        if (planeData.is_ibus_failsafe_active)
                        {
                                // Hold level with stabilization when link is lost.
                                planeData.rollTarget = 0.0f;
                                planeData.pitchTarget = 0.0f;
                        }

                        const float rawAileron = planeData.aileronTrim +
                                aileronPID->calculate(planeData.rollTarget,
                                                      planeData.rollCurrent);
                        const float rawElevator = planeData.elevatorTrim +
                                elevatorPID->calculate(planeData.pitchTarget,
                                                       planeData.pitchCurrent);

                        const float aileronMin = planeData.aileronTrim -
                                planeData.aileronDownTravel;
                        const float aileronMax = planeData.aileronTrim +
                                planeData.aileronUpTravel;
                        const float elevatorMin = planeData.elevatorTrim -
                                planeData.elevatorDownTravel;
                        const float elevatorMax = planeData.elevatorTrim +
                                planeData.elevatorUpTravel;

                        planeData.cmdAileron = std::clamp(rawAileron,
                                                          aileronMin,
                                                          aileronMax);
                        planeData.cmdElevator = std::clamp(rawElevator,
                                                           elevatorMin,
                                                           elevatorMax);
                }

                float cmdAileron = planeData.cmdAileron;
                float cmdElevator = planeData.cmdElevator;

                xSemaphoreGive(FlightDataMutex);

                iot_servo_write_angle(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, cmdAileron);
                iot_servo_write_angle(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, cmdElevator);

        }
}

void CONTROLTASKS::logTask(void *pvParameters)
{
        TickType_t lastWake = xTaskGetTickCount();
        const TickType_t period = pdMS_TO_TICKS(100); // 5 Hz logs

        while (true)
        {
                // Stack watermark is the minimum free stack seen so far.
                const UBaseType_t free_words = uxTaskGetStackHighWaterMark(nullptr);
                const uint32_t free_bytes = static_cast<uint32_t>(free_words) *
                                            sizeof(StackType_t);

                FlightData snapshot = {};
                bool locked = (xSemaphoreTake(FlightDataMutex, pdMS_TO_TICKS(2))
                               == pdTRUE);

                if (locked)
                {
                        snapshot = planeData;
                        xSemaphoreGive(FlightDataMutex);
                }

                if (locked)
                {
                        const uint32_t now_ms = pdTICKS_TO_MS(xTaskGetTickCount());
                        const uint32_t ibus_age = now_ms - snapshot.last_ibus_timestamp_ms;
                        const uint32_t imu_age = now_ms - snapshot.last_imu_timestamp_ms;

                        ESP_LOGI(
                                TAG,
                                "P=%6.2f R=%6.2f Pt=%6.2f Rt=%6.2f "
                                "A=%6.2f E=%6.2f Atr=%5.1f Etr=%5.1f "
                                "FS(i:%d m:%d) Age(i:%u m:%u) Stk:%uW/%uB",
                                snapshot.pitchCurrent, snapshot.rollCurrent,
                                snapshot.pitchTarget, snapshot.rollTarget,
                                snapshot.cmdAileron, snapshot.cmdElevator,
                                snapshot.aileronTrim, snapshot.elevatorTrim,
                                snapshot.is_ibus_failsafe_active,
                                snapshot.is_imu_failsafe_active,
                                static_cast<unsigned int>(ibus_age),
                                static_cast<unsigned int>(imu_age),
                                static_cast<unsigned int>(free_words),
                                static_cast<unsigned int>(free_bytes));
                }
                else
                {
                        ESP_LOGW(TAG, "logTask: mutex timeout");
                }

                vTaskDelayUntil(&lastWake, period);
        }
}


