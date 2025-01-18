#pragma once

#include "Constants.hpp"
#include "ffi.h"
#include "nlohmann/json.hpp"
#include "boost/asio.hpp"
#include "dynarmic/interface/A32/a32.h"
#include "dynarmic/interface/A32/config.h"
#include "unicorn/unicorn.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "libzippp.h"
#include "zlib.h"
#include "tracy/Tracy.hpp"
#include "tracy/TracyLua.hpp"

#ifdef USE_CAPSTONE
#include "capstone/platform.h"
#include "capstone/capstone.h"
#endif

#include <vector>
#include <memory>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <bit>
#include <chrono>
#include <functional>
#include <queue>
#include <stdexcept>
#include <string>
#include <format>
#include <limits>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <cmath>
#include <cctype>
#include <concepts>
#include <optional>
#include <regex>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <utility>
#include <variant>
#include <numbers>

namespace cranked {

    class Cranked;

    enum class ResourceType;

    using std::stoi, std::stol, std::from_chars, std::to_string, std::isalpha, std::popcount;
    using std::abs, std::min, std::max, std::ceil, std::floor, std::pow, std::log2, std::round;
    using std::string, std::basic_string, std::string_view;
    using std::array, std::vector, std::set, std::map, std::unordered_set, std::unordered_map, std::multimap, std::queue, std::priority_queue;
    using std::function, std::invoke;
    using std::exception, std::runtime_error, std::range_error, std::out_of_range, std::logic_error;
    using std::strong_ordering, std::less, std::greater, std::errc;
    using std::optional, std::tuple, std::make_tuple, std::tie, std::ignore, std::pair, std::make_pair, std::integer_sequence, std::make_integer_sequence;
    using std::variant, std::get, std::get_if, std::holds_alternative;
    using std::false_type, std::tuple_element_t, std::is_same_v, std::is_pointer_v, std::is_class_v, std::remove_pointer_t, std::is_enum_v, std::is_void_v;
    using std::derived_from, std::integral, std::floating_point, std::is_standard_layout_v, std::is_trivial_v, std::remove_const_t, std::is_base_of_v, std::invocable;
    using std::hash, std::equal_to, std::less;
    using std::char_traits, std::numeric_limits;
    using std::size_t, std::time_t;
    using std::ostringstream, std::istringstream, std::ifstream, std::ofstream, std::getline, std::ios;
    using std::free, std::bad_alloc, std::aligned_alloc, std::addressof;
    using std::sort, std::find, std::find_if, std::erase, std::remove, std::swap, std::mismatch;
    using std::unique_ptr, std::shared_ptr, std::make_shared, std::make_unique;
    using std::format, std::vformat, std::make_format_args;
    using std::regex, std::regex_match, std::match_results, std::cmatch;

    namespace fs = std::filesystem;
    namespace chrono = std::chrono;
    namespace this_thread = std::this_thread;
    namespace numbers = std::numbers;

    typedef int64_t int64;
    typedef uint64_t uint64;
    typedef int32_t int32;
    typedef uint32_t uint32;
    typedef int16_t int16;
    typedef uint16_t uint16;
    typedef int8_t int8;
    typedef uint8_t uint8;

    enum class LogLevel {
        Verbose,
        Info,
        Warning,
        Error
    };

    typedef function<void(Cranked &cranked, LogLevel level, const char *fmt, va_list args)> LoggingCallback;
    typedef function<void(Cranked &cranked)> InternalUpdateCallback;

    class CrankedError : public runtime_error {
    public:
        explicit CrankedError(const char *message) : runtime_error(message) {}

        explicit CrankedError(const string &message) : runtime_error(message) {}

        template<class... Args>
        explicit CrankedError(string_view fmt, Args&&... args) : CrankedError(vformat(fmt, make_format_args(args...))) {}

        CrankedError(const CrankedError &other) = default;

        CrankedError(CrankedError &&other) noexcept : runtime_error(std::move(other)) {}
    };

    struct Config {
        LoggingCallback loggingCallback;
        InternalUpdateCallback updateCallback;
        void *userdata;
        int debugPort;
        vector<string> argv;
    };

