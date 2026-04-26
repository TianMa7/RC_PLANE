#pragma once

#include <cstdint>

/* =============================================================
 * MAHONY COMPLEMENTARY FILTER
 *
 * Fuses accelerometer and gyroscope data to produce a
 * drift-free orientation estimate (quaternion -> euler).
 *
 * Algorithm:
 *   1. Normalise the accel vector.
 *   2. Estimate gravity direction from current quaternion.
 *   3. Cross-product gives the orientation error.
 *   4. PI controller corrects the gyro bias in real-time.
 *   5. Integrate corrected gyro rates via quaternion update.
 *
 * Tuning:
 *   Kp (proportional) — how aggressively accel corrects
 *       the gyro.  Higher = faster settling, more noise.
 *   Ki (integral) — slowly removes gyro bias over time.
 *       Set to 0 if you don't need bias compensation.
 * ============================================================= */

/** Euler angles in degrees */
struct MahonyEuler
{
    float pitch;  // nose up/down   (deg)
    float roll;   // wing tilt      (deg)
    float yaw;    // heading        (deg, magnetometer-less → drifts)
};

/** Filter state — create one instance, call update() in your loop */
class MahonyFilter
{
public:
    /**
     * @param kp  Proportional gain  (start with 2.0)
     * @param ki  Integral gain      (start with 0.005)
     */
    MahonyFilter(float kp = 2.0f, float ki = 0.005f);

    /**
     * @brief Feed new IMU sample and advance the filter.
     *
     * @param ax,ay,az  Accelerometer in m/s² (any unit
     *                  works — vector is normalised).
     * @param gx,gy,gz  Gyroscope in **radians/sec**.
     * @param dt        Time step in seconds since last call.
     */
    void update(float ax, float ay, float az,
                float gx, float gy, float gz,
                float dt);

    /** Get current orientation as Euler angles (deg). */
    MahonyEuler getEuler() const;

    /** Get raw quaternion [w, x, y, z]. */
    void getQuaternion(float &w, float &x,
                       float &y, float &z) const;

private:
    /* Tuning gains */
    float m_kp;
    float m_ki;

    /* Quaternion state (unit quaternion) */
    float m_q0, m_q1, m_q2, m_q3;

    /* Integral error accumulator (gyro-bias estimate) */
    float m_ix, m_iy, m_iz;
};

