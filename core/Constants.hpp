#pragma once

#include <cstdint>

// Todo: Utilize namespaces

constexpr double PI = 3.14159265358979323846;

constexpr int DISPLAY_WIDTH = 400;
constexpr int DISPLAY_HEIGHT = 240;
constexpr int DISPLAY_BUFFER_WIDTH = 416;

constexpr float AUDIO_SAMPLING_RATE = 44100;
constexpr float NOTE_C4_FREQUENCY = 261.626f;
constexpr int NOTE_C4_HALF_STEPS = 60;

constexpr uint32_t CODE_ADDRESS = 0x60000020;
constexpr uint32_t CODE_PAGE_ADDRESS = 0x60000000;
constexpr uint32_t CODE_OFFSET = CODE_ADDRESS - CODE_PAGE_ADDRESS;
constexpr uint32_t API_ADDRESS = 0x70000000;
constexpr uint32_t API_MEM_SIZE = 0x4000; // 16KB generous buffer
constexpr uint32_t FUNC_TABLE_ADDRESS = 0x80000000;
constexpr uint32_t STACK_ADDRESS = 0x90000000;
constexpr uint32_t STACK_SIZE = 0x10000; // 61800 rounded up
constexpr uint32_t HEAP_ADDRESS = 0xA0000000;
constexpr uint32_t HEAP_SIZE = 0x1000000; // 16 MB or 16*2^20
