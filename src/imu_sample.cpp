#include "imu_sample.hpp"
#include <cmath>

ImuSample generate_sample(uint32_t t_us)
{
    const float t = t_us * 1e-6f;

    ImuSample s{};
    s.t_us = t_us;

    // accel (pretend gravity + movement)
    s.ax = 0.1f * std::sin(2.0f * 3.1415926f * 0.7f * t);
    s.ay = 0.1f * std::sin(2.0f * 3.1415926f * 0.9f * t);
    s.az = 1.0f + 0.02f * std::sin(2.0f * 3.1415926f * 0.5f * t);

    // gyro (pretend rotation rates)
    s.gx = 20.0f * std::sin(2.0f * 3.1415926f * 0.6f * t);
    s.gy = 10.0f * std::sin(2.0f * 3.1415926f * 0.4f * t);
    s.gz = 5.0f  * std::sin(2.0f * 3.1415926f * 0.2f * t);

    return s;
}
