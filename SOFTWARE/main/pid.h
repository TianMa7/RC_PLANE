//
// Created by tianm on 2026-03-29.
//

#ifndef RC_PLANE_PID_H
#define RC_PLANE_PID_H

class PIDimpl;
class PID
{
        public:

                PID(float dt, float max, float min, float Kp, float Ki, float Kd, float center);

                float calculate(float setpoint, float pv);
                ~PID();
        private:
                PIDimpl *impl;
};

#endif //RC_PLANE_PID_H