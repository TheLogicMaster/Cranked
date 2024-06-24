// Based off of: https://github.com/mtrebi/memory-allocators/blob/master/src/FreeListAllocator.cpp

// Uses the first match rather than best, so fragmentation is probably not great

#pragma once

#include "Utils.hpp"

namespace cranked {

// Todo: Mirrored data structure to detect heap corruption, or just keep structure separate and use cookies for corruption
    class HeapAllocator {
    public:
        const size_t ALIGNMENT = 8; // Should probably use `alignof(max_align_t)`, but memory is currently aligned to multiples of `Node` (8 byte)

        template<typename T>
        struct AdaptorSTL {
            typedef T value_type;

            inline AdaptorSTL() : allocator(nullptr) { // This should be deleted, but it doesn't compile
                throw CrankedError("Null allocator");
            }

            inline explicit AdaptorSTL(HeapAllocator &allocator) noexcept: allocator(&allocator) {}

            inline AdaptorSTL(const AdaptorSTL &other) noexcept: allocator(other.allocator) {};

            template<typename U>
            inline AdaptorSTL(const AdaptorSTL<U> &other) noexcept : allocator(other.allocator) {}; // NOLINT: Intended conversion

            [[nodiscard]] inline constexpr T *allocate(size_t n) {
                return (T *) allocator->allocate(n * sizeof(T));
            }

            constexpr inline void deallocate(T *ptr, [[maybe_unused]] size_t n) {
                allocator->free(ptr);
            }

            inline bool operator==(const AdaptorSTL<T> &rhs) const noexcept {
                return allocator->startPtr == rhs.allocator->startPtr;
            }

            HeapAllocator *allocator;
        };

        explicit HeapAllocator(size_t size);

        HeapAllocator(const HeapAllocator &) = delete;

        HeapAllocator &operator=(const HeapAllocator &) = delete;

        ~HeapAllocator();

        void *allocate(size_t size);

        void *reallocate(void *ptr, size_t size);

        void free(void *ptr);

        void *baseAddress();

        void reset();

        const char *allocateString(const char *string) {
            auto ptr = (char *) allocate(strlen(string) + 1);
            strcpy(ptr, string);
            return ptr;
        }

        template<typename T, typename... Args>
        T *construct(Args &&... args) {
            auto ptr = (T *) allocate(sizeof(T));
            new(ptr) T(args...);
            return ptr;
        }

        template<typename T>
        void destruct(T *ptr) {
            if (!ptr)
                return;
            ptr->~T();
            free(ptr);
        }

        template<typename T>
        AdaptorSTL<T> allocator() {
            return AdaptorSTL<T>(*this);
        }

        template<typename Key, typename T>
        AdaptorSTL<pair<const Key, T>> pairAllocator() {
            return AdaptorSTL<pair<const Key, T>>(*this);
        }

    private:
        struct Node {
            size_t size; // Always the size of the entire block
            Node *next; // Only used for free list
        };

        size_t totalSize;
        void *startPtr;
        Node *freeList;

        void coalescence(Node *previousNode, Node *freeNode);

        void find(size_t size, Node *&previousNode, Node *&foundNode);

        void insertFreeNode(Node *previousNode, Node *node);

        void removeFreeNode(Node *previousNode, Node *node);
    };

    // The following containers can be annoying to use correctly because they must always be constructed with a custom allocator/deleter,
    // or they will use the default allocator for the normal heap, which is problematic for emulated access.

    template<typename Key, typename T>
    using HeapAllocatorMap = HeapAllocator::AdaptorSTL<pair<const Key, T>>;

    template<typename Key, typename T, typename Hash = hash<Key>, typename KeyEqual = equal_to<Key>>
    using vheap_unordered_map = unordered_map<Key, T, Hash, KeyEqual, HeapAllocatorMap<Key, T>>;

    template<typename Key, typename T, typename Compare = less<Key>>
    using vheap_map = map<Key, T, Compare, HeapAllocatorMap<Key, T>>;

    template<typename T>
    using vheap_vector = vector<T, HeapAllocator::AdaptorSTL<T>>;

    // Be careful using this because it starts with a small local buffer which isn't heap stored, prefer vector, or store the heap_string itself in the heap
    using vheap_string = basic_string<char, char_traits<char>, HeapAllocator::AdaptorSTL<char>>;

    template<typename T, typename Compare = less<T>>
    using vheap_set = set<T, Compare, HeapAllocator::AdaptorSTL<T>>;

    template<typename T, typename Hash = hash<T>, typename Equal = equal_to<T>>
    using vheap_unordered_set = unordered_set<T, Hash, Equal, HeapAllocator::AdaptorSTL<T>>;

    template<typename T>
    using HeapAllocatorSTL = HeapAllocator::AdaptorSTL<T>;

    template<typename T>
    class HeapDeleter {
    public:
        explicit HeapDeleter(HeapAllocator &allocator) : allocator(allocator) {}

        void operator()(T* ptr) const {
            allocator.destruct(ptr);
        }
    private:
        HeapAllocator &allocator;
    };

    template<typename T>
    using vheap_unique_ptr = unique_ptr<T, HeapDeleter<T>>;
}
