#include "parser.hpp"
#include "packet.hpp" // for checksum_xor
#include <algorithm>

Parser::Parser() 
: state_(State::WaitSync),
expected_len_(0),
payload_(),
good_(0),
bad_checksum_(0),
resyncs_(0)
{
}

std::vector<PacketView> Parser::feed(const uint8_t* data, size_t n)
{
    std::vector<PacketView> out;

    for(size_t i = 0; i < n; ++i)
    {
        uint8_t b = data[i];

        switch(state_)
        {
            case State::WaitSync:
            {
                if (b == 0xAA)
                {
                    state_ = State::ReadLen;
                }
                break;
            }

            case State::ReadLen:
            {
                expected_len_ = b;

                if(expected_len_ == 0 || expected_len_ > 200)
                {
                    // treat as invalid packet so resync
                    resyncs_++;
                    state_ = State::WaitSync;
                    break;
                }
                // sets payload clear to reuse for next
                payload_.clear();
                payload_.reserve(expected_len_);
                state_ = State::ReadPayload;
                break;
            }

            case State::ReadPayload:
            {
                payload_.push_back(b);
                if(payload_.size() == expected_len_)
                {
                    state_ = State::ReadChecksum;
                }
                break;
            }

            case State::ReadChecksum:
            {
                uint8_t chk = b;
                uint8_t computed = checksum_xor(payload_);

                if(chk == computed)
                {
                    good_++;
                    out.push_back(PacketView{payload_});
                }

                else
                {
                    bad_checksum_++;
                }

                state_ = State::WaitSync; 
                break;
            }
        }  
    }
    return out;
}