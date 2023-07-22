#pragma once

#include <cstdint>

static constexpr int DISPLAY_WIDTH = 400;
static constexpr int DISPLAY_HEIGHT = 240;
static constexpr int DISPLAY_BUFFER_WIDTH = 416;

static constexpr uint32_t CODE_ADDRESS = 0x60000020;
static constexpr uint32_t CODE_PAGE_ADDRESS = 0x60000000;
static constexpr uint32_t CODE_OFFSET = CODE_ADDRESS - CODE_PAGE_ADDRESS;
static constexpr uint32_t API_ADDRESS = 0x70000000;
static constexpr uint32_t API_MEM_SIZE = 0x4000; // 16KB generous buffer
static constexpr uint32_t FUNC_TABLE_ADDRESS = 0x80000000;
static constexpr uint32_t STACK_ADDRESS = 0x90000000;
static constexpr uint32_t STACK_SIZE = 0x10000; // 61800 rounded up
static constexpr uint32_t HEAP_ADDRESS = 0xA0000000;
static constexpr uint32_t HEAP_SIZE = 0x1000000; // 16 MB or 16*2^20
