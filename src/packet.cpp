#include "packet.hpp"



uint8_t checksum_xor(const std::vector<uint8_t>& payload)
{
    uint8_t c = 0;

    for(uint8_t x : payload)
    {
        c^=x;
    }

    return c;
};

Packet build_packet(const std::vector<uint8_t>& payload)
{
    Packet p; 
    p.bytes.reserve(2 + payload.size() + 1);

    p.bytes.push_back(0xAA);
    p.bytes.push_back(static_cast<uint8_t>(payload.size()));
    p.bytes.insert(p.bytes.end(), payload.begin(), payload.end());
    p.bytes.push_back(checksum_xor(payload));

    return p;
};
