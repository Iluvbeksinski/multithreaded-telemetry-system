#pragma once 
#include <cstdint>
#include <cstddef>
#include <vector>

struct PacketView
{
    std::vector<uint8_t> payload;
};

class Parser
{
public:

    Parser();
    std::vector<PacketView> feed(const uint8_t* data, size_t n);

    uint64_t good_packets() const { return good_; }
    uint64_t bad_checksum() const { return bad_checksum_; }
    uint64_t resyncs() const { return resyncs_; }

private:
    enum class State { WaitSync, ReadLen, ReadPayload, ReadChecksum};

    State state_;
    uint8_t expected_len_;
    std::vector<uint8_t> payload_;

    uint64_t good_;
    uint64_t bad_checksum_;
    uint64_t resyncs_;

    
};