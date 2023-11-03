#include "pid.h"

long PID::step(double err)
{
    auto now = std::chrono::system_clock::now();
    unsigned long long dt = std::chrono::duration_cast<std::chrono::microseconds>(now - last_step).count();
 
    if (dt > 100000 || dt == 0) { // too far from the past, no longer applicable
        clear_stats();
        last_step = std::chrono::system_clock::now();// possibly problematic code, synchronization can be a problems
        return 0l;
    }
    
    double derivative = static_cast<double>((err - prev_err) / dt);
    
    //if (dt != 0) { // prevent division by zero
    integral += static_cast<double>(err * dt);
    //derivative /= dt;
    //}
    long long output = static_cast<long long>(kp * err + ki * integral + kd * derivative);

    if (output > 13ll) output = 13ll;
    else if (output < -13ll) output = -13ll;

    last_step = now;
    prev_err = err;

    return static_cast<long>(output);
}

// reset prev_err and integral
void PID::clear_stats()
{
    this->prev_err = 0.000000;
    this->integral = 0.000000;
};

void PID::clear_time() {
    this->last_step = std::chrono::system_clock::time_point{};
}

void PID::change_parameter(const double input_kp, const double input_ki, const double input_kd) {
    this->kp = input_kp;
    this->ki = input_ki;
    this->kd = input_kd;
}