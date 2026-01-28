#pragma once
#include <cstdint>
#include <cstddef>
#include <vector>
#include <deque>
#include <mutex>
#include <condition_variable>



class ByteQueue
{
public:
    ByteQueue();

    void push_bytes(const uint8_t* data, size_t n);

    size_t pop_bytes(uint8_t* out, size_t max_n);

    void shutdown();

    size_t size();

    private:
        std::mutex m_;
        std::condition_variable cv_;
        std::deque<uint8_t> q_;
        bool shutdown_;
};