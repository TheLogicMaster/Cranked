#pragma once

#include <vector>
#include <memory>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <bit>
#include <chrono>
#include <functional>
#include <stdexcept>
#include <string>
#include <format>

namespace cranked {

    class Cranked;

    enum class LogLevel {
        Verbose,
        Info,
        Warning,
        Error
    };

    typedef std::function<void(Cranked &cranked, LogLevel level, const char *format, va_list args)> LoggingCallback;
    typedef std::function<void(Cranked &cranked)> InternalUpdateCallback;

    class CrankedError : public std::runtime_error {
    public:
        explicit CrankedError(const char *message) : std::runtime_error(message) {}

        explicit CrankedError(const std::string &message) : std::runtime_error(message) {}

        template<class... Args>
        explicit CrankedError(std::string_view fmt, Args&&... args) : CrankedError(std::vformat(fmt, std::make_format_args(args...))) {}

        CrankedError(const CrankedError &other) = default;

        CrankedError(CrankedError &&other) noexcept : std::runtime_error(std::move(other)) {}
    };

    struct Config {
        LoggingCallback loggingCallback;
        InternalUpdateCallback updateCallback;
        void *userdata;
        int debugPort;
    };

    inline std::vector<std::string> splitString(std::string string, const std::string &delimiter) {
        std::vector<std::string> split;
        size_t pos;
        while ((pos = string.find(delimiter)) != std::string::npos) {
            split.push_back(string.substr(0, pos));
            string.erase(0, pos + delimiter.length());
        }
        split.push_back(string);
        return split;
    }

    inline std::vector<std::string> splitString(std::string string, char delimiter) {
        std::vector<std::string> split;
        size_t pos;
        while ((pos = string.find(delimiter)) != std::string::npos) {
            split.push_back(string.substr(0, pos));
            string.erase(0, pos + 1);
        }
        split.push_back(string);
        return split;
    }

    class Version {
    public:
        Version() : version(0) {}

        explicit Version(const std::string &string) : version(parseVersion(string)) {}

        explicit Version(int version) : version(version) {}

        static int parseVersion(const std::string &string) {
            auto split = splitString(string, '.');
            if (split.size() != 3) throw CrankedError("Invalid version string");
            return std::stoi(split[0]) * 10000 + std::stoi(split[1]) * 100 + std::stoi(split[2]);
        }

        std::strong_ordering operator<=>(Version rhs) const {
            return version <=> rhs.version;
        }

        [[nodiscard]] int getMajor() const {
            return version / 10000;
        }

        [[nodiscard]] int getMinor() const {
            return (version / 100) % 100;
        }

        [[nodiscard]] int getPatch() const { // Todo: For some reason this is always zero
            return version % 100;
        }

        [[nodiscard]] int getCode() const {
            return version;
        }

        [[nodiscard]] bool isValid() const {
            return version > 0;
        }

        [[nodiscard]] std::string toString() const {
            return std::format("{}.{}.{}", getMajor(), getMinor(), getPatch());
        }

    private:
        int version;
    };

    inline static Version VERSION_2_4_1{"2.4.1"};

    template<typename TP>
    inline std::time_t to_time_t(TP tp) {
        using namespace std::chrono;
#if __ANDROID__
        return system_clock::to_time_t(time_point_cast<system_clock::duration>(tp - TP::clock::now() + system_clock::now()));
#else
        return system_clock::to_time_t(clock_cast<system_clock>(tp));
#endif
    }

    template<class T2, class T1>
    inline T2 bit_cast(T1 t1) {
#if __ANDROID__
        static_assert(sizeof(T1) == sizeof(T2), "Types must match sizes");
        static_assert(std::is_standard_layout_v<T1> && std::is_trivial_v<T1>, "Requires POD input");
        static_assert(std::is_standard_layout_v<T2> && std::is_trivial_v<T2>, "Requires POD output");

        T2 t2;
        memcpy( std::addressof(t2), std::addressof(t1), sizeof(T1) );
        return t2;
#else
        return std::bit_cast<T2>(t1);
#endif
    }

    std::vector<uint8_t> decompressData(const uint8_t *data, size_t length, size_t expectedSize);

    inline std::vector<uint8_t> readFileData(const std::string &path) {
        std::vector<uint8_t> data;
        if (!std::filesystem::is_regular_file(path))
            throw CrankedError("No such file: {}", path);
        std::ifstream input(path, std::ios::binary | std::ios::ate);
        auto size = input.tellg();
        if (size <= 0)
            throw CrankedError("Failed to read file: {}", path);
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
        std::ostringstream stream;
        stream << std::hex << value;
        return stream.str();
    }

    template<typename T>
    static std::string formatHexStringLE(T t) {
        std::ostringstream stream;
        stream << std::hex;
        for (int i = 0; i < (int)sizeof(T); i++)
            stream << std::format("{:02x}", (t >> 8 * i) & 0xFF);
        return stream.str();
    }

    template<typename T>
    static T decodeHexStringLE(std::string_view string) {
        if (string.size() / 2 != sizeof(T))
            throw CrankedError("Invalid hex string for type");
        T t{};
        uint8_t byte{};
        for (int i = 0; i < (int)sizeof(T); i++) {
            auto result = std::from_chars(string.begin() + i * 2, string.begin() + i * 2 + 2, byte, 16);
            if (result.ec != std::errc{})
                throw CrankedError("Invalid hex byte");
            t |= byte << i * 8;
        }
        return t;
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

}
