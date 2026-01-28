#pragma once
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <vector>

template <typename T>
inline void append_bytes(std::vector<uint8_t>& out, const T& value)
{
    static_assert(std::is_trivially_copyable_v<T>, "append_bytes must be trivially copyable");
    const uint8_t* p = reinterpret_cast<const uint8_t*>(&value);

    out.insert(out.end(), p, p + sizeof(T));
}   