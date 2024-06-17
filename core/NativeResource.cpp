#include "NativeResource.hpp"

#include "Cranked.hpp"

using namespace cranked;

NativeResource::NativeResource(Cranked &cranked) : cranked(cranked) {
    cref_t address = cranked.nativeEngine.toVirtualAddress(this, false);
    if (address < HEAP_ADDRESS or address >= HEAP_ADDRESS + HEAP_SIZE)
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
    if (--refCount <= 0) {
        cranked.heap.destruct(this); // Seems sketchy deleting `this`, but it's fine
        return true;
    }
    return false;
}
