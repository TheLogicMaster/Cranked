#pragma once

#include <vector>
#include <memory>
#include <filesystem>
#include <fstream>

std::vector<uint8_t> decompressData(const uint8_t *data, size_t length, size_t expectedSize);

inline std::vector<uint8_t> readFileData(const std::string &path) {
    std::vector<uint8_t> data;
    if (!std::filesystem::is_regular_file(path))
        throw std::runtime_error("No such file: " + path);
    std::ifstream input(path, std::ios::binary | std::ios::ate);
    auto size = input.tellg();
    if (size <= 0)
        throw std::runtime_error("Failed to read file: " + path);
    input.seekg(0, std::ios::beg);
    data.resize(size);
    input.read((char *) data.data(), size);
    return data;
}

inline uint32_t readUint32LE(const uint8_t *data) {
    auto ptr = (uint8_t *) data;
    return ptr[0] | ptr[1] << 8 | ptr[2] << 16 | ptr[3] << 24;
}

inline void writeUint32LE(uint8_t *data, uint32_t value) {
    for (int i = 0; i < 4; i++) {
        data[i] = value & 0xFF;
        value >>= 8;
    }
}

inline void writeUint32BE(uint8_t *data, uint32_t value) {
    for (int i = 0; i < 4; i++) {
        data[3 - i] = value & 0xFF;
        value >>= 8;
    }
}

inline uint16_t readUint16LE(const uint8_t *data) {
    auto ptr = (uint8_t *) data;
    return ptr[0] | ptr[1] << 8;
}

inline size_t roundMemorySize(size_t size, size_t multiple) {
    if (multiple == 0)
        return size;
    auto remainder = size % multiple;
    if (remainder == 0)
        return size;
    return size + multiple - remainder;
}

template<typename T>
inline std::string formatHex(T value) {
    std::stringstream stream;
    stream << std::hex << value;
    return stream.str();
}

template<typename S>
struct dependent_false : std::false_type {};

template<typename F>
struct FunctionTypeHelper;

template<typename R, typename ... Args>
struct FunctionTypeHelper<R(*)(Args...)> {
    using Pointer = R(*)(Args...);
    using RetType = R;
    using ArgTypes = std::tuple<Args...>;
    static constexpr std::size_t ArgCount = sizeof...(Args);
    template<std::size_t N>
    using NthArg = std::tuple_element_t<N, ArgTypes>;
};
