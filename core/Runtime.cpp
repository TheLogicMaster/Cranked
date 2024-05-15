#include "Cranked.hpp"
#include "gen/PlaydateAPI.hpp"

#include <filesystem>

using namespace cranked;

LCDVideoPlayer_32 *cranked::playdate_video_loadVideo(Cranked *cranked, uint8_t *path) {
    // Todo
    return nullptr;
}

void cranked::playdate_video_freePlayer(Cranked *cranked, LCDVideoPlayer_32 *p) {
    // Todo
}

int32_t cranked::playdate_video_setContext(Cranked *cranked, LCDVideoPlayer_32 *p, LCDBitmap_32 *context) {
    // Todo
    return 0;
}

void cranked::playdate_video_useScreenContext(Cranked *cranked, LCDVideoPlayer_32 *p) {
    // Todo
}

int32_t cranked::playdate_video_renderFrame(Cranked *cranked, LCDVideoPlayer_32 *p, int32_t n) {
    // Todo
    return 0;
}

uint8_t *cranked::playdate_video_getError(Cranked *cranked, LCDVideoPlayer_32 *p) {
    // Todo
    return nullptr;
}

void cranked::playdate_video_getInfo(Cranked *cranked, LCDVideoPlayer_32 *p, int32_t *outWidth, int32_t *outHeight, float *outFrameRate, int32_t *outFrameCount, int32_t *outCurrentFrame) {
    // Todo
}

LCDBitmap_32 *cranked::playdate_video_getContext(Cranked *cranked, LCDVideoPlayer_32 *p) {
    // Todo
    return nullptr;
}

void cranked::playdate_graphics_clear(Cranked *cranked, uint32_t color) {
    cranked->graphics.frameBufferContext.bitmap->clear(color);
}

void cranked::playdate_graphics_setBackgroundColor(Cranked *cranked, int32_t color) {
    cranked->graphics.getCurrentDisplayContext().backgroundColor = LCDSolidColor(color);
}

void cranked::playdate_graphics_setStencil(Cranked *cranked, LCDBitmap_32 *stencil) {
    cranked->graphics.getCurrentDisplayContext().stencilImage = stencil;
}

void cranked::playdate_graphics_setDrawMode(Cranked *cranked, int32_t mode) {
    cranked->graphics.getCurrentDisplayContext().bitmapDrawMode = LCDBitmapDrawMode(mode);
}

void cranked::playdate_graphics_setDrawOffset(Cranked *cranked, int32_t dx, int32_t dy) {
    cranked->graphics.getCurrentDisplayContext().drawOffset = {dx, dy};
}

void cranked::playdate_graphics_setClipRect(Cranked *cranked, int32_t x, int32_t y, int32_t width, int32_t height) {
    cranked->graphics.getCurrentDisplayContext().clipRect = {x, y, width, height};
}

void cranked::playdate_graphics_clearClipRect(Cranked *cranked) {
    cranked->graphics.getCurrentDisplayContext().clipRect = {};
}

void cranked::playdate_graphics_setLineCapStyle(Cranked *cranked, int32_t endCapStyle) {
    cranked->graphics.getCurrentDisplayContext().lineEndCapStype = LCDLineCapStyle(endCapStyle);
}

void cranked::playdate_graphics_setFont(Cranked *cranked, LCDFont_32 *font) {
    cranked->graphics.getCurrentDisplayContext().font = font;
}

void cranked::playdate_graphics_setTextTracking(Cranked *cranked, int32_t tracking) {
    cranked->graphics.getCurrentDisplayContext().textTracking = tracking;
}

void cranked::playdate_graphics_pushContext(Cranked *cranked, LCDBitmap_32 *target) {
    cranked->graphics.pushContext(target);
}

void cranked::playdate_graphics_popContext(Cranked *cranked) {
    cranked->graphics.popContext();
}

void cranked::playdate_graphics_drawBitmap(Cranked *cranked, LCDBitmap_32 *bitmap, int32_t x, int32_t y, int32_t flip) {
    cranked->graphics.drawBitmap(bitmap, x, y, LCDBitmapFlip(flip));
}

void cranked::playdate_graphics_tileBitmap(Cranked *cranked, LCDBitmap_32 *bitmap, int32_t x, int32_t y, int32_t width, int32_t height, int32_t flip) {
    // Todo
}

void cranked::playdate_graphics_drawLine(Cranked *cranked, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t width, uint32_t color) {
    cranked->graphics.drawLine(x1, y1, x2, y2, width, color);
}

void cranked::playdate_graphics_fillTriangle(Cranked *cranked, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint32_t color) {
    cranked->graphics.fillTriangle(x1, y1, x2, y2, x3, y3, color);
}

void cranked::playdate_graphics_drawRect(Cranked *cranked, int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color) {
    cranked->graphics.drawRect(x, y, width, height, color);
}

void cranked::playdate_graphics_fillRect(Cranked *cranked, int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color) {
    cranked->graphics.fillRect(x, y, width, height, color);
}

void cranked::playdate_graphics_drawEllipse(Cranked *cranked, int32_t x, int32_t y, int32_t width, int32_t height, int32_t lineWidth, float startAngle, float endAngle, uint32_t color) {
    cranked->graphics.drawEllipse(x, y, width, height, lineWidth, startAngle, endAngle, color, false);
}

void cranked::playdate_graphics_fillEllipse(Cranked *cranked, int32_t x, int32_t y, int32_t width, int32_t height, float startAngle, float endAngle, uint32_t color) {
    cranked->graphics.drawEllipse(x, y, width, height, 0, startAngle, endAngle, color, true);
}

void cranked::playdate_graphics_drawScaledBitmap(Cranked *cranked, LCDBitmap_32 *bitmap, int32_t x, int32_t y, float xscale, float yscale) {
    // Todo
}

int32_t cranked::playdate_graphics_drawText(Cranked *cranked, void *text, uint32_t len, int32_t encoding, int32_t x, int32_t y) {
    cranked->graphics.drawText(text, len, PDStringEncoding(encoding), x, y);
    return 0; // Todo: Return value?
}

LCDBitmap_32 *cranked::playdate_graphics_newBitmap(Cranked *cranked, int32_t width, int32_t height, uint32_t bgcolor) {
    auto bitmap = cranked->graphics.allocateBitmap(width, height);
    bitmap->clear(bgcolor);
    return bitmap;
}

void cranked::playdate_graphics_freeBitmap(Cranked *cranked, LCDBitmap_32 *ptr) {
    cranked->graphics.freeBitmap(ptr);
}

LCDBitmap_32 *cranked::playdate_graphics_loadBitmap(Cranked *cranked, uint8_t *path, cref_t *outerr) {
    try {
        return cranked->graphics.getImage((const char *) path);
    } catch (std::exception &ex) {
        *outerr = cranked->getEmulatedStringLiteral(ex.what());
        return nullptr;
    }
}

LCDBitmap_32 *cranked::playdate_graphics_copyBitmap(Cranked *cranked, LCDBitmap_32 *bitmap) {
    return cranked->heap.construct<LCDBitmap_32>(*bitmap);
}

void cranked::playdate_graphics_loadIntoBitmap(Cranked *cranked, uint8_t *path, LCDBitmap_32 *bitmap, cref_t *outerr) {
    try {
        auto loaded = cranked->graphics.getImage((const char *) path);
        *bitmap = *loaded;
        cranked->graphics.freeBitmap(loaded);
    } catch (std::exception &ex) {
        if (outerr)
            *outerr = cranked->getEmulatedStringLiteral(ex.what());
    }
}

void cranked::playdate_graphics_getBitmapData(Cranked *cranked, LCDBitmap_32 *bitmap, int32_t *width, int32_t *height, int32_t *rowbytes, cref_t *mask, cref_t *data) {
    *width = bitmap->width;
    *height = bitmap->height;
    *rowbytes = 1 + bitmap->width / 8;
    *data = cranked->toVirtualAddress(bitmap->data.data());
    *mask = cranked->toVirtualAddress(bitmap->mask ? bitmap->mask->data.data() : nullptr);
}

void cranked::playdate_graphics_clearBitmap(Cranked *cranked, LCDBitmap_32 *bitmap, uint32_t bgcolor) {
    bitmap->clear(bgcolor);
}

LCDBitmap_32 *cranked::playdate_graphics_rotatedBitmap(Cranked *cranked, LCDBitmap_32 *bitmap, float rotation, float xscale, float yscale, int32_t *allocedSize) {
    // Todo
    return nullptr;
}

LCDBitmapTable_32 *cranked::playdate_graphics_newBitmapTable(Cranked *cranked, int32_t count, int32_t width, int32_t height) {
    return cranked->graphics.allocateBitmapTable(count);
}

void cranked::playdate_graphics_freeBitmapTable(Cranked *cranked, LCDBitmapTable_32 *table) {
    cranked->graphics.freeBitmapTable(table);
}

LCDBitmapTable_32 *cranked::playdate_graphics_loadBitmapTable(Cranked *cranked, uint8_t *path, cref_t *outerr) {
    try {
        return cranked->graphics.getBitmapTable((const char *) path);
    } catch (std::exception &ex) {
        *outerr = cranked->getEmulatedStringLiteral(ex.what());
        return nullptr;
    }
}

void cranked::playdate_graphics_loadIntoBitmapTable(Cranked *cranked, uint8_t *path, LCDBitmapTable_32 *table, cref_t *outerr) {
    try {
        auto loaded = cranked->graphics.getBitmapTable((const char *) path);
        *table = *loaded;
        cranked->graphics.freeBitmapTable(loaded);
    } catch (std::exception &ex) {
        *outerr = cranked->getEmulatedStringLiteral(ex.what());
    }
}

LCDBitmap_32 *cranked::playdate_graphics_getTableBitmap(Cranked *cranked, LCDBitmapTable_32 *table, int32_t idx) {
    return idx < table->bitmaps.size() ? &table->bitmaps[idx] : nullptr;
}

LCDFont_32 *cranked::playdate_graphics_loadFont(Cranked *cranked, uint8_t *path, cref_t *outerr) {
    try {
        return cranked->graphics.getFont((const char *) path);
    } catch (std::exception &ex) {
        *outerr = cranked->getEmulatedStringLiteral(ex.what());
        return nullptr;
    }
}

LCDFontPage_32 *cranked::playdate_graphics_getFontPage(Cranked *cranked, LCDFont_32 *font, uint32_t c) {
    try {
        return &font->pages.at(c / 256);
    } catch (std::out_of_range &ex) { // Todo: Don't use exceptions
        return nullptr;
    }
}

LCDFontGlyph_32 *cranked::playdate_graphics_getPageGlyph(Cranked *cranked, LCDFontPage_32 *page, uint32_t c, cref_t *bitmap, int32_t *advance) {
    try {
        auto glyph = &page->glyphs.at(c % 256);
        if (bitmap)
            *bitmap = cranked->toVirtualAddress(&glyph->bitmap);
        if (advance)
            *advance = glyph->advance;
        return glyph;
    } catch (std::out_of_range &ex) { // Todo: Don't use exceptions
        return nullptr;
    }
}

int32_t cranked::playdate_graphics_getGlyphKerning(Cranked *cranked, LCDFontGlyph_32 *glyph, uint32_t glyphcode, uint32_t nextcode) {
    try {
        return nextcode < 256 ? glyph->shortKerningTable.at(nextcode) : glyph->longKerningTable.at(nextcode);
    } catch (std::out_of_range &ex) { // Todo: Don't use exceptions
        return 0;
    }
}

int32_t cranked::playdate_graphics_getTextWidth(Cranked *cranked, LCDFont_32 *font, void *text, uint32_t len, int32_t encoding, int32_t tracking) {
    // Todo: Support other encodings
    int size = 0;
    auto string = (const char *) text;
    while (*string) {
        LCDFontGlyph_32 *glyph{};
        char character = *string;
        size += font->glyphWidth;
        try {
            glyph = &font->pages.at(character / 256).glyphs.at(character % 256);
        } catch (std::out_of_range &ex) {}
        if (glyph) {
            size += glyph->advance;
            // Todo: Kerning
        }
        string++;
        if (*string)
            size += tracking;
    }
    return size;
}

uint8_t *cranked::playdate_graphics_getFrame(Cranked *cranked) {
    return (uint8_t *) cranked->graphics.frameBuffer->data.data();
}

uint8_t *cranked::playdate_graphics_getDisplayFrame(Cranked *cranked) {
    return (uint8_t *) cranked->graphics.previousFrameBuffer->data.data();
}

LCDBitmap_32 *cranked::playdate_graphics_copyFrameBufferBitmap(Cranked *cranked) {
    return cranked->heap.construct<LCDBitmap_32>(*cranked->graphics.frameBuffer);
}

void cranked::playdate_graphics_markUpdatedRows(Cranked *cranked, int32_t start, int32_t end) {
    // Not needed, since getFrame() returns the actual frame buffer
}

void cranked::playdate_graphics_display(Cranked *cranked) {
    cranked->graphics.flushDisplayBuffer();
}

void cranked::playdate_graphics_setColorToPattern(Cranked *cranked, uint32_t *color, LCDBitmap_32 *bitmap, int32_t x, int32_t y) {
    // Todo
}

int32_t cranked::playdate_graphics_checkMaskCollision(Cranked *cranked, LCDBitmap_32 *bitmap1, int32_t x1, int32_t y1, int32_t flip1, LCDBitmap_32 *bitmap2, int32_t x2, int32_t y2, int32_t flip2, LCDRect_32 rect) {
    // Todo
    return 0;
}

void cranked::playdate_graphics_setScreenClipRect(Cranked *cranked, int32_t x, int32_t y, int32_t width, int32_t height) {
    cranked->graphics.getCurrentDisplayContext().clipRect = {x, y, width, height};
}

void cranked::playdate_graphics_fillPolygon(Cranked *cranked, int32_t nPoints, int32_t *coords, uint32_t color, int32_t fillrule) {
    // Todo
}

uint8_t cranked::playdate_graphics_getFontHeight(Cranked *cranked, LCDFont_32 *font) {
    // Todo
    return 0;
}

LCDBitmap_32 *cranked::playdate_graphics_getDisplayBufferBitmap(Cranked *cranked) {
    return cranked->graphics.frameBuffer;
}

void cranked::playdate_graphics_drawRotatedBitmap(Cranked *cranked, LCDBitmap_32 *bitmap, int32_t x, int32_t y, float rotation, float centerx, float centery, float xscale, float yscale) {
    // Todo
}

void cranked::playdate_graphics_setTextLeading(Cranked *cranked, int32_t lineHeightAdustment) {
    cranked->graphics.getCurrentDisplayContext().textLeading = lineHeightAdustment;
}

int32_t cranked::playdate_graphics_setBitmapMask(Cranked *cranked, LCDBitmap_32 *bitmap, LCDBitmap_32 *mask) {
    if (bitmap->mask)
        cranked->graphics.freeBitmap(bitmap->mask);
    bitmap->mask = mask; // Todo: Make copy?
    return 0; // Todo: Return value?
}

