#include "Utils.hpp"
#include "gif-h/gif.h"
#include "Cranked.hpp"

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

NativeResource::NativeResource(Cranked &cranked, ResourceType type, void *address) : cranked(cranked), type(type), address(address) {
    if (cref_t virtAddress = cranked.nativeEngine.toVirtualAddress(this, false); virtAddress < HEAP_ADDRESS or virtAddress >= HEAP_ADDRESS + HEAP_SIZE)
        throw CrankedError("Native resource allocated in non-heap memory");
    cranked.nativeEngine.nativeResources.emplace(this);
}

NativeResource::~NativeResource() {
    disposed = true;
    cranked.nativeEngine.nativeResources.erase(this);
}

bool NativeResource::dereference() {
    if (disposed)
        throw CrankedError("Attempted to dereference disposed resource");
    if (refCount <= 0)
        throw CrankedError("Attempted to dereference unreferenced resource");
    if (int count = --refCount; not disposing and count <= 0) {
        disposing = true;
        cranked.heap.destruct(this); // Seems sketchy deleting `this`, but it's fine
        if (count != refCount)
            throw CrankedError("Attempted resource resurrection detected");
        return true;
    }
    return false;
}
