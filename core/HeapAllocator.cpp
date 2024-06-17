#include "HeapAllocator.hpp"
#include "Utils.hpp"

#include <cstdlib>
#include <cstring>

using namespace cranked;

HeapAllocator::HeapAllocator(size_t size) : totalSize(size), freeList(nullptr) {
    startPtr = std::aligned_alloc(4096, size);
    reset();
}

HeapAllocator::~HeapAllocator() {
    std::free(startPtr);
}

void *HeapAllocator::allocate(size_t size) {
    size = roundMemorySize(size, ALIGNMENT);
    auto blockSize = size + sizeof(Node);

    Node *node, *previous;
    find(blockSize, previous, node);
    if (!node)
        throw std::bad_alloc();

    auto remaining = node->size - blockSize;

    if (remaining >= sizeof(Node) + ALIGNMENT) {
        Node *newNode = (Node *) ((size_t) node + blockSize);
        newNode->size = remaining;
        insertFreeNode(node, newNode);
    }
    removeFreeNode(previous, node);

    node->next = nullptr;
    if (remaining >= sizeof(Node) + ALIGNMENT)
        node->size = blockSize;

    auto ptr = (void *)((intptr_t) node + sizeof(Node));
    memset(ptr, 0, size); // No need to bother with heap garbage

    return ptr;
}

void *HeapAllocator::reallocate(void *ptr, size_t size) {
    if (!ptr)
        return allocate(size);
    Node *node = (Node *)((intptr_t) ptr - sizeof(Node));
    if (node->size - sizeof(Node) >= size)
        return ptr;
    auto newBlock = allocate(size);
    memcpy(newBlock, ptr, node->size - sizeof(Node));
    free(ptr);
    return newBlock;
}

void HeapAllocator::free(void *ptr) {
    if (!ptr)
        return;

    Node *node = (Node *)((intptr_t) ptr - sizeof(Node));

    if (node->size > totalSize or node->next != nullptr)
        throw CrankedError("Corrupted heap node");

    // Insert into free node list based on address
    auto it = freeList;
    Node *prev = nullptr;
    while (it) {
        if (node < it) {
            insertFreeNode(prev, node);
            break;
        }
        prev = it;
        it = it->next;
    }

    coalescence(prev, node);
}

void *HeapAllocator::baseAddress() {
    return startPtr;
}

void HeapAllocator::reset() {
    freeList = (Node *) startPtr;
    freeList->size = totalSize;
    freeList->next = nullptr;
}

void HeapAllocator::coalescence(Node *previousNode, Node *freeNode) {
    if (freeNode->next and (size_t) freeNode + freeNode->size == (size_t) freeNode->next) {
        freeNode->size += freeNode->next->size;
        removeFreeNode(freeNode, freeNode->next);
    }
    if (previousNode and (size_t) previousNode + previousNode->size == (size_t) freeNode) {
        previousNode->size += freeNode->size;
        removeFreeNode(previousNode, freeNode);
    }
}

void HeapAllocator::find(size_t size, Node *&previousNode, Node *&foundNode) {
    Node *it = freeList, *prev = nullptr;
    while (it != nullptr) {
        if (it->size >= size)
            break;
        prev = it;
        it = it->next;
    }
    previousNode = prev;
    foundNode = it;
}

void HeapAllocator::insertFreeNode(Node *previousNode, Node *node) {
    if (previousNode == nullptr) {
        node->next = freeList;
        freeList = node;
    } else {
        if (previousNode->next == nullptr) {
            previousNode->next = node;
            node->next = nullptr;
        } else {
            node->next = previousNode->next;
            previousNode->next = node;
        }
    }
}

void HeapAllocator::removeFreeNode(Node *previousNode, Node *node) {
    if (previousNode == nullptr)
        freeList = node->next;
    else
        previousNode->next = node->next;
}