LCDBitmap_32 *cranked::playdate_graphics_getBitmapMask([[maybe_unused]] Cranked *cranked, LCDBitmap_32 *bitmap) {
    return bitmap->mask;
}

void cranked::playdate_graphics_setStencilImage(Cranked *cranked, LCDBitmap_32 *stencil, int32_t tile) {
    cranked->graphics.getCurrentDisplayContext().stencilImage = stencil;
    cranked->graphics.getCurrentDisplayContext().stencilTiled = tile;
}

LCDFont_32 *cranked::playdate_graphics_makeFontFromData(Cranked *cranked, LCDFontData_32 *data, int32_t wide) {
    return cranked->graphics.getFont((uint8_t *) data, wide);
}

int32_t cranked::playdate_graphics_getTextTracking(Cranked *cranked) {
    return cranked->graphics.getCurrentDisplayContext().textTracking;
}

void *cranked::playdate_sys_realloc(Cranked *cranked, void* ptr, uint32_t size) {
    // Todo: Check for objects that need to be user freed and call destructors
    if (size == 0) {
        if (!cranked->graphics.handleFree(ptr)) // Handle allocated objects that need to be freed with `system->realloc`
            cranked->heap.free(ptr);
        return nullptr;
    } else
        return cranked->heap.reallocate(ptr, size);
}

int32_t cranked::playdate_sys_formatString(Cranked *cranked, cref_t *ret, uint8_t *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    auto size = vsnprintf(nullptr, 0, (const char *) fmt, args);
    auto buffer = (uint8_t *) cranked->heap.allocate(size + 1);
    *ret = cranked->toVirtualAddress(buffer);
    vsprintf((char *) buffer, (const char *) fmt, args);
    va_end(args);
    return size;
}

// Todo: Args are passed as emulated sizes, which will cause issues when `int` size is different, and such
// Todo: Probably needs conversion when calling vararg functions
void cranked::playdate_sys_logToConsole([[maybe_unused]] Cranked *cranked, uint8_t *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf((const char *) fmt, args);
    va_end(args);
    printf("\n");// Doesn't depend on Lua setNewlinePrinted flag
}

void cranked::playdate_sys_error(Cranked *cranked, uint8_t *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf((const char *) fmt, args);
    va_end(args);
    cranked->stop();
}

int32_t cranked::playdate_sys_getLanguage([[maybe_unused]] Cranked *cranked) {
    return (int) PDLanguage::English;
}

uint32_t cranked::playdate_sys_getCurrentTimeMilliseconds(Cranked *cranked) {
    return cranked->currentMillis;
}

