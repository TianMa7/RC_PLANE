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
#include "controlTasks.h"
#include "driver/i2c_master.h"
#include "lsm6ds3.h"
#include "mahony_filter.h"
#include "iot_servo.h"
#include "pid.h"


extern "C"
{
#include "ibus/ibus.h"
}

extern "C" void app_main()
{
    ESP_LOGI(TAG, "Starting RC Plane Application");


    /* ---- I2C master bus ---- */
    i2c_master_bus_config_t i2c_mst_config =
    {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        .trans_queue_depth = 0,
        .flags =
     {
            .enable_internal_pullup = true,
            .allow_pd = false
        },
    };

    i2c_master_bus_handle_t bus_handle;
    esp_err_t ret = i2c_new_master_bus(&i2c_mst_config, &bus_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "I2C bus failed: %s", esp_err_to_name(ret));
        while (true)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
    ESP_LOGI(TAG, "I2C bus OK");
    /* ---- end I2C master bus ---- */

    /* ---- LSM6DS3 sensor ---- */
    lsm6ds3_config_t imu_config =
    {
        .i2c_bus_handle = bus_handle,
        .i2c_freq_hz    = I2C_MASTER_FREQ_HZ,
        .device_address  = LSM6DS3_ADDR_SA0_HIGH,
    };

    lsm6ds3_handle_t imu_handle;
    ret = lsm6ds3_init(&imu_config, &imu_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "LSM6DS3 init failed: %s", esp_err_to_name(ret));
        i2c_del_master_bus(bus_handle);
        while (true)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
    ESP_LOGI(TAG, "LSM6DS3 OK");
    /* ---- end LSM6DS3 sensor ---- */



    /* ---- servo control ---- */
    /* Adjust frequency as needed */
    servo_config_t servo_config = {
        .max_angle = 180,
        .min_width_us = 500,
        .max_width_us = 2500,
        .freq = 50,
        .timer_number = LEDC_TIMER_0,
        .channels = {
            .servo_pin =
         {
                AILERON_SERVO_PIN,
                ELEVATOR_SERVO_PIN,
            },
            .ch =
         {
                LEDC_CHANNEL_0,
                LEDC_CHANNEL_1
            }
        },
        .channel_number = 2,
    };

    ret = iot_servo_init(LEDC_LOW_SPEED_MODE, &servo_config);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Servo init failed: %s", esp_err_to_name(ret));
        lsm6ds3_deinit(imu_handle);
        i2c_del_master_bus(bus_handle);
        while (true)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
    ESP_LOGI(TAG, "Servo OK");
    /* ---- end servo control ---- */

    /* ---- IBUS setup ---- */
    uart_lowlevel_config config;
    config.port = IBUS_UART;
    config.tx_pin = UART_PIN_NO_CHANGE; // Not transmitting, so leave TX unconfigured
    config.rx_pin = IBUS_RX_PIN;
    ibus_context_t cfx = ibus_init(&config);
    if (cfx == nullptr)
    {
        ESP_LOGE(TAG, "IBUS init failed");
        ibus_set_channel_handler(cfx, nullptr, nullptr); // Unset handler if init failed
        iot_servo_deinit(LEDC_LOW_SPEED_MODE);
        lsm6ds3_deinit(imu_handle);
        i2c_del_master_bus(bus_handle);
        while (true)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
    ESP_LOGI(TAG, "IBUS OK");
    /* ---- end IBUS setup ---- */

    static CONTROLTASKS controlTasks(imu_handle);
    ibus_set_channel_handler(cfx, CONTROLTASKS::channel_handler, nullptr);

    BaseType_t ok = xTaskCreate(
        CONTROLTASKS::flightControlTask, //function
        "flight_control_task", //task name
        4096, //allocated memory
        nullptr, //pvParameters
        10, //priority, higher is more important
        nullptr //task handler
    );

    if (ok != pdPASS)
    {
        ESP_LOGE(TAG, "failed to create flight control task");
        while (true)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    BaseType_t log_ok = xTaskCreate(
    CONTROLTASKS::logTask,
    "flight_log_task",
    4096,
    nullptr,
    3,
    nullptr
    );

    if (log_ok != pdPASS)
    {
        ESP_LOGE(TAG, "failed to create log task");
    }



}



