#pragma once

#include <cstdint>
#include <cstddef>
#include "esp_err.h"
#include "driver/i2c_master.h"

/* =============================================================
 * I2C ADDRESS
 *   SA0 pin LOW  -> 0x6A
 *   SA0 pin HIGH -> 0x6B
 * ============================================================= */
constexpr uint8_t LSM6DS3_ADDR_SA0_LOW  = 0x6A;
constexpr uint8_t LSM6DS3_ADDR_SA0_HIGH = 0x6B;

/* =============================================================
 * IDENTITY
 * ============================================================= */
constexpr uint8_t LSM6DS3_WHO_AM_I_REG   = 0x0F;
constexpr uint8_t LSM6DS3_WHO_AM_I_VALUE = 0x69;

/* =============================================================
 * CONTROL REGISTERS
 * ============================================================= */
constexpr uint8_t LSM6DS3_CTRL1_XL = 0x10; // Accel
constexpr uint8_t LSM6DS3_CTRL2_G  = 0x11; // Gyro
constexpr uint8_t LSM6DS3_CTRL3_C  = 0x12; // Common (reset/BDU)
constexpr uint8_t LSM6DS3_CTRL4_C  = 0x13; // Common

// Internal gyro filtering: LPF1 selection on gyro output path.
constexpr uint8_t LSM6DS3_CTRL4_C_CONFIG = 0x02;

/* =============================================================
 * CONTROL REGISTER CONFIG VALUES
 *
 * CTRL1_XL  (accel):
 *   [7:4] ODR_XL = 0110 -> 416 Hz  (closest to 400)
 *   [3:2] FS_XL  = 10   -> +/-4 g
 *   [1:0] BW_XL  = 10   -> ~100 Hz anti-alias bandwidth
 *   => 0110 1010 = 0x6A
 *
 * CTRL2_G  (gyro):
 *   [7:4] ODR_G  = 0110 -> 416 Hz
 *   [3:2] FS_G   = 01   -> 500 dps
 *   [1]   FS_125 = 0
 *   [0]          = 0
 *   => 0110 0100 = 0x64
 *
 * CTRL4_C:
 *   LPF1_SEL_G = 1 -> enable internal gyro LPF1 path
 *   => 0000 0010 = 0x02
 * ============================================================= */
constexpr uint8_t LSM6DS3_CTRL1_XL_CONFIG = 0x6A;
constexpr uint8_t LSM6DS3_CTRL2_G_CONFIG  = 0x64;
constexpr uint8_t LSM6DS3_CTRL3_C_CONFIG = 0x44; // BDU + IF_INC

/* =============================================================
 * OUTPUT REGISTERS  (gyro 0x22-0x27, accel 0x28-0x2D)
 * They are consecutive — one 12-byte burst read covers both.
 * ============================================================= */
constexpr uint8_t LSM6DS3_OUTX_L_G  = 0x22;
constexpr uint8_t LSM6DS3_OUTX_H_G  = 0x23;
constexpr uint8_t LSM6DS3_OUTY_L_G  = 0x24;
constexpr uint8_t LSM6DS3_OUTY_H_G  = 0x25;
constexpr uint8_t LSM6DS3_OUTZ_L_G  = 0x26;
constexpr uint8_t LSM6DS3_OUTZ_H_G  = 0x27;

constexpr uint8_t LSM6DS3_OUTX_L_XL = 0x28;
constexpr uint8_t LSM6DS3_OUTX_H_XL = 0x29;
constexpr uint8_t LSM6DS3_OUTY_L_XL = 0x2A;
constexpr uint8_t LSM6DS3_OUTY_H_XL = 0x2B;
constexpr uint8_t LSM6DS3_OUTZ_L_XL = 0x2C;
constexpr uint8_t LSM6DS3_OUTZ_H_XL = 0x2D;

/* =============================================================
 * SCALE FACTORS  (raw int16 -> physical units)
 *
 * Accel +/-4 g : 0.122 mg/LSB  = 0.000122  g/LSB
 *   multiply by GRAVITY (9.80665) to get m/s²
 *
 * Gyro  500 dps: 17.50 mdps/LSB = 0.01750  dps/LSB
 * ============================================================= */
constexpr float LSM6DS3_ACCEL_SCALE = 0.000122f; // g/LSB
constexpr float LSM6DS3_GRAVITY     = 9.80665f;  // m/s² per g
constexpr float LSM6DS3_GYRO_SCALE  = 0.01750f;  // dps/LSB

/* =============================================================
 * I2C TIMING / BUFFER
 * ============================================================= */
constexpr int    LSM6DS3_I2C_TIMEOUT_MS = 10;
constexpr size_t LSM6DS3_BUFFER_SIZE    = 12;

/* =============================================================
 * DATA STRUCTURES
 * ============================================================= */

/** Raw 16-bit signed sensor output (burst-read order) */
struct LSM6DS3_RawData
{
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
};

/**
 * Physical-unit sensor output
 *   accel : m/s²  (+/-4 g range)
 *   gyro  : dps   (500 dps range)
 */
struct LSM6DS3_ScaledData
{
    float accel_x;
    float accel_y;
    float accel_z;
    float gyro_x;
    float gyro_y;
    float gyro_z;
};

/* =============================================================
 * DRIVER TYPES
 * ============================================================= */

/** Opaque handle — created by lsm6ds3_init() */
typedef struct lsm6ds3_t *lsm6ds3_handle_t;

/** Config passed to lsm6ds3_init() */
typedef struct {
    i2c_master_bus_handle_t i2c_bus_handle;
    uint32_t  i2c_freq_hz;     /*!< typically 400000    */
    uint8_t   device_address;  /*!< 0x6A or 0x6B       */
} lsm6ds3_config_t;

/* =============================================================
 * PUBLIC API
 * ============================================================= */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Init LSM6DS3 over I2C.
 *
 * Software-resets the chip, verifies WHO_AM_I, then
 * configures accel (416 Hz, +/-4 g) and gyro (416 Hz,
 * 500 dps).
 *
 * @param config  Filled config struct.
 * @param handle  Receives the new handle.
 * @return ESP_OK on success.
 */
esp_err_t lsm6ds3_init(
    const lsm6ds3_config_t *config,
    lsm6ds3_handle_t       *handle);

/**
 * @brief  Burst-read raw 16-bit data (gyro + accel).
 *
 * @param handle  Handle from lsm6ds3_init().
 * @param data    Output structure.
 * @return ESP_OK on success.
 */
esp_err_t lsm6ds3_read_raw(
    lsm6ds3_handle_t  handle,
    LSM6DS3_RawData   *data);

/**
 * @brief  Read data converted to physical units.
 *         accel -> m/s²,  gyro -> dps.
 *
 * @param handle  Handle from lsm6ds3_init().
 * @param data    Output structure.
 * @return ESP_OK on success.
 */
esp_err_t lsm6ds3_read_scaled(
    lsm6ds3_handle_t    handle,
    LSM6DS3_ScaledData  *data);

/**
 * @brief  Free all resources.  Handle is invalid after.
 *
 * @param handle  Handle from lsm6ds3_init().
 * @return ESP_OK on success.
 */
esp_err_t lsm6ds3_deinit(lsm6ds3_handle_t handle);

#ifdef __cplusplus
}
#endif