    inline vector<string> splitString(string str, const string &delimiter) {
        vector<string> split;
        size_t pos;
        while ((pos = str.find(delimiter)) != string::npos) {
            split.push_back(str.substr(0, pos));
            str.erase(0, pos + delimiter.length());
        }
        split.push_back(str);
        return split;
    }

    inline vector<string> splitString(string str, char delimiter) {
        vector<string> split;
        size_t pos;
        while ((pos = str.find(delimiter)) != string::npos) {
            split.push_back(str.substr(0, pos));
            str.erase(0, pos + 1);
        }
        split.push_back(str);
        return split;
    }

    inline string stringToLower(string str) {
        transform(str.begin(), str.end(), str.begin(), [](unsigned char c){ return tolower(c); });
        return str;
    }

    inline string stringToUpper(string str) {
        transform(str.begin(), str.end(), str.begin(), [](unsigned char c){ return toupper(c); });
        return str;
    }

    class Version {
    public:
        Version() : version(0) {}

        explicit Version(const string &str) : version(parseVersion(str)) {}

        explicit Version(int version) : version(version) {}

        static int parseVersion(const string &str) {
            auto split = splitString(str, '.');
            if (split.size() != 3) throw CrankedError("Invalid version string");
            return stoi(split[0]) * 10000 + stoi(split[1]) * 100 + stoi(split[2]);
        }

        strong_ordering operator<=>(Version rhs) const {
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

        [[nodiscard]] string toString() const {
            return format("{}.{}.{}", getMajor(), getMinor(), getPatch());
        }

    private:
        int version;
    };

    inline static Version VERSION_2_4_1{"2.4.1"};

    template<typename TP>
    time_t to_time_t(TP tp) {
#if __ANDROID__
        return chrono::system_clock::to_time_t(chrono::time_point_cast<chrono::system_clock::duration>(tp - TP::clock::now() + chrono::system_clock::now()));
#else
        return chrono::system_clock::to_time_t(chrono::clock_cast<chrono::system_clock>(tp));
#endif
    }

    template<class T2, class T1>
    T2 bit_cast(T1 t1) {
#if __ANDROID__
        static_assert(sizeof(T1) == sizeof(T2), "Types must match sizes");
        static_assert(is_standard_layout_v<T1> and is_trivial_v<T1>, "Requires POD input");
        static_assert(is_standard_layout_v<T2> and is_trivial_v<T2>, "Requires POD output");
        T2 t2;
        memcpy(addressof(t2), addressof(t1), sizeof(T1));
        return t2;
#else
        return std::bit_cast<T2>(t1);
#endif
    }

    vector<uint8> decompressData(const uint8 *data, size_t length, size_t expectedSize);

    inline vector<uint8> readFileData(const string &path) {
        vector<uint8> data;
        if (!fs::is_regular_file(path))
            throw CrankedError("No such file: {}", path);
        ifstream input(path, ios::binary | ios::ate);
        auto size = input.tellg();
        if (size <= 0)
            throw CrankedError("Failed to read file: {}", path);
        input.seekg(0, ios::beg);
        data.resize(size);
        input.read((char *) data.data(), size);
        return data;
    }

    void writeGIF(const char *path, const uint8 *data, int width, int height);

    inline uint32 readUint32LE(const uint8 *data) {
        auto ptr = (uint8 *) data;
        return ptr[0] | ptr[1] << 8 | ptr[2] << 16 | ptr[3] << 24;
    }

    inline void writeUint32LE(uint8 *data, uint32 value) {
        for (int i = 0; i < 4; i++) {
            data[i] = value & 0xFF;
            value >>= 8;
        }
    }

    inline void writeUint32BE(uint8 *data, uint32 value) {
        for (int i = 0; i < 4; i++) {
            data[3 - i] = value & 0xFF;
            value >>= 8;
        }
    }

