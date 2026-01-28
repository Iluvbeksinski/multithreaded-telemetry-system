#include "byte_queue.hpp"
#include <cassert>

// std::mutex m_;
//         std::condition_variable cv_;
//         std::deque<uint8_t> q_;
//         bool shutdown_;
ByteQueue::ByteQueue() : shutdown_(false) {};

void ByteQueue::push_bytes(const uint8_t* data, size_t n)
{
    //lock here since both producer and consumer queues are
    // are / would be touching the deque / buffer (removing and adding elements to it)
    std::lock_guard<std::mutex> lock(m_);
    for(size_t i = 0; i<n; i++)
    {
        q_.push_back(data[i]);
    }
    // here we can notify the consumer to start taking stuff
    cv_.notify_one();
}
// policy descision, shutdown means stop producing, btu we can still take stuff thats remaining from buffer.
size_t ByteQueue::pop_bytes(uint8_t* out, size_t max_n)
{
    std::unique_lock<std::mutex> lock(m_);
    //even if its shutdown, we might still have stuff to give, hence if we are shutdown we can still proceed
    cv_.wait(lock, [&](){return shutdown_ || !q_.empty();});
    assert(max_n > 0);
    
    //if shutdown and empty just return 0
    if(q_.empty() && shutdown_)
    {
        return 0;
    }

    size_t count = 0;

    while(count < max_n && !q_.empty())
    {
        out[count++] = q_.front();
        q_.pop_front();

    }
    return count;
}

void ByteQueue::shutdown()
{
    {
        std::lock_guard<std::mutex> lock(m_);
        shutdown_ = true;
    }

    cv_.notify_all();
}

size_t ByteQueue::size()
{
    std::lock_guard<std::mutex> lock(m_);
    return q_.size();
}

