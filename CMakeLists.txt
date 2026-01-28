cmake_minimum_required(VERSION 3.16)
project(multithreaded_telemetry_system LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(telemetry
  src/main.cpp
  src/byte_queue.cpp
  src/imu_sample.cpp
  src/packet.cpp
  src/parser.cpp
)

target_include_directories(telemetry PRIVATE include)

# Optional: warnings (nice signal)
if (MSVC)
  target_compile_options(telemetry PRIVATE /W4)
else()
  target_compile_options(telemetry PRIVATE -Wall -Wextra -Wpedantic)
endif()