    inline uint16 readUint16LE(const uint8 *data) {
        auto ptr = (uint8 *) data;
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
    string formatHex(T value) {
        ostringstream stream;
        stream << std::hex << value;
        return stream.str();
    }

    template<typename T>
    static string formatHexStringLE(T t) {
        ostringstream stream;
        stream << std::hex;
        for (int i = 0; i < (int)sizeof(T); i++)
            stream << format("{:02x}", (t >> 8 * i) & 0xFF);
        return stream.str();
    }

    template<typename T>
    static T decodeHexStringLE(string_view string) {
        if (string.size() / 2 != sizeof(T))
            throw CrankedError("Invalid hex string for type");
        T t{};
        uint8 byte{};
        for (int i = 0; i < (int)sizeof(T); i++) {
            auto result = from_chars(string.begin() + i * 2, string.begin() + i * 2 + 2, byte, 16);
            if (result.ec != errc{})
                throw CrankedError("Invalid hex byte");
            t |= byte << i * 8;
        }
        return t;
    }

    template<typename S, typename T>
    constexpr T nearestValue(T a, T b, S test) {
        T val = T(test);
        return abs(a - val) < abs(b - val) ? a : b;
    }

    template<typename T, typename V>
        bool containsEquivalentValue(vector<T> &vec, const V &value) {
        return find_if(vec.begin(), vec.end(), [&](const T &t) { return t == value; }) != vec.end();
    }

    template<typename T, typename V>
    bool eraseByEquivalentValue(vector<T> &vec, const V &value) {
        if (auto it = find_if(vec.begin(), vec.end(), [&](const T &t) { return t == value; }); it != vec.end()) {
            vec.erase(it);
            return true;
        }
        return false;
    }

    template<typename T, typename V, typename H, typename P>
    bool eraseByEquivalentKey(unordered_set<T, H, P> &set, const V &value) {
        if (auto found = set.template find<V>(value); found != set.end()) {
            set.erase(found);
            return true;
        }
        return false;
    }

    template<typename K, typename V, typename H, typename P>
    bool eraseByEquivalentKey(unordered_map<K, V, H, P> &map, const K &key) {
        if (auto found = map.template find<V>(key); found != map.end()) {
            map.erase(found);
            return true;
        }
        return false;
    }

    template<typename K, typename V, typename H, typename P>
    V &atEquivalentKey(unordered_map<K, V, H, P> &map, const K &key) {
        if (auto found = map.template find<K>(key); found != map.end())
            return found->second;
        throw out_of_range("No such entry for equivalant key");
    }

    template<typename K, typename V, typename H, typename P>
    const V &atEquivalentKey(const unordered_map<K, V, H, P> &map, const K &key) {
        if (auto found = map.template find<K>(key); found != map.end())
            return found->second;
        throw out_of_range("No such entry for equivalant key");
    }

    template <typename T>
    concept numeric_type = integral<T> or floating_point<T>;

    template<typename S>
    struct dependent_false : false_type {};

    template<typename S>
    inline constexpr bool dependent_false_v = dependent_false<S>{};

    template<typename T, typename ...L>
    inline constexpr bool is_type_listed = (is_same_v<T, L> or ...);

    template<typename F>
    struct FunctionTypeHelper;

    template<typename R, typename ... Args>
    struct FunctionTypeHelper<R(*)(Args...)> {
        using Pointer = R(*)(Args...);
        using RetType = R;
        using ArgTypes = tuple<Args...>;
        static constexpr size_t ArgCount = sizeof...(Args);
        template<size_t N>
        using NthArg = tuple_element_t<N, ArgTypes>;
    };

    template<numeric_type T, int N>
    class RollingAverage {
    public:
        void addSample(T val) {
            buffer[index] = val;
            index = (index + 1) % N;
        }

        constexpr T value() {
            T sum{};
            for (int i = 0; i < N; i++) {
                int offset = index - 1 - i;
                sum += buffer[offset >= 0 ? offset : N + offset];
            }
            return sum / N;
        }

    private:
        array<T, N> buffer{};
        int index{}; // Index where to place next
    };

    class ScopeExitHelper {
    public:
        explicit ScopeExitHelper(const function<void()> &func) : func(func) {}

        ~ScopeExitHelper() {
            func();
        }

    private:
        const function<void()> func;
    };

    class NativeResource {
    public:
        explicit NativeResource(Cranked &cranked, ResourceType type, void *address);

        NativeResource(const NativeResource &other) = delete;

        NativeResource(NativeResource &&other) = delete;

        virtual ~NativeResource() = 0;

        void reference() {
            if (disposed)
                throw CrankedError("Attempted to reference disposed resource");
            refCount++;
        }

        bool dereference();

        [[nodiscard]] int getRefCount() const {
            return refCount;
        }

        [[nodiscard]] bool isDisposed() const {
            return disposed;
        }

        [[nodiscard]] bool isDisposing() const {
            return disposing;
        }

        Cranked &cranked;
        const ResourceType type;
        void * const address;

    private:
        int refCount{};
        bool disposing{}; // This is needed to prevent any references temporarily made during destruction from causing an infinite loop
        bool disposed{};
    };

    template<typename T> // Can't use a NativeResource constraint here, or it would be impossible to use recursively due to incomplete typing
    class ResourceRef {
    public:

        ResourceRef(T *resource = nullptr) : resource(resource) { // NOLINT(*-explicit-constructor)
            if (resource)
                resource->reference();
        }

        ResourceRef(const ResourceRef &other) : ResourceRef(other.resource) {}

        ResourceRef(ResourceRef &&other) noexcept : resource(other.resource) {
            other.resource = nullptr;
        }

        ~ResourceRef() {
            if (resource)
                resource->dereference();
        }

        T *get() const {
            return resource;
        }

        void reset(T *newResource) {
            if (resource == newResource)
                return;
            if (resource)
                resource->dereference();
            resource = newResource;
            if (newResource)
                newResource->reference();
        }

        /// Meant to be used for resetting system components without worrying about heap corruption exceptions and such
        void reset(bool throws = false) {
            try {
                reset(nullptr);
            } catch (exception &) {
                if (throws)
                    throw;
                resource = nullptr;
            }
        }

        ResourceRef &operator=(T *newResource) {
            reset(newResource);
            return *this;
        }

        ResourceRef &operator=(const ResourceRef &other) {
            if (&other != this)
                reset(other.resource);
            return *this;
        }

        ResourceRef &operator=(ResourceRef &&other) noexcept {
            if (resource)
                resource->dereference();
            resource = other.resource;
            other.resource = nullptr;
            return *this;
        }

        bool operator==(const ResourceRef &other) const = default;

        bool operator==(const T *other) const {
            return resource == other;
        }

        T *operator->() const {
            return resource;
        }

        T &operator*() const {
            return *resource;
        }

        operator bool() const { // NOLINT(*-explicit-constructor)
            return resource;
        }

    private:
        T *resource;
    };

    template <typename T>
    concept is_resource = is_base_of_v<NativeResource, T>;

    template <typename T>
    concept is_resource_ptr = is_base_of_v<NativeResource, remove_pointer_t<T>>;

    template<typename T>
    struct resource_hash
    {
        using is_transparent = void;

        size_t operator()(const ResourceRef<T>& s) const
        {
            return (size_t)s.get();
        }

        size_t operator()(const T* s) const
        {
            return (size_t)s;
        }
    };

    template<typename R> requires is_pointer_v<R>
    using unordered_resource_set = unordered_set<ResourceRef<remove_pointer_t<R>>, resource_hash<remove_pointer_t<R>>, equal_to<>>;

    template<typename R, typename T> requires is_pointer_v<R>
    using unordered_resource_map = unordered_map<ResourceRef<remove_pointer_t<R>>, T, resource_hash<remove_pointer_t<R>>, equal_to<>>;

    struct string_hash
    {
        using hash_type = hash<string_view>;
        using is_transparent = void;
        size_t operator()(const char* str) const { return hash_type{}(str); }
        size_t operator()(string_view str) const { return hash_type{}(str); }
        size_t operator()(string const& str) const { return hash_type{}(str); }
    };

    using unordered_string_set = unordered_set<string, string_hash, equal_to<>>;

    template<typename T>
    using unordered_string_map = unordered_map<string, T, string_hash, equal_to<>>;

}
