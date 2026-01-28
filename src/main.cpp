#include "imu_sample.hpp"
#include "bytes.hpp"
#include "packet.hpp"
#include "parser.hpp"
#include "byte_queue.hpp"
#include <random>

#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>

#include <fstream>
#include "decode.hpp"
#include <filesystem>


int main(int argc, char** argv)
{
    std::filesystem::create_directories("logs");
    // do not let compiler assume variable do not change across threads, thus use atomic
    // main thread is running along with producer consumer, hence atomic 
    // main thread is seeing decode as 0 and consumer is updating it
    // therefore we need to make decode shared / able to be seen and updated across threads
    (void)argc; (void)argv;

    
    using clock = std::chrono::steady_clock;

    ByteQueue q;
    Parser parser;

    std::atomic<uint64_t> decoded{0};
    std::atomic<bool> done{false};

    std::thread monitor([&] {
        using namespace std::chrono;
        uint64_t last_decoded = 0;
    
        while(!done.load(std::memory_order_relaxed))
        {
            std::this_thread::sleep_for(seconds(1));
    
            uint64_t now_decoded = decoded.load(std::memory_order_relaxed);
            uint64_t d = now_decoded - last_decoded;
            last_decoded = now_decoded;
    
            std::cout
                << "[1s] pkt/s=" << d
                << " total=" << now_decoded
                << " q_depth=" << q.size()
                << " good=" << parser.good_packets()
                << " bad_chk=" << parser.bad_checksum()
                << " resync=" << parser.resyncs()
                << "\n";
        }
    }); 


    // Consumer: blocks waiting for bytes, parses packets
    std::thread consumer([&] {
        // creating a buffer to hold popped bytes
        std::vector<uint8_t> buf(256);
        std::ofstream csv("logs/telemetry.csv");
        csv << "t_us,ax,ay,az,gx,gy,gz\n";
        csv.flush();


        while(true)
        {
            // popping bytes from queue into the buffer
            size_t n = q.pop_bytes(buf.data(), buf.size());
            // if no bytes are popped, we break loop,
            if(n == 0) break; // there will never be anymore bytes.

            auto packets = parser.feed(buf.data(), n);

            for(const auto& pv : packets)
            {
                ImuSample sample;
                if(decode_imu(pv.payload, sample))
                {
                    csv << sample.t_us << ","
                        << sample.ax << "," << sample.ay << "," << sample.az << ","
                        << sample.gx << "," << sample.gy << "," << sample.gz << "\n";
                }
            }
            // mem lax cause we dont need this order to be synchronized with other thread
            decoded.fetch_add(packets.size(), std::memory_order_relaxed);
        }
    });
    // since work is, time spent on work + time slept, we are not producing every 5 ms;
    // ex: 1 + 5, so produce next at 6, then 6 + 2 + 5, produce next at 13
    // so work around is tell thread what time to wake up at, ie 5 ms 10 ms 15 ms etc.
    // Producer: generates telemetry and pushes bytes
    std::thread producer([&] {
        std::mt19937 rng(12345);
        std::uniform_int_distribution<int> garbage_len_dist(5, 20);
        std::uniform_int_distribution<int> byte_dist(0, 255);
        std::uniform_int_distribution<int> coin(0, 99);

        using clock = std::chrono::steady_clock;
        auto start = clock::now();
        
        auto period = std::chrono::milliseconds(5);
        auto next_tick = start;

        long long worst_late_us = 0;
        long long sum_late_us = 0;
        int late_samples = 0;
        for(int i = 0; i < 200; ++i) // 200 samples
        {
            

            // 5ms 10 ms etc
            // ex next tick is 15, but current is 16, return then next tick will be 20, so self correcting. 
            next_tick += period;
            
            std::this_thread::sleep_until(next_tick);
            // we know what time we should've woken up at, did we wake up at the right time? more positive more late 
            auto late_us = std::chrono::duration_cast<std::chrono::microseconds>(clock::now() - next_tick).count();

            auto now = clock::now();
            
            if(late_us < 0) late_us = 0;  

            if(late_us > worst_late_us) worst_late_us = late_us;
            sum_late_us += late_us;
            late_samples += 1;



            auto us = std::chrono::duration_cast<std::chrono::microseconds>(now - start).count(); 
            ImuSample s = generate_sample(static_cast<uint32_t>(us));


            
            

            std::vector<uint8_t> payload;
            payload.reserve(sizeof(ImuSample));
            append_bytes(payload, s);

            Packet pkt = build_packet(payload);
            // CORRUPT PACKET
            if (coin(rng) < 8 && pkt.bytes.size() > 6) // 8% chance
            {
                // flip one bit in payload area (avoid header bytes)
                pkt.bytes[3] ^= 0x01;
            }

            // DROP OUT BYTES
            if (coin(rng) < 3 && pkt.bytes.size() > 6) // 3% chance
            {
                // drop a middle byte
                pkt.bytes.erase(pkt.bytes.begin() + 4);
            }

            // ADD GARBAGE
            if (coin(rng) < 10) // 10% chance
            {
                int g = garbage_len_dist(rng);
                std::vector<uint8_t> garbage;
                garbage.reserve(g);
                for(int i = 0; i < g; ++i)
                    garbage.push_back(static_cast<uint8_t>(byte_dist(rng)));
            
                q.push_bytes(garbage.data(), garbage.size());
            }

            q.push_bytes(pkt.bytes.data(), pkt.bytes.size());

        
        }
        double avg_late = (late_samples > 0) ? (double)sum_late_us / late_samples : 0.0;
        std::cout << "Producer jitter (late): avg=" << avg_late
          << " us, worst=" << worst_late_us << " us\n";

        q.shutdown();
    });

    producer.join();
    consumer.join();

    done.store(true, std::memory_order_relaxed);
    monitor.join();
    

    return 0;
}
