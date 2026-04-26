#include "lsm6ds3.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstring>
#include <cstdlib>

static const char *TAG = "LSM6DS3";

/* =============================================================
 * INTERNAL DRIVER STRUCTURE
 * ============================================================= */
struct lsm6ds3_t {
    i2c_master_dev_handle_t i2c_dev_handle;
};

/* =============================================================
 * LOW-LEVEL I2C HELPERS
 * ============================================================= */

/**
 * @brief Write a single byte to a register.
 */
static esp_err_t lsm6ds3_write_reg(
    lsm6ds3_handle_t handle,
    uint8_t reg,
    uint8_t value)
{
    if (handle == nullptr) return ESP_ERR_INVALID_ARG;

    auto *drv = reinterpret_cast<lsm6ds3_t *>(handle);
    uint8_t buf[2] = { reg, value };

    esp_err_t ret = i2c_master_transmit(
        drv->i2c_dev_handle, buf, sizeof(buf),
        LSM6DS3_I2C_TIMEOUT_MS);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "write reg 0x%02X failed: %s",
                 reg, esp_err_to_name(ret));
    }
    return ret;
}

/**
 * @brief Read one byte from a register.
 */
static esp_err_t lsm6ds3_read_reg(
    lsm6ds3_handle_t handle,
    uint8_t reg,
    uint8_t *out)
{
    if (handle == nullptr || out == nullptr)
        return ESP_ERR_INVALID_ARG;

    auto *drv = reinterpret_cast<lsm6ds3_t *>(handle);

    esp_err_t ret = i2c_master_transmit_receive(
        drv->i2c_dev_handle, &reg, 1, out, 1,
        LSM6DS3_I2C_TIMEOUT_MS);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "read reg 0x%02X failed: %s",
                 reg, esp_err_to_name(ret));
    }
    return ret;
}

/**
 * @brief Burst-read len bytes starting at reg.
 */
static esp_err_t lsm6ds3_read_regs(
    lsm6ds3_handle_t handle,
    uint8_t reg,
    uint8_t *buf,
    size_t len)
{
    if (handle == nullptr || buf == nullptr || len == 0)
        return ESP_ERR_INVALID_ARG;

    auto *drv = reinterpret_cast<lsm6ds3_t *>(handle);

    esp_err_t ret = i2c_master_transmit_receive(
        drv->i2c_dev_handle, &reg, 1, buf, len,
        LSM6DS3_I2C_TIMEOUT_MS);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "burst read at 0x%02X failed: %s",
                 reg, esp_err_to_name(ret));
    }
    return ret;
}

/* =============================================================
 * HELPER: combine two bytes into a signed 16-bit value
 * Casts to uint16_t before shifting to avoid UB.
 * ============================================================= */
static inline int16_t combine_bytes(uint8_t lo, uint8_t hi)
{
    return static_cast<int16_t>(
        (static_cast<uint16_t>(hi) << 8) | lo);
}

/* =============================================================
 * HELPER: cleanup on init failure
 * ============================================================= */
static void cleanup(lsm6ds3_t *drv)
{
    i2c_master_bus_rm_device(drv->i2c_dev_handle);
    free(drv);
}

/* =============================================================
 * PUBLIC: lsm6ds3_init
 * ============================================================= */
esp_err_t lsm6ds3_init(
    const lsm6ds3_config_t *config,
    lsm6ds3_handle_t       *handle)
{
    if (config == nullptr || handle == nullptr) {
        ESP_LOGE(TAG, "init: null argument");
        return ESP_ERR_INVALID_ARG;
    }

    /* ---- allocate driver struct ---- */
    auto *drv = static_cast<lsm6ds3_t *>(
        malloc(sizeof(lsm6ds3_t)));
    if (drv == nullptr) {
        ESP_LOGE(TAG, "init: out of memory");
        return ESP_ERR_NO_MEM;
    }

    /* ---- register I2C device on bus ---- */
    i2c_device_config_t dev_cfg = {};
    dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    dev_cfg.device_address  = config->device_address;
    dev_cfg.scl_speed_hz    = config->i2c_freq_hz;

    esp_err_t ret = i2c_master_bus_add_device(
        config->i2c_bus_handle,
        &dev_cfg,
        &drv->i2c_dev_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "add device failed: %s",
                 esp_err_to_name(ret));
        free(drv);
        return ret;
    }

    auto h = reinterpret_cast<lsm6ds3_handle_t>(drv);

    /* ---- software reset (CTRL3_C bit 0) ---- */
    ret = lsm6ds3_write_reg(h, LSM6DS3_CTRL3_C, 0x01);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "software reset failed");
        cleanup(drv);
        return ret;
    }
    vTaskDelay(pdMS_TO_TICKS(50)); // wait for reset

    /* ---- verify WHO_AM_I ---- */
    uint8_t who = 0;
    ret = lsm6ds3_read_reg(h, LSM6DS3_WHO_AM_I_REG, &who);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "read WHO_AM_I failed");
        cleanup(drv);
        return ret;
    }
    if (who != LSM6DS3_WHO_AM_I_VALUE) {
        ESP_LOGE(TAG, "WHO_AM_I mismatch: 0x%02X (expected 0x%02X)",
                 who, LSM6DS3_WHO_AM_I_VALUE);
        cleanup(drv);
        return ESP_ERR_NOT_FOUND;
    }
    ESP_LOGI(TAG, "WHO_AM_I OK: 0x%02X", who);

    /* Enable stable multi-byte reads after reset (BDU + IF_INC). */
    ret = lsm6ds3_write_reg(h, LSM6DS3_CTRL3_C, LSM6DS3_CTRL3_C_CONFIG);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "CTRL3_C config failed");
        cleanup(drv);
        return ret;
    }

    /* ---- configure accelerometer (416 Hz, +/-4 g, ~100 Hz AA) ---- */
    ret = lsm6ds3_write_reg(
        h, LSM6DS3_CTRL1_XL, LSM6DS3_CTRL1_XL_CONFIG);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "accel config failed");
        cleanup(drv);
        return ret;
    }
    ESP_LOGI(TAG, "accel configured (416 Hz, +/-4 g, BW~100 Hz)");

    /* ---- configure gyroscope (416 Hz, 500 dps) ---- */
    ret = lsm6ds3_write_reg(
        h, LSM6DS3_CTRL2_G, LSM6DS3_CTRL2_G_CONFIG);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "gyro config failed");
        cleanup(drv);
        return ret;
    }
    ESP_LOGI(TAG, "gyro configured (416 Hz, 500 dps)");

    /* Enable internal gyro LPF1 path for vibration attenuation. */
    ret = lsm6ds3_write_reg(h, LSM6DS3_CTRL4_C, LSM6DS3_CTRL4_C_CONFIG);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "CTRL4_C config failed");
        cleanup(drv);
        return ret;
    }
    ESP_LOGI(TAG, "gyro LPF1 enabled (CTRL4_C=0x%02X)",
             LSM6DS3_CTRL4_C_CONFIG);

    /* ---- done ---- */
    *handle = h;
    ESP_LOGI(TAG, "init complete");
    return ESP_OK;
}

