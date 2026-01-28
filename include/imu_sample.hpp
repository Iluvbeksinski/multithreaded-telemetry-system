#pragma once
#include <cstdint>

struct ImuSample
{
    float ax, ay, az; 
    float gx, gy, gz;
    uint32_t t_us;
};

ImuSample generate_sample(uint32_t t_us);