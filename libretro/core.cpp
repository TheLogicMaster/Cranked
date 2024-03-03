#include "Emulator.hpp"
#include "libretro.h"

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstdarg>
#include <cstring>

static Emulator *emulator;
static std::shared_ptr<Rom> rom;

static retro_log_callback logging;
static retro_log_printf_t log_cb;
static retro_video_refresh_t video_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_environment_t environ_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;

static void fallback_log(retro_log_level level, const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
}

static void emulatorCallback(Emulator *context) {

}

void retro_init() {
    emulator = new Emulator(emulatorCallback);
    emulator->graphics.displayBufferNativeEndian = true;
}

void retro_deinit() {
    delete emulator;
    emulator = nullptr;
    rom.reset();
}

void retro_reset() {
    emulator->reset();
}

void retro_run() {
    input_poll_cb();

    audio_cb(0, 0);

    emulator->update();

    video_cb(emulator->graphics.displayBufferRGBA, DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_WIDTH * sizeof(uint32_t));
}

bool retro_load_game(const retro_game_info *info) {
    auto fmt = RETRO_PIXEL_FORMAT_XRGB8888;
    if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt)) {
        log_cb(RETRO_LOG_ERROR, "XRGB8888 is not supported\n");
        return false;
    }

    emulator->load(info->path);
    emulator->start();

    return true;
}

void retro_unload_game() {
    emulator->unload();
}

uint retro_api_version(void) {
    return RETRO_API_VERSION;
}

void retro_set_controller_port_device(uint port, uint device) {

}

void retro_get_system_info(struct retro_system_info *info) {
    memset(info, 0, sizeof(*info));
    info->library_name = "Cranked";
    info->library_version = "v1";
    info->need_fullpath = true;
    info->block_extract = true;
    info->valid_extensions = "pdz.zip|.zip";
}

void retro_get_system_av_info(struct retro_system_av_info *info) {
    info->timing = {.fps = 60.0, .sample_rate = 0.0};

    info->geometry = {
            .base_width = DISPLAY_WIDTH,
            .base_height = DISPLAY_HEIGHT,
            .max_width = DISPLAY_WIDTH,
            .max_height = DISPLAY_HEIGHT,
            .aspect_ratio = (float) DISPLAY_WIDTH / DISPLAY_HEIGHT,
    };
}

void retro_set_environment(retro_environment_t cb) {
    environ_cb = cb;

    bool no_content = true;
    cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &no_content);

    if (cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logging))
        log_cb = logging.log;
    else
        log_cb = fallback_log;
}

void retro_set_audio_sample(retro_audio_sample_t cb) {
    audio_cb = cb;
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) {
    audio_batch_cb = cb;
}

void retro_set_input_poll(retro_input_poll_t cb) {
    input_poll_cb = cb;
}

void retro_set_input_state(retro_input_state_t cb) {
    input_state_cb = cb;
}

void retro_set_video_refresh(retro_video_refresh_t cb) {
    video_cb = cb;
}

uint retro_get_region() {
    return RETRO_REGION_NTSC;
}

bool retro_load_game_special(uint type, const retro_game_info *info, size_t num) {
    return false;
}

size_t retro_serialize_size() {
    return 0;
}

bool retro_serialize(void *data, size_t size) {
    return true;
}

bool retro_unserialize(const void *data, size_t size) {
    return true;
}

void *retro_get_memory_data(uint id) {
    return nullptr;
}

size_t retro_get_memory_size(uint id) {
    return 0;
}

void retro_cheat_reset() {

}

void retro_cheat_set(uint index, bool enabled, const char *code) {

}
