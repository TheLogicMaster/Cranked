#include "Utils.hpp"
#include "gif-h/gif.h"

using namespace cranked;

vector<uint8> cranked::decompressData(const uint8 *data, size_t length, size_t expectedSize) {
    static constexpr auto CHUNK_SIZE = 0x1000;
    vector<uint8> buffer(expectedSize > 0 ? expectedSize : CHUNK_SIZE);
    z_stream inflater{};
    inflater.avail_in = length;
    inflater.next_in = (uint8 *) data;
    auto result = inflateInit(&inflater);
    if (result != Z_OK)
        throw runtime_error("Failed to decompress data");
    while (true) {
        inflater.avail_out = buffer.size() - inflater.total_out;
        inflater.next_out = buffer.data() + inflater.total_out;
        result = inflate(&inflater, Z_FULL_FLUSH);
        if (result < 0)
            throw runtime_error("Failed to decompress data");
        if (result == Z_STREAM_END)
            break;
        buffer.resize(buffer.size() + CHUNK_SIZE);
    }
    buffer.resize(inflater.total_out);
    inflateEnd(&inflater);
    return buffer;
}

void cranked::writeGIF(const char *path, const uint8 *data, int width, int height) {
    GifWriter g;
    GifBegin(&g, path, width, height, 0);
    GifWriteFrame(&g, data, width, height, 0);
    GifEnd(&g);
}
