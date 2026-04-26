#include "mahony_filter.h"
#include <cmath>

/* =============================================================
 * CONSTRUCTOR
 * ============================================================= */
MahonyFilter::MahonyFilter(float kp, float ki)
    : m_kp(kp)
    , m_ki(ki)
    , m_q0(1.0f), m_q1(0.0f), m_q2(0.0f), m_q3(0.0f)
    , m_ix(0.0f), m_iy(0.0f), m_iz(0.0f)
{
}

/* =============================================================
 * UPDATE — call once per IMU sample
 *
 * Inputs:
 *   ax,ay,az  — accelerometer (any consistent unit)
 *   gx,gy,gz  — gyroscope in RAD/S
 *   dt        — seconds since last call
 * ============================================================= */
void MahonyFilter::update(
    float ax, float ay, float az,
    float gx, float gy, float gz,
    float dt)
{
    /* ---- 1.  Normalise accelerometer ---- */
    float norm = sqrtf(ax * ax + ay * ay + az * az);
    if (norm < 1e-6f) return;   // free-fall or no data
    norm = 1.0f / norm;
    ax *= norm;
    ay *= norm;
    az *= norm;

    /* ---- 2.  Estimated gravity from quaternion ---- *
     * This is the third column of the rotation matrix
     * (body-frame Z expressed in world frame).         */
    float vx = 2.0f * (m_q1 * m_q3 - m_q0 * m_q2);
    float vy = 2.0f * (m_q0 * m_q1 + m_q2 * m_q3);
    float vz = m_q0 * m_q0 - m_q1 * m_q1
             - m_q2 * m_q2 + m_q3 * m_q3;

    /* ---- 3.  Error = cross(accel, estimated_gravity) ---- *
     * Points in the direction the quaternion needs to
     * rotate to align with the real gravity vector.        */
    float ex = ay * vz - az * vy;
    float ey = az * vx - ax * vz;
    float ez = ax * vy - ay * vx;

    /* ---- 4.  PI controller ---- */
    /* Integral term (slow bias removal) */
    if (m_ki > 0.0f) {
        m_ix += m_ki * ex * dt;
        m_iy += m_ki * ey * dt;
        m_iz += m_ki * ez * dt;
    }

    /* Apply correction to gyro */
    gx += m_kp * ex + m_ix;
    gy += m_kp * ey + m_iy;
    gz += m_kp * ez + m_iz;

    /* ---- 5.  Integrate quaternion (first-order) ---- */
    float halfdt = 0.5f * dt;
    float dq0 = (-m_q1 * gx - m_q2 * gy - m_q3 * gz) * halfdt;
    float dq1 = ( m_q0 * gx + m_q2 * gz - m_q3 * gy) * halfdt;
    float dq2 = ( m_q0 * gy - m_q1 * gz + m_q3 * gx) * halfdt;
    float dq3 = ( m_q0 * gz + m_q1 * gy - m_q2 * gx) * halfdt;

    m_q0 += dq0;
    m_q1 += dq1;
    m_q2 += dq2;
    m_q3 += dq3;

    /* Re-normalise (keeps quaternion unit-length) */
    norm = sqrtf(m_q0*m_q0 + m_q1*m_q1
               + m_q2*m_q2 + m_q3*m_q3);
    norm = 1.0f / norm;
    m_q0 *= norm;
    m_q1 *= norm;
    m_q2 *= norm;
    m_q3 *= norm;
}

/* =============================================================
 * GET EULER ANGLES (degrees)
 *
 *   pitch = rotation about Y  (nose up = positive)
 *   roll  = rotation about X  (right wing down = positive)
 *   yaw   = rotation about Z  (drifts without magnetometer)
 * ============================================================= */
MahonyEuler MahonyFilter::getEuler() const
{
    MahonyEuler e;

    /* pitch (asin clamp to [-1,1] for safety) */
    float sinp = 2.0f * (m_q0 * m_q2 - m_q3 * m_q1);
    if (sinp >  1.0f) sinp =  1.0f;
    if (sinp < -1.0f) sinp = -1.0f;
    e.pitch = asinf(sinp) * (180.0f / M_PI);

    /* roll */
    e.roll = atan2f(
        2.0f * (m_q0 * m_q1 + m_q2 * m_q3),
        1.0f - 2.0f * (m_q1 * m_q1 + m_q2 * m_q2))
        * (180.0f / M_PI);

    /* yaw */
    e.yaw = atan2f(
        2.0f * (m_q0 * m_q3 + m_q1 * m_q2),
        1.0f - 2.0f * (m_q2 * m_q2 + m_q3 * m_q3))
        * (180.0f / M_PI);

    return e;
}

/* =============================================================
 * GET RAW QUATERNION
 * ============================================================= */
void MahonyFilter::getQuaternion(
    float &w, float &x, float &y, float &z) const
{
    w = m_q0;
    x = m_q1;
    y = m_q2;
    z = m_q3;
}

