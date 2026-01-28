#pragma once
#include <cstdint> 
#include <vector>
#include "imu_sample.hpp"

struct Packet
{
    std::vector<uint8_t> bytes;
};

uint8_t checksum_xor(const std::vector<uint8_t>& payload);

Packet build_packet(const std::vector<uint8_t>& payload);