/* =============================================================
 * PUBLIC: lsm6ds3_read_raw
 * ============================================================= */
esp_err_t lsm6ds3_read_raw(
    lsm6ds3_handle_t handle,
    LSM6DS3_RawData  *data)
{
    if (handle == nullptr || data == nullptr)
        return ESP_ERR_INVALID_ARG;

    uint8_t buf[LSM6DS3_BUFFER_SIZE];

    /*
     * Burst-read 12 bytes starting at OUTX_L_G (0x22):
     *   bytes  0-5  -> gyro  X/Y/Z  (low, high)
     *   bytes  6-11 -> accel X/Y/Z  (low, high)
     */
    esp_err_t ret = lsm6ds3_read_regs(
        handle, LSM6DS3_OUTX_L_G,
        buf, LSM6DS3_BUFFER_SIZE);
    if (ret != ESP_OK) return ret;

    /* Gyroscope (first 6 bytes) */
    data->gyro_x  = combine_bytes(buf[0],  buf[1]);
    data->gyro_y  = combine_bytes(buf[2],  buf[3]);
    data->gyro_z  = combine_bytes(buf[4],  buf[5]);

    /* Accelerometer (next 6 bytes) */
    data->accel_x = combine_bytes(buf[6],  buf[7]);
    data->accel_y = combine_bytes(buf[8],  buf[9]);
    data->accel_z = combine_bytes(buf[10], buf[11]);

    return ESP_OK;
}

/* =============================================================
 * PUBLIC: lsm6ds3_read_scaled
 * ============================================================= */
esp_err_t lsm6ds3_read_scaled(
    lsm6ds3_handle_t   handle,
    LSM6DS3_ScaledData *data)
{
    if (handle == nullptr || data == nullptr)
        return ESP_ERR_INVALID_ARG;

    LSM6DS3_RawData raw;
    esp_err_t ret = lsm6ds3_read_raw(handle, &raw);
    if (ret != ESP_OK) return ret;

    /* Accelerometer -> m/s²  (g/LSB * 9.80665) */
    constexpr float accel_to_ms2 =
        LSM6DS3_ACCEL_SCALE * LSM6DS3_GRAVITY;

    data->accel_x = raw.accel_x * accel_to_ms2;
    data->accel_y = raw.accel_y * accel_to_ms2;
    data->accel_z = raw.accel_z * accel_to_ms2;

    /* Gyroscope -> dps */
    data->gyro_x = raw.gyro_x * LSM6DS3_GYRO_SCALE;
    data->gyro_y = raw.gyro_y * LSM6DS3_GYRO_SCALE;
    data->gyro_z = raw.gyro_z * LSM6DS3_GYRO_SCALE;

    return ESP_OK;
}

/* =============================================================
 * PUBLIC: lsm6ds3_deinit
 * ============================================================= */
esp_err_t lsm6ds3_deinit(lsm6ds3_handle_t handle)
{
    if (handle == nullptr)
        return ESP_ERR_INVALID_ARG;

    auto *drv = reinterpret_cast<lsm6ds3_t *>(handle);

    esp_err_t ret =
        i2c_master_bus_rm_device(drv->i2c_dev_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "rm device failed: %s",
                 esp_err_to_name(ret));
    }

    free(drv);
    ESP_LOGI(TAG, "deinitialized");
    return ret;
}

