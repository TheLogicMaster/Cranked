#pragma once

#include "Utils.hpp"

namespace cranked {

    class Cranked;

    class NativeResource {
    public:
        explicit NativeResource(Cranked &cranked);

        NativeResource(const NativeResource &other) = delete;

        NativeResource(NativeResource &&other) = delete;

        virtual ~NativeResource();

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

}
