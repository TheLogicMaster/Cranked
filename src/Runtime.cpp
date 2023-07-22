#include "Emulator.hpp"
#include "gen/PlaydateAPI.hpp"

#include <filesystem>

LCDVideoPlayer_32 *playdate_video_loadVideo(Emulator *emulator, uint8_t *path) {
    // Todo
    return nullptr;
}

void playdate_video_freePlayer(Emulator *emulator, LCDVideoPlayer_32 *p) {
    // Todo
}

int32_t playdate_video_setContext(Emulator *emulator, LCDVideoPlayer_32 *p, LCDBitmap_32 *context) {
    // Todo
    return 0;
}

void playdate_video_useScreenContext(Emulator *emulator, LCDVideoPlayer_32 *p) {
    // Todo
}

int32_t playdate_video_renderFrame(Emulator *emulator, LCDVideoPlayer_32 *p, int32_t n) {
    // Todo
    return 0;
}

uint8_t *playdate_video_getError(Emulator *emulator, LCDVideoPlayer_32 *p) {
    // Todo
    return nullptr;
}

void playdate_video_getInfo(Emulator *emulator, LCDVideoPlayer_32 *p, int32_t *outWidth, int32_t *outHeight, float *outFrameRate, int32_t *outFrameCount, int32_t *outCurrentFrame) {
    // Todo
}

LCDBitmap_32 *playdate_video_getContext(Emulator *emulator, LCDVideoPlayer_32 *p) {
    // Todo
    return nullptr;
}

void playdate_graphics_clear(Emulator *emulator, uint32_t color) {
    emulator->graphics.frameBufferContext.bitmap->clear(color);
}

void playdate_graphics_setBackgroundColor(Emulator *emulator, int32_t color) {
    emulator->graphics.getCurrentDisplayContext().backgroundColor = LCDSolidColor(color);
}

void playdate_graphics_setStencil(Emulator *emulator, LCDBitmap_32 *stencil) {
    emulator->graphics.getCurrentDisplayContext().stencilImage = stencil;
}

void playdate_graphics_setDrawMode(Emulator *emulator, int32_t mode) {
    emulator->graphics.getCurrentDisplayContext().bitmapDrawMode = LCDBitmapDrawMode(mode);
}

void playdate_graphics_setDrawOffset(Emulator *emulator, int32_t dx, int32_t dy) {
    emulator->graphics.getCurrentDisplayContext().drawOffset = {dx, dy};
}

void playdate_graphics_setClipRect(Emulator *emulator, int32_t x, int32_t y, int32_t width, int32_t height) {
    emulator->graphics.getCurrentDisplayContext().clipRect = {x, y, width, height};
}

void playdate_graphics_clearClipRect(Emulator *emulator) {
    emulator->graphics.getCurrentDisplayContext().clipRect = {};
}

void playdate_graphics_setLineCapStyle(Emulator *emulator, int32_t endCapStyle) {
    emulator->graphics.getCurrentDisplayContext().lineEndCapStype = LCDLineCapStyle(endCapStyle);
}

void playdate_graphics_setFont(Emulator *emulator, LCDFont_32 *font) {
    emulator->graphics.getCurrentDisplayContext().font = font;
}

void playdate_graphics_setTextTracking(Emulator *emulator, int32_t tracking) {
    emulator->graphics.getCurrentDisplayContext().textTracking = tracking;
}

void playdate_graphics_pushContext(Emulator *emulator, LCDBitmap_32 *target) {
    emulator->graphics.pushContext(target);
}

void playdate_graphics_popContext(Emulator *emulator) {
    emulator->graphics.popContext();
}

void playdate_graphics_drawBitmap(Emulator *emulator, LCDBitmap_32 *bitmap, int32_t x, int32_t y, int32_t flip) {
    emulator->graphics.drawBitmap(bitmap, x, y, LCDBitmapFlip(flip));
}

void playdate_graphics_tileBitmap(Emulator *emulator, LCDBitmap_32 *bitmap, int32_t x, int32_t y, int32_t width, int32_t height, int32_t flip) {
    // Todo
}

void playdate_graphics_drawLine(Emulator *emulator, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t width, uint32_t color) {
    emulator->graphics.drawLine(x1, y1, x2, y2, width, color);
}

void playdate_graphics_fillTriangle(Emulator *emulator, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint32_t color) {
    emulator->graphics.fillTriangle(x1, y1, x2, y2, x3, y3, color);
}

void playdate_graphics_drawRect(Emulator *emulator, int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color) {
    emulator->graphics.drawRect(x, y, width, height, color);
}

void playdate_graphics_fillRect(Emulator *emulator, int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color) {
    emulator->graphics.fillRect(x, y, width, height, color);
}

void playdate_graphics_drawEllipse(Emulator *emulator, int32_t x, int32_t y, int32_t width, int32_t height, int32_t lineWidth, float startAngle, float endAngle, uint32_t color) {
    emulator->graphics.drawEllipse(x, y, width, height, lineWidth, startAngle, endAngle, color, false);
}

void playdate_graphics_fillEllipse(Emulator *emulator, int32_t x, int32_t y, int32_t width, int32_t height, float startAngle, float endAngle, uint32_t color) {
    emulator->graphics.drawEllipse(x, y, width, height, 0, startAngle, endAngle, color, true);
}

void playdate_graphics_drawScaledBitmap(Emulator *emulator, LCDBitmap_32 *bitmap, int32_t x, int32_t y, float xscale, float yscale) {
    // Todo
}

int32_t playdate_graphics_drawText(Emulator *emulator, void *text, uint32_t len, int32_t encoding, int32_t x, int32_t y) {
    emulator->graphics.drawText(text, len, PDStringEncoding(encoding), x, y);
    return 0; // Todo: Return value?
}

LCDBitmap_32 *playdate_graphics_newBitmap(Emulator *emulator, int32_t width, int32_t height, uint32_t bgcolor) {
    auto bitmap = emulator->graphics.allocateBitmap(width, height);
    bitmap->clear(bgcolor);
    return bitmap;
}

void playdate_graphics_freeBitmap(Emulator *emulator, LCDBitmap_32 *ptr) {
    emulator->graphics.freeBitmap(ptr);
}

LCDBitmap_32 *playdate_graphics_loadBitmap(Emulator *emulator, uint8_t *path, cref_t *outerr) {
    try {
        return emulator->graphics.getImage((const char *) path);
    } catch (std::exception &ex) {
        *outerr = emulator->getEmulatedStringLiteral(ex.what());
        return nullptr;
    }
}

LCDBitmap_32 *playdate_graphics_copyBitmap(Emulator *emulator, LCDBitmap_32 *bitmap) {
    return emulator->heap.construct<LCDBitmap_32>(*bitmap);
}

void playdate_graphics_loadIntoBitmap(Emulator *emulator, uint8_t *path, LCDBitmap_32 *bitmap, cref_t *outerr) {
    try {
        auto loaded = emulator->graphics.getImage((const char *) path);
        *bitmap = *loaded;
        emulator->graphics.freeBitmap(loaded);
    } catch (std::exception &ex) {
        if (outerr)
            *outerr = emulator->getEmulatedStringLiteral(ex.what());
    }
}

void playdate_graphics_getBitmapData(Emulator *emulator, LCDBitmap_32 *bitmap, int32_t *width, int32_t *height, int32_t *rowbytes, cref_t *mask, cref_t *data) {
    *width = bitmap->width;
    *height = bitmap->height;
    *rowbytes = 1 + bitmap->width / 8;
    *data = emulator->toVirtualAddress(bitmap->data.data());
    *mask = emulator->toVirtualAddress(bitmap->mask ? bitmap->mask->data.data() : nullptr);
}

void playdate_graphics_clearBitmap(Emulator *emulator, LCDBitmap_32 *bitmap, uint32_t bgcolor) {
    bitmap->clear(bgcolor);
}

LCDBitmap_32 *playdate_graphics_rotatedBitmap(Emulator *emulator, LCDBitmap_32 *bitmap, float rotation, float xscale, float yscale, int32_t *allocedSize) {
    // Todo
    return nullptr;
}

LCDBitmapTable_32 *playdate_graphics_newBitmapTable(Emulator *emulator, int32_t count, int32_t width, int32_t height) {
    return emulator->graphics.allocateBitmapTable(count);
}

void playdate_graphics_freeBitmapTable(Emulator *emulator, LCDBitmapTable_32 *table) {
    emulator->graphics.freeBitmapTable(table);
}

LCDBitmapTable_32 *playdate_graphics_loadBitmapTable(Emulator *emulator, uint8_t *path, cref_t *outerr) {
    try {
        return emulator->graphics.getBitmapTable((const char *) path);
    } catch (std::exception &ex) {
        *outerr = emulator->getEmulatedStringLiteral(ex.what());
        return nullptr;
    }
}

void playdate_graphics_loadIntoBitmapTable(Emulator *emulator, uint8_t *path, LCDBitmapTable_32 *table, cref_t *outerr) {
    try {
        auto loaded = emulator->graphics.getBitmapTable((const char *) path);
        *table = *loaded;
        emulator->graphics.freeBitmapTable(loaded);
    } catch (std::exception &ex) {
        *outerr = emulator->getEmulatedStringLiteral(ex.what());
    }
}

LCDBitmap_32 *playdate_graphics_getTableBitmap(Emulator *emulator, LCDBitmapTable_32 *table, int32_t idx) {
    return idx < table->bitmaps.size() ? &table->bitmaps[idx] : nullptr;
}

LCDFont_32 *playdate_graphics_loadFont(Emulator *emulator, uint8_t *path, cref_t *outerr) {
    try {
        return emulator->graphics.getFont((const char *) path);
    } catch (std::exception &ex) {
        *outerr = emulator->getEmulatedStringLiteral(ex.what());
        return nullptr;
    }
}

LCDFontPage_32 *playdate_graphics_getFontPage(Emulator *emulator, LCDFont_32 *font, uint32_t c) {
    try {
        return &font->pages.at(c / 256);
    } catch (std::out_of_range &ex) {
        return nullptr;
    }
}

LCDFontGlyph_32 *playdate_graphics_getPageGlyph(Emulator *emulator, LCDFontPage_32 *page, uint32_t c, cref_t *bitmap, int32_t *advance) {
    try {
        auto glyph = &page->glyphs.at(c % 256);
        if (bitmap)
            *bitmap = emulator->toVirtualAddress(&glyph->bitmap);
        if (advance)
            *advance = glyph->advance;
        return glyph;
    } catch (std::out_of_range &ex) {
        return nullptr;
    }
}

int32_t playdate_graphics_getGlyphKerning(Emulator *emulator, LCDFontGlyph_32 *glyph, uint32_t glyphcode, uint32_t nextcode) {
    try {
        return nextcode < 256 ? glyph->shortKerningTable.at(nextcode) : glyph->longKerningTable.at(nextcode);
    } catch (std::out_of_range &ex) {
        return 0;
    }
}

