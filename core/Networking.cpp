#include "Networking.hpp"
#include "Cranked.hpp"

using namespace cranked;

HTTPConnection_32::HTTPConnection_32(Cranked &cranked)
        : NativeResource(cranked, ResourceType::HttpConnection, this), resolver(cranked.networking.getIOContext()), socket(cranked.networking.getIOContext()) {}

NetErr HTTPConnection_32::sendRequest(const string &method, const string &path, const string &headers, const string &body) {
    // Todo: Cancel any existing request?

    asio::co_spawn(cranked.networking.getIOContext(), sendRequestAsync(method, path, headers, body), asio::detached);

    return PDNetErr::OK;
}

asio::awaitable<void> HTTPConnection_32::sendRequestAsync(string method, string path, string headers, string body) {
    asio::error_code err{};
    asio::streambuf response{MaxNetworkReceiveBufferSize};

    // Todo: Errors initiated by the underlying connection destroyed must immediately return and not access `this` (May require a separate cookie to detect)

    if (!socket.is_open()) {
        auto endpoints = co_await resolver.async_resolve(server, to_string(port), asio::redirect_error(asio::use_awaitable, err));
        if (err) {
            // Todo: Err
            co_return;
        }

        co_await asio::async_connect(socket, endpoints, asio::redirect_error(asio::use_awaitable, err));
        if (err) {
            // Todo: Err
            co_return;
        }
    }

    asio::streambuf request;
    std::ostream requestStream(&request);
    requestStream << method << " " << path << " HTTP/1.0\r\n";
    requestStream << "Host: " << address << ':' << port << "\r\n";
    requestStream << "Accept: */*\r\n";
    if (byteRangeStart >= 0 and byteRangeEnd >= 0)
        requestStream << "Range: bytes=" << byteRangeStart << "-" << byteRangeEnd << "\r\n";
    if (!body.empty())
        requestStream << "Content-Length: " << body.size() << "\r\n";
    if (!headers.empty()) // Todo: Is newline replacing needed? Is ending newline needed?
        requestStream << headers << "\r\n";
    // Todo: Keep-alive
    // if (keepAlive)
        // request_stream << "Connection: Keep-Alive\r\n";
    // else
        requestStream << "Connection: close\r\n";
    requestStream << "\r\n";
    if (!body.empty())
        requestStream << body;

    co_await asio::async_write(socket, request, asio::redirect_error(asio::use_awaitable, err));
    if (err) {
        // Todo: Err
        co_return;
    }

    co_await asio::async_read_until(socket, response, "\r\n", asio::redirect_error(asio::use_awaitable, err));
    if (err) {
        // Todo: Err
        co_return;
    }

    std::istream statusStream(&response);
    std::string http_version;
    statusStream >> http_version;
    unsigned int status_code;
    statusStream >> status_code;
    std::string status_message;
    std::getline(statusStream, status_message);
    if (!statusStream or http_version.substr(0, 5) != "HTTP/") {
        // Todo: Err
        co_return;
    }
    if (status_code != 200) {
        // Todo: Err
        co_return;
    }

    co_await asio::async_read_until(socket, response, "\r\n\r\n", asio::redirect_error(asio::use_awaitable, err));
    if (err) {
        // Todo: Err
        co_return;
    }

    std::istream headerStream(&response);
    std::string header;
    while (std::getline(headerStream, header) && header != "\r")
        readBuffer.insert(readBuffer.end(), std::istream_iterator<uint8>{ headerStream }, {}); // Todo: Limit

    while (true) {
        co_await asio::async_read(socket, response, asio::transfer_at_least(1), asio::redirect_error(asio::use_awaitable, err));
        if (err == asio::error::eof)
            break;
        if (err) {
            // Todo: Err
        }
        std::istream responseStream(&response);
        readBuffer.insert(readBuffer.end(), std::istream_iterator<uint8>{ responseStream }, {}); // Todo: Limit
    }

    // Todo: Complete
}

int32 HTTPConnection_32::readData(span<uint8> buffer) {
    auto start = chrono::high_resolution_clock::now();
    while (readBuffer.size() < buffer.size() and chrono::high_resolution_clock::now() - start < chrono::milliseconds(readTimeout))
        cranked.graphics.sleep(15);
    size_t count = min(buffer.size(), readBuffer.size());
    memcpy(buffer.data(), readBuffer.data(), count);
    readBuffer.resize(readBuffer.size() - count);
    return (int)count;
}

void HTTPConnection_32::close() {
    resolver.cancel();
    socket.close();
    readBuffer.clear();
    lastStatusCode = 0;
    lastError = {};
    // Todo: Closed callback?
}

TCPConnection_32::TCPConnection_32(Cranked &cranked)
        : NativeResource(cranked, ResourceType::TcpConnection, this), resolver(cranked.networking.getIOContext()), socket(cranked.networking.getIOContext()) {
    socket.non_blocking(true);
}

NetErr TCPConnection_32::open() {
    asio::co_spawn(cranked.networking.getIOContext(), openAsync(), asio::detached);
    return PDNetErr::OK;
}

asio::awaitable<void> TCPConnection_32::openAsync() {
    asio::error_code err{};
    asio::streambuf response{MaxNetworkReceiveBufferSize};

    // Todo: Errors initiated by the underlying connection destroyed must immediately return and not access `this` (May require a separate cookie to detect)

    auto endpoints = co_await resolver.async_resolve(server, to_string(port), asio::redirect_error(asio::use_awaitable, err));
    if (err) {
        // Todo: Err
        co_return;
    }

    co_await asio::async_connect(socket, endpoints, asio::redirect_error(asio::use_awaitable, err));
    if (err) {
        // Todo: Err
        co_return;
    }

    while (true) {
        co_await asio::async_read(socket, response, asio::transfer_at_least(1), asio::redirect_error(asio::use_awaitable, err));
        if (err) {
            // Todo: Err
            co_return;
        }
        std::istream responseStream(&response);
        readBuffer.insert(readBuffer.end(), std::istream_iterator<uint8>{ responseStream }, {}); // Todo: Limit
    }
}

NetErr TCPConnection_32::close() {
    socket.close();
    resolver.cancel();
    return PDNetErr::OK;
}

int32 TCPConnection_32::send(span<const uint8> data) {
    asio::error_code err{};
    size_t count = asio::write(socket, asio::buffer(data.data(), data.size()), err);
    if (err) return (int)NetErr::WriteError; // Todo
    return (int)count;
}

Networking::Networking(Cranked &cranked) : cranked(cranked) {}

void Networking::init() {
}

void Networking::reset() {
    ioContext.restart();
}

void Networking::update() {
    ioContext.poll();
}
