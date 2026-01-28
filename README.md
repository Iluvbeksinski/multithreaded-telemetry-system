# Multithreaded Telemetry System (C++17)

A multithreaded telemetry pipeline that simulates a noisy UART-like byte stream, parses framed packets from a streaming buffer, validates checksums, decodes IMU samples, and logs results to CSV while reporting throughput/health metrics.

## Why this exists
I built this to practice systems skills that show up in embedded/defense software:
- producer/consumer concurrency
- binary protocol framing + streaming parsing
- robustness to corruption/dropped bytes
- instrumentation (rate, jitter/latency, error counters)

## Features
- **Producer → Consumer pipeline** using `std::thread`, `std::mutex`, `std::condition_variable`
- Thread-safe **ByteQueue** with blocking pop + graceful shutdown
- Streaming **state-machine parser** with resynchronization after garbage/corruption
- Packet framing (sync + length + payload + checksum)
- CSV logging (decoded samples) + runtime stats (pkt/s, queue depth, checksum failures, resync count)
- Fault injection (optional): random garbage bytes, bit flips, dropped bytes

## Architecture
High level dataflow:

Producer thread (samples + packetize + inject faults)
        │
        ▼
   ByteQueue (thread-safe)
        │
        ▼
Consumer thread (streaming parse → decode → CSV)
        │
        ▼
 Metrics (pkt/s, queue depth, jitter/latency, errors)

## Build
> Requires a C++17 compiler (MSVC / clang / g++)

### Option A: CMake (recommended)
```bash
cmake -S . -B build
cmake --build build
