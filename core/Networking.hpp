#pragma once

#include "gen/PlaydateAPI.hpp"

namespace cranked {

    class Cranked;

    constexpr inline uint32 DefaultNetworkReceiveBufferSize = 0x10000;
    constexpr inline uint32 MaxNetworkReceiveBufferSize = 0x1000000u;
    constexpr inline uint32 MinNetworkReceiveBufferSize = 0x100u;

    struct HTTPConnection_32 final : NativeResource {
        explicit HTTPConnection_32(Cranked &cranked);

        NetErr sendRequest(const string &method, const string &url, const string &headers, const string &body);
        asio::awaitable<void> sendRequestAsync(string method, string path, string headers, string body);

        void setReadBufferSize(uint32 size) {
            readBuffer.resize(clamp(size, MinNetworkReceiveBufferSize, MaxNetworkReceiveBufferSize));
        }

        int32 readData(span<uint8> buffer);

        void close();

        int32 connectTimeout{}, readTimeout{};
        bool keepAlive{};
        int32 byteRangeStart{}, byteRangeEnd{};
        void *userdata{};
        cref_t headerReceivedCallback{}, headersReadCallback{};
        cref_t responseCallback{}, completeCallback{};
        cref_t closedCallback{};
        string server;
        int port{};
        asio::ip::tcp::resolver resolver;
        asio::ip::tcp::socket socket;
        vector<uint8> readBuffer = vector<uint8>(DefaultNetworkReceiveBufferSize);
        int32 lastStatusCode{};
        NetErr lastError{};
    };

    struct TCPConnection_32 final : NativeResource {
        explicit TCPConnection_32(Cranked &cranked);

        void setReadBufferSize(uint32 size) {
            readBuffer.resize(clamp(size, MinNetworkReceiveBufferSize, MaxNetworkReceiveBufferSize));
        }

        NetErr open();
        asio::awaitable<void> openAsync();

        NetErr close();

        int32 send(span<const uint8> data);

        int32 connectTimeout{}, readTimeout{};
        void *userdata{}, *openUserdata{};
        cref_t openCallback{}, closedCallback{};
        string server;
        int port{};
        asio::ip::tcp::resolver resolver;
        asio::ip::tcp::socket socket;
        vector<uint8> readBuffer = vector<uint8>(DefaultNetworkReceiveBufferSize);
        vector<uint8> transmitBuffer = vector<uint8>(DefaultNetworkReceiveBufferSize);
        NetErr lastError{};
    };

    class Networking final {
    public:
        explicit Networking(Cranked &cranked);

        void init();

        void reset();

        void update();

        asio::io_context &getIOContext() {
            return ioContext;
        }

        Cranked &cranked;

    private:
        asio::io_context ioContext;
    };
}