int32_t playdate_graphics_getTextWidth(Emulator *emulator, LCDFont_32 *font, void *text, uint32_t len, int32_t encoding, int32_t tracking) {
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

uint8_t *playdate_graphics_getFrame(Emulator *emulator) {
    return (uint8_t *) emulator->graphics.frameBuffer->data.data();
}

uint8_t *playdate_graphics_getDisplayFrame(Emulator *emulator) {
    return (uint8_t *) emulator->graphics.previousFrameBuffer->data.data();
}

LCDBitmap_32 *playdate_graphics_copyFrameBufferBitmap(Emulator *emulator) {
    return emulator->heap.construct<LCDBitmap_32>(*emulator->graphics.frameBuffer);
}

void playdate_graphics_markUpdatedRows(Emulator *emulator, int32_t start, int32_t end) {
    // Not needed, since getFrame() returns the actual frame buffer
}

void playdate_graphics_display(Emulator *emulator) {
    emulator->graphics.flushDisplayBuffer();
}

void playdate_graphics_setColorToPattern(Emulator *emulator, uint32_t *color, LCDBitmap_32 *bitmap, int32_t x, int32_t y) {
    // Todo
}

int32_t playdate_graphics_checkMaskCollision(Emulator *emulator, LCDBitmap_32 *bitmap1, int32_t x1, int32_t y1, int32_t flip1, LCDBitmap_32 *bitmap2, int32_t x2, int32_t y2, int32_t flip2, LCDRect_32 rect) {
    // Todo
    return 0;
}

void playdate_graphics_setScreenClipRect(Emulator *emulator, int32_t x, int32_t y, int32_t width, int32_t height) {
    emulator->graphics.getCurrentDisplayContext().clipRect = {x, y, width, height};
}

void playdate_graphics_fillPolygon(Emulator *emulator, int32_t nPoints, int32_t *coords, uint32_t color, int32_t fillrule) {
    // Todo
}

uint8_t playdate_graphics_getFontHeight(Emulator *emulator, LCDFont_32 *font) {
    // Todo
    return 0;
}

LCDBitmap_32 *playdate_graphics_getDisplayBufferBitmap(Emulator *emulator) {
    return emulator->graphics.frameBuffer;
}

void playdate_graphics_drawRotatedBitmap(Emulator *emulator, LCDBitmap_32 *bitmap, int32_t x, int32_t y, float rotation, float centerx, float centery, float xscale, float yscale) {
    // Todo
}

void playdate_graphics_setTextLeading(Emulator *emulator, int32_t lineHeightAdustment) {
    emulator->graphics.getCurrentDisplayContext().textLeading = lineHeightAdustment;
}

int32_t playdate_graphics_setBitmapMask(Emulator *emulator, LCDBitmap_32 *bitmap, LCDBitmap_32 *mask) {
    if (bitmap->mask)
        emulator->graphics.freeBitmap(bitmap->mask);
    bitmap->mask = mask; // Todo: Make copy?
    return 0; // Todo: Return value?
}

LCDBitmap_32 *playdate_graphics_getBitmapMask(Emulator *emulator, LCDBitmap_32 *bitmap) {
    return bitmap->mask;
}

void playdate_graphics_setStencilImage(Emulator *emulator, LCDBitmap_32 *stencil, int32_t tile) {
    emulator->graphics.getCurrentDisplayContext().stencilImage = stencil;
    emulator->graphics.getCurrentDisplayContext().stencilTiled = tile;
}

LCDFont_32 *playdate_graphics_makeFontFromData(Emulator *emulator, LCDFontData_32 *data, int32_t wide) {
    return emulator->graphics.getFont((uint8_t *) data, wide);
}

void *playdate_sys_realloc(Emulator *emulator, void* ptr, uint32_t size) {
    // Todo: Check for objects that need to be user freed and call destructors
    if (size == 0) {
        if (!emulator->graphics.handleFree(ptr)) // Handle allocated objects that need to be freed with `system->realloc`
            emulator->heap.free(ptr);
        return nullptr;
    } else
        return emulator->heap.reallocate(ptr, size);
}

int32_t playdate_sys_formatString(Emulator *emulator, cref_t *ret, uint8_t *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    auto size = vsnprintf(nullptr, 0, (const char *) fmt, args);
    auto buffer = (uint8_t *) emulator->heap.allocate(size + 1);
    *ret = emulator->toVirtualAddress(buffer);
    vsprintf((char *) buffer, (const char *) fmt, args);
    va_end(args);
    return size;
}

void playdate_sys_logToConsole(Emulator *emulator, uint8_t *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf((const char *) fmt, args);
    va_end(args);
    printf("\n");// Doesn't depend on Lua setNewlinePrinted flag
}

void playdate_sys_error(Emulator *emulator, uint8_t *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf((const char *) fmt, args);
    va_end(args);
    emulator->stop();
}

int32_t playdate_sys_getLanguage(Emulator *emulator) {
    return (int) PDLanguage::English;
}

uint32_t playdate_sys_getCurrentTimeMilliseconds(Emulator *emulator) {
    return emulator->currentMillis;
}

uint32_t playdate_sys_getSecondsSinceEpoch(Emulator *emulator, uint32_t *milliseconds) {
    return duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

void playdate_sys_drawFPS(Emulator *emulator, int32_t x, int32_t y) {
    // Todo: Fix (There's probably a memory bug involved here...)
    auto delta = duration_cast<std::chrono::milliseconds>((std::chrono::system_clock::now() - emulator->lastFrameTime)).count();
    auto string = std::to_string(int(1000.0f / delta));
    emulator->graphics.fillRect(x - 2, y - 2, 14 * string.size() + 2, 30, LCDSolidColor::White);
    emulator->graphics.pushContext(emulator->graphics.getTargetBitmap());
    emulator->graphics.getCurrentDisplayContext().bitmapDrawMode = LCDBitmapDrawMode::Copy;
    emulator->graphics.drawText(string.c_str(), string.size(), PDStringEncoding::ASCII, x, y, emulator->graphics.systemFont);
    emulator->graphics.popContext();
}

void playdate_sys_setUpdateCallback(Emulator *emulator, cref_t update, void *userdata) {
    emulator->nativeUpdateCallback = update;
    emulator->nativeUpdateUserdata = emulator->toVirtualAddress(userdata);
}

void playdate_sys_getButtonState(Emulator *emulator, int32_t *current, int32_t *pushed, int32_t *released) {
    *current = emulator->currentInputs;
    *pushed = emulator->pressedInputs;
    *released = emulator->releasedInputs;
}

void playdate_sys_setPeripheralsEnabled(Emulator *emulator, int32_t mask) {
    // Not needed
}

void playdate_sys_getAccelerometer(Emulator *emulator, float *outx, float *outy, float *outz) {
    *outx = emulator->accelerometerX;
    *outy = emulator->accelerometerY;
    *outz = emulator->accelerometerZ;
}

float playdate_sys_getCrankChange(Emulator *emulator) {
    return emulator->getCrankChange();
}

float playdate_sys_getCrankAngle(Emulator *emulator) {
    return emulator->crankAngle;
}

int32_t playdate_sys_isCrankDocked(Emulator *emulator) {
    return emulator->crankDocked;
}

int32_t playdate_sys_setCrankSoundsDisabled(Emulator *emulator, int32_t flag) {
    auto prev = emulator->crankSoundsEnabled;
    emulator->crankSoundsEnabled = flag;
    return prev;
}

int32_t playdate_sys_getFlipped(Emulator *emulator) {
    return emulator->systemFlipped;
}

void playdate_sys_setAutoLockDisabled(Emulator *emulator, int32_t disable) {
    emulator->autoLockDisabled = disable;
}

void playdate_sys_setMenuImage(Emulator *emulator, LCDBitmap_32 *bitmap, int32_t xOffset) {
    // Todo
}

PDMenuItem_32 *playdate_sys_addMenuItem(Emulator *emulator, uint8_t *title, cref_t callback, void *userdata) {
    // Todo
    return nullptr;
}

PDMenuItem_32 *playdate_sys_addCheckmarkMenuItem(Emulator *emulator, uint8_t *title, int32_t value, cref_t callback, void *userdata) {
    // Todo
    return nullptr;
}

PDMenuItem_32 *playdate_sys_addOptionsMenuItem(Emulator *emulator, uint8_t *title, cref_t *optionTitles, int32_t optionsCount, cref_t f, void *userdata) {
    // Todo
    return nullptr;
}

void playdate_sys_removeAllMenuItems(Emulator *emulator) {
    // Todo
}

void playdate_sys_removeMenuItem(Emulator *emulator, PDMenuItem_32 *menuItem) {
    // Todo
}

int32_t playdate_sys_getMenuItemValue(Emulator *emulator, PDMenuItem_32 *menuItem) {
    // Todo
    return 0;
}

void playdate_sys_setMenuItemValue(Emulator *emulator, PDMenuItem_32 *menuItem, int32_t value) {
    // Todo
}

uint8_t *playdate_sys_getMenuItemTitle(Emulator *emulator, PDMenuItem_32 *menuItem) {
    // Todo
    return nullptr;
}

void playdate_sys_setMenuItemTitle(Emulator *emulator, PDMenuItem_32 *menuItem, uint8_t *title) {
    // Todo
}

void *playdate_sys_getMenuItemUserdata(Emulator *emulator, PDMenuItem_32 *menuItem) {
    // Todo
    return nullptr;
}

void playdate_sys_setMenuItemUserdata(Emulator *emulator, PDMenuItem_32 *menuItem, void *ud) {
    // Todo
}

int32_t playdate_sys_getReduceFlashing(Emulator *emulator) {
    return emulator->tryReduceFlashing;
}

float playdate_sys_getElapsedTime(Emulator *emulator) {
    return duration_cast<std::chrono::microseconds>((std::chrono::system_clock::now() - emulator->elapsedTimeStart)).count() / 1000000.0f;
}

void playdate_sys_resetElapsedTime(Emulator *emulator) {
    emulator->elapsedTimeStart = std::chrono::system_clock::now();
}

float playdate_sys_getBatteryPercentage(Emulator *emulator) {
    return 100;
}

float playdate_sys_getBatteryVoltage(Emulator *emulator) {
    return 4.2f;
}

int32_t playdate_sys_getTimezoneOffset(Emulator *emulator) {
    return 0; // Todo
}

int32_t playdate_sys_shouldDisplay24HourTime(Emulator *emulator) {
    return 0; // Todo
}

void playdate_sys_convertEpochToDateTime(Emulator *emulator, uint32_t epoch, PDDateTime_32 *datetime) {
    // Todo
}

uint32_t playdate_sys_convertDateTimeToEpoch(Emulator *emulator, PDDateTime_32 *datetime) {
    // Todo
    return 0;
}

void playdate_sys_clearICache(Emulator *emulator) {} // Not needed, probably

int32_t playdate_lua_addFunction(Emulator *emulator, cref_t f, uint8_t *name, cref_t *outErr) {
    if (!emulator->getLuaContext()) {
        if (outErr)
            *outErr = emulator->getEmulatedStringLiteral("Lua environment not initialized");
        return 0;
    }
    emulator->pushEmulatedLuaFunction(f);
    emulator->setQualifiedLuaGlobal((const char *) name);
    if (outErr)
        *outErr = 0;
    return 1;
}

int32_t playdate_lua_registerClass(Emulator *emulator, uint8_t *name, lua_reg_32 *reg, lua_val_32 *vals, int32_t isstatic, cref_t *outErr) {
    if (!emulator->getLuaContext()) {
        if (outErr)
            *outErr = emulator->getEmulatedStringLiteral("Lua environment not initialized");
        return 0;
    }
    // Todo
    return 0;
}

void playdate_lua_pushFunction(Emulator *emulator, cref_t f) {
    if (emulator->getLuaContext())
        emulator->pushEmulatedLuaFunction(f);
}

int32_t playdate_lua_indexMetatable(Emulator *emulator) {
    // Todo
    return 0;
}

void playdate_lua_stop(Emulator *emulator) {
    emulator->disableUpdateLoop = true;
}

void playdate_lua_start(Emulator *emulator) {
    emulator->disableUpdateLoop = false;
}

int32_t playdate_lua_getArgCount(Emulator *emulator) {
    if (!emulator->getLuaContext())
        return 0;
    return lua_gettop(emulator->getLuaContext());
}

int32_t playdate_lua_getArgType(Emulator *emulator, int32_t pos, cref_t *outClass) {
    if (!emulator->getLuaContext())
        return 0;
    return 0;
}

int32_t playdate_lua_argIsNil(Emulator *emulator, int32_t pos) {
    if (!emulator->getLuaContext())
        return 0;
    return lua_isnil(emulator->getLuaContext(), pos);
}

int32_t playdate_lua_getArgBool(Emulator *emulator, int32_t pos) {
    if (!emulator->getLuaContext())
        return 0;
    return lua_toboolean(emulator->getLuaContext(), pos);
}

int32_t playdate_lua_getArgInt(Emulator *emulator, int32_t pos) {
    if (!emulator->getLuaContext())
        return 0;
    return lua_tointeger(emulator->getLuaContext(), pos);
}

float playdate_lua_getArgFloat(Emulator *emulator, int32_t pos) {
    if (!emulator->getLuaContext())
        return 0;
    return lua_tonumber(emulator->getLuaContext(), pos);
}

uint8_t *playdate_lua_getArgString(Emulator *emulator, int32_t pos) {
    if (!emulator->getLuaContext())
        return nullptr;
    return (uint8_t *) lua_tostring(emulator->getLuaContext(), pos);
}

uint8_t *playdate_lua_getArgBytes(Emulator *emulator, int32_t pos, uint32_t *outlen) {
    if (!emulator->getLuaContext())
        return nullptr;
    size_t len;
    auto string = lua_tolstring(emulator->getLuaContext(), pos, &len);
    *outlen = len;
    return (uint8_t *) string;
}

void *playdate_lua_getArgObject(Emulator *emulator, int32_t pos, uint8_t *type, cref_t *outud) {
    if (!emulator->getLuaContext())
        return nullptr;
    // Todo
    return nullptr;
}

LCDBitmap_32 *playdate_lua_getBitmap(Emulator *emulator, int32_t pos) {
    if (!emulator->getLuaContext())
        return nullptr;
    // Todo
    return nullptr;
}

LCDSprite_32 *playdate_lua_getSprite(Emulator *emulator, int32_t pos) {
    if (!emulator->getLuaContext())
        return nullptr;
    // Todo
    return nullptr;
}

void playdate_lua_pushNil(Emulator *emulator) {
    if (!emulator->getLuaContext())
        return;
    lua_pushnil(emulator->getLuaContext());
}

void playdate_lua_pushBool(Emulator *emulator, int32_t val) {
    if (!emulator->getLuaContext())
        return;
    lua_pushboolean(emulator->getLuaContext(), val);
}

void playdate_lua_pushInt(Emulator *emulator, int32_t val) {
    if (!emulator->getLuaContext())
        return;
    lua_pushinteger(emulator->getLuaContext(), val);
}

void playdate_lua_pushFloat(Emulator *emulator, float val) {
    if (!emulator->getLuaContext())
        return;
    lua_pushnumber(emulator->getLuaContext(), val);
}

void playdate_lua_pushString(Emulator *emulator, uint8_t *str) {
    if (!emulator->getLuaContext())
        return;
    lua_pushstring(emulator->getLuaContext(), (const char *) str);
}

void playdate_lua_pushBytes(Emulator *emulator, uint8_t *str, uint32_t len) {
    if (!emulator->getLuaContext())
        return;
    lua_pushlstring(emulator->getLuaContext(), (const char *) str, len);
}

void playdate_lua_pushBitmap(Emulator *emulator, LCDBitmap_32 *bitmap) {
    if (!emulator->getLuaContext())
        return;
    // Todo: Ownership?
}

void playdate_lua_pushSprite(Emulator *emulator, LCDSprite_32 *sprite) {
    if (!emulator->getLuaContext())
        return;
    // Todo: Ownership?
}

LuaUDObject_32 *playdate_lua_pushObject(Emulator *emulator, void* obj, uint8_t * type, int32_t nValues) {
    if (!emulator->getLuaContext())
        return nullptr;
    // Todo
    return nullptr;
}

LuaUDObject_32 *playdate_lua_retainObject(Emulator *emulator, LuaUDObject_32 *obj) {
    if (!emulator->getLuaContext())
        return nullptr;
    // Todo: Presumably this should hold a strong reference in an internal global table so object isn't GC'd and so it can be referenced with a C pointer off the stack
    return nullptr;
}

void playdate_lua_releaseObject(Emulator *emulator, LuaUDObject_32 *obj) {
    if (!emulator->getLuaContext())
        return;
    // Todo
}

void playdate_lua_setUserValue(Emulator *emulator, LuaUDObject_32 *obj, uint32_t slot) {
    if (!emulator->getLuaContext())
        return;
    // Todo
}

int32_t playdate_lua_getUserValue(Emulator *emulator, LuaUDObject_32 *obj, uint32_t slot) {
    if (!emulator->getLuaContext())
        return 0;
    // Todo
    return 0;
}

void playdate_lua_callFunction_deprecated(Emulator *emulator, uint8_t *name, int32_t nargs) {
    if (!emulator->getLuaContext())
        return;
    emulator->getQualifiedLuaGlobal((const char *) name);
    lua_pcall(emulator->getLuaContext(), nargs, LUA_MULTRET, 0);
}

int32_t playdate_lua_callFunction(Emulator *emulator, uint8_t *name, int32_t nargs, cref_t *outerr) {
    if (!emulator->getLuaContext()) {
        if (outerr)
            *outerr = emulator->getEmulatedStringLiteral("Lua environment not initialized");
        return 0;
    }
    if (nargs > 0)
        lua_pushnil(emulator->getLuaContext());
    for (int i = 0; i < nargs; i++)
        lua_copy(emulator->getLuaContext(), -i - 2, -i - 1);
    emulator->getQualifiedLuaGlobal((const char *) name);
    if (nargs > 0) {
        lua_copy(emulator->getLuaContext(), -1, -2 - nargs);
        lua_pop(emulator->getLuaContext(), 1);
    }
    if (lua_pcall(emulator->getLuaContext(), nargs, LUA_MULTRET, 0) != LUA_OK) {
        auto message = lua_tostring(emulator->getLuaContext(), -1);
        *outerr = emulator->getEmulatedStringLiteral(std::string("Error calling function: ") + emulator->getLuaError());
        return 0;
    }
    return 1;
}

void playdate_json_initEncoder(Emulator *emulator, json_encoder_32 *encoder, cref_t write, void *userdata, int32_t pretty) {
    // Encoder API is stored directly after Playdate API, as subtract json_encoder_32 struct size from API size to get address
    *encoder = *(json_encoder_32 *) emulator->fromVirtualAddress(API_ADDRESS + emulator->apiSize - sizeof(json_encoder_32));
    encoder->writeStringFunc = write;
    encoder->userdata = emulator->toVirtualAddress(userdata);
    encoder->pretty = pretty;
}

// Todo: Fix this mess
static int json_decode(Emulator *emulator, json_decoder_32 *functions, json_value_32 *outval, const std::string &data) {
    struct JsonContext {
        inline JsonContext(Emulator *emu, json_decoder_32 *decoder, const std::vector<char> &path) : decoder(*decoder), path(path.begin(), path.end(), emu->heap.allocator<char>()) {}
        json_decoder_32 decoder;
        vheap_vector<char> path;
        bool inArray{};
        int arrayIndex{};
        std::string lastKey;
    };
    const std::string rootString = "_root";
    vheap_vector<JsonContext> stack(emulator->heap.allocator<JsonContext>());
    nlohmann::json::parser_callback_t cb = [&](int depth, nlohmann::json::parse_event_t event, nlohmann::json &parsed) -> bool {
        switch (event) {
            case nlohmann::json::parse_event_t::object_start:
            case nlohmann::json::parse_event_t::array_start: {
                bool isRoot = stack.empty();
                bool isArray = event == nlohmann::json::parse_event_t::array_start;
                if (isRoot)
                    stack.emplace_back(emulator, functions, std::vector<char>(rootString.begin(), rootString.end() + 1));
                auto &context = stack.back();
                bool inArray = context.inArray;
                bool willDecode = not inArray;
                if (inArray and context.decoder.shouldDecodeArrayValueAtIndex) {
                    context.arrayIndex++;
                    willDecode = emulator->invokeEmulatedFunction<int32_t, ArgType::int32_t, ArgType::ptr_t, ArgType::int32_t>
                            (context.decoder.shouldDecodeArrayValueAtIndex, &context.decoder, context.arrayIndex);
                }
                if (not willDecode)
                    return false;
                if (!isRoot and willDecode) {
                    if (inArray) {
//                        stack.back().arrayIndex++;
                        std::string path(context.path.begin(), context.path.end());
                        path += "[" + std::to_string(context.arrayIndex) + "]";
                        stack.emplace_back(emulator, &context.decoder,
                                           std::vector<char>(path.begin(), path.end() + 1)); // Todo: Does this copy the original or the current context?
                    } else {
                        std::string path(context.path.begin(), context.path.end());
                        path += "." + context.lastKey;
                        stack.emplace_back(emulator, &context.decoder, std::vector<char>(path.begin(), path.end() + 1));
                    }
                }
                auto &newContext = stack.back();
                context.decoder.path = emulator->toVirtualAddress(newContext.path.data());
                newContext.inArray = isArray;
                if (isRoot or context.decoder.returnString) // Copy returnString for all children
                    return true;
                if (context.decoder.willDecodeSublist)
                    emulator->invokeEmulatedFunction<void, ArgType::void_t, ArgType::ptr_t, ArgType::ptr_t, ArgType::int32_t>
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
                    value = emulator->invokeEmulatedFunction<void *, ArgType::ptr_t, ArgType::ptr_t, ArgType::ptr_t, ArgType::int32_t>
                            (context.decoder.didDecodeSublist, &context.decoder, context.path.data(), type);
                if (context.decoder.returnString) { // Todo: How does memory ownership work here? User freed?
                    auto string = parsed.get<std::string>();
                    value = emulator->heap.allocate(string.size());
                    memcpy(value, string.data(), string.size());
                }
                stack.pop_back();
                auto &parentContext = stack.back();
                auto sublist = json_value_32{uint8_t(type), {.tableval = emulator->toVirtualAddress(value)}};
                if (stack.empty()) {
                    *outval = sublist;
                } else {
                    if (parentContext.inArray) {
                        if (parentContext.decoder.didDecodeArrayValue)
                            emulator->invokeEmulatedFunction<void, ArgType::void_t, ArgType::ptr_t, ArgType::ptr_t, ArgType::struct2_t>
                                    (parentContext.decoder.didDecodeArrayValue, &parentContext.decoder, parentContext.arrayIndex, sublist);
                    } else {
                        auto keyData = vheap_vector<char>(parentContext.lastKey.begin(), parentContext.lastKey.end() + 1, emulator->heap.allocator<char>());
                        if (parentContext.decoder.didDecodeTableValue)
                            emulator->invokeEmulatedFunction<void, ArgType::void_t, ArgType::ptr_t, ArgType::ptr_t, ArgType::struct2_t>
                                    (parentContext.decoder.didDecodeTableValue, &parentContext.decoder, keyData.data(), sublist);
                    }
                }
//                if (context.decoder.returnString)
//                    stack.back().lastStringResult = ;
//                else
//                    stack.back().lastResult = {uint8_t(type), {.tableval = emulator->toVirtualAddress(value)}}; // Todo: What is value when didDecodeSublist isn't set?
//                printf("End %i %s\n", (int) event, parsed.dump().c_str());
                break;
            }
            case nlohmann::json::parse_event_t::key: {
                auto &context = stack.back();
                if (context.decoder.returnString)
                    return true;
                auto key = parsed.get<std::string>();
                auto keyData = vheap_vector<char>(key.begin(), key.end(), emulator->heap.allocator<char>());
                bool shouldDecode = true;
                context.lastKey = key;
                if (context.decoder.shouldDecodeTableValueForKey)
                    shouldDecode = emulator->invokeEmulatedFunction<int32_t, ArgType::int32_t, ArgType::ptr_t, ArgType::ptr_t>
                            (context.decoder.shouldDecodeTableValueForKey, &stack.back().decoder, keyData.data());
//                if (shouldDecode)
//                    stack.emplace_back(emulator, functions, std::vector<char>(keyData.begin(), keyData.end())); // Todo: Does this copy the original or the current context?
//                printf("Key %i %s\n", (int) event, parsed.dump().c_str());
                return shouldDecode;
            }
            case nlohmann::json::parse_event_t::value: {
                auto &context = stack.back();
                if (context.decoder.returnString)
                    return true;
                auto stringValue = vheap_vector<char>(emulator->heap.allocator<char>()); // Todo: Is this expected to be valid after the `didDecode` functions?
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
                        value.data.stringval = emulator->toVirtualAddress(stringValue.data());
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
                        shouldDecode = emulator->invokeEmulatedFunction<int32_t, ArgType::int32_t, ArgType::ptr_t, ArgType::int32_t>
                                (context.decoder.shouldDecodeArrayValueAtIndex, &stack.back().decoder, context.arrayIndex);
                    if (shouldDecode) {
                        if (context.decoder.didDecodeArrayValue)
                            emulator->invokeEmulatedFunction<void, ArgType::void_t, ArgType::ptr_t, ArgType::int32_t, ArgType::struct2_t>
                                    (context.decoder.didDecodeArrayValue, &stack.back().decoder, context.arrayIndex, parseValue());
                    }
                } else {
                    auto keyData = vheap_vector<char>(context.lastKey.begin(), context.lastKey.end(), emulator->heap.allocator<char>());
                    if (context.decoder.didDecodeTableValue)
                        emulator->invokeEmulatedFunction<void, ArgType::void_t, ArgType::ptr_t, ArgType::ptr_t, ArgType::struct2_t>
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
            emulator->invokeEmulatedFunction<void, ArgType::void_t, ArgType::ptr_t, ArgType::uint32_t, ArgType::int32_t>
                    (functions->decodeError, functions, emulator->getEmulatedStringLiteral(ex.what()), -1); // Todo: Is line number retreivable?
        return 0;
    }
}

int32_t playdate_json_decode(Emulator *emulator, json_decoder_32 *functions, json_reader_32 reader, json_value_32 *outval) {
    constexpr int BUFFER_SEGMENT = 512;
    int size = 0;
    vheap_vector<char> buffer(BUFFER_SEGMENT, emulator->heap.allocator<char>());
    while (true) {
        int returned = emulator->invokeEmulatedFunction<int32_t, ArgType::int32_t, ArgType::uint32_t, ArgType::ptr_t, ArgType::int32_t>
                (reader.read, reader.userdata, buffer.data() + size, buffer.size() - size);
        if (returned < BUFFER_SEGMENT)
            break;
        size += returned;
        buffer.resize(buffer.size() + BUFFER_SEGMENT);
    }
    return json_decode(emulator, functions, outval, buffer.data());
}

int32_t playdate_json_decodeString(Emulator *emulator, json_decoder_32 *functions, uint8_t *jsonString, json_value_32 *outval) {
    return json_decode(emulator, functions, outval, (const char *) jsonString);
}

inline static void encoderWrite(Emulator *emulator, json_encoder_32 *encoder, cref_t string, int len) {
    emulator->invokeEmulatedFunction<void, ArgType::void_t, ArgType::uint32_t, ArgType::uint32_t, ArgType::int32_t>
            (encoder->writeStringFunc, encoder->userdata, string, len);
}

inline static void encoderWrite(Emulator *emulator, json_encoder_32 *encoder, const char *string) {
    encoderWrite(emulator, encoder, emulator->getEmulatedStringLiteral(string), strlen(string));
}

inline static void encoderWrite(Emulator *emulator, json_encoder_32 *encoder, std::string string) {
    // Use vector rather than heap_string to prevent non-heap addresses for small strings
    auto data = vheap_vector<char>(string.c_str(), string.c_str() + string.size(), emulator->heap.allocator<char>());
    encoderWrite(emulator, encoder, emulator->toVirtualAddress(data.data()), string.length());
}

void json_encoder_startArray(Emulator *emulator, json_encoder_32 *encoder) {
    encoder->startedArray = true;
    encoder->depth++;
    if (encoder->pretty and encoder->depth > 1)
        encoderWrite(emulator, encoder, " ");
    encoderWrite(emulator, encoder, "[");
}

void json_encoder_addArrayMember(Emulator *emulator, json_encoder_32 *encoder) {
    if (!encoder->startedArray)
        encoderWrite(emulator, encoder, ",");
    if (encoder->pretty) {
        encoderWrite(emulator, encoder, "\n");
        for (int i = 0; i < encoder->depth; i++)
            encoderWrite(emulator, encoder, "\t");
    }
    encoder->startedArray = false;
}

void json_encoder_endArray(Emulator *emulator, json_encoder_32 *encoder) {
    encoder->depth--;
    if (encoder->pretty) {
        encoderWrite(emulator, encoder, "\n");
        for (int i = 0; i < encoder->depth; i++)
            encoderWrite(emulator, encoder, "\t");
    }
    encoderWrite(emulator, encoder, "]");
}

void json_encoder_startTable(Emulator *emulator, json_encoder_32 *encoder) {
    encoder->startedTable = true;
    encoder->depth++;
    if (encoder->pretty and encoder->depth > 1)
        encoderWrite(emulator, encoder, " ");
    encoderWrite(emulator, encoder, "{");
}

void json_encoder_addTableMember(Emulator *emulator, json_encoder_32 *encoder, uint8_t *name, int32_t len) {
    if (!encoder->startedTable)
        encoderWrite(emulator, encoder, ",");
    if (encoder->pretty) {
        encoderWrite(emulator, encoder, "\n");
        for (int i = 0; i < encoder->depth; i++)
            encoderWrite(emulator, encoder, "\t");
    }
    encoderWrite(emulator, encoder, nlohmann::json(std::string((const char *) name, len)).dump());
    encoderWrite(emulator, encoder, ":");
    encoder->startedTable = false;
}

void json_encoder_endTable(Emulator *emulator, json_encoder_32 *encoder) {
    encoder->depth--;
    if (encoder->pretty) {
        encoderWrite(emulator, encoder, "\n");
        for (int i = 0; i < encoder->depth; i++)
            encoderWrite(emulator, encoder, "\t");
    }
    encoderWrite(emulator, encoder, "}");
}

void json_encoder_writeNull(Emulator *emulator, json_encoder_32 *encoder) {
    encoderWrite(emulator, encoder, "null");
}

void json_encoder_writeFalse(Emulator *emulator, json_encoder_32 *encoder) {
    encoderWrite(emulator, encoder, "false");
}

void json_encoder_writeTrue(Emulator *emulator, json_encoder_32 *encoder) {
    encoderWrite(emulator, encoder, "true");
}

void json_encoder_writeInt(Emulator *emulator, json_encoder_32 *encoder, int32_t num) {
    encoderWrite(emulator, encoder, std::to_string(num));
}

void json_encoder_writeDouble(Emulator *emulator, json_encoder_32 *encoder, double num) {
    encoderWrite(emulator, encoder, std::to_string(num));
}

void json_encoder_writeString(Emulator *emulator, json_encoder_32 *encoder, uint8_t *str, int32_t len) {
    encoderWrite(emulator, encoder, nlohmann::json(std::string((const char *) str, len)).dump());
}

uint8_t *playdate_file_geterr(Emulator *emulator) {
    return (uint8_t *) emulator->fromVirtualAddress(emulator->files.lastError); // This conversion is redundant, but simplifies things
}

int32_t playdate_file_listfiles(Emulator *emulator, uint8_t *path, cref_t callback, void *userdata, int32_t showhidden) {
    std::vector<std::string> files;
    if (emulator->files.listFiles((const char *) path, showhidden, files))
        return -1;
    for (auto &file : files)
        emulator->invokeEmulatedFunction<void, ArgType::ptr_t, ArgType::ptr_t>(callback, file.c_str(), userdata);
    return 0;
}

int32_t playdate_file_stat(Emulator *emulator, uint8_t *path, FileStat_32 *stats) {
    return emulator->files.stat((const char *) path, *stats);
}

int32_t playdate_file_mkdir(Emulator *emulator, uint8_t *path) {
    return emulator->files.mkdir((const char *) path);
}

int32_t playdate_file_unlink(Emulator *emulator, uint8_t *name, int32_t recursive) {
    return emulator->files.unlink((const char *) name, recursive);
}

int32_t playdate_file_rename(Emulator *emulator, uint8_t *from, uint8_t *to) {
    return emulator->files.rename((const char *) from, (const char *) to);
}

SDFile_32 *playdate_file_open(Emulator *emulator, uint8_t *name, int32_t mode) {
    return emulator->files.open((const char *) name, FileOptions(mode));
}

int32_t playdate_file_close(Emulator *emulator, SDFile_32 *file) {
    return emulator->files.close(file);
}

int32_t playdate_file_read(Emulator *emulator, SDFile_32 *file, void *buf, uint32_t len) {
    return emulator->files.read(file, buf, len);
}

int32_t playdate_file_write(Emulator *emulator, SDFile_32 *file, void *buf, uint32_t len) {
    return emulator->files.write(file, buf, len);
}

int32_t playdate_file_flush(Emulator *emulator, SDFile_32 *file) {
    return emulator->files.flush(file);
}

int32_t playdate_file_tell(Emulator *emulator, SDFile_32 *file) {
    return emulator->files.tell(file);
}

int32_t playdate_file_seek(Emulator *emulator, SDFile_32 *file, int32_t pos, int32_t whence) {
    return emulator->files.seek(file, pos, whence);
}

void playdate_sprite_setAlwaysRedraw(Emulator *emulator, int32_t flag) {
    // Todo
}

void playdate_sprite_addDirtyRect(Emulator *emulator, LCDRect_32 dirtyRect) {
    // Todo
}

void playdate_sprite_drawSprites(Emulator *emulator) {
    // Todo
}

void playdate_sprite_updateAndDrawSprites(Emulator *emulator) {
    // Todo
}

LCDSprite_32 *playdate_sprite_newSprite(Emulator *emulator) {
    // Todo
    return nullptr;
}

void playdate_sprite_freeSprite(Emulator *emulator, LCDSprite_32 *sprite) {
    // Todo
    emulator->graphics.freeSprite(sprite);
}

LCDSprite_32 *playdate_sprite_copy(Emulator *emulator, LCDSprite_32 *sprite) {
    // Todo
    return nullptr;
}

void playdate_sprite_addSprite(Emulator *emulator, LCDSprite_32 *sprite) {
    // Todo
}

void playdate_sprite_removeSprite(Emulator *emulator, LCDSprite_32 *sprite) {
    // Todo
}

void playdate_sprite_removeSprites(Emulator *emulator, cref_t *sprites, int32_t count) {
    // Todo
}

void playdate_sprite_removeAllSprites(Emulator *emulator) {
    // Todo
}

int32_t playdate_sprite_getSpriteCount(Emulator *emulator) {
    // Todo
    return 0;
}

void playdate_sprite_setBounds(Emulator *emulator, LCDSprite_32 *sprite, PDRect_32 bounds) {
    // Todo
}

PDRect_32 playdate_sprite_getBounds(Emulator *emulator, LCDSprite_32 *sprite) {
    // Todo
    return {};
}

void playdate_sprite_moveTo(Emulator *emulator, LCDSprite_32 *sprite, float x, float y) {
    // Todo
}

void playdate_sprite_moveBy(Emulator *emulator, LCDSprite_32 *sprite, float dx, float dy) {
    // Todo
}

void playdate_sprite_setImage(Emulator *emulator, LCDSprite_32 *sprite, LCDBitmap_32 *image, int32_t flip) {
    // Todo
}

LCDBitmap_32 *playdate_sprite_getImage(Emulator *emulator, LCDSprite_32 *sprite) {
    // Todo
    return nullptr;
}

void playdate_sprite_setSize(Emulator *emulator, LCDSprite_32 *s, float width, float height) {
    // Todo
}

void playdate_sprite_setZIndex(Emulator *emulator, LCDSprite_32 *sprite, int16_t zIndex) {
    // Todo
}

int16_t playdate_sprite_getZIndex(Emulator *emulator, LCDSprite_32 *sprite) {
    // Todo
    return 0;
}

void playdate_sprite_setDrawMode(Emulator *emulator, LCDSprite_32 *sprite, int32_t mode) {
    // Todo
}

void playdate_sprite_setImageFlip(Emulator *emulator, LCDSprite_32 *sprite, int32_t flip) {
    // Todo
}

int32_t playdate_sprite_getImageFlip(Emulator *emulator, LCDSprite_32 *sprite) {
    // Todo
    return 0;
}

void playdate_sprite_setStencil(Emulator *emulator, LCDSprite_32 *sprite, LCDBitmap_32 *stencil) {
    // Todo
}

void playdate_sprite_setClipRect(Emulator *emulator, LCDSprite_32 *sprite, LCDRect_32 clipRect) {
    // Todo
}

void playdate_sprite_clearClipRect(Emulator *emulator, LCDSprite_32 *sprite) {
    // Todo
}

void playdate_sprite_setClipRectsInRange(Emulator *emulator, LCDRect_32 clipRect, int32_t startZ, int32_t endZ) {
    // Todo
}

void playdate_sprite_clearClipRectsInRange(Emulator *emulator, int32_t startZ, int32_t endZ) {
    // Todo
}

void playdate_sprite_setUpdatesEnabled(Emulator *emulator, LCDSprite_32 *sprite, int32_t flag) {
    // Todo
}

int32_t playdate_sprite_updatesEnabled(Emulator *emulator, LCDSprite_32 *sprite) {
    // Todo
    return 0;
}

void playdate_sprite_setCollisionsEnabled(Emulator *emulator, LCDSprite_32 *sprite, int32_t flag) {
    // Todo
}

int32_t playdate_sprite_collisionsEnabled(Emulator *emulator, LCDSprite_32 *sprite) {
    // Todo
    return 0;
}

void playdate_sprite_setVisible(Emulator *emulator, LCDSprite_32 *sprite, int32_t flag) {
    // Todo
}

int32_t playdate_sprite_isVisible(Emulator *emulator, LCDSprite_32 *sprite) {
    // Todo
    return 0;
}

void playdate_sprite_setOpaque(Emulator *emulator, LCDSprite_32 *sprite, int32_t flag) {
    // Todo
}

void playdate_sprite_markDirty(Emulator *emulator, LCDSprite_32 *sprite) {
    // Todo
}

void playdate_sprite_setTag(Emulator *emulator, LCDSprite_32 *sprite, uint8_t tag) {
    // Todo
}

uint8_t playdate_sprite_getTag(Emulator *emulator, LCDSprite_32 *sprite) {
    // Todo
    return 0;
}

void playdate_sprite_setIgnoresDrawOffset(Emulator *emulator, LCDSprite_32 *sprite, int32_t flag) {
    // Todo
}

void playdate_sprite_setUpdateFunction(Emulator *emulator, LCDSprite_32 *sprite, cref_t func) {
    // Todo
}

void playdate_sprite_setDrawFunction(Emulator *emulator, LCDSprite_32 *sprite, cref_t func) {
    // Todo
}

void playdate_sprite_getPosition(Emulator *emulator, LCDSprite_32 *sprite, float *x, float *y) {
    // Todo
}

void playdate_sprite_resetCollisionWorld(Emulator *emulator) {
    // Todo
}

void playdate_sprite_setCollideRect(Emulator *emulator, LCDSprite_32 *sprite, PDRect_32 collideRect) {
    // Todo
}

PDRect_32 playdate_sprite_getCollideRect(Emulator *emulator, LCDSprite_32 *sprite) {
    // Todo
    return {};
}

void playdate_sprite_clearCollideRect(Emulator *emulator, LCDSprite_32 *sprite) {
    // Todo
}

void playdate_sprite_setCollisionResponseFunction(Emulator *emulator, LCDSprite_32 *sprite, cref_t func) {
    // Todo
}

SpriteCollisionInfo_32 *playdate_sprite_checkCollisions(Emulator *emulator, LCDSprite_32 *sprite, float goalX, float goalY, float *actualX, float *actualY, int32_t *len) {
    // Todo
    return nullptr;
}

SpriteCollisionInfo_32 *playdate_sprite_moveWithCollisions(Emulator *emulator, LCDSprite_32 *sprite, float goalX, float goalY, float *actualX, float *actualY, int32_t *len) {
    // Todo
    return nullptr;
}

cref_t *playdate_sprite_querySpritesAtPoint(Emulator *emulator, float x, float y, int32_t *len) {
    // Todo
    return nullptr;
}

cref_t *playdate_sprite_querySpritesInRect(Emulator *emulator, float x, float y, float width, float height, int32_t *len) {
    // Todo
    return nullptr;
}

cref_t *playdate_sprite_querySpritesAlongLine(Emulator *emulator, float x1, float y1, float x2, float y2, int32_t *len) {
    // Todo
    return nullptr;
}

SpriteQueryInfo_32 *playdate_sprite_querySpriteInfoAlongLine(Emulator *emulator, float x1, float y1, float x2, float y2, int32_t *len) {
    // Todo
    return nullptr;
}

cref_t *playdate_sprite_overlappingSprites(Emulator *emulator, LCDSprite_32 *sprite, int32_t *len) {
    // Todo
    return nullptr;
}

cref_t *playdate_sprite_allOverlappingSprites(Emulator *emulator, int32_t *len) {
    // Todo
    return nullptr;
}

void playdate_sprite_setStencilPattern(Emulator *emulator, LCDSprite_32 *sprite, uint8_t pattern) {
    // Todo
}

void playdate_sprite_clearStencil(Emulator *emulator, LCDSprite_32 *sprite) {
    // Todo
}

void playdate_sprite_setUserdata(Emulator *emulator, LCDSprite_32 *sprite, void *userdata) {
    // Todo
}

void *playdate_sprite_getUserdata(Emulator *emulator, LCDSprite_32 *sprite) {
    // Todo
    return nullptr;
}

void playdate_sprite_setStencilImage(Emulator *emulator, LCDSprite_32 *sprite, LCDBitmap_32 *stencil, int32_t tile) {
    // Todo
}

void playdate_sound_source_setVolume(Emulator *emulator, SoundSource_32 *c, float lvol, float rvol) {
    // Todo
}

void playdate_sound_source_getVolume(Emulator *emulator, SoundSource_32 *c, float *outl, float *outr) {
    // Todo
}

int32_t playdate_sound_source_isPlaying(Emulator *emulator, SoundSource_32 *c) {
    // Todo
    return 0;
}

void playdate_sound_source_setFinishCallback(Emulator *emulator, SoundSource_32 *c, cref_t callback) {
    // Todo
}

FilePlayer_32 *playdate_sound_fileplayer_newPlayer(Emulator *emulator) {
    // Todo
    return nullptr;
}

void playdate_sound_fileplayer_freePlayer(Emulator *emulator, FilePlayer_32 *player) {
    // Todo
}

int32_t playdate_sound_fileplayer_loadIntoPlayer(Emulator *emulator, FilePlayer_32 *player, uint8_t *path) {
    // Todo
    return 0;
}

void playdate_sound_fileplayer_setBufferLength(Emulator *emulator, FilePlayer_32 *player, float bufferLen) {
    // Todo
}

int32_t playdate_sound_fileplayer_play(Emulator *emulator, FilePlayer_32 *player, int32_t repeat) {
    // Todo
    return 0;
}

int32_t playdate_sound_fileplayer_isPlaying(Emulator *emulator, FilePlayer_32 *player) {
    // Todo
    return 0;
}

void playdate_sound_fileplayer_pause(Emulator *emulator, FilePlayer_32 *player) {
    // Todo
}

void playdate_sound_fileplayer_stop(Emulator *emulator, FilePlayer_32 *player) {
    // Todo
}

void playdate_sound_fileplayer_setVolume(Emulator *emulator, FilePlayer_32 *player, float left, float right) {
    // Todo
}

void playdate_sound_fileplayer_getVolume(Emulator *emulator, FilePlayer_32 *player, float *left, float *right) {
    // Todo
}

float playdate_sound_fileplayer_getLength(Emulator *emulator, FilePlayer_32 *player) {
    // Todo
    return 0;
}

void playdate_sound_fileplayer_setOffset(Emulator *emulator, FilePlayer_32 *player, float offset) {
    // Todo
}

void playdate_sound_fileplayer_setRate(Emulator *emulator, FilePlayer_32 *player, float rate) {
    // Todo
}

void playdate_sound_fileplayer_setLoopRange(Emulator *emulator, FilePlayer_32 *player, float start, float end) {
    // Todo
}

int32_t playdate_sound_fileplayer_didUnderrun(Emulator *emulator, FilePlayer_32 *player) {
    // Todo
    return 0;
}

void playdate_sound_fileplayer_setFinishCallback(Emulator *emulator, FilePlayer_32 *player, cref_t callback) {
    // Todo
}

void playdate_sound_fileplayer_setLoopCallback(Emulator *emulator, FilePlayer_32 *player, cref_t callback) {
    // Todo
}

float playdate_sound_fileplayer_getOffset(Emulator *emulator, FilePlayer_32 *player) {
    // Todo
    return 0;
}

float playdate_sound_fileplayer_getRate(Emulator *emulator, FilePlayer_32 *player) {
    // Todo
    return 0;
}

void playdate_sound_fileplayer_setStopOnUnderrun(Emulator *emulator, FilePlayer_32 *player, int32_t flag) {
    // Todo
}

void playdate_sound_fileplayer_fadeVolume(Emulator *emulator, FilePlayer_32 *player, float left, float right, int32_t len, cref_t finishCallback) {
    // Todo
}

void playdate_sound_fileplayer_setMP3StreamSource(Emulator *emulator, FilePlayer_32 *player, cref_t dataSource, void *userdata, float bufferLen) {
    // Todo
}

AudioSample_32 *playdate_sound_sample_newSampleBuffer(Emulator *emulator, int32_t byteCount) {
    // Todo
    return nullptr;
}

int32_t playdate_sound_sample_loadIntoSample(Emulator *emulator, AudioSample_32 *sample, uint8_t *path) {
    // Todo
    return 0;
}

AudioSample_32 *playdate_sound_sample_load(Emulator *emulator, uint8_t *path) {
    // Todo
    return nullptr;
}

AudioSample_32 *playdate_sound_sample_newSampleFromData(Emulator *emulator, uint8_t *data, int32_t format, uint32_t sampleRate, int32_t byteCount) {
    // Todo
    return nullptr;
}

void playdate_sound_sample_getData(Emulator *emulator, AudioSample_32 *sample, cref_t *data, int32_t *format, uint32_t *sampleRate, uint32_t *bytelength) {
    // Todo
}

void playdate_sound_sample_freeSample(Emulator *emulator, AudioSample_32 *sample) {
    // Todo
}

float playdate_sound_sample_getLength(Emulator *emulator, AudioSample_32 *sample) {
    // Todo
    return 0;
}

SamplePlayer_32 *playdate_sound_sampleplayer_newPlayer(Emulator *emulator) {
    // Todo
    return nullptr;
}

void playdate_sound_sampleplayer_freePlayer(Emulator *emulator, SamplePlayer_32 *player) {
    // Todo
}

void playdate_sound_sampleplayer_setSample(Emulator *emulator, SamplePlayer_32 *player, AudioSample_32 *sample) {
    // Todo
}

int32_t playdate_sound_sampleplayer_play(Emulator *emulator, SamplePlayer_32 *player, int32_t repeat, float rate) {
    // Todo
    return 0;
}

int32_t playdate_sound_sampleplayer_isPlaying(Emulator *emulator, SamplePlayer_32 *player) {
    // Todo
    return 0;
}

void playdate_sound_sampleplayer_stop(Emulator *emulator, SamplePlayer_32 *player) {
    // Todo
}

void playdate_sound_sampleplayer_setVolume(Emulator *emulator, SamplePlayer_32 *player, float left, float right) {
    // Todo
}

void playdate_sound_sampleplayer_getVolume(Emulator *emulator, SamplePlayer_32 *player, float *left, float *right) {
    // Todo
}

float playdate_sound_sampleplayer_getLength(Emulator *emulator, SamplePlayer_32 *player) {
    // Todo
    return 0;
}

void playdate_sound_sampleplayer_setOffset(Emulator *emulator, SamplePlayer_32 *player, float offset) {
    // Todo
}

void playdate_sound_sampleplayer_setRate(Emulator *emulator, SamplePlayer_32 *player, float rate) {
    // Todo
}

void playdate_sound_sampleplayer_setPlayRange(Emulator *emulator, SamplePlayer_32 *player, int32_t start, int32_t end) {
    // Todo
}

void playdate_sound_sampleplayer_setFinishCallback(Emulator *emulator, SamplePlayer_32 *player, cref_t callback) {
    // Todo
}

void playdate_sound_sampleplayer_setLoopCallback(Emulator *emulator, SamplePlayer_32 *player, cref_t callback) {
    // Todo
}

float playdate_sound_sampleplayer_getOffset(Emulator *emulator, SamplePlayer_32 *player) {
    // Todo
    return 0;
}

float playdate_sound_sampleplayer_getRate(Emulator *emulator, SamplePlayer_32 *player) {
    // Todo
    return 0;
}

void playdate_sound_sampleplayer_setPaused(Emulator *emulator, SamplePlayer_32 *player, int32_t flag) {
    // Todo
}

PDSynthSignal_32 *playdate_sound_signal_newSignal(Emulator *emulator, cref_t step, cref_t noteOn, cref_t noteOff, cref_t dealloc, void *userdata) {
    // Todo
    return nullptr;
}

void playdate_sound_signal_freeSignal(Emulator *emulator, PDSynthSignal_32 *signal) {
    // Todo
}

float playdate_sound_signal_getValue(Emulator *emulator, PDSynthSignal_32 *signal) {
    // Todo
    return 0;
}

void playdate_sound_signal_setValueScale(Emulator *emulator, PDSynthSignal_32 *signal, float scale) {
    // Todo
}

void playdate_sound_signal_setValueOffset(Emulator *emulator, PDSynthSignal_32 *signal, float offset) {
    // Todo
}

PDSynthLFO_32 *playdate_sound_lfo_newLFO(Emulator *emulator, int32_t type) {
    // Todo
    return nullptr;
}

void playdate_sound_lfo_freeLFO(Emulator *emulator, PDSynthLFO_32 *lfo) {
    // Todo
}

void playdate_sound_lfo_setType(Emulator *emulator, PDSynthLFO_32 *lfo, int32_t type) {
    // Todo
}

void playdate_sound_lfo_setRate(Emulator *emulator, PDSynthLFO_32 *lfo, float rate) {
    // Todo
}

void playdate_sound_lfo_setPhase(Emulator *emulator, PDSynthLFO_32 *lfo, float phase) {
    // Todo
}

void playdate_sound_lfo_setCenter(Emulator *emulator, PDSynthLFO_32 *lfo, float center) {
    // Todo
}

void playdate_sound_lfo_setDepth(Emulator *emulator, PDSynthLFO_32 *lfo, float depth) {
    // Todo
}

void playdate_sound_lfo_setArpeggiation(Emulator *emulator, PDSynthLFO_32 *lfo, int32_t nSteps, float *steps) {
    // Todo
}

void playdate_sound_lfo_setFunction(Emulator *emulator, PDSynthLFO_32 *lfo, cref_t lfoFunc, void *userdata, int32_t interpolate) {
    // Todo
}

void playdate_sound_lfo_setDelay(Emulator *emulator, PDSynthLFO_32 *lfo, float holdoff, float ramptime) {
    // Todo
}

void playdate_sound_lfo_setRetrigger(Emulator *emulator, PDSynthLFO_32 *lfo, int32_t flag) {
    // Todo
}

float playdate_sound_lfo_getValue(Emulator *emulator, PDSynthLFO_32 *lfo) {
    // Todo
    return 0;
}

void playdate_sound_lfo_setGlobal(Emulator *emulator, PDSynthLFO_32 *lfo, int32_t global) {
    // Todo
}

PDSynthEnvelope_32 *playdate_sound_envelope_newEnvelope(Emulator *emulator, float attack, float decay, float sustain, float release) {
    // Todo
    return nullptr;
}

void playdate_sound_envelope_freeEnvelope(Emulator *emulator, PDSynthEnvelope_32 *env) {
    // Todo
}

void playdate_sound_envelope_setAttack(Emulator *emulator, PDSynthEnvelope_32 *env, float attack) {
    // Todo
}

void playdate_sound_envelope_setDecay(Emulator *emulator, PDSynthEnvelope_32 *env, float decay) {
    // Todo
}

void playdate_sound_envelope_setSustain(Emulator *emulator, PDSynthEnvelope_32 *env, float sustain) {
    // Todo
}

void playdate_sound_envelope_setRelease(Emulator *emulator, PDSynthEnvelope_32 *env, float release) {
    // Todo
}

void playdate_sound_envelope_setLegato(Emulator *emulator, PDSynthEnvelope_32 *env, int32_t flag) {
    // Todo
}

void playdate_sound_envelope_setRetrigger(Emulator *emulator, PDSynthEnvelope_32 *lfo, int32_t flag) {
    // Todo
}

float playdate_sound_envelope_getValue(Emulator *emulator, PDSynthEnvelope_32 *env) {
    // Todo
    return 0;
}

void playdate_sound_envelope_setCurvature(Emulator *emulator, PDSynthEnvelope_32 *env, float amount) {
    // Todo
}

void playdate_sound_envelope_setVelocitySensitivity(Emulator *emulator, PDSynthEnvelope_32 *env, float velsens) {
    // Todo
}

void playdate_sound_envelope_setRateScaling(Emulator *emulator, PDSynthEnvelope_32 *env, float scaling, float start, float end) {
    // Todo
}

PDSynth_32 *playdate_sound_synth_newSynth(Emulator *emulator) {
    // Todo
    return nullptr;
}

void playdate_sound_synth_freeSynth(Emulator *emulator, PDSynth_32 *synth) {
    // Todo
}

void playdate_sound_synth_setWaveform(Emulator *emulator, PDSynth_32 *synth, int32_t wave) {
    // Todo
}

void playdate_sound_synth_setGenerator(Emulator *emulator, PDSynth_32 *synth, int32_t stereo, cref_t render, cref_t noteOn, cref_t release, cref_t setparam, cref_t dealloc, void *userdata) {
    // Todo
}

void playdate_sound_synth_setSample(Emulator *emulator, PDSynth_32 *synth, AudioSample_32 *sample, uint32_t sustainStart, uint32_t sustainEnd) {
    // Todo
}

void playdate_sound_synth_setAttackTime(Emulator *emulator, PDSynth_32 *synth, float attack) {
    // Todo
}

void playdate_sound_synth_setDecayTime(Emulator *emulator, PDSynth_32 *synth, float decay) {
    // Todo
}

void playdate_sound_synth_setSustainLevel(Emulator *emulator, PDSynth_32 *synth, float sustain) {
    // Todo
}

void playdate_sound_synth_setReleaseTime(Emulator *emulator, PDSynth_32 *synth, float release) {
    // Todo
}

void playdate_sound_synth_setTranspose(Emulator *emulator, PDSynth_32 *synth, float halfSteps) {
    // Todo
}

void playdate_sound_synth_setFrequencyModulator(Emulator *emulator, PDSynth_32 *synth, PDSynthSignalValue_32 *mod) {
    // Todo
}

PDSynthSignalValue_32 *playdate_sound_synth_getFrequencyModulator(Emulator *emulator, PDSynth_32 *synth) {
    // Todo
    return nullptr;
}

void playdate_sound_synth_setAmplitudeModulator(Emulator *emulator, PDSynth_32 *synth, PDSynthSignalValue_32 *mod) {
    // Todo
}

PDSynthSignalValue_32 *playdate_sound_synth_getAmplitudeModulator(Emulator *emulator, PDSynth_32 *synth) {
    // Todo
    return nullptr;
}

int32_t playdate_sound_synth_getParameterCount(Emulator *emulator, PDSynth_32 *synth) {
    // Todo
    return 0;
}

int32_t playdate_sound_synth_setParameter(Emulator *emulator, PDSynth_32 *synth, int32_t parameter, float value) {
    // Todo
    return 0;
}

void playdate_sound_synth_setParameterModulator(Emulator *emulator, PDSynth_32 *synth, int32_t parameter, PDSynthSignalValue_32 *mod) {
    // Todo
}

PDSynthSignalValue_32 *playdate_sound_synth_getParameterModulator(Emulator *emulator, PDSynth_32 *synth, int32_t parameter) {
    // Todo
    return nullptr;
}

void playdate_sound_synth_playNote(Emulator *emulator, PDSynth_32 *synth, float freq, float vel, float len, uint32_t when) {
    // Todo
}

void playdate_sound_synth_playMIDINote(Emulator *emulator, PDSynth_32 *synth, float note, float vel, float len, uint32_t when) {
    // Todo
}

void playdate_sound_synth_noteOff(Emulator *emulator, PDSynth_32 *synth, uint32_t when) {
    // Todo
}

void playdate_sound_synth_stop(Emulator *emulator, PDSynth_32 *synth) {
    // Todo
}

void playdate_sound_synth_setVolume(Emulator *emulator, PDSynth_32 *synth, float left, float right) {
    // Todo
}

void playdate_sound_synth_getVolume(Emulator *emulator, PDSynth_32 *synth, float *left, float *right) {
    // Todo
}

int32_t playdate_sound_synth_isPlaying(Emulator *emulator, PDSynth_32 *synth) {
    // Todo
    return 0;
}

PDSynthEnvelope_32 *playdate_sound_synth_getEnvelope(Emulator *emulator, PDSynth_32 *synth) {
    // Todo
    return nullptr;
}

ControlSignal_32 *playdate_control_signal_newSignal(Emulator *emulator) {
    // Todo
    return nullptr;
}

void playdate_control_signal_freeSignal(Emulator *emulator, ControlSignal_32 *signal) {
    // Todo
}

void playdate_control_signal_clearEvents(Emulator *emulator, ControlSignal_32 *control) {
    // Todo
}

void playdate_control_signal_addEvent(Emulator *emulator, ControlSignal_32 *control, int32_t step, float value, int32_t interpolate) {
    // Todo
}

void playdate_control_signal_removeEvent(Emulator *emulator, ControlSignal_32 *control, int32_t step) {
    // Todo
}

int32_t playdate_control_signal_getMIDIControllerNumber(Emulator *emulator, ControlSignal_32 *control) {
    // Todo
    return 0;
}

PDSynthInstrument_32 *playdate_sound_instrument_newInstrument(Emulator *emulator) {
    // Todo
    return nullptr;
}

void playdate_sound_instrument_freeInstrument(Emulator *emulator, PDSynthInstrument_32 *inst) {
    // Todo
}

int32_t playdate_sound_instrument_addVoice(Emulator *emulator, PDSynthInstrument_32 *inst, PDSynth_32 *synth, float rangeStart, float rangeEnd, float transpose) {
    // Todo
    return 0;
}

PDSynth_32 *playdate_sound_instrument_playNote(Emulator *emulator, PDSynthInstrument_32 *inst, float frequency, float vel, float len, uint32_t when) {
    // Todo
    return nullptr;
}

PDSynth_32 *playdate_sound_instrument_playMIDINote(Emulator *emulator, PDSynthInstrument_32 *inst, float note, float vel, float len, uint32_t when) {
    // Todo
    return nullptr;
}

void playdate_sound_instrument_setPitchBend(Emulator *emulator, PDSynthInstrument_32 *inst, float bend) {
    // Todo
}

void playdate_sound_instrument_setPitchBendRange(Emulator *emulator, PDSynthInstrument_32 *inst, float halfSteps) {
    // Todo
}

void playdate_sound_instrument_setTranspose(Emulator *emulator, PDSynthInstrument_32 *inst, float halfSteps) {
    // Todo
}

void playdate_sound_instrument_noteOff(Emulator *emulator, PDSynthInstrument_32 *inst, float note, uint32_t when) {
    // Todo
}

void playdate_sound_instrument_allNotesOff(Emulator *emulator, PDSynthInstrument_32 *inst, uint32_t when) {
    // Todo
}

void playdate_sound_instrument_setVolume(Emulator *emulator, PDSynthInstrument_32 *inst, float left, float right) {
    // Todo
}

void playdate_sound_instrument_getVolume(Emulator *emulator, PDSynthInstrument_32 *inst, float *left, float *right) {
    // Todo
}

int32_t playdate_sound_instrument_activeVoiceCount(Emulator *emulator, PDSynthInstrument_32 *inst) {
    // Todo
    return 0;
}

SequenceTrack_32 *playdate_sound_track_newTrack(Emulator *emulator) {
    // Todo
    return nullptr;
}

void playdate_sound_track_freeTrack(Emulator *emulator, SequenceTrack_32 *track) {
    // Todo
}

void playdate_sound_track_setInstrument(Emulator *emulator, SequenceTrack_32 *track, PDSynthInstrument_32 *inst) {
    // Todo
}

PDSynthInstrument_32 *playdate_sound_track_getInstrument(Emulator *emulator, SequenceTrack_32 *track) {
    // Todo
    return nullptr;
}

void playdate_sound_track_addNoteEvent(Emulator *emulator, SequenceTrack_32 *track, uint32_t step, uint32_t len, float note, float velocity) {
    // Todo
}

void playdate_sound_track_removeNoteEvent(Emulator *emulator, SequenceTrack_32 *track, uint32_t step, float note) {
    // Todo
}

void playdate_sound_track_clearNotes(Emulator *emulator, SequenceTrack_32 *track) {
    // Todo
}

int32_t playdate_sound_track_getControlSignalCount(Emulator *emulator, SequenceTrack_32 *track) {
    // Todo
    return 0;
}

ControlSignal_32 *playdate_sound_track_getControlSignal(Emulator *emulator, SequenceTrack_32 *track, int32_t idx) {
    // Todo
    return nullptr;
}

void playdate_sound_track_clearControlEvents(Emulator *emulator, SequenceTrack_32 *track) {
    // Todo
}

int32_t playdate_sound_track_getPolyphony(Emulator *emulator, SequenceTrack_32 *track) {
    // Todo
    return 0;
}

int32_t playdate_sound_track_activeVoiceCount(Emulator *emulator, SequenceTrack_32 *track) {
    // Todo
    return 0;
}

void playdate_sound_track_setMuted(Emulator *emulator, SequenceTrack_32 *track, int32_t mute) {
    // Todo
}

uint32_t playdate_sound_track_getLength(Emulator *emulator, SequenceTrack_32 *track) {
    // Todo
    return 0;
}

int32_t playdate_sound_track_getIndexForStep(Emulator *emulator, SequenceTrack_32 *track, uint32_t step) {
    // Todo
    return 0;
}

int32_t playdate_sound_track_getNoteAtIndex(Emulator *emulator, SequenceTrack_32 *track, int32_t index, uint32_t *outStep, uint32_t *outLen, float *outNote, float *outVelocity) {
    // Todo
    return 0;
}

ControlSignal_32 *playdate_sound_track_getSignalForController(Emulator *emulator, SequenceTrack_32 *track, int32_t controller, int32_t create) {
    // Todo
    return nullptr;
}

SoundSequence_32 *playdate_sound_sequence_newSequence(Emulator *emulator) {
    // Todo
    return nullptr;
}

void playdate_sound_sequence_freeSequence(Emulator *emulator, SoundSequence_32 *sequence) {
    // Todo
}

int32_t playdate_sound_sequence_loadMidiFile(Emulator *emulator, SoundSequence_32 *seq, uint8_t *path) {
    // Todo
    return 0;
}

uint32_t playdate_sound_sequence_getTime(Emulator *emulator, SoundSequence_32 *seq) {
    // Todo
    return 0;
}

void playdate_sound_sequence_setTime(Emulator *emulator, SoundSequence_32 *seq, uint32_t time) {
    // Todo
}

void playdate_sound_sequence_setLoops(Emulator *emulator, SoundSequence_32 *seq, int32_t loopstart, int32_t loopend, int32_t loops) {
    // Todo
}

int32_t playdate_sound_sequence_getTempo(Emulator *emulator, SoundSequence_32 *seq) {
    // Todo
    return 0;
}

void playdate_sound_sequence_setTempo(Emulator *emulator, SoundSequence_32 *seq, int32_t stepsPerSecond) {
    // Todo
}

int32_t playdate_sound_sequence_getTrackCount(Emulator *emulator, SoundSequence_32 *seq) {
    // Todo
    return 0;
}

SequenceTrack_32 *playdate_sound_sequence_addTrack(Emulator *emulator, SoundSequence_32 *seq) {
    // Todo
    return nullptr;
}

SequenceTrack_32 *playdate_sound_sequence_getTrackAtIndex(Emulator *emulator, SoundSequence_32 *seq, uint32_t track) {
    // Todo
    return nullptr;
}

void playdate_sound_sequence_setTrackAtIndex(Emulator *emulator, SoundSequence_32 *seq, SequenceTrack_32 *track, uint32_t idx) {
    // Todo
}

void playdate_sound_sequence_allNotesOff(Emulator *emulator, SoundSequence_32 *seq) {
    // Todo
}

int32_t playdate_sound_sequence_isPlaying(Emulator *emulator, SoundSequence_32 *seq) {
    // Todo
    return 0;
}

uint32_t playdate_sound_sequence_getLength(Emulator *emulator, SoundSequence_32 *seq) {
    // Todo
    return 0;
}

void playdate_sound_sequence_play(Emulator *emulator, SoundSequence_32 *seq, cref_t finishCallback, void *userdata) {
    // Todo
}

void playdate_sound_sequence_stop(Emulator *emulator, SoundSequence_32 *seq) {
    // Todo
}

int32_t playdate_sound_sequence_getCurrentStep(Emulator *emulator, SoundSequence_32 *seq, int32_t *timeOffset) {
    // Todo
    return 0;
}

void playdate_sound_sequence_setCurrentStep(Emulator *emulator, SoundSequence_32 *seq, int32_t step, int32_t timeOffset, int32_t playNotes) {
    // Todo
}

TwoPoleFilter_32 *playdate_sound_effect_twopolefilter_newFilter(Emulator *emulator) {
    // Todo
    return nullptr;
}

void playdate_sound_effect_twopolefilter_freeFilter(Emulator *emulator, TwoPoleFilter_32 *filter) {
    // Todo
}

void playdate_sound_effect_twopolefilter_setType(Emulator *emulator, TwoPoleFilter_32 *filter, int32_t type) {
    // Todo
}

void playdate_sound_effect_twopolefilter_setFrequency(Emulator *emulator, TwoPoleFilter_32 *filter, float frequency) {
    // Todo
}

void playdate_sound_effect_twopolefilter_setFrequencyModulator(Emulator *emulator, TwoPoleFilter_32 *filter, PDSynthSignalValue_32 *signal) {
    // Todo
}

PDSynthSignalValue_32 *playdate_sound_effect_twopolefilter_getFrequencyModulator(Emulator *emulator, TwoPoleFilter_32 *filter) {
    // Todo
    return nullptr;
}

void playdate_sound_effect_twopolefilter_setGain(Emulator *emulator, TwoPoleFilter_32 *filter, float gain) {
    // Todo
}

void playdate_sound_effect_twopolefilter_setResonance(Emulator *emulator, TwoPoleFilter_32 *filter, float resonance) {
    // Todo
}

void playdate_sound_effect_twopolefilter_setResonanceModulator(Emulator *emulator, TwoPoleFilter_32 *filter, PDSynthSignalValue_32 *signal) {
    // Todo
}

PDSynthSignalValue_32 *playdate_sound_effect_twopolefilter_getResonanceModulator(Emulator *emulator, TwoPoleFilter_32 *filter) {
    // Todo
    return nullptr;
}

OnePoleFilter_32 *playdate_sound_effect_onepolefilter_newFilter(Emulator *emulator) {
    // Todo
    return nullptr;
}

void playdate_sound_effect_onepolefilter_freeFilter(Emulator *emulator, OnePoleFilter_32 *filter) {
    // Todo
}

void playdate_sound_effect_onepolefilter_setParameter(Emulator *emulator, OnePoleFilter_32 *filter, float parameter) {
    // Todo
}

void playdate_sound_effect_onepolefilter_setParameterModulator(Emulator *emulator, OnePoleFilter_32 *filter, PDSynthSignalValue_32 *signal) {
    // Todo
}

PDSynthSignalValue_32 *playdate_sound_effect_onepolefilter_getParameterModulator(Emulator *emulator, OnePoleFilter_32 *filter) {
    // Todo
    return nullptr;
}

BitCrusher_32 *playdate_sound_effect_bitcrusher_newBitCrusher(Emulator *emulator) {
    // Todo
    return nullptr;
}

void playdate_sound_effect_bitcrusher_freeBitCrusher(Emulator *emulator, BitCrusher_32 *filter) {
    // Todo
}

void playdate_sound_effect_bitcrusher_setAmount(Emulator *emulator, BitCrusher_32 *filter, float amount) {
    // Todo
}

void playdate_sound_effect_bitcrusher_setAmountModulator(Emulator *emulator, BitCrusher_32 *filter, PDSynthSignalValue_32 *signal) {
    // Todo
}

PDSynthSignalValue_32 *playdate_sound_effect_bitcrusher_getAmountModulator(Emulator *emulator, BitCrusher_32 *filter) {
    // Todo
    return nullptr;
}

void playdate_sound_effect_bitcrusher_setUndersampling(Emulator *emulator, BitCrusher_32 *filter, float undersampling) {
    // Todo
}

void playdate_sound_effect_bitcrusher_setUndersampleModulator(Emulator *emulator, BitCrusher_32 *filter, PDSynthSignalValue_32 *signal) {
    // Todo
}

PDSynthSignalValue_32 *playdate_sound_effect_bitcrusher_getUndersampleModulator(Emulator *emulator, BitCrusher_32 *filter) {
    // Todo
    return nullptr;
}

RingModulator_32 *playdate_sound_effect_ringmodulator_newRingmod(Emulator *emulator) {
    // Todo
    return nullptr;
}

void playdate_sound_effect_ringmodulator_freeRingmod(Emulator *emulator, RingModulator_32 *filter) {
    // Todo
}

void playdate_sound_effect_ringmodulator_setFrequency(Emulator *emulator, RingModulator_32 *filter, float frequency) {
    // Todo
}

void playdate_sound_effect_ringmodulator_setFrequencyModulator(Emulator *emulator, RingModulator_32 *filter, PDSynthSignalValue_32 *signal) {
    // Todo
}

PDSynthSignalValue_32 *playdate_sound_effect_ringmodulator_getFrequencyModulator(Emulator *emulator, RingModulator_32 *filter) {
    // Todo
    return nullptr;
}

DelayLine_32 *playdate_sound_effect_delayline_newDelayLine(Emulator *emulator, int32_t length, int32_t stereo) {
    // Todo
    return nullptr;
}

void playdate_sound_effect_delayline_freeDelayLine(Emulator *emulator, DelayLine_32 *filter) {
    // Todo
}

void playdate_sound_effect_delayline_setLength(Emulator *emulator, DelayLine_32 *d, int32_t frames) {
    // Todo
}

void playdate_sound_effect_delayline_setFeedback(Emulator *emulator, DelayLine_32 *d, float fb) {
    // Todo
}

DelayLineTap_32 *playdate_sound_effect_delayline_addTap(Emulator *emulator, DelayLine_32 *d, int32_t delay) {
    // Todo
    return nullptr;
}

void playdate_sound_effect_delayline_freeTap(Emulator *emulator, DelayLineTap_32 *tap) {
    // Todo
}

void playdate_sound_effect_delayline_setTapDelay(Emulator *emulator, DelayLineTap_32 *t, int32_t frames) {
    // Todo
}

void playdate_sound_effect_delayline_setTapDelayModulator(Emulator *emulator, DelayLineTap_32 *t, PDSynthSignalValue_32 *mod) {
    // Todo
}

PDSynthSignalValue_32 *playdate_sound_effect_delayline_getTapDelayModulator(Emulator *emulator, DelayLineTap_32 *t) {
    // Todo
    return nullptr;
}

void playdate_sound_effect_delayline_setTapChannelsFlipped(Emulator *emulator, DelayLineTap_32 *t, int32_t flip) {
    // Todo
}

Overdrive_32 *playdate_sound_effect_overdrive_newOverdrive(Emulator *emulator) {
    // Todo
    return nullptr;
}

void playdate_sound_effect_overdrive_freeOverdrive(Emulator *emulator, Overdrive_32 *filter) {
    // Todo
}

void playdate_sound_effect_overdrive_setGain(Emulator *emulator, Overdrive_32 *o, float gain) {
    // Todo
}

void playdate_sound_effect_overdrive_setLimit(Emulator *emulator, Overdrive_32 *o, float limit) {
    // Todo
}

void playdate_sound_effect_overdrive_setLimitModulator(Emulator *emulator, Overdrive_32 *o, PDSynthSignalValue_32 *mod) {
    // Todo
}

PDSynthSignalValue_32 *playdate_sound_effect_overdrive_getLimitModulator(Emulator *emulator, Overdrive_32 *o) {
    // Todo
    return nullptr;
}

void playdate_sound_effect_overdrive_setOffset(Emulator *emulator, Overdrive_32 *o, float offset) {
    // Todo
}

void playdate_sound_effect_overdrive_setOffsetModulator(Emulator *emulator, Overdrive_32 *o, PDSynthSignalValue_32 *mod) {
    // Todo
}

PDSynthSignalValue_32 *playdate_sound_effect_overdrive_getOffsetModulator(Emulator *emulator, Overdrive_32 *o) {
    // Todo
    return nullptr;
}

SoundEffect_32 *playdate_sound_effect_newEffect(Emulator *emulator, cref_t proc, void *userdata) {
    // Todo
    return nullptr;
}

void playdate_sound_effect_freeEffect(Emulator *emulator, SoundEffect_32 *effect) {
    // Todo
}

void playdate_sound_effect_setMix(Emulator *emulator, SoundEffect_32 *effect, float level) {
    // Todo
}

void playdate_sound_effect_setMixModulator(Emulator *emulator, SoundEffect_32 *effect, PDSynthSignalValue_32 *signal) {
    // Todo
}

PDSynthSignalValue_32 *playdate_sound_effect_getMixModulator(Emulator *emulator, SoundEffect_32 *effect) {
    // Todo
    return nullptr;
}

void playdate_sound_effect_setUserdata(Emulator *emulator, SoundEffect_32 *effect, void *userdata) {
    // Todo
}

void *playdate_sound_effect_getUserdata(Emulator *emulator, SoundEffect_32 *effect) {
    // Todo
    return nullptr;
}

SoundChannel_32 *playdate_sound_channel_newChannel(Emulator *emulator) {
    // Todo
    return nullptr;
}

void playdate_sound_channel_freeChannel(Emulator *emulator, SoundChannel_32 *channel) {
    // Todo
}

int32_t playdate_sound_channel_addSource(Emulator *emulator, SoundChannel_32 *channel, SoundSource_32 *source) {
    // Todo
    return 0;
}

int32_t playdate_sound_channel_removeSource(Emulator *emulator, SoundChannel_32 *channel, SoundSource_32 *source) {
    // Todo
    return 0;
}

SoundSource_32 *playdate_sound_channel_addCallbackSource(Emulator *emulator, SoundChannel_32 *channel, cref_t callback, void *context, int32_t stereo) {
    // Todo
    return nullptr;
}

void playdate_sound_channel_addEffect(Emulator *emulator, SoundChannel_32 *channel, SoundEffect_32 *effect) {
    // Todo
}

void playdate_sound_channel_removeEffect(Emulator *emulator, SoundChannel_32 *channel, SoundEffect_32 *effect) {
    // Todo
}

void playdate_sound_channel_setVolume(Emulator *emulator, SoundChannel_32 *channel, float volume) {
    // Todo
}

float playdate_sound_channel_getVolume(Emulator *emulator, SoundChannel_32 *channel) {
    // Todo
    return 0;
}

void playdate_sound_channel_setVolumeModulator(Emulator *emulator, SoundChannel_32 *channel, PDSynthSignalValue_32 *mod) {
    // Todo
}

PDSynthSignalValue_32 *playdate_sound_channel_getVolumeModulator(Emulator *emulator, SoundChannel_32 *channel) {
    // Todo
    return nullptr;
}

void playdate_sound_channel_setPan(Emulator *emulator, SoundChannel_32 *channel, float pan) {
    // Todo
}

void playdate_sound_channel_setPanModulator(Emulator *emulator, SoundChannel_32 *channel, PDSynthSignalValue_32 *mod) {
    // Todo
}

PDSynthSignalValue_32 *playdate_sound_channel_getPanModulator(Emulator *emulator, SoundChannel_32 *channel) {
    // Todo
    return nullptr;
}

PDSynthSignalValue_32 *playdate_sound_channel_getDryLevelSignal(Emulator *emulator, SoundChannel_32 *channel) {
    // Todo
    return nullptr;
}

PDSynthSignalValue_32 *playdate_sound_channel_getWetLevelSignal(Emulator *emulator, SoundChannel_32 *channel) {
    // Todo
    return nullptr;
}

uint32_t playdate_sound_getCurrentTime(Emulator *emulator) {
    // Todo
    return 0;
}

SoundSource_32 *playdate_sound_addSource(Emulator *emulator, cref_t callback, void *context, int32_t stereo) {
    // Todo
    return nullptr;
}

SoundChannel_32 *playdate_sound_getDefaultChannel(Emulator *emulator) {
    // Todo
    return nullptr;
}

int32_t playdate_sound_addChannel(Emulator *emulator, SoundChannel_32 *channel) {
    // Todo
    return 0;
}

int32_t playdate_sound_removeChannel(Emulator *emulator, SoundChannel_32 *channel) {
    // Todo
    return 0;
}

void playdate_sound_setMicCallback(Emulator *emulator, cref_t callback, void *context, int32_t forceInternal) {
    // Todo
}

void playdate_sound_getHeadphoneState(Emulator *emulator, int32_t *headphone, int32_t *headsetmic, cref_t changeCallback) {
    // Todo
}

void playdate_sound_setOutputsActive(Emulator *emulator, int32_t headphone, int32_t speaker) {
    // Todo
}

int32_t playdate_sound_removeSource(Emulator *emulator, SoundSource_32 *source) {
    // Todo
    return 0;
}

int32_t playdate_display_getWidth(Emulator *emulator) {
    return DISPLAY_WIDTH / emulator->graphics.displayScale;
}

int32_t playdate_display_getHeight(Emulator *emulator) {
    return DISPLAY_HEIGHT / emulator->graphics.displayScale;
}

void playdate_display_setRefreshRate(Emulator *emulator, float rate) {
    emulator->graphics.framerate = std::max(1.0f, rate);
}

void playdate_display_setInverted(Emulator *emulator, int32_t flag) {
    emulator->graphics.displayInverted = flag;
}

void playdate_display_setScale(Emulator *emulator, uint32_t s) {
    if (s == 1 or s == 2 or s == 4 or s == 8)
        emulator->graphics.displayScale = s;
    else
        emulator->graphics.displayScale = 1;
}

void playdate_display_setMosaic(Emulator *emulator, uint32_t x, uint32_t y) {
    emulator->graphics.displayMosaic = {(int)x & 0x3, (int)y & 3};
}

void playdate_display_setFlipped(Emulator *emulator, int32_t x, int32_t y) {
    emulator->graphics.displayFlippedX = x;
    emulator->graphics.displayFlippedY = y;
}

void playdate_display_setOffset(Emulator *emulator, int32_t x, int32_t y) {
    emulator->graphics.displayOffset = {x, y};
}

int32_t playdate_scoreboards_addScore(Emulator *emulator, uint8_t *boardId, uint32_t value, cref_t callback) {
    // Todo
    return 0;
}

int32_t playdate_scoreboards_getPersonalBest(Emulator *emulator, uint8_t *boardId, cref_t callback) {
    // Todo
    return 0;
}

void playdate_scoreboards_freeScore(Emulator *emulator, PDScore_32 *score) {
    // Todo
}

int32_t playdate_scoreboards_getScoreboards(Emulator *emulator, cref_t callback) {
    // Todo
    return 0;
}

void playdate_scoreboards_freeBoardsList(Emulator *emulator, PDBoardsList_32 *boardsList) {
    // Todo
}

int32_t playdate_scoreboards_getScores(Emulator *emulator, uint8_t *boardId, cref_t callback) {
    // Todo
    return 0;
}

void playdate_scoreboards_freeScoresList(Emulator *emulator, PDScoresList_32 *scoresList) {
    // Todo
}
