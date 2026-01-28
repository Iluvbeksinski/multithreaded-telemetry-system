#pragma once
#include <cstdint>
#include <cstring>
#include <vector>
#include "imu_sample.hpp"

inline bool decode_imu(const std::vector<uint8_t>& payload, ImuSample& out)
{
    if(payload.size() != sizeof(ImuSample))
        return false;

    std::memcpy(&out, payload.data(), sizeof(ImuSample));
    return true;
}
