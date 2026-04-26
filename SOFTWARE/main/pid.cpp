//
// Created by tianm on 2026-03-29.
//
#include "pid.h"
#include <cmath>

class PIDimpl
{
        public:
                PIDimpl(float dt, float max, float min, float kp, float ki, float kd, float center);
                ~PIDimpl();
                float calculate(float setpoint, float pv);

        private:
                float _dt;
                float _max;
                float _min;
                float _Kp;
                float _Ki;
                float _Kd;
                float _pre_error;
                float _integral;
                float _center;

};

PID::PID(float dt, float max, float min, float Kp, float Ki, float Kd, float center)
{
        impl = new PIDimpl(dt, max, min, Kp, Ki, Kd, center);
}
float PID::calculate(float setpoint, float pv)
{
        return impl->calculate(setpoint, pv);
}
PID::~PID()
{
        delete impl;
}

/*implementation*/

PIDimpl::PIDimpl(float dt, float max, float min, float kp, float ki, float kd, float center):
        _dt(dt),
        _max(max),
        _min(min),
        _Kp(kp),
        _Ki(ki),
        _Kd(kd),
        _pre_error(0),
        _integral(0),
        _center(center)
{
}

float PIDimpl::calculate(float setpoint, float pv)
{
        //calculate error
        float error = setpoint - pv;

        //proportional term
        float Pout = _Kp * error;

        //integral term
        _integral += error * _dt;

        if (_Ki > 0.0) {
                float max_integral = _max / _Ki;
                float min_integral = _min / _Ki;

                if (_integral > max_integral) {
                        _integral = max_integral;
                } else if (_integral < min_integral) {
                        _integral = min_integral;
                }
        }

        float Iout = _Ki * _integral;

        //derivative term
        float derivative = (error - _pre_error) / _dt;
        float Dout = _Kd * derivative;

        float output = Pout + Iout + Dout + _center;

        if (output > _max)
                output = _max;
        else if (output < _min)
                output = _min;

        //save error to previous error
        _pre_error = error;

        return output;
}

PIDimpl::~PIDimpl()
{

}
