#include "Utils.hpp"

#include <zlib.h>
#include <stdexcept>

using namespace cranked;

std::vector<uint8_t> cranked::decompressData(const uint8_t *data, size_t length, size_t expectedSize) {
    static constexpr auto CHUNK_SIZE = 0x1000;
    std::vector<uint8_t> buffer(expectedSize > 0 ? expectedSize : CHUNK_SIZE);
    z_stream inflater{};
    inflater.avail_in = length;
    inflater.next_in = (uint8_t *) data;
    auto result = inflateInit(&inflater);
    if (result != Z_OK)
        throw std::runtime_error("Failed to decompress data");
    while (true) {
        inflater.avail_out = buffer.size() - inflater.total_out;
        inflater.next_out = buffer.data() + inflater.total_out;
        result = inflate(&inflater, Z_FULL_FLUSH);
        if (result < 0)
            throw std::runtime_error("Failed to decompress data");
        if (result == Z_STREAM_END)
            break;
        buffer.resize(buffer.size() + CHUNK_SIZE);
    }
    buffer.resize(inflater.total_out);
    inflateEnd(&inflater);
    return buffer;
}
