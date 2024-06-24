#pragma once

#include "Utils.hpp"

namespace cranked {

    class Cranked;

    class NativeResource {
    public:
        explicit NativeResource(Cranked &cranked);

        // Reference count and disposed flag are not copied when moving or copying, since they referred to the previous object's memory
        NativeResource(const NativeResource &other) : cranked(other.cranked) {}

        // There's no point in moving resources since they are always referred to by pointer
        NativeResource(NativeResource &&other) = delete;

        virtual ~NativeResource();

        void reference() {
            refCount++;
        }

        bool dereference();

        [[nodiscard]] inline int getRefCount() const {
            return refCount;
        }

        [[nodiscard]] inline bool isDisposed() const {
            return disposed;
        }

        Cranked &cranked;

    private:
        int refCount{};
        bool disposed{};
    };

    template<typename T> // Can't use a constraint here, or it would be impossible to use recursively due to incomplete typing
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
            if (resource)
                resource->dereference();
            resource = newResource;
            if (newResource)
                newResource->reference();
        }

        /// Meant to be used for resetting system components without worrying about heap corruption exceptions
        void reset(bool throws = false) {
            try {
                reset(nullptr);
            } catch (exception &ex) {
                if (throws)
                    throw ex;
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

        bool operator==(ResourceRef const &other) const = default;

        bool operator==(T *other) const {
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

}

template<typename T>
struct std::hash<cranked::ResourceRef<T>> // NOLINT(*-dcl58-cpp)
{
    size_t operator()(const cranked::ResourceRef<T>& s) const noexcept
    {
        return (size_t)s.get();
    }
};
