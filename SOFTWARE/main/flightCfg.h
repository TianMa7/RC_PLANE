//
// Created by tianm on 2026-04-13.
//

#ifndef RC_PLANE_FLIGHTCFG_H
#define RC_PLANE_FLIGHTCFG_H

/* =============================================================
 * LOGGING TAG
 * ============================================================= */
static const char *TAG = "CONTROLTASK";
static const char *TAG_REMOTE = "IBUS";

/* =============================================================
 * HARDWARE CONFIG - adjust pins for your board
 * ============================================================= */
#define I2C_MASTER_SDA_IO   GPIO_NUM_8
#define I2C_MASTER_SCL_IO   GPIO_NUM_9
#define I2C_MASTER_FREQ_HZ  400000
#define AILERON_SERVO_PIN   GPIO_NUM_0
#define ELEVATOR_SERVO_PIN  GPIO_NUM_2
#define IBUS_UART           UART_NUM_1
#define IBUS_RX_PIN         GPIO_NUM_4

constexpr float DEG_TO_RAD = M_PI / 180.0f;

#endif //RC_PLANE_FLIGHTCFG_H