uint32_t cranked::playdate_sys_getSecondsSinceEpoch([[maybe_unused]] Cranked *cranked, uint32_t *milliseconds) {
    return duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

void cranked::playdate_sys_drawFPS(Cranked *cranked, int32_t x, int32_t y) {
    // Todo: Fix (There's probably a memory bug involved here...)
    auto delta = duration_cast<std::chrono::milliseconds>((std::chrono::system_clock::now() - cranked->lastFrameTime)).count();
    auto string = std::to_string(int(1000.0f / delta));
    cranked->graphics.fillRect(x - 2, y - 2, 14 * string.size() + 2, 30, LCDSolidColor::White);
    cranked->graphics.pushContext(cranked->graphics.getTargetBitmap());
    cranked->graphics.getCurrentDisplayContext().bitmapDrawMode = LCDBitmapDrawMode::Copy;
    cranked->graphics.drawText(string.c_str(), string.size(), PDStringEncoding::ASCII, x, y, cranked->graphics.systemFont);
    cranked->graphics.popContext();
}

void cranked::playdate_sys_setUpdateCallback(Cranked *cranked, cref_t update, void *userdata) {
    cranked->nativeEngine.setUpdateCallback(update, cranked->toVirtualAddress(userdata));
}

void cranked::playdate_sys_getButtonState(Cranked *cranked, int32_t *current, int32_t *pushed, int32_t *released) {
    *current = cranked->currentInputs;
    *pushed = cranked->pressedInputs;
    *released = cranked->releasedInputs;
}

void cranked::playdate_sys_setPeripheralsEnabled(Cranked *cranked, int32_t mask) {
    // Not needed
}

void cranked::playdate_sys_getAccelerometer(Cranked *cranked, float *outx, float *outy, float *outz) {
    *outx = cranked->accelerometerX;
    *outy = cranked->accelerometerY;
    *outz = cranked->accelerometerZ;
}

float cranked::playdate_sys_getCrankChange(Cranked *cranked) {
    return cranked->getCrankChange();
}

float cranked::playdate_sys_getCrankAngle(Cranked *cranked) {
    return cranked->crankAngle;
}

int32_t cranked::playdate_sys_isCrankDocked(Cranked *cranked) {
    return cranked->crankDocked;
}

int32_t cranked::playdate_sys_setCrankSoundsDisabled(Cranked *cranked, int32_t flag) {
    auto prev = cranked->crankSoundsEnabled;
    cranked->crankSoundsEnabled = flag;
    return prev;
}

int32_t cranked::playdate_sys_getFlipped(Cranked *cranked) {
    return cranked->systemFlipped;
}

void cranked::playdate_sys_setAutoLockDisabled(Cranked *cranked, int32_t disable) {
    cranked->autoLockDisabled = disable;
}

void cranked::playdate_sys_setMenuImage(Cranked *cranked, LCDBitmap_32 *bitmap, int32_t xOffset) {
    cranked->menu.setImage(bitmap, xOffset);
}

PDMenuItem_32 *cranked::playdate_sys_addMenuItem(Cranked *cranked, uint8_t *title, cref_t callback, void *userdata) {
    return cranked->menu.addItem((char *) title, PDMenuItem_32::Type::Button, {}, 0, callback, 0);
}

PDMenuItem_32 *cranked::playdate_sys_addCheckmarkMenuItem(Cranked *cranked, uint8_t *title, int32_t value, cref_t callback, void *userdata) {
    return cranked->menu.addItem((char *) title, PDMenuItem_32::Type::Checkmark, {}, value, callback, 0);
}

PDMenuItem_32 *cranked::playdate_sys_addOptionsMenuItem(Cranked *cranked, uint8_t *title, cref_t *optionTitles, int32_t optionsCount, cref_t f, void *userdata) {
    std::vector<std::string> options;
    for (int i = 0; i < optionsCount; i++)
        options.emplace_back((char *) cranked->fromVirtualAddress(optionTitles[i]));
    return cranked->menu.addItem((char *) title, PDMenuItem_32::Type::Options, options, 0, f, 0);
}

void cranked::playdate_sys_removeAllMenuItems(Cranked *cranked) {
    cranked->menu.clearItems();
}

void cranked::playdate_sys_removeMenuItem(Cranked *cranked, PDMenuItem_32 *menuItem) {
    cranked->menu.removeItem(menuItem);
}

int32_t cranked::playdate_sys_getMenuItemValue(Cranked *cranked, PDMenuItem_32 *menuItem) {
    return menuItem->value;
}

void cranked::playdate_sys_setMenuItemValue(Cranked *cranked, PDMenuItem_32 *menuItem, int32_t value) {
    menuItem->value = value;
}

uint8_t *cranked::playdate_sys_getMenuItemTitle(Cranked *cranked, PDMenuItem_32 *menuItem) {
    return (uint8_t *) menuItem->title;
}

void cranked::playdate_sys_setMenuItemTitle(Cranked *cranked, PDMenuItem_32 *menuItem, uint8_t *title) {
    cranked->menu.setItemTitle(menuItem, (char *) title);
}

void *cranked::playdate_sys_getMenuItemUserdata(Cranked *cranked, PDMenuItem_32 *menuItem) {
    return cranked->fromVirtualAddress(menuItem->userdata);
}

void cranked::playdate_sys_setMenuItemUserdata(Cranked *cranked, PDMenuItem_32 *menuItem, void *ud) {
    menuItem->userdata = cranked->toVirtualAddress(ud);
}

int32_t cranked::playdate_sys_getReduceFlashing(Cranked *cranked) {
    return cranked->tryReduceFlashing;
}

float cranked::playdate_sys_getElapsedTime(Cranked *cranked) {
    return duration_cast<std::chrono::microseconds>((std::chrono::system_clock::now() - cranked->elapsedTimeStart)).count() / 1000000.0f;
}

void cranked::playdate_sys_resetElapsedTime(Cranked *cranked) {
    cranked->elapsedTimeStart = std::chrono::system_clock::now();
}

float cranked::playdate_sys_getBatteryPercentage(Cranked *cranked) {
    return cranked->batteryPercentage;
}

float cranked::playdate_sys_getBatteryVoltage(Cranked *cranked) {
    return cranked->batteryVoltage;
}

int32_t cranked::playdate_sys_getTimezoneOffset(Cranked *cranked) {
    return cranked->timezoneOffset;
}

int32_t cranked::playdate_sys_shouldDisplay24HourTime(Cranked *cranked) {
    return cranked->use24HourTime;
}

void cranked::playdate_sys_convertEpochToDateTime(Cranked *cranked, uint32_t epoch, PDDateTime_32 *datetime) {
    // Todo
}

uint32_t cranked::playdate_sys_convertDateTimeToEpoch(Cranked *cranked, PDDateTime_32 *datetime) {
    // Todo
    return 0;
}

void cranked::playdate_sys_clearICache(Cranked *cranked) {
    // Todo
}

void cranked::playdate_sys_setButtonCallback(Cranked *cranked, cref_t cb, void *buttonud, int32_t queuesize) {
    cranked->buttonCallback = cb;
    cranked->buttonCallbackUserdata = cranked->toVirtualAddress(buttonud);
    cranked->buttonCallbackQueueSize = queuesize;
}

void cranked::playdate_sys_setSerialMessageCallback(Cranked *cranked, cref_t callback) {
    cranked->serialMessageCallback = callback;
}

int32_t cranked::playdate_sys_vaFormatString(Cranked *cranked, cref_t *outstr, uint8_t *fmt, ...) {
    char *buffer{};
    va_list list{};
    va_start(list, fmt);
    int size = vasprintf(&buffer, (char *)fmt, list);
    va_end(list);
    if (size < 0)
        return -1;
    try {
        void *string = cranked->heap.allocate(size);
        memcpy(string, buffer, size);
        *outstr = cranked->toVirtualAddress(string);
    } catch (std::bad_alloc &ex) {
        size = -1;
    } catch (...) {
        free(buffer);
        throw;
    }
    free(buffer);
    return size;
}

int32_t cranked::playdate_sys_parseString(Cranked *cranked, uint8_t *str, uint8_t *fmt, ...) {
    va_list list{};
    va_start(list, fmt);
    auto string = (const char *) str;
    auto format = (const char *) fmt;
    int stringIndex = 0;
    int formatIndex = 0;
    int stringLen = strlen(string);
    int formatLen = strlen(format);
    int matches = 0;

    while (formatIndex < formatLen) {
        char c = format[formatIndex++];

        if (isspace(c)) {
            while (formatIndex < formatLen and isspace(format[formatIndex]))
                formatIndex++;
            if (stringIndex >= stringLen or !isspace(string[stringIndex++]))
                break;
            while (stringIndex < stringLen and isspace(string[stringIndex]))
                stringIndex++;
            continue;
        } else if (c != '%') {
            if (stringIndex >= stringLen or string[stringIndex++] != c)
                break;
            continue;
        }

        if (formatIndex >= formatLen)
            break;
        c = format[formatIndex++];

        bool ignored = false;
        if (c == '*') {
            ignored = true;
            if (formatIndex >= formatLen)
                break;
            c = format[formatIndex++];
        }

        int widthStart = formatIndex - 1;
        int width = 0;
        if (isdigit(c)) {
            char *end{};
            strtol(format + widthStart, &end, 10);
            width = formatIndex = (end - format) - widthStart; // Todo: Verify
            c = format[formatIndex++];
        }

        bool failed = false;
        switch(c) {
            case '%':
                if (stringIndex >= stringLen or string[stringIndex++] != '%')
                    failed = true;
                break;
            case 'c':
                if (width == 0)
                    width = 1;
                if (stringLen - stringIndex < width) {
                    failed = true;
                    break;
                }
                if (!ignored)
                    memcpy(va_arg(list, char *), string + stringIndex, width);
                stringIndex += width;
                matches++;
                break;
            case 's': {
                int i = stringIndex;
                while (i < stringLen) {
                    if (isspace(string[i]))
                        break;
                    i++;
                }
                if (i == stringIndex) {
                    failed = true;
                    break;
                }
                if (!ignored) {
                    auto dest = va_arg(list, char *);
                    memcpy(dest, string + stringIndex, i - stringIndex);
                    dest[i - stringIndex] = 0;
                }
                stringIndex = i;
                matches++;
                break;
            }
            case '[': { // Not supporting `-` ranges
                bool inverted = false;
                if (formatIndex < formatLen and format[formatIndex] == '^') {
                    inverted = true;
                    formatIndex++;
                }
                std::set<char> set;
                if (formatIndex < formatLen and format[formatIndex] == ']') {
                    set.emplace(']');
                    formatIndex++;
                }
                while (true) {
                    if (formatIndex >= formatLen) {
                        failed = true;
                        break;
                    }
                    c = format[formatIndex++];
                    if (c == ']')
                        break;
                    set.emplace(c);
                }
                if (failed)
                    break;
                int i = stringIndex;
                while (i < stringLen and (width == 0 or i < stringIndex + width)) {
                    if (set.contains(string[i]) == inverted)
                        break;
                    i++;
                }
                if (i == stringIndex)
                    failed = true;
                else {
                    auto dest = va_arg(list, char*);
                    memcpy(dest, string + stringIndex, i - stringIndex);
                    dest[i - stringIndex] = 0;
                    stringIndex = i;
                    matches++;
                }
                break;
            }
            case 'd':
            case 'i':
            case 'u':
            case 'o':
            case 'x':
            case 'X': {
                int start = stringIndex;
                while (stringIndex < stringLen) {
                    char modifier = string[stringIndex];
                    if (modifier != 'h' and modifier != 'l' and modifier != 'j' and modifier != 'z' and modifier != 't' and modifier != 'L')
                        break;
                    stringIndex++;
                }
                int modifierCount = stringIndex - start;
                int64_t result{};
                uint64_t uresult{};
                char *end{};
                switch (c) {
                    default:
                    case 'd':
                        result = strtol(string + stringIndex, &end, 10);
                        break;
                    case 'i':
                        result = strtol(string + stringIndex, &end, 0);
                        break;
                    case 'u':
                        uresult = strtoul(string + stringIndex, &end, 10);
                        break;
                    case 'o':
                        uresult = strtoul(string + stringIndex, &end, 8);
                        break;
                    case 'x':
                    case 'X':
                        uresult = strtoul(string + stringIndex, &end, 16);
                        break;
                }
                if (end == string + stringIndex) {
                    failed = true;
                    break;
                }
                ArgType type;
                if (modifierCount == 0)
                    type = c == 'd' or c == 'i' ? ArgType::int32_t : ArgType::uint32_t;
                else if (modifierCount == 1) {
                    char mod = string[start];
                    if (mod == 'h')
                        type = c == 'd' or c == 'i' ? ArgType::int16_t : ArgType::uint16_t;
                    else if (mod == 'l')
                        type = c == 'd' or c == 'i' ? ArgType::int32_t : ArgType::uint32_t;
                    else if (mod == 'j')
                        type = c == 'd' or c == 'i' ? ArgType::int64_t : ArgType::uint64_t;
                    else if (mod == 't')
                        type = ArgType::int32_t;
                    else if (mod == 'z')
                        type = ArgType::uint32_t;
                    else {
                        failed = true;
                        break;
                    }
                } else if (modifierCount == 2) {
                    char first = string[start];
                    char second = string[start + 1];
                    if (first != second or (first != 'h' and first != 'l')) {
                        failed = true;
                        break;
                    }
                    if (first == 'h')
                        type = c == 'd' or c == 'i' ? ArgType::int8_t : ArgType::uint8_t;
                    else
                        type = c == 'd' or c == 'i' ? ArgType::int64_t : ArgType::uint64_t;
                } else {
                    failed = true;
                    break;
                }
                if (!ignored) {
                    switch (type) {
                        default:
                        case ArgType::int8_t:
                            *va_arg(list, int8_t *) = result;
                            break;
                        case ArgType::uint8_t:
                            *va_arg(list, uint8_t *) = result;
                            break;
                        case ArgType::int16_t:
                            *va_arg(list, int16_t *) = result;
                            break;
                        case ArgType::uint16_t:
                            *va_arg(list, uint16_t *) = result;
                            break;
                        case ArgType::int32_t:
                            *va_arg(list, int32_t *) = result;
                            break;
                        case ArgType::uint32_t:
                            *va_arg(list, uint32_t *) = result;
                            break;
                        case ArgType::int64_t:
                            *va_arg(list, int64_t *) = result;
                            break;
                        case ArgType::uint64_t:
                            *va_arg(list, uint64_t *) = result;
                            break;
                    }
                }
                stringIndex = end - string;
                matches++;
                break;
            }
            case 'n':
                if (!ignored)
                    *va_arg(list, int32_t *) = stringIndex;
                break;
            case 'a':
            case 'A':
            case 'e':
            case 'E':
            case 'f':
            case 'F':
            case 'g':
            case 'G': {
                bool wide = false; // Ignoring `long double`
                if (stringIndex < stringLen and string[stringIndex] == 'l') {
                    wide = true;
                    stringIndex++;
                }
                char *end{};
                if (wide) {
                    double value = strtod(string + stringIndex, &end);
                    if (end == string + stringIndex) {
                        failed = true;
                        break;
                    }
                    if (!ignored)
                        *va_arg(list, double *) = value;
                } else {
                    float value = strtof(string + stringIndex, &end);
                    if (end == string + stringIndex) {
                        failed = true;
                        break;
                    }
                    if (!ignored)
                        *va_arg(list, float *) = value;
                }
                stringIndex = end - string;
                matches++;
                break;
            }
            case 'p': {
                char *end{};
                cref_t ptr = strtol(string + stringIndex, &end, 16);
                if (end == string + stringIndex) {
                    failed = true;
                    break;
                }
                if (!ignored)
                    *va_arg(list, cref_t *) = ptr;
                stringIndex = end - string;
                matches++;
                break;
            }
            default:
                failed = true;
                break;
        }
        if (failed)
            break;
    }

    va_end(list);

    return matches;
}

int32_t cranked::playdate_lua_addFunction(Cranked *cranked, cref_t f, uint8_t *name, cref_t *outErr) {
    if (!cranked->getLuaContext()) {
        if (outErr)
            *outErr = cranked->getEmulatedStringLiteral("Lua environment not initialized");
        return 0;
    }
    cranked->nativeEngine.pushEmulatedLuaFunction(f);
    cranked->luaEngine.setQualifiedLuaGlobal((const char *) name);
    if (outErr)
        *outErr = 0;
    return 1;
}

int32_t cranked::playdate_lua_registerClass(Cranked *cranked, uint8_t *name, lua_reg_32 *reg, lua_val_32 *vals, int32_t isstatic, cref_t *outErr) {
    if (!cranked->getLuaContext()) {
        if (outErr)
            *outErr = cranked->getEmulatedStringLiteral("Lua environment not initialized");
        return 0;
    }
    // Todo
    return 0;
}

void cranked::playdate_lua_pushFunction(Cranked *cranked, cref_t f) {
    if (cranked->getLuaContext())
        cranked->nativeEngine.pushEmulatedLuaFunction(f);
}

int32_t cranked::playdate_lua_indexMetatable(Cranked *cranked) {
    // Todo
    return 0;
}

void cranked::playdate_lua_stop(Cranked *cranked) {
    cranked->disableUpdateLoop = true;
}

void cranked::playdate_lua_start(Cranked *cranked) {
    cranked->disableUpdateLoop = false;
}

int32_t cranked::playdate_lua_getArgCount(Cranked *cranked) {
    if (!cranked->getLuaContext())
        return 0;
    return lua_gettop(cranked->getLuaContext());
}

int32_t cranked::playdate_lua_getArgType(Cranked *cranked, int32_t pos, cref_t *outClass) {
    if (!cranked->getLuaContext())
        return 0;
    return 0;
}

int32_t cranked::playdate_lua_argIsNil(Cranked *cranked, int32_t pos) {
    if (!cranked->getLuaContext())
        return 0;
    return lua_isnil(cranked->getLuaContext(), pos);
}

int32_t cranked::playdate_lua_getArgBool(Cranked *cranked, int32_t pos) {
    if (!cranked->getLuaContext())
        return 0;
    return lua_toboolean(cranked->getLuaContext(), pos);
}

int32_t cranked::playdate_lua_getArgInt(Cranked *cranked, int32_t pos) {
    if (!cranked->getLuaContext())
        return 0;
    return lua_tointeger(cranked->getLuaContext(), pos);
}

float cranked::playdate_lua_getArgFloat(Cranked *cranked, int32_t pos) {
    if (!cranked->getLuaContext())
        return 0;
    return lua_tonumber(cranked->getLuaContext(), pos);
}

uint8_t *cranked::playdate_lua_getArgString(Cranked *cranked, int32_t pos) {
    if (!cranked->getLuaContext())
        return nullptr;
    return (uint8_t *) lua_tostring(cranked->getLuaContext(), pos);
}

uint8_t *cranked::playdate_lua_getArgBytes(Cranked *cranked, int32_t pos, uint32_t *outlen) {
    if (!cranked->getLuaContext())
        return nullptr;
    size_t len;
    auto string = lua_tolstring(cranked->getLuaContext(), pos, &len);
    *outlen = len;
    return (uint8_t *) string;
}

void *cranked::playdate_lua_getArgObject(Cranked *cranked, int32_t pos, uint8_t *type, cref_t *outud) {
    if (!cranked->getLuaContext())
        return nullptr;
    // Todo
    return nullptr;
}

LCDBitmap_32 *cranked::playdate_lua_getBitmap(Cranked *cranked, int32_t pos) {
    if (!cranked->getLuaContext())
        return nullptr;
    // Todo
    return nullptr;
}

LCDSprite_32 *cranked::playdate_lua_getSprite(Cranked *cranked, int32_t pos) {
    if (!cranked->getLuaContext())
        return nullptr;
    // Todo
    return nullptr;
}

void cranked::playdate_lua_pushNil(Cranked *cranked) {
    if (!cranked->getLuaContext())
        return;
    lua_pushnil(cranked->getLuaContext());
}

void cranked::playdate_lua_pushBool(Cranked *cranked, int32_t val) {
    if (!cranked->getLuaContext())
        return;
    lua_pushboolean(cranked->getLuaContext(), val);
}

void cranked::playdate_lua_pushInt(Cranked *cranked, int32_t val) {
    if (!cranked->getLuaContext())
        return;
    lua_pushinteger(cranked->getLuaContext(), val);
}

void cranked::playdate_lua_pushFloat(Cranked *cranked, float val) {
    if (!cranked->getLuaContext())
        return;
    lua_pushnumber(cranked->getLuaContext(), val);
}

void cranked::playdate_lua_pushString(Cranked *cranked, uint8_t *str) {
    if (!cranked->getLuaContext())
        return;
    lua_pushstring(cranked->getLuaContext(), (const char *) str);
}

void cranked::playdate_lua_pushBytes(Cranked *cranked, uint8_t *str, uint32_t len) {
    if (!cranked->getLuaContext())
        return;
    lua_pushlstring(cranked->getLuaContext(), (const char *) str, len);
}

void cranked::playdate_lua_pushBitmap(Cranked *cranked, LCDBitmap_32 *bitmap) {
    if (!cranked->getLuaContext())
        return;
    // Todo: Ownership?
}

void cranked::playdate_lua_pushSprite(Cranked *cranked, LCDSprite_32 *sprite) {
    if (!cranked->getLuaContext())
        return;
    // Todo: Ownership?
}

LuaUDObject_32 *cranked::playdate_lua_pushObject(Cranked *cranked, void* obj, uint8_t * type, int32_t nValues) {
    if (!cranked->getLuaContext())
        return nullptr;
    // Todo
    return nullptr;
}

LuaUDObject_32 *cranked::playdate_lua_retainObject(Cranked *cranked, LuaUDObject_32 *obj) {
    if (!cranked->getLuaContext())
        return nullptr;
    // Todo: Presumably this should hold a strong reference in an internal global table so object isn't GC'd and so it can be referenced with a C pointer off the stack
    return nullptr;
}

void cranked::playdate_lua_releaseObject(Cranked *cranked, LuaUDObject_32 *obj) {
    if (!cranked->getLuaContext())
        return;
    // Todo
}

void cranked::playdate_lua_setUserValue(Cranked *cranked, LuaUDObject_32 *obj, uint32_t slot) {
    if (!cranked->getLuaContext())
        return;
    // Todo
}

int32_t cranked::playdate_lua_getUserValue(Cranked *cranked, LuaUDObject_32 *obj, uint32_t slot) {
    if (!cranked->getLuaContext())
        return 0;
    // Todo
    return 0;
}

void cranked::playdate_lua_callFunction_deprecated(Cranked *cranked, uint8_t *name, int32_t nargs) {
    if (!cranked->getLuaContext())
        return;
    cranked->luaEngine.getQualifiedLuaGlobal((const char *) name);
    lua_pcall(cranked->getLuaContext(), nargs, LUA_MULTRET, 0);
}

int32_t cranked::playdate_lua_callFunction(Cranked *cranked, uint8_t *name, int32_t nargs, cref_t *outerr) {
    if (!cranked->getLuaContext()) {
        if (outerr)
            *outerr = cranked->getEmulatedStringLiteral("Lua environment not initialized");
        return 0;
    }
    if (nargs > 0)
        lua_pushnil(cranked->getLuaContext());
    for (int i = 0; i < nargs; i++)
        lua_copy(cranked->getLuaContext(), -i - 2, -i - 1);
    cranked->luaEngine.getQualifiedLuaGlobal((const char *) name);
    if (nargs > 0) {
        lua_copy(cranked->getLuaContext(), -1, -2 - nargs);
        lua_pop(cranked->getLuaContext(), 1);
    }
    if (int result = lua_pcall(cranked->getLuaContext(), nargs, LUA_MULTRET, 0); result != LUA_OK) {
        if (outerr)
            *outerr = cranked->getEmulatedStringLiteral(std::string("Error calling function: ") + cranked->luaEngine.getLuaError(result));
        return 0;
    }
    return 1;
}

void cranked::playdate_json_initEncoder(Cranked *cranked, json_encoder_32 *encoder, cref_t write, void *userdata, int32_t pretty) {
    // Encoder API is stored directly after Playdate API, as subtract json_encoder_32 struct size from API size to get address
    *encoder = *cranked->fromVirtualAddress<json_encoder_32>(API_ADDRESS + cranked->nativeEngine.getApiSize() - sizeof(json_encoder_32));
    encoder->writeStringFunc = write;
    encoder->userdata = cranked->toVirtualAddress(userdata);
    encoder->pretty = pretty;
}

// Todo: Fix this mess
static int json_decode(Cranked *cranked, json_decoder_32 *functions, json_value_32 *outval, const std::string &data) {
    struct JsonContext {
        inline JsonContext(Cranked *emu, json_decoder_32 *decoder, const std::vector<char> &path) : decoder(*decoder), path(path.begin(), path.end(), emu->heap.allocator<char>()) {}
        json_decoder_32 decoder;
        vheap_vector<char> path;
        bool inArray{};
        int arrayIndex{};
        std::string lastKey;
    };
    const std::string rootString = "_root";
    vheap_vector<JsonContext> stack(cranked->heap.allocator<JsonContext>());
    nlohmann::json::parser_callback_t cb = [&](int depth, nlohmann::json::parse_event_t event, nlohmann::json &parsed) -> bool {
        switch (event) {
            case nlohmann::json::parse_event_t::object_start:
            case nlohmann::json::parse_event_t::array_start: {
                bool isRoot = stack.empty();
                bool isArray = event == nlohmann::json::parse_event_t::array_start;
                if (isRoot)
                    stack.emplace_back(cranked, functions, std::vector<char>(rootString.begin(), rootString.end() + 1));
                auto &context = stack.back();
                bool inArray = context.inArray;
                bool willDecode = not inArray;
                if (inArray and context.decoder.shouldDecodeArrayValueAtIndex) {
                    context.arrayIndex++;
                    willDecode = cranked->nativeEngine.invokeEmulatedFunction<int32_t, ArgType::int32_t, ArgType::ptr_t, ArgType::int32_t>
                            (context.decoder.shouldDecodeArrayValueAtIndex, &context.decoder, context.arrayIndex);
                }
                if (not willDecode)
                    return false;
                if (!isRoot and willDecode) {
                    if (inArray) {
//                        stack.back().arrayIndex++;
                        std::string path(context.path.begin(), context.path.end());
                        path += "[" + std::to_string(context.arrayIndex) + "]";
                        stack.emplace_back(cranked, &context.decoder,
                                           std::vector<char>(path.begin(), path.end() + 1)); // Todo: Does this copy the original or the current context?
                    } else {
                        std::string path(context.path.begin(), context.path.end());
                        path += "." + context.lastKey;
                        stack.emplace_back(cranked, &context.decoder, std::vector<char>(path.begin(), path.end() + 1));
                    }
                }
                auto &newContext = stack.back();
                context.decoder.path = cranked->toVirtualAddress(newContext.path.data());
                newContext.inArray = isArray;
                if (isRoot or context.decoder.returnString) // Copy returnString for all children
                    return true;
                if (context.decoder.willDecodeSublist)
                    cranked->nativeEngine.invokeEmulatedFunction<void, ArgType::void_t, ArgType::ptr_t, ArgType::ptr_t, ArgType::int32_t>
                            (context.decoder.willDecodeSublist, &context.decoder, context.path.data(), inArray ? JsonValueType::Array : JsonValueType::Table);
//                printf("Start %i %s\n", (int) event, parsed.dump().c_str());
                break;
            }
            case nlohmann::json::parse_event_t::object_end:
            case nlohmann::json::parse_event_t::array_end: {
                auto &context = stack.back();
                void *value{};
                auto type = event == nlohmann::json::parse_event_t::object_end ? JsonValueType::Table : JsonValueType::Array;
                // Todo: Is it supposed to call didDecodeSublist when returnString is set?
                if (!context.decoder.returnString and context.decoder.didDecodeSublist)
                    value = cranked->nativeEngine.invokeEmulatedFunction<void *, ArgType::ptr_t, ArgType::ptr_t, ArgType::ptr_t, ArgType::int32_t>
                            (context.decoder.didDecodeSublist, &context.decoder, context.path.data(), type);
                if (context.decoder.returnString) { // Todo: How does memory ownership work here? User freed?
                    auto string = parsed.get<std::string>();
                    value = cranked->heap.allocate(string.size());
                    memcpy(value, string.data(), string.size());
                }
                stack.pop_back();
                auto &parentContext = stack.back();
                auto sublist = json_value_32{uint8_t(type), {.tableval = cranked->toVirtualAddress(value)}};
                if (stack.empty()) {
                    *outval = sublist;
                } else {
                    if (parentContext.inArray) {
                        if (parentContext.decoder.didDecodeArrayValue)
                            cranked->nativeEngine.invokeEmulatedFunction<void, ArgType::void_t, ArgType::ptr_t, ArgType::ptr_t, ArgType::struct2_t>
                                    (parentContext.decoder.didDecodeArrayValue, &parentContext.decoder, parentContext.arrayIndex, sublist);
                    } else {
                        auto keyData = vheap_vector<char>(parentContext.lastKey.begin(), parentContext.lastKey.end() + 1, cranked->heap.allocator<char>());
                        if (parentContext.decoder.didDecodeTableValue)
                            cranked->nativeEngine.invokeEmulatedFunction<void, ArgType::void_t, ArgType::ptr_t, ArgType::ptr_t, ArgType::struct2_t>
                                    (parentContext.decoder.didDecodeTableValue, &parentContext.decoder, keyData.data(), sublist);
                    }
                }
//                if (context.decoder.returnString)
//                    stack.back().lastStringResult = ;
//                else
//                    stack.back().lastResult = {uint8_t(type), {.tableval = cranked->toVirtualAddress(value)}}; // Todo: What is value when didDecodeSublist isn't set?
//                printf("End %i %s\n", (int) event, parsed.dump().c_str());
                break;
            }
            case nlohmann::json::parse_event_t::key: {
                auto &context = stack.back();
                if (context.decoder.returnString)
                    return true;
                auto key = parsed.get<std::string>();
                auto keyData = vheap_vector<char>(key.begin(), key.end(), cranked->heap.allocator<char>());
                bool shouldDecode = true;
                context.lastKey = key;
                if (context.decoder.shouldDecodeTableValueForKey)
                    shouldDecode = cranked->nativeEngine.invokeEmulatedFunction<int32_t, ArgType::int32_t, ArgType::ptr_t, ArgType::ptr_t>
                            (context.decoder.shouldDecodeTableValueForKey, &stack.back().decoder, keyData.data());
//                if (shouldDecode)
//                    stack.emplace_back(cranked, functions, std::vector<char>(keyData.begin(), keyData.end())); // Todo: Does this copy the original or the current context?
//                printf("Key %i %s\n", (int) event, parsed.dump().c_str());
                return shouldDecode;
            }
            case nlohmann::json::parse_event_t::value: {
                auto &context = stack.back();
                if (context.decoder.returnString)
                    return true;
                auto stringValue = vheap_vector<char>(cranked->heap.allocator<char>()); // Todo: Is this expected to be valid after the `didDecode` functions?
                auto parseValue = [&]{
                    json_value_32 value{};
                    if (parsed.is_null())
                        value.type = (int) JsonValueType::Null;
                    else if (parsed.is_boolean()) {
                        value.data.intval = parsed.get<bool>();
                        value.type = (int) (value.data.intval ? JsonValueType::True : JsonValueType::False);
                    } else if (parsed.is_string()) {
                        value.type = (int) JsonValueType::String;
                        auto string = parsed.get<std::string>();
                        stringValue.resize(string.size());
                        memcpy(stringValue.data(), string.c_str(), string.size());
                        value.data.stringval = cranked->toVirtualAddress(stringValue.data());
                    } else if (parsed.is_number_integer()) {
                        value.data.intval = parsed.get<int>();
                        value.type = (int) JsonValueType::Integer;
                    } else if (parsed.is_number_float()) {
                        value.data.floatval = parsed.get<float>();
                        value.type = (int) JsonValueType::Float;
                    } else
                        throw std::runtime_error("Invalid JSON value type");
                    return value;
                };
                if (context.inArray) {
                    bool shouldDecode = true;
                    context.arrayIndex++; // Indices start at 1...
                    if (context.decoder.shouldDecodeArrayValueAtIndex)
                        shouldDecode = cranked->nativeEngine.invokeEmulatedFunction<int32_t, ArgType::int32_t, ArgType::ptr_t, ArgType::int32_t>
                                (context.decoder.shouldDecodeArrayValueAtIndex, &stack.back().decoder, context.arrayIndex);
                    if (shouldDecode) {
                        if (context.decoder.didDecodeArrayValue)
                            cranked->nativeEngine.invokeEmulatedFunction<void, ArgType::void_t, ArgType::ptr_t, ArgType::int32_t, ArgType::struct2_t>
                                    (context.decoder.didDecodeArrayValue, &stack.back().decoder, context.arrayIndex, parseValue());
                    }
                } else {
                    auto keyData = vheap_vector<char>(context.lastKey.begin(), context.lastKey.end(), cranked->heap.allocator<char>());
                    if (context.decoder.didDecodeTableValue)
                        cranked->nativeEngine.invokeEmulatedFunction<void, ArgType::void_t, ArgType::ptr_t, ArgType::ptr_t, ArgType::struct2_t>
                                (context.decoder.didDecodeTableValue, &stack.back().decoder, keyData.data(), parseValue());
                }
//                printf("Value %i %s\n", (int) event, parsed.dump().c_str());
                break;
            }
        }
        return true;
    };
    try {
        auto result [[maybe_unused]] = nlohmann::json::parse(data, cb);
        return 1;
    } catch (std::exception &ex) {
        if (functions->decodeError)
            cranked->nativeEngine.invokeEmulatedFunction<void, ArgType::void_t, ArgType::ptr_t, ArgType::uint32_t, ArgType::int32_t>
                    (functions->decodeError, functions, cranked->getEmulatedStringLiteral(ex.what()), -1); // Todo: Is line number retreivable?
        return 0;
    }
}

int32_t cranked::playdate_json_decode(Cranked *cranked, json_decoder_32 *functions, json_reader_32 reader, json_value_32 *outval) {
    constexpr int BUFFER_SEGMENT = 512;
    int size = 0;
    vheap_vector<char> buffer(BUFFER_SEGMENT, cranked->heap.allocator<char>());
    while (true) {
        int returned = cranked->nativeEngine.invokeEmulatedFunction<int32_t, ArgType::int32_t, ArgType::uint32_t, ArgType::ptr_t, ArgType::int32_t>
                (reader.read, reader.userdata, buffer.data() + size, buffer.size() - size);
        if (returned < BUFFER_SEGMENT)
            break;
        size += returned;
        buffer.resize(buffer.size() + BUFFER_SEGMENT);
    }
    return json_decode(cranked, functions, outval, buffer.data());
}

int32_t cranked::playdate_json_decodeString(Cranked *cranked, json_decoder_32 *functions, uint8_t *jsonString, json_value_32 *outval) {
    return json_decode(cranked, functions, outval, (const char *) jsonString);
}

inline static void encoderWrite(Cranked *cranked, json_encoder_32 *encoder, cref_t string, int len) {
    cranked->nativeEngine.invokeEmulatedFunction<void, ArgType::void_t, ArgType::uint32_t, ArgType::uint32_t, ArgType::int32_t>
            (encoder->writeStringFunc, encoder->userdata, string, len);
}

inline static void encoderWrite(Cranked *cranked, json_encoder_32 *encoder, const char *string) {
    encoderWrite(cranked, encoder, cranked->getEmulatedStringLiteral(string), strlen(string));
}

inline static void encoderWrite(Cranked *cranked, json_encoder_32 *encoder, std::string string) {
    // Use vector rather than heap_string to prevent non-heap addresses for small strings
    auto data = vheap_vector<char>(string.c_str(), string.c_str() + string.size(), cranked->heap.allocator<char>());
    encoderWrite(cranked, encoder, cranked->toVirtualAddress(data.data()), string.length());
}

void cranked::json_encoder_startArray(Cranked *cranked, json_encoder_32 *encoder) {
    encoder->startedArray = true;
    encoder->depth++;
    if (encoder->pretty and encoder->depth > 1)
        encoderWrite(cranked, encoder, " ");
    encoderWrite(cranked, encoder, "[");
}

void cranked::json_encoder_addArrayMember(Cranked *cranked, json_encoder_32 *encoder) {
    if (!encoder->startedArray)
        encoderWrite(cranked, encoder, ",");
    if (encoder->pretty) {
        encoderWrite(cranked, encoder, "\n");
        for (int i = 0; i < encoder->depth; i++)
            encoderWrite(cranked, encoder, "\t");
    }
    encoder->startedArray = false;
}

void cranked::json_encoder_endArray(Cranked *cranked, json_encoder_32 *encoder) {
    encoder->depth--;
    if (encoder->pretty) {
        encoderWrite(cranked, encoder, "\n");
        for (int i = 0; i < encoder->depth; i++)
            encoderWrite(cranked, encoder, "\t");
    }
    encoderWrite(cranked, encoder, "]");
}

void cranked::json_encoder_startTable(Cranked *cranked, json_encoder_32 *encoder) {
    encoder->startedTable = true;
    encoder->depth++;
    if (encoder->pretty and encoder->depth > 1)
        encoderWrite(cranked, encoder, " ");
    encoderWrite(cranked, encoder, "{");
}

void cranked::json_encoder_addTableMember(Cranked *cranked, json_encoder_32 *encoder, uint8_t *name, int32_t len) {
    if (!encoder->startedTable)
        encoderWrite(cranked, encoder, ",");
    if (encoder->pretty) {
        encoderWrite(cranked, encoder, "\n");
        for (int i = 0; i < encoder->depth; i++)
            encoderWrite(cranked, encoder, "\t");
    }
    encoderWrite(cranked, encoder, nlohmann::json(std::string((const char *) name, len)).dump());
    encoderWrite(cranked, encoder, ":");
    encoder->startedTable = false;
}

void cranked::json_encoder_endTable(Cranked *cranked, json_encoder_32 *encoder) {
    encoder->depth--;
    if (encoder->pretty) {
        encoderWrite(cranked, encoder, "\n");
        for (int i = 0; i < encoder->depth; i++)
            encoderWrite(cranked, encoder, "\t");
    }
    encoderWrite(cranked, encoder, "}");
}

void cranked::json_encoder_writeNull(Cranked *cranked, json_encoder_32 *encoder) {
    encoderWrite(cranked, encoder, "null");
}

void cranked::json_encoder_writeFalse(Cranked *cranked, json_encoder_32 *encoder) {
    encoderWrite(cranked, encoder, "false");
}

void cranked::json_encoder_writeTrue(Cranked *cranked, json_encoder_32 *encoder) {
    encoderWrite(cranked, encoder, "true");
}

void cranked::json_encoder_writeInt(Cranked *cranked, json_encoder_32 *encoder, int32_t num) {
    encoderWrite(cranked, encoder, std::to_string(num));
}

void cranked::json_encoder_writeDouble(Cranked *cranked, json_encoder_32 *encoder, double num) {
    encoderWrite(cranked, encoder, std::to_string(num));
}

void cranked::json_encoder_writeString(Cranked *cranked, json_encoder_32 *encoder, uint8_t *str, int32_t len) {
    encoderWrite(cranked, encoder, nlohmann::json(std::string((const char *) str, len)).dump());
}

uint8_t *cranked::playdate_file_geterr(Cranked *cranked) {
    return cranked->fromVirtualAddress<uint8_t>(cranked->files.lastError); // This conversion is redundant, but simplifies things
}

int32_t cranked::playdate_file_listfiles(Cranked *cranked, uint8_t *path, cref_t callback, void *userdata, int32_t showhidden) {
    std::vector<std::string> files;
    if (cranked->files.listFiles((const char *) path, showhidden, files))
        return -1;
    for (auto &file : files)
        cranked->nativeEngine.invokeEmulatedFunction<void, ArgType::ptr_t, ArgType::ptr_t>(callback, file.c_str(), userdata);
    return 0;
}

int32_t cranked::playdate_file_stat(Cranked *cranked, uint8_t *path, FileStat_32 *stats) {
    return cranked->files.stat((const char *) path, *stats);
}

int32_t cranked::playdate_file_mkdir(Cranked *cranked, uint8_t *path) {
    return cranked->files.mkdir((const char *) path);
}

int32_t cranked::playdate_file_unlink(Cranked *cranked, uint8_t *name, int32_t recursive) {
    return cranked->files.unlink((const char *) name, recursive);
}

int32_t cranked::playdate_file_rename(Cranked *cranked, uint8_t *from, uint8_t *to) {
    return cranked->files.rename((const char *) from, (const char *) to);
}

SDFile_32 *cranked::playdate_file_open(Cranked *cranked, uint8_t *name, int32_t mode) {
    return cranked->files.open((const char *) name, FileOptions(mode));
}

int32_t cranked::playdate_file_close(Cranked *cranked, SDFile_32 *file) {
    return cranked->files.close(file);
}

int32_t cranked::playdate_file_read(Cranked *cranked, SDFile_32 *file, void *buf, uint32_t len) {
    return cranked->files.read(file, buf, len);
}

int32_t cranked::playdate_file_write(Cranked *cranked, SDFile_32 *file, void *buf, uint32_t len) {
    return cranked->files.write(file, buf, len);
}

int32_t cranked::playdate_file_flush(Cranked *cranked, SDFile_32 *file) {
    return cranked->files.flush(file);
}

int32_t cranked::playdate_file_tell(Cranked *cranked, SDFile_32 *file) {
    return cranked->files.tell(file);
}

int32_t cranked::playdate_file_seek(Cranked *cranked, SDFile_32 *file, int32_t pos, int32_t whence) {
    return cranked->files.seek(file, pos, whence);
}

void cranked::playdate_sprite_setAlwaysRedraw(Cranked *cranked, int32_t flag) {
    cranked->graphics.alwaysRedrawSprites = flag;
}

void cranked::playdate_sprite_addDirtyRect(Cranked *cranked, LCDRect_32 dirtyRect) {
    // Todo
}

void cranked::playdate_sprite_drawSprites(Cranked *cranked) {
    // Todo
}

void cranked::playdate_sprite_updateAndDrawSprites(Cranked *cranked) {
    // Todo
}

LCDSprite_32 *cranked::playdate_sprite_newSprite(Cranked *cranked) {
    // Todo
    return nullptr;
}

void cranked::playdate_sprite_freeSprite(Cranked *cranked, LCDSprite_32 *sprite) {
    // Todo
    cranked->graphics.freeSprite(sprite);
}

LCDSprite_32 *cranked::playdate_sprite_copy(Cranked *cranked, LCDSprite_32 *sprite) {
    // Todo
    return nullptr;
}

void cranked::playdate_sprite_addSprite(Cranked *cranked, LCDSprite_32 *sprite) {
    // Todo
}

void cranked::playdate_sprite_removeSprite(Cranked *cranked, LCDSprite_32 *sprite) {
    // Todo
}

void cranked::playdate_sprite_removeSprites(Cranked *cranked, cref_t *sprites, int32_t count) {
    // Todo
}

void cranked::playdate_sprite_removeAllSprites(Cranked *cranked) {
    // Todo
}

int32_t cranked::playdate_sprite_getSpriteCount(Cranked *cranked) {
    return (int32_t ) cranked->graphics.spriteDrawList.size();
}

void cranked::playdate_sprite_setBounds(Cranked *cranked, LCDSprite_32 *sprite, PDRect_32 bounds) {
    // Todo
}

PDRect_32 cranked::playdate_sprite_getBounds(Cranked *cranked, LCDSprite_32 *sprite) {
    // Todo
    return {};
}

void cranked::playdate_sprite_moveTo(Cranked *cranked, LCDSprite_32 *sprite, float x, float y) {
    // Todo
}

void cranked::playdate_sprite_moveBy(Cranked *cranked, LCDSprite_32 *sprite, float dx, float dy) {
    // Todo
}

void cranked::playdate_sprite_setImage(Cranked *cranked, LCDSprite_32 *sprite, LCDBitmap_32 *image, int32_t flip) {
    // Todo
}

LCDBitmap_32 *cranked::playdate_sprite_getImage(Cranked *cranked, LCDSprite_32 *sprite) {
    // Todo
    return nullptr;
}

void cranked::playdate_sprite_setSize(Cranked *cranked, LCDSprite_32 *s, float width, float height) {
    s->size = Vec2(width, height);
}

void cranked::playdate_sprite_setZIndex(Cranked *cranked, LCDSprite_32 *sprite, int16_t zIndex) {
    sprite->zIndex = zIndex;
}

int16_t cranked::playdate_sprite_getZIndex(Cranked *cranked, LCDSprite_32 *sprite) {
    return sprite->zIndex;
}

void cranked::playdate_sprite_setDrawMode(Cranked *cranked, LCDSprite_32 *sprite, int32_t mode) {
    sprite->drawMode = (LCDBitmapDrawMode) mode;
}

void cranked::playdate_sprite_setImageFlip(Cranked *cranked, LCDSprite_32 *sprite, int32_t flip) {
    sprite->flip = (LCDBitmapFlip) flip;
}

int32_t cranked::playdate_sprite_getImageFlip(Cranked *cranked, LCDSprite_32 *sprite) {
    return (int32_t)sprite->flip;
}

void cranked::playdate_sprite_setStencil(Cranked *cranked, LCDSprite_32 *sprite, LCDBitmap_32 *stencil) {
    // Todo
}

void cranked::playdate_sprite_setClipRect(Cranked *cranked, LCDSprite_32 *sprite, LCDRect_32 clipRect) {
    // Todo
}

void cranked::playdate_sprite_clearClipRect(Cranked *cranked, LCDSprite_32 *sprite) {
    // Todo
}

void cranked::playdate_sprite_setClipRectsInRange(Cranked *cranked, LCDRect_32 clipRect, int32_t startZ, int32_t endZ) {
    // Todo
}

void cranked::playdate_sprite_clearClipRectsInRange(Cranked *cranked, int32_t startZ, int32_t endZ) {
    // Todo
}

void cranked::playdate_sprite_setUpdatesEnabled(Cranked *cranked, LCDSprite_32 *sprite, int32_t flag) {
    sprite->updatesEnabled = flag;
}

int32_t cranked::playdate_sprite_updatesEnabled(Cranked *cranked, LCDSprite_32 *sprite) {
    return sprite->updatesEnabled;
}

void cranked::playdate_sprite_setCollisionsEnabled(Cranked *cranked, LCDSprite_32 *sprite, int32_t flag) {
    sprite->collisionsEnabled = flag;
}

int32_t cranked::playdate_sprite_collisionsEnabled(Cranked *cranked, LCDSprite_32 *sprite) {
    return sprite->collisionsEnabled;
}

void cranked::playdate_sprite_setVisible(Cranked *cranked, LCDSprite_32 *sprite, int32_t flag) {
    sprite->visible = flag;
}

int32_t cranked::playdate_sprite_isVisible(Cranked *cranked, LCDSprite_32 *sprite) {
    return sprite->visible;
}

void cranked::playdate_sprite_setOpaque(Cranked *cranked, LCDSprite_32 *sprite, int32_t flag) {
    sprite->opaque = flag;
}

void cranked::playdate_sprite_markDirty(Cranked *cranked, LCDSprite_32 *sprite) {
    sprite->dirty = true;
}

void cranked::playdate_sprite_setTag(Cranked *cranked, LCDSprite_32 *sprite, uint8_t tag) {
    sprite->tag = tag;
}

uint8_t cranked::playdate_sprite_getTag(Cranked *cranked, LCDSprite_32 *sprite) {
    return sprite->tag;
}

void cranked::playdate_sprite_setIgnoresDrawOffset(Cranked *cranked, LCDSprite_32 *sprite, int32_t flag) {
    sprite->ignoresDrawOffset = flag;
}

void cranked::playdate_sprite_setUpdateFunction(Cranked *cranked, LCDSprite_32 *sprite, cref_t func) {
    sprite->updateFunction = func;
}

void cranked::playdate_sprite_setDrawFunction(Cranked *cranked, LCDSprite_32 *sprite, cref_t func) {
    sprite->drawFunction = func;
}

void cranked::playdate_sprite_getPosition(Cranked *cranked, LCDSprite_32 *sprite, float *x, float *y) {
    *x = sprite->pos.x;
    *y = sprite->pos.y;
}

void cranked::playdate_sprite_resetCollisionWorld(Cranked *cranked) {
    // Todo
}

void cranked::playdate_sprite_setCollideRect(Cranked *cranked, LCDSprite_32 *sprite, PDRect_32 collideRect) {
    // Todo
}

PDRect_32 cranked::playdate_sprite_getCollideRect(Cranked *cranked, LCDSprite_32 *sprite) {
    // Todo
    return {};
}

void cranked::playdate_sprite_clearCollideRect(Cranked *cranked, LCDSprite_32 *sprite) {
    // Todo
}

void cranked::playdate_sprite_setCollisionResponseFunction(Cranked *cranked, LCDSprite_32 *sprite, cref_t func) {
    // Todo
}

SpriteCollisionInfo_32 *cranked::playdate_sprite_checkCollisions(Cranked *cranked, LCDSprite_32 *sprite, float goalX, float goalY, float *actualX, float *actualY, int32_t *len) {
    // Todo
    return nullptr;
}

SpriteCollisionInfo_32 *cranked::playdate_sprite_moveWithCollisions(Cranked *cranked, LCDSprite_32 *sprite, float goalX, float goalY, float *actualX, float *actualY, int32_t *len) {
    // Todo
    return nullptr;
}

cref_t *cranked::playdate_sprite_querySpritesAtPoint(Cranked *cranked, float x, float y, int32_t *len) {
    // Todo
    return nullptr;
}

cref_t *cranked::playdate_sprite_querySpritesInRect(Cranked *cranked, float x, float y, float width, float height, int32_t *len) {
    // Todo
    return nullptr;
}

cref_t *cranked::playdate_sprite_querySpritesAlongLine(Cranked *cranked, float x1, float y1, float x2, float y2, int32_t *len) {
    // Todo
    return nullptr;
}

SpriteQueryInfo_32 *cranked::playdate_sprite_querySpriteInfoAlongLine(Cranked *cranked, float x1, float y1, float x2, float y2, int32_t *len) {
    // Todo
    return nullptr;
}

cref_t *cranked::playdate_sprite_overlappingSprites(Cranked *cranked, LCDSprite_32 *sprite, int32_t *len) {
    // Todo
    return nullptr;
}

cref_t *cranked::playdate_sprite_allOverlappingSprites(Cranked *cranked, int32_t *len) {
    // Todo
    return nullptr;
}

void cranked::playdate_sprite_setStencilPattern(Cranked *cranked, LCDSprite_32 *sprite, uint8_t pattern) {
    // Todo
}

void cranked::playdate_sprite_clearStencil(Cranked *cranked, LCDSprite_32 *sprite) {
    // Todo
}

void cranked::playdate_sprite_setUserdata(Cranked *cranked, LCDSprite_32 *sprite, void *userdata) {
    sprite->userdata = cranked->toVirtualAddress(userdata);
}

void *cranked::playdate_sprite_getUserdata(Cranked *cranked, LCDSprite_32 *sprite) {
    return cranked->fromVirtualAddress(sprite->userdata);
}

void cranked::playdate_sprite_setStencilImage(Cranked *cranked, LCDSprite_32 *sprite, LCDBitmap_32 *stencil, int32_t tile) {
    // Todo
}

void cranked::playdate_sprite_setCenter(Cranked *cranked, LCDSprite_32 *s, float x, float y) {
    s->centerX = x;
    s->centerY = y;
}

void cranked::playdate_sprite_getCenter(Cranked *cranked, LCDSprite_32 *s, float * x, float * y) {
    if (x)
        *x = s->centerX;
    if (y)
        *y = s->centerY;
}

void cranked::playdate_sound_source_setVolume(Cranked *cranked, SoundSource_32 *c, float lvol, float rvol) {
    c->leftVolume = lvol;
    c->rightVolume = rvol;
}

void cranked::playdate_sound_source_getVolume(Cranked *cranked, SoundSource_32 *c, float *outl, float *outr) {
    *outl = c->leftVolume;
    *outr = c->rightVolume;
}

int32_t cranked::playdate_sound_source_isPlaying(Cranked *cranked, SoundSource_32 *c) {
    return c->playing;
}

void cranked::playdate_sound_source_setFinishCallback(Cranked *cranked, SoundSource_32 *c, cref_t callback, void *userdata) {
    c->completionCallback = callback;
    c->completionCallbackUserdata = cranked->version < VERSION_2_4_1 ? 0 : cranked->toVirtualAddress(userdata);
}

FilePlayer_32 *cranked::playdate_sound_fileplayer_newPlayer(Cranked *cranked) {
    return cranked->audio.allocateSoundSource<FilePlayer_32>();
}

void cranked::playdate_sound_fileplayer_freePlayer(Cranked *cranked, FilePlayer_32 *player) {
    cranked->audio.freeSoundSource(player);
}

int32_t cranked::playdate_sound_fileplayer_loadIntoPlayer(Cranked *cranked, FilePlayer_32 *player, uint8_t *path) {
    // Todo
    return 0;
}

void cranked::playdate_sound_fileplayer_setBufferLength(Cranked *cranked, FilePlayer_32 *player, float bufferLen) {
    // Todo
}

int32_t cranked::playdate_sound_fileplayer_play(Cranked *cranked, FilePlayer_32 *player, int32_t repeat) {
    // Todo
    return 0;
}

int32_t cranked::playdate_sound_fileplayer_isPlaying(Cranked *cranked, FilePlayer_32 *player) {
    return player->playing; // Todo: Does pausing affect this
}

void cranked::playdate_sound_fileplayer_pause(Cranked *cranked, FilePlayer_32 *player) {
    player->paused = true;
}

void cranked::playdate_sound_fileplayer_stop(Cranked *cranked, FilePlayer_32 *player) {
    player->playing = false;
    player->sampleOffset = 0;
}

void cranked::playdate_sound_fileplayer_setVolume(Cranked *cranked, FilePlayer_32 *player, float left, float right) {
    player->leftVolume = left;
    player->rightVolume = right;
}

void cranked::playdate_sound_fileplayer_getVolume(Cranked *cranked, FilePlayer_32 *player, float *left, float *right) {
    *left = player->leftVolume;
    *right = player->rightVolume;
}

float cranked::playdate_sound_fileplayer_getLength(Cranked *cranked, FilePlayer_32 *player) {
    return player->file ? framesToSeconds((int) player->file->romData->size()) : 0;
}

void cranked::playdate_sound_fileplayer_setOffset(Cranked *cranked, FilePlayer_32 *player, float offset) {
    // Todo
}

void cranked::playdate_sound_fileplayer_setRate(Cranked *cranked, FilePlayer_32 *player, float rate) {
    player->rate = rate;
}

void cranked::playdate_sound_fileplayer_setLoopRange(Cranked *cranked, FilePlayer_32 *player, float start, float end) {
    player->loopStart = framesFromSeconds(start);
    player->loopEnd = framesFromSeconds(end);
}

int32_t cranked::playdate_sound_fileplayer_didUnderrun(Cranked *cranked, FilePlayer_32 *player) {
    return player->underran;
}

void cranked::playdate_sound_fileplayer_setFinishCallback(Cranked *cranked, FilePlayer_32 *player, cref_t callback, void *userdata) {
    player->completionCallback = callback;
    player->completionCallbackUserdata = cranked->version < VERSION_2_4_1 ? 0 : cranked->toVirtualAddress(userdata);
}

void cranked::playdate_sound_fileplayer_setLoopCallback(Cranked *cranked, FilePlayer_32 *player, cref_t callback, void *userdata) {
    player->loopCallback = callback;
    player->loopCallbackUserdata = cranked->version < VERSION_2_4_1 ? 0 : cranked->toVirtualAddress(userdata);
}

float cranked::playdate_sound_fileplayer_getOffset(Cranked *cranked, FilePlayer_32 *player) {
    return framesToSeconds(player->sampleOffset);
}

float cranked::playdate_sound_fileplayer_getRate(Cranked *cranked, FilePlayer_32 *player) {
    return player->rate;
}

void cranked::playdate_sound_fileplayer_setStopOnUnderrun(Cranked *cranked, FilePlayer_32 *player, int32_t flag) {
    player->stopOnUnderrun = flag;
}
void cranked::playdate_sound_fileplayer_fadeVolume(Cranked *cranked, FilePlayer_32 *player, float left, float right, int32_t len, cref_t finishCallback, void * userdata) {
    // Todo (userdata was added in VERSION_2_4_1, otherwise use 0)
}

void cranked::playdate_sound_fileplayer_setMP3StreamSource(Cranked *cranked, FilePlayer_32 *player, cref_t dataSource, void *userdata, float bufferLen) {
    // Todo
}

AudioSample_32 *cranked::playdate_sound_sample_newSampleBuffer(Cranked *cranked, int32_t byteCount) {
    return cranked->audio.allocateAudioSample(byteCount);
}

int32_t cranked::playdate_sound_sample_loadIntoSample(Cranked *cranked, AudioSample_32 *sample, uint8_t *path) {
    // Todo
    return 0;
}

AudioSample_32 *cranked::playdate_sound_sample_load(Cranked *cranked, uint8_t *path) {
    // Todo
    return nullptr;
}

AudioSample_32 *cranked::playdate_sound_sample_newSampleFromData(Cranked *cranked, uint8_t *data, int32_t format, uint32_t sampleRate, int32_t byteCount) {
    // Todo
    return nullptr;
}

void cranked::playdate_sound_sample_getData(Cranked *cranked, AudioSample_32 *sample, cref_t *data, int32_t *format, uint32_t *sampleRate, uint32_t *bytelength) {
    if (data)
        *data = cranked->toVirtualAddress(sample->data.data());
    if (format)
        *format = (int32_t )sample->format;
    if (sampleRate)
        *sampleRate = sample->sampleRate;
    if (bytelength)
        *bytelength = (int32_t)sample->data.size();
}

void cranked::playdate_sound_sample_freeSample(Cranked *cranked, AudioSample_32 *sample) {
    cranked->audio.freeAudioSample(sample);
}

float cranked::playdate_sound_sample_getLength(Cranked *cranked, AudioSample_32 *sample) {
    return framesToSeconds((int) sample->data.size());
}

int32_t cranked::playdate_sound_sample_decompress(Cranked *cranked, AudioSample_32 *sample) {
    return 0; // Todo
}

SamplePlayer_32 *cranked::playdate_sound_sampleplayer_newPlayer(Cranked *cranked) {
    return cranked->audio.allocateSoundSource<SamplePlayer_32>();
}

void cranked::playdate_sound_sampleplayer_freePlayer(Cranked *cranked, SamplePlayer_32 *player) {
    cranked->audio.freeSoundSource(player);
}

void cranked::playdate_sound_sampleplayer_setSample(Cranked *cranked, SamplePlayer_32 *player, AudioSample_32 *sample) {
    player->sample = sample;
}

int32_t cranked::playdate_sound_sampleplayer_play(Cranked *cranked, SamplePlayer_32 *player, int32_t repeat, float rate) {
    player->playing = true;
    player->repeat = repeat;
    player->rate = rate;
    return true;
}

int32_t cranked::playdate_sound_sampleplayer_isPlaying(Cranked *cranked, SamplePlayer_32 *player) {
    return player->playing; // Todo: Does pausing affect this?
}

void cranked::playdate_sound_sampleplayer_stop(Cranked *cranked, SamplePlayer_32 *player) {
    player->playing = false;
    player->sampleOffset = 0;
}

void cranked::playdate_sound_sampleplayer_setVolume(Cranked *cranked, SamplePlayer_32 *player, float left, float right) {
    player->leftVolume = left;
    player->rightVolume = right;
}

void cranked::playdate_sound_sampleplayer_getVolume(Cranked *cranked, SamplePlayer_32 *player, float *left, float *right) {
    *left = player->leftVolume;
    *right = player->rightVolume;
}

float cranked::playdate_sound_sampleplayer_getLength(Cranked *cranked, SamplePlayer_32 *player) {
    return player->sample ? framesToSeconds((int) player->sample->data.size()) : 0;
}

void cranked::playdate_sound_sampleplayer_setOffset(Cranked *cranked, SamplePlayer_32 *player, float offset) {
    player->sampleOffset = framesFromSeconds(offset);
}

void cranked::playdate_sound_sampleplayer_setRate(Cranked *cranked, SamplePlayer_32 *player, float rate) {
    player->rate = rate;
}

void cranked::playdate_sound_sampleplayer_setPlayRange(Cranked *cranked, SamplePlayer_32 *player, int32_t start, int32_t end) {
    player->loopStart = start;
    player->loopEnd = end;
}

void cranked::playdate_sound_sampleplayer_setFinishCallback(Cranked *cranked, SamplePlayer_32 *player, cref_t callback, void *userdata) {
    player->completionCallback = callback;
    player->completionCallbackUserdata = cranked->toVirtualAddress(userdata);
}

void cranked::playdate_sound_sampleplayer_setLoopCallback(Cranked *cranked, SamplePlayer_32 *player, cref_t callback, void *userdata) {
    player->loopCallback = callback;
    player->loopCallbackUserdata = cranked->version < VERSION_2_4_1 ? 0 : cranked->toVirtualAddress(userdata);
}

float cranked::playdate_sound_sampleplayer_getOffset(Cranked *cranked, SamplePlayer_32 *player) {
    return framesToSeconds(player->sampleOffset);
}

float cranked::playdate_sound_sampleplayer_getRate(Cranked *cranked, SamplePlayer_32 *player) {
    return player->rate;
}

void cranked::playdate_sound_sampleplayer_setPaused(Cranked *cranked, SamplePlayer_32 *player, int32_t flag) {
    player->paused = flag;
}

PDSynthSignal_32 *cranked::playdate_sound_signal_newSignal(Cranked *cranked, cref_t step, cref_t noteOn, cref_t noteOff, cref_t dealloc, void *userdata) {
    // Todo
    return nullptr;
}

void cranked::playdate_sound_signal_freeSignal(Cranked *cranked, PDSynthSignal_32 *signal) {
    cranked->audio.freeSynthSignal(signal);
}

float cranked::playdate_sound_signal_getValue(Cranked *cranked, PDSynthSignal_32 *signal) {
    // Todo
    return 0;
}

void cranked::playdate_sound_signal_setValueScale(Cranked *cranked, PDSynthSignal_32 *signal, float scale) {
    signal->scale = scale;
}

void cranked::playdate_sound_signal_setValueOffset(Cranked *cranked, PDSynthSignal_32 *signal, float offset) {
    signal->offset = framesFromSeconds(offset);
}

uint8_t *cranked::playdate_sound_getError(Cranked *cranked) {
    return cranked->fromVirtualAddress<uint8_t>(cranked->audio.lastError);
}

PDSynthLFO_32 *cranked::playdate_sound_lfo_newLFO(Cranked *cranked, int32_t type) {
    return cranked->audio.allocateSynthSignal<PDSynthLFO_32>((LFOType)type);
}

void cranked::playdate_sound_lfo_freeLFO(Cranked *cranked, PDSynthLFO_32 *lfo) {
    cranked->audio.freeSynthSignal(lfo);
}

void cranked::playdate_sound_lfo_setType(Cranked *cranked, PDSynthLFO_32 *lfo, int32_t type) {
    lfo->type = (LFOType)type;
}

void cranked::playdate_sound_lfo_setRate(Cranked *cranked, PDSynthLFO_32 *lfo, float rate) {
    lfo->rate = rate;
}

void cranked::playdate_sound_lfo_setPhase(Cranked *cranked, PDSynthLFO_32 *lfo, float phase) {
    lfo->phase = phase;
}

void cranked::playdate_sound_lfo_setCenter(Cranked *cranked, PDSynthLFO_32 *lfo, float center) {
    lfo->center = center;
}

void cranked::playdate_sound_lfo_setDepth(Cranked *cranked, PDSynthLFO_32 *lfo, float depth) {
    lfo->depth = depth;
}

void cranked::playdate_sound_lfo_setArpeggiation(Cranked *cranked, PDSynthLFO_32 *lfo, int32_t nSteps, float *steps) {
    lfo->type = LFOType::Arpeggiator;
    lfo->arpeggiationSteps.clear();
    for (int i = 0; i < nSteps; i++)
        lfo->arpeggiationSteps.push_back(steps[i]);
}

void cranked::playdate_sound_lfo_setFunction(Cranked *cranked, PDSynthLFO_32 *lfo, cref_t lfoFunc, void *userdata, int32_t interpolate) {
    lfo->function = lfoFunc;
    lfo->functionUserdata = cranked->toVirtualAddress(userdata);
    lfo->functionInterpolate = interpolate;
    lfo->type = LFOType::Function;
}

void cranked::playdate_sound_lfo_setDelay(Cranked *cranked, PDSynthLFO_32 *lfo, float holdoff, float ramptime) {
    lfo->holdOffSamples = framesFromSeconds(holdoff);
    lfo->rampTimeSamples = framesFromSeconds(ramptime);
}

void cranked::playdate_sound_lfo_setRetrigger(Cranked *cranked, PDSynthLFO_32 *lfo, int32_t flag) {
    lfo->reTrigger = flag;
}

float cranked::playdate_sound_lfo_getValue(Cranked *cranked, PDSynthLFO_32 *lfo) {
    // Todo
    return 0;
}

void cranked::playdate_sound_lfo_setGlobal(Cranked *cranked, PDSynthLFO_32 *lfo, int32_t global) {
    lfo->global = global;
}

void cranked::playdate_sound_lfo_setStartPhase(Cranked *cranked, PDSynthLFO_32 * lfo, float phase) {
    lfo->phase = phase;
}

PDSynthEnvelope_32 *cranked::playdate_sound_envelope_newEnvelope(Cranked *cranked, float attack, float decay, float sustain, float release) {
    return cranked->audio.allocateSynthSignal<PDSynthEnvelope_32>(attack, decay, sustain, release);
}

void cranked::playdate_sound_envelope_freeEnvelope(Cranked *cranked, PDSynthEnvelope_32 *env) {
    cranked->audio.freeSynthSignal(env);
}

void cranked::playdate_sound_envelope_setAttack(Cranked *cranked, PDSynthEnvelope_32 *env, float attack) {
    env->attack = attack;
}

void cranked::playdate_sound_envelope_setDecay(Cranked *cranked, PDSynthEnvelope_32 *env, float decay) {
    env->decay = decay;
}

void cranked::playdate_sound_envelope_setSustain(Cranked *cranked, PDSynthEnvelope_32 *env, float sustain) {
    env->sustain = sustain;
}

void cranked::playdate_sound_envelope_setRelease(Cranked *cranked, PDSynthEnvelope_32 *env, float release) {
    env->release = release;
}

void cranked::playdate_sound_envelope_setLegato(Cranked *cranked, PDSynthEnvelope_32 *env, int32_t flag) {
    env->legato = flag;
}

void cranked::playdate_sound_envelope_setRetrigger(Cranked *cranked, PDSynthEnvelope_32 *env, int32_t flag) {
    env->reTrigger = flag;
}

float cranked::playdate_sound_envelope_getValue(Cranked *cranked, PDSynthEnvelope_32 *env) {
    // Todo
    return 0;
}

void cranked::playdate_sound_envelope_setCurvature(Cranked *cranked, PDSynthEnvelope_32 *env, float amount) {
    env->curvature = amount;
}

void cranked::playdate_sound_envelope_setVelocitySensitivity(Cranked *cranked, PDSynthEnvelope_32 *env, float velsens) {
    env->velocitySensitivity = velsens;
}

void cranked::playdate_sound_envelope_setRateScaling(Cranked *cranked, PDSynthEnvelope_32 *env, float scaling, float start, float end) {
    env->rateScaling = scaling;
    env->rateStart = start;
    env->rateEnd = end;
}

PDSynth_32 *cranked::playdate_sound_synth_newSynth(Cranked *cranked) {
    return cranked->audio.allocateSoundSource<PDSynth_32>();
}

void cranked::playdate_sound_synth_freeSynth(Cranked *cranked, PDSynth_32 *synth) {
    cranked->audio.freeSoundSource(synth);
}

void cranked::playdate_sound_synth_setWaveform(Cranked *cranked, PDSynth_32 *synth, int32_t wave) {
    synth->waveform = (SoundWaveform)wave;
}

void cranked::playdate_sound_synth_setGenerator(Cranked *cranked, PDSynth_32 *synth, int32_t stereo, cref_t render, cref_t noteOn, cref_t release, cref_t setparam, cref_t dealloc, cref_t copyUserdata, void *userdata) {
    cranked::playdate_sound_synth_setGenerator_deprecated(cranked, synth, stereo, render, noteOn, release, setparam, dealloc, userdata);
    synth->generatorCopyUserdataFunc = copyUserdata;
}

PDSynth_32 * cranked::playdate_sound_synth_copy(Cranked *cranked, PDSynth_32 * synth) {
    return nullptr; // Todo
}

void cranked::playdate_sound_synth_setGenerator_deprecated(Cranked *cranked, PDSynth_32 *synth, int32_t stereo, cref_t render, cref_t noteOn, cref_t release, cref_t setparam, cref_t dealloc, void *userdata) {
    synth->generatorStereo = stereo;
    synth->generatorRenderFunc = render;
    synth->generatorNoteOnFunc = noteOn;
    synth->generatorReleaseFunc = release;
    synth->generatorSetParameterFunc = setparam;
    synth->generatorDeallocFunc = dealloc;
    synth->generatorUserdata = cranked->toVirtualAddress(userdata);
    synth->generatorCopyUserdataFunc = 0;
}

void cranked::playdate_sound_synth_setSample(Cranked *cranked, PDSynth_32 *synth, AudioSample_32 *sample, uint32_t sustainStart, uint32_t sustainEnd) {
    synth->sample = sample;
    synth->sampleSustainStart = sustainStart;
    synth->sampleSustainEnd = sustainEnd;
}

void cranked::playdate_sound_synth_setAttackTime(Cranked *cranked, PDSynth_32 *synth, float attack) {
    synth->envelope.attack = attack;
}

void cranked::playdate_sound_synth_setDecayTime(Cranked *cranked, PDSynth_32 *synth, float decay) {
    synth->envelope.decay = decay;
}

void cranked::playdate_sound_synth_setSustainLevel(Cranked *cranked, PDSynth_32 *synth, float sustain) {
    synth->envelope.sustain = sustain;
}

void cranked::playdate_sound_synth_setReleaseTime(Cranked *cranked, PDSynth_32 *synth, float release) {
    synth->envelope.release = release;
}

void cranked::playdate_sound_synth_setTranspose(Cranked *cranked, PDSynth_32 *synth, float halfSteps) {
    synth->transposeHalfSteps = halfSteps;
}

void cranked::playdate_sound_synth_setFrequencyModulator(Cranked *cranked, PDSynth_32 *synth, PDSynthSignalValue_32 *mod) {
    synth->frequencyModulator = mod;
}

PDSynthSignalValue_32 *cranked::playdate_sound_synth_getFrequencyModulator(Cranked *cranked, PDSynth_32 *synth) {
    return synth->frequencyModulator;
}

void cranked::playdate_sound_synth_setAmplitudeModulator(Cranked *cranked, PDSynth_32 *synth, PDSynthSignalValue_32 *mod) {
    synth->amplitudeModulator = mod;
}

PDSynthSignalValue_32 *cranked::playdate_sound_synth_getAmplitudeModulator(Cranked *cranked, PDSynth_32 *synth) {
    return synth->amplitudeModulator;
}

int32_t cranked::playdate_sound_synth_getParameterCount(Cranked *cranked, PDSynth_32 *synth) {
    // Todo
    return 0;
}

int32_t cranked::playdate_sound_synth_setParameter(Cranked *cranked, PDSynth_32 *synth, int32_t parameter, float value) {
    // Todo
    return 0;
}

void cranked::playdate_sound_synth_setParameterModulator(Cranked *cranked, PDSynth_32 *synth, int32_t parameter, PDSynthSignalValue_32 *mod) {
    synth->parameterModulators[parameter] = mod;
}

PDSynthSignalValue_32 *cranked::playdate_sound_synth_getParameterModulator(Cranked *cranked, PDSynth_32 *synth, int32_t parameter) {
    return synth->parameterModulators[parameter];
}

void cranked::playdate_sound_synth_playNote(Cranked *cranked, PDSynth_32 *synth, float freq, float vel, float len, uint32_t when) {
    // Todo
}

void cranked::playdate_sound_synth_playMIDINote(Cranked *cranked, PDSynth_32 *synth, float note, float vel, float len, uint32_t when) {
    // Todo
}

void cranked::playdate_sound_synth_noteOff(Cranked *cranked, PDSynth_32 *synth, uint32_t when) {
    // Todo
}

void cranked::playdate_sound_synth_stop(Cranked *cranked, PDSynth_32 *synth) {
    // Todo
}

void cranked::playdate_sound_synth_setVolume(Cranked *cranked, PDSynth_32 *synth, float left, float right) {
    synth->leftVolume = left;
    synth->rightVolume = right;
}

void cranked::playdate_sound_synth_getVolume(Cranked *cranked, PDSynth_32 *synth, float *left, float *right) {
    *left = synth->leftVolume;
    *right = synth->rightVolume;
}

int32_t cranked::playdate_sound_synth_isPlaying(Cranked *cranked, PDSynth_32 *synth) {
    return synth->playing;
}

PDSynthEnvelope_32 *cranked::playdate_sound_synth_getEnvelope(Cranked *cranked, PDSynth_32 *synth) {
    return &synth->envelope;
}

int32_t cranked::playdate_sound_synth_setWavetable(Cranked *cranked, PDSynth_32 * synth, AudioSample_32 * sample, int32_t log2size, int32_t columns, int32_t rows) {
    return 0; // Todo
}

ControlSignal_32 *cranked::playdate_control_signal_newSignal(Cranked *cranked) {
    return cranked->audio.allocateSynthSignal<ControlSignal_32>();
}

void cranked::playdate_control_signal_freeSignal(Cranked *cranked, ControlSignal_32 *signal) {
    cranked->audio.freeSynthSignal(signal);
}

void cranked::playdate_control_signal_clearEvents(Cranked *cranked, ControlSignal_32 *control) {
    control->events.clear();
}

void cranked::playdate_control_signal_addEvent(Cranked *cranked, ControlSignal_32 *control, int32_t step, float value, int32_t interpolate) {
    control->events.insert(std::pair(step, ControlSignal_32::Event{value, (bool)interpolate}));
}

void cranked::playdate_control_signal_removeEvent(Cranked *cranked, ControlSignal_32 *control, int32_t step) {
    control->events.erase(step);
}

int32_t cranked::playdate_control_signal_getMIDIControllerNumber(Cranked *cranked, ControlSignal_32 *control) {
    return control->controllerNumber;
}

PDSynthInstrument_32 *cranked::playdate_sound_instrument_newInstrument(Cranked *cranked) {
    return cranked->audio.allocateSoundSource<PDSynthInstrument_32>();
}

void cranked::playdate_sound_instrument_freeInstrument(Cranked *cranked, PDSynthInstrument_32 *inst) {
    cranked->audio.freeSoundSource(inst);
}

int32_t cranked::playdate_sound_instrument_addVoice(Cranked *cranked, PDSynthInstrument_32 *inst, PDSynth_32 *synth, float rangeStart, float rangeEnd, float transpose) {
    // Todo: Check if already in an instrument or channel
    inst->voices.push_back(synth);
    synth->instrumentStartFrequency = rangeStart;
    synth->instrumentEndFrequency = rangeEnd;
    synth->instrumentTranspose = transpose;
    return true;
}

PDSynth_32 *cranked::playdate_sound_instrument_playNote(Cranked *cranked, PDSynthInstrument_32 *inst, float frequency, float vel, float len, uint32_t when) {
    // Todo
    return nullptr;
}

PDSynth_32 *cranked::playdate_sound_instrument_playMIDINote(Cranked *cranked, PDSynthInstrument_32 *inst, float note, float vel, float len, uint32_t when) {
    // Todo
    return nullptr;
}

void cranked::playdate_sound_instrument_setPitchBend(Cranked *cranked, PDSynthInstrument_32 *inst, float bend) {
    inst->pitchBend = bend;
}

void cranked::playdate_sound_instrument_setPitchBendRange(Cranked *cranked, PDSynthInstrument_32 *inst, float halfSteps) {
    inst->pitchBendRangeHalfSteps = halfSteps;
}

void cranked::playdate_sound_instrument_setTranspose(Cranked *cranked, PDSynthInstrument_32 *inst, float halfSteps) {
    inst->transposeHalfSteps = halfSteps;
}

void cranked::playdate_sound_instrument_noteOff(Cranked *cranked, PDSynthInstrument_32 *inst, float note, uint32_t when) {
    // Todo
}

void cranked::playdate_sound_instrument_allNotesOff(Cranked *cranked, PDSynthInstrument_32 *inst, uint32_t when) {
    // Todo
}

void cranked::playdate_sound_instrument_setVolume(Cranked *cranked, PDSynthInstrument_32 *inst, float left, float right) {
    inst->leftVolume = left;
    inst->rightVolume = right;
}

void cranked::playdate_sound_instrument_getVolume(Cranked *cranked, PDSynthInstrument_32 *inst, float *left, float *right) {
    *left = inst->leftVolume;
    *right = inst->rightVolume;
}

int32_t cranked::playdate_sound_instrument_activeVoiceCount(Cranked *cranked, PDSynthInstrument_32 *inst) {
    // Todo
    return 0;
}

SequenceTrack_32 *cranked::playdate_sound_track_newTrack(Cranked *cranked) {
    return cranked->audio.allocateSoundSource<SequenceTrack_32>();
}

void cranked::playdate_sound_track_freeTrack(Cranked *cranked, SequenceTrack_32 *track) {
    cranked->audio.freeSoundSource(track);
}

void cranked::playdate_sound_track_setInstrument(Cranked *cranked, SequenceTrack_32 *track, PDSynthInstrument_32 *inst) {
    track->instrument = inst;
}

PDSynthInstrument_32 *cranked::playdate_sound_track_getInstrument(Cranked *cranked, SequenceTrack_32 *track) {
    return track->instrument;
}

void cranked::playdate_sound_track_addNoteEvent(Cranked *cranked, SequenceTrack_32 *track, uint32_t step, uint32_t len, float note, float velocity) {
    // Todo
}

void cranked::playdate_sound_track_removeNoteEvent(Cranked *cranked, SequenceTrack_32 *track, uint32_t step, float note) {
    // Todo
}

void cranked::playdate_sound_track_clearNotes(Cranked *cranked, SequenceTrack_32 *track) {
    // Todo
}

int32_t cranked::playdate_sound_track_getControlSignalCount(Cranked *cranked, SequenceTrack_32 *track) {
    // Todo
    return 0;
}

ControlSignal_32 *cranked::playdate_sound_track_getControlSignal(Cranked *cranked, SequenceTrack_32 *track, int32_t idx) {
    // Todo
    return nullptr;
}

void cranked::playdate_sound_track_clearControlEvents(Cranked *cranked, SequenceTrack_32 *track) {
    // Todo
}

int32_t cranked::playdate_sound_track_getPolyphony(Cranked *cranked, SequenceTrack_32 *track) {
    // Todo
    return 0;
}

int32_t cranked::playdate_sound_track_activeVoiceCount(Cranked *cranked, SequenceTrack_32 *track) {
    // Todo
    return 0;
}

void cranked::playdate_sound_track_setMuted(Cranked *cranked, SequenceTrack_32 *track, int32_t mute) {
    track->muted = mute;
}

uint32_t cranked::playdate_sound_track_getLength(Cranked *cranked, SequenceTrack_32 *track) {
    // Todo
    return 0;
}

int32_t cranked::playdate_sound_track_getIndexForStep(Cranked *cranked, SequenceTrack_32 *track, uint32_t step) {
    // Todo
    return 0;
}

int32_t cranked::playdate_sound_track_getNoteAtIndex(Cranked *cranked, SequenceTrack_32 *track, int32_t index, uint32_t *outStep, uint32_t *outLen, float *outNote, float *outVelocity) {
    // Todo
    return 0;
}

ControlSignal_32 *cranked::playdate_sound_track_getSignalForController(Cranked *cranked, SequenceTrack_32 *track, int32_t controller, int32_t create) {
    // Todo
    return nullptr;
}

SoundSequence_32 *cranked::playdate_sound_sequence_newSequence(Cranked *cranked) {
    return cranked->audio.allocateSoundSource<SoundSequence_32>();
}

void cranked::playdate_sound_sequence_freeSequence(Cranked *cranked, SoundSequence_32 *sequence) {
    cranked->audio.freeSoundSource(sequence);
}
int32_t cranked::playdate_sound_sequence_loadMIDIFile(Cranked *cranked, SoundSequence_32 *seq, uint8_t *path) {
    // Todo
    return 0;
}

uint32_t cranked::playdate_sound_sequence_getTime(Cranked *cranked, SoundSequence_32 *seq) {
    return seq->time;
}

void cranked::playdate_sound_sequence_setTime(Cranked *cranked, SoundSequence_32 *seq, uint32_t time) {
    seq->time = time;
}

void cranked::playdate_sound_sequence_setLoops(Cranked *cranked, SoundSequence_32 *seq, int32_t loopstart, int32_t loopend, int32_t loops) {
    seq->currentLoop = 0;
    seq->loopStart = loopstart;
    seq->loopEnd = loopend;
    seq->loops = loops;
}

int32_t cranked::playdate_sound_sequence_getTempo(Cranked *cranked, SoundSequence_32 *seq) {
    return (int32_t) seq->tempo; // Why still an integer? Breaking ABI change?
}

void cranked::playdate_sound_sequence_setTempo(Cranked *cranked, SoundSequence_32 * seq, float stepsPerSecond) {
    seq->tempo = stepsPerSecond;
}

void cranked::playdate_sound_sequence_setTempo_int(Cranked *cranked, SoundSequence_32 * seq, int32_t stepsPerSecond) {
    seq->tempo = stepsPerSecond;
}

int32_t cranked::playdate_sound_sequence_getTrackCount(Cranked *cranked, SoundSequence_32 *seq) {
    // Todo
    return 0;
}

SequenceTrack_32 *cranked::playdate_sound_sequence_addTrack(Cranked *cranked, SoundSequence_32 *seq) {
    // Todo
    return nullptr;
}

SequenceTrack_32 *cranked::playdate_sound_sequence_getTrackAtIndex(Cranked *cranked, SoundSequence_32 *seq, uint32_t track) {
    // Todo
    return nullptr;
}

void cranked::playdate_sound_sequence_setTrackAtIndex(Cranked *cranked, SoundSequence_32 *seq, SequenceTrack_32 *track, uint32_t idx) {
    // Todo
}

void cranked::playdate_sound_sequence_allNotesOff(Cranked *cranked, SoundSequence_32 *seq) {
    // Todo
}

int32_t cranked::playdate_sound_sequence_isPlaying(Cranked *cranked, SoundSequence_32 *seq) {
    return seq->playing;
}

uint32_t cranked::playdate_sound_sequence_getLength(Cranked *cranked, SoundSequence_32 *seq) {
    // Todo
    return 0;
}

void cranked::playdate_sound_sequence_play(Cranked *cranked, SoundSequence_32 *seq, cref_t finishCallback, void *userdata) {
    // Todo
}

void cranked::playdate_sound_sequence_stop(Cranked *cranked, SoundSequence_32 *seq) {
    // Todo
}

int32_t cranked::playdate_sound_sequence_getCurrentStep(Cranked *cranked, SoundSequence_32 *seq, int32_t *timeOffset) {
    // Todo
    return 0;
}

void cranked::playdate_sound_sequence_setCurrentStep(Cranked *cranked, SoundSequence_32 *seq, int32_t step, int32_t timeOffset, int32_t playNotes) {
    // Todo
}

TwoPoleFilter_32 *cranked::playdate_sound_effect_twopolefilter_newFilter(Cranked *cranked) {
    return cranked->audio.allocateSoundEffect<TwoPoleFilter_32>();
}

void cranked::playdate_sound_effect_twopolefilter_freeFilter(Cranked *cranked, TwoPoleFilter_32 *filter) {
    cranked->audio.freeSoundEffect(filter);
}

void cranked::playdate_sound_effect_twopolefilter_setType(Cranked *cranked, TwoPoleFilter_32 *filter, int32_t type) {
    filter->type = (TwoPoleFilterType) type;
}

void cranked::playdate_sound_effect_twopolefilter_setFrequency(Cranked *cranked, TwoPoleFilter_32 *filter, float frequency) {
    filter->frequency = frequency;
}

void cranked::playdate_sound_effect_twopolefilter_setFrequencyModulator(Cranked *cranked, TwoPoleFilter_32 *filter, PDSynthSignalValue_32 *signal) {
    filter->frequencyModulator = signal;
}

PDSynthSignalValue_32 *cranked::playdate_sound_effect_twopolefilter_getFrequencyModulator(Cranked *cranked, TwoPoleFilter_32 *filter) {
    return filter->frequencyModulator;
}

void cranked::playdate_sound_effect_twopolefilter_setGain(Cranked *cranked, TwoPoleFilter_32 *filter, float gain) {
    filter->gain = gain;
}

void cranked::playdate_sound_effect_twopolefilter_setResonance(Cranked *cranked, TwoPoleFilter_32 *filter, float resonance) {
    filter->resonance = resonance;
}

void cranked::playdate_sound_effect_twopolefilter_setResonanceModulator(Cranked *cranked, TwoPoleFilter_32 *filter, PDSynthSignalValue_32 *signal) {
    filter->resonanceModulator = signal;
}

PDSynthSignalValue_32 *cranked::playdate_sound_effect_twopolefilter_getResonanceModulator(Cranked *cranked, TwoPoleFilter_32 *filter) {
    return filter->resonanceModulator;
}

OnePoleFilter_32 *cranked::playdate_sound_effect_onepolefilter_newFilter(Cranked *cranked) {
    return cranked->audio.allocateSoundEffect<OnePoleFilter_32>();
}

void cranked::playdate_sound_effect_onepolefilter_freeFilter(Cranked *cranked, OnePoleFilter_32 *filter) {
    cranked->audio.freeSoundEffect(filter);
}

void cranked::playdate_sound_effect_onepolefilter_setParameter(Cranked *cranked, OnePoleFilter_32 *filter, float parameter) {
    filter->cutoffFrequency = parameter;
}

void cranked::playdate_sound_effect_onepolefilter_setParameterModulator(Cranked *cranked, OnePoleFilter_32 *filter, PDSynthSignalValue_32 *signal) {
    filter->cutoffFrequencyModulator = signal;
}

PDSynthSignalValue_32 *cranked::playdate_sound_effect_onepolefilter_getParameterModulator(Cranked *cranked, OnePoleFilter_32 *filter) {
    return filter->cutoffFrequencyModulator;
}

BitCrusher_32 *cranked::playdate_sound_effect_bitcrusher_newBitCrusher(Cranked *cranked) {
    return cranked->audio.allocateSoundEffect<BitCrusher_32>();
}

void cranked::playdate_sound_effect_bitcrusher_freeBitCrusher(Cranked *cranked, BitCrusher_32 *filter) {
    cranked->audio.freeSoundEffect(filter);
}

void cranked::playdate_sound_effect_bitcrusher_setAmount(Cranked *cranked, BitCrusher_32 *filter, float amount) {
    filter->amount = amount;
}

void cranked::playdate_sound_effect_bitcrusher_setAmountModulator(Cranked *cranked, BitCrusher_32 *filter, PDSynthSignalValue_32 *signal) {
    filter->amountModulator = signal;
}

PDSynthSignalValue_32 *cranked::playdate_sound_effect_bitcrusher_getAmountModulator(Cranked *cranked, BitCrusher_32 *filter) {
    return filter->amountModulator;
}

void cranked::playdate_sound_effect_bitcrusher_setUndersampling(Cranked *cranked, BitCrusher_32 *filter, float undersampling) {
    filter->undersampling = undersampling;
}

void cranked::playdate_sound_effect_bitcrusher_setUndersampleModulator(Cranked *cranked, BitCrusher_32 *filter, PDSynthSignalValue_32 *signal) {
    filter->undersamplingModulator = signal;
}

PDSynthSignalValue_32 *cranked::playdate_sound_effect_bitcrusher_getUndersampleModulator(Cranked *cranked, BitCrusher_32 *filter) {
    return filter->undersamplingModulator;
}

RingModulator_32 *cranked::playdate_sound_effect_ringmodulator_newRingmod(Cranked *cranked) {
    return cranked->audio.allocateSoundEffect<RingModulator_32>();
}

void cranked::playdate_sound_effect_ringmodulator_freeRingmod(Cranked *cranked, RingModulator_32 *filter) {
    cranked->audio.freeSoundEffect(filter);
}

void cranked::playdate_sound_effect_ringmodulator_setFrequency(Cranked *cranked, RingModulator_32 *filter, float frequency) {
    filter->frequency = frequency;
}

void cranked::playdate_sound_effect_ringmodulator_setFrequencyModulator(Cranked *cranked, RingModulator_32 *filter, PDSynthSignalValue_32 *signal) {
    filter->frequencyModulator = signal;
}

PDSynthSignalValue_32 *cranked::playdate_sound_effect_ringmodulator_getFrequencyModulator(Cranked *cranked, RingModulator_32 *filter) {
    return filter->frequencyModulator;
}

DelayLine_32 *cranked::playdate_sound_effect_delayline_newDelayLine(Cranked *cranked, int32_t length, int32_t stereo) {
    return cranked->audio.allocateSoundEffect<DelayLine_32>((int) length, (bool) stereo);
}

void cranked::playdate_sound_effect_delayline_freeDelayLine(Cranked *cranked, DelayLine_32 *filter) {
    cranked->audio.freeSoundEffect(filter);
}

void cranked::playdate_sound_effect_delayline_setLength(Cranked *cranked, DelayLine_32 *d, int32_t frames) {
    d->data.clear();
    d->data.resize((d->stereo ? 2 : 1) * frames);
}

void cranked::playdate_sound_effect_delayline_setFeedback(Cranked *cranked, DelayLine_32 *d, float fb) {
    d->feedback = fb;
}

DelayLineTap_32 *cranked::playdate_sound_effect_delayline_addTap(Cranked *cranked, DelayLine_32 *d, int32_t delay) {
    return cranked->audio.allocateSoundSource<DelayLineTap_32>(d, (int) delay);
}

void cranked::playdate_sound_effect_delayline_freeTap(Cranked *cranked, DelayLineTap_32 *tap) {
    cranked->audio.freeSoundSource(tap);
}

void cranked::playdate_sound_effect_delayline_setTapDelay(Cranked *cranked, DelayLineTap_32 *t, int32_t frames) {
    t->delayFrames = frames;
}

void cranked::playdate_sound_effect_delayline_setTapDelayModulator(Cranked *cranked, DelayLineTap_32 *t, PDSynthSignalValue_32 *mod) {
    t->delayModulator = mod;
}

PDSynthSignalValue_32 *cranked::playdate_sound_effect_delayline_getTapDelayModulator(Cranked *cranked, DelayLineTap_32 *t) {
    return t->delayModulator;
}

void cranked::playdate_sound_effect_delayline_setTapChannelsFlipped(Cranked *cranked, DelayLineTap_32 *t, int32_t flip) {
    t->channelsFlipped = flip;
}

Overdrive_32 *cranked::playdate_sound_effect_overdrive_newOverdrive(Cranked *cranked) {
    return cranked->audio.allocateSoundEffect<Overdrive_32>();
}

void cranked::playdate_sound_effect_overdrive_freeOverdrive(Cranked *cranked, Overdrive_32 *filter) {
    cranked->audio.freeSoundEffect(filter);
}

void cranked::playdate_sound_effect_overdrive_setGain(Cranked *cranked, Overdrive_32 *o, float gain) {
    o->gain = gain;
}

void cranked::playdate_sound_effect_overdrive_setLimit(Cranked *cranked, Overdrive_32 *o, float limit) {
    o->limit = limit;
}

void cranked::playdate_sound_effect_overdrive_setLimitModulator(Cranked *cranked, Overdrive_32 *o, PDSynthSignalValue_32 *mod) {
    o->limitModulator = mod;
}

PDSynthSignalValue_32 *cranked::playdate_sound_effect_overdrive_getLimitModulator(Cranked *cranked, Overdrive_32 *o) {
    return o->limitModulator;
}

void cranked::playdate_sound_effect_overdrive_setOffset(Cranked *cranked, Overdrive_32 *o, float offset) {
    o->offset = offset;
}

void cranked::playdate_sound_effect_overdrive_setOffsetModulator(Cranked *cranked, Overdrive_32 *o, PDSynthSignalValue_32 *mod) {
    o->offsetModulator = mod;
}

PDSynthSignalValue_32 *cranked::playdate_sound_effect_overdrive_getOffsetModulator(Cranked *cranked, Overdrive_32 *o) {
    return o->offsetModulator;
}

SoundEffect_32 *cranked::playdate_sound_effect_newEffect(Cranked *cranked, cref_t proc, void *userdata) {
    // Todo
    return nullptr;
}

void cranked::playdate_sound_effect_freeEffect(Cranked *cranked, SoundEffect_32 *effect) {
    cranked->audio.freeSoundEffect(effect);
}

void cranked::playdate_sound_effect_setMix(Cranked *cranked, SoundEffect_32 *effect, float level) {
    effect->mixLevel = level;
}

void cranked::playdate_sound_effect_setMixModulator(Cranked *cranked, SoundEffect_32 *effect, PDSynthSignalValue_32 *signal) {
    effect->mixModulator = signal;
}

PDSynthSignalValue_32 *cranked::playdate_sound_effect_getMixModulator(Cranked *cranked, SoundEffect_32 *effect) {
    return effect->mixModulator;
}

void cranked::playdate_sound_effect_setUserdata(Cranked *cranked, SoundEffect_32 *effect, void *userdata) {
    effect->userdata = cranked->toVirtualAddress(userdata);
}

void *cranked::playdate_sound_effect_getUserdata(Cranked *cranked, SoundEffect_32 *effect) {
    return cranked->fromVirtualAddress(effect->userdata);
}

SoundChannel_32 *cranked::playdate_sound_channel_newChannel(Cranked *cranked) {
    return cranked->audio.allocateChannel();
}

void cranked::playdate_sound_channel_freeChannel(Cranked *cranked, SoundChannel_32 *channel) {
    cranked->audio.freeChannel(channel);
}

int32_t cranked::playdate_sound_channel_addSource(Cranked *cranked, SoundChannel_32 *channel, SoundSource_32 *source) {
    if (std::find(channel->sources.begin(), channel->sources.end(), source) != channel->sources.end())
        return false;
    channel->sources.push_back(source);
    return true;
}

int32_t cranked::playdate_sound_channel_removeSource(Cranked *cranked, SoundChannel_32 *channel, SoundSource_32 *source) {
    return std::erase(channel->sources, source) > 0;
}

SoundSource_32 *cranked::playdate_sound_channel_addCallbackSource(Cranked *cranked, SoundChannel_32 *channel, cref_t callback, void *context, int32_t stereo) {
    // Todo
    return nullptr;
}

void cranked::playdate_sound_channel_addEffect(Cranked *cranked, SoundChannel_32 *channel, SoundEffect_32 *effect) {
    channel->effects.push_back(effect);
}

void cranked::playdate_sound_channel_removeEffect(Cranked *cranked, SoundChannel_32 *channel, SoundEffect_32 *effect) {
    std::erase(channel->effects, effect);
}

void cranked::playdate_sound_channel_setVolume(Cranked *cranked, SoundChannel_32 *channel, float volume) {
    channel->volume = volume;
}

float cranked::playdate_sound_channel_getVolume(Cranked *cranked, SoundChannel_32 *channel) {
    return channel->volume;
}

void cranked::playdate_sound_channel_setVolumeModulator(Cranked *cranked, SoundChannel_32 *channel, PDSynthSignalValue_32 *mod) {
    channel->volumeModulator = mod;
}

PDSynthSignalValue_32 *cranked::playdate_sound_channel_getVolumeModulator(Cranked *cranked, SoundChannel_32 *channel) {
    return channel->volumeModulator;
}

void cranked::playdate_sound_channel_setPan(Cranked *cranked, SoundChannel_32 *channel, float pan) {
    channel->pan = pan;
}

void cranked::playdate_sound_channel_setPanModulator(Cranked *cranked, SoundChannel_32 *channel, PDSynthSignalValue_32 *mod) {
    channel->panModulator = mod;
}

PDSynthSignalValue_32 *cranked::playdate_sound_channel_getPanModulator(Cranked *cranked, SoundChannel_32 *channel) {
    return channel->panModulator;
}

PDSynthSignalValue_32 *cranked::playdate_sound_channel_getDryLevelSignal(Cranked *cranked, SoundChannel_32 *channel) {
    // Todo
    return nullptr;
}

PDSynthSignalValue_32 *cranked::playdate_sound_channel_getWetLevelSignal(Cranked *cranked, SoundChannel_32 *channel) {
    // Todo
    return nullptr;
}

uint32_t cranked::playdate_sound_getCurrentTime(Cranked *cranked) {
    return cranked->audio.sampleTime;
}

SoundSource_32 *cranked::playdate_sound_addSource(Cranked *cranked, cref_t callback, void *context, int32_t stereo) {
    // Todo
    return nullptr;
}

SoundChannel_32 *cranked::playdate_sound_getDefaultChannel(Cranked *cranked) {
    return cranked->audio.mainChannel;
}

int32_t cranked::playdate_sound_addChannel(Cranked *cranked, SoundChannel_32 *channel) {
    if (channel == cranked->audio.mainChannel)
        return false;
    bool success = !cranked->audio.activeChannels.contains(channel);
    cranked->audio.activeChannels.emplace(channel);
    return success;
}

int32_t cranked::playdate_sound_removeChannel(Cranked *cranked, SoundChannel_32 *channel) {
    bool success = cranked->audio.activeChannels.contains(channel);
    cranked->audio.activeChannels.erase(channel);
    return success;
}

int32_t cranked::playdate_sound_setMicCallback(Cranked *cranked, cref_t callback, void *context, int32_t forceInternal) {
    cranked->audio.micCallback = callback;
    cranked->audio.micCallbackUserdata = cranked->toVirtualAddress(context);
    cranked->audio.micCallbackSource = (MicSource)forceInternal;
    return (int)MicSource::Autodetect; // Todo
}

void cranked::playdate_sound_getHeadphoneState(Cranked *cranked, int32_t *headphone, int32_t *headsetmic, cref_t changeCallback) {
    *headphone = cranked->audio.headphonesConnected;
    *headsetmic = cranked->audio.headsetMicConnected;
    cranked->audio.headsetStateCallback = changeCallback;
}

void cranked::playdate_sound_setOutputsActive(Cranked *cranked, int32_t headphone, int32_t speaker) {
    cranked->audio.headphoneOutputActive = headphone;
    cranked->audio.speakerOutputActive = speaker;
}

int32_t cranked::playdate_sound_removeSource(Cranked *cranked, SoundSource_32 *source) {
    if (std::erase(cranked->audio.mainChannel->sources, source))
        return true;
    for (auto channel : cranked->audio.activeChannels)
        if (std::erase(channel->sources, source))
            return true;
    return false;
}

int32_t cranked::playdate_display_getWidth(Cranked *cranked) {
    return DISPLAY_WIDTH / cranked->graphics.displayScale;
}

int32_t cranked::playdate_display_getHeight(Cranked *cranked) {
    return DISPLAY_HEIGHT / cranked->graphics.displayScale;
}

void cranked::playdate_display_setRefreshRate(Cranked *cranked, float rate) {
    cranked->graphics.framerate = std::max(1.0f, rate);
}

void cranked::playdate_display_setInverted(Cranked *cranked, int32_t flag) {
    cranked->graphics.displayInverted = flag;
}

void cranked::playdate_display_setScale(Cranked *cranked, uint32_t s) {
    if (s == 1 or s == 2 or s == 4 or s == 8)
        cranked->graphics.displayScale = s;
    else
        cranked->graphics.displayScale = 1;
}

void cranked::playdate_display_setMosaic(Cranked *cranked, uint32_t x, uint32_t y) {
    cranked->graphics.displayMosaic = {(int)x & 0x3, (int)y & 3};
}

void cranked::playdate_display_setFlipped(Cranked *cranked, int32_t x, int32_t y) {
    cranked->graphics.displayFlippedX = x;
    cranked->graphics.displayFlippedY = y;
}

void cranked::playdate_display_setOffset(Cranked *cranked, int32_t x, int32_t y) {
    cranked->graphics.displayOffset = {x, y};
}

int32_t cranked::playdate_scoreboards_addScore(Cranked *cranked, uint8_t *boardId, uint32_t value, cref_t callback) {
    // Todo
    return 0;
}

int32_t cranked::playdate_scoreboards_getPersonalBest(Cranked *cranked, uint8_t *boardId, cref_t callback) {
    // Todo
    return 0;
}

void cranked::playdate_scoreboards_freeScore(Cranked *cranked, PDScore_32 *score) {
    // Todo
}

int32_t cranked::playdate_scoreboards_getScoreboards(Cranked *cranked, cref_t callback) {
    // Todo
    return 0;
}

void cranked::playdate_scoreboards_freeBoardsList(Cranked *cranked, PDBoardsList_32 *boardsList) {
    // Todo
}

int32_t cranked::playdate_scoreboards_getScores(Cranked *cranked, uint8_t *boardId, cref_t callback) {
    // Todo
    return 0;
}

void cranked::playdate_scoreboards_freeScoresList(Cranked *cranked, PDScoresList_32 *scoresList) {
    // Todo
}
