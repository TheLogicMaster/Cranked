// ReSharper disable CppParameterMayBeConstPtrOrRef
// ReSharper disable CppParameterMayBeConst
// NOLINTBEGIN(*-non-const-parameter)

#include "Cranked.hpp"
#include "gen/PlaydateAPI.hpp"

using namespace cranked;

VideoPlayer cranked::playdate_video_loadVideo(Cranked *cranked, uint8 *path) {
    auto player = cranked->graphics.getVideoPlayer((char *)path);
    player->reference();
    return player;
}

void cranked::playdate_video_freePlayer(Cranked *cranked, VideoPlayer p) {
    p->dereference();
}

int32 cranked::playdate_video_setContext(Cranked *cranked, VideoPlayer p, Bitmap context) {
    // Todo: What errors are possible? Should this check size?
    p->target = context;
    return true;
}

void cranked::playdate_video_useScreenContext(Cranked *cranked, VideoPlayer p) {
    p->target = cranked->graphics.frameBufferContext.bitmap;
}

int32 cranked::playdate_video_renderFrame(Cranked *cranked, VideoPlayer p, int32 n) {
    // Todo: What errors?
    if (n < 0 or n >= p->frames.size()) {
        p->lastError = cranked->nativeEngine.getEmulatedStringLiteral("Frame out of bounds");
        return false;
    }
    p->currentFrame = n;
    p->target->drawBitmap(p->frames[n].get(), 0, 0);
    return true;
}

uint8 *cranked::playdate_video_getError(Cranked *cranked, VideoPlayer p) {
    return cranked->nativeEngine.fromVirtualAddress<uint8>(p->lastError);
}

void cranked::playdate_video_getInfo(Cranked *cranked, VideoPlayer p, int32 *outWidth, int32 *outHeight, float *outFrameRate, int32 *outFrameCount, int32 *outCurrentFrame) {
    if (outWidth)
        *outWidth = p->size.x;
    if (outHeight)
        *outHeight = p->size.y;
    if (outFrameRate)
        *outFrameRate = p->frameRate;
    if (outFrameCount)
        *outFrameCount = (int)p->frames.size();
    if (outCurrentFrame)
        *outCurrentFrame = p->currentFrame;
}

Bitmap cranked::playdate_video_getContext(Cranked *cranked, VideoPlayer p) {
    if (!p->target)
        p->target = cranked->heap.construct<LCDBitmap_32>(*cranked, p->size.x, p->size.y);
    return p->target.get();
}

void cranked::playdate_graphics_clear(Cranked *cranked, uint32 color) {
    cranked->graphics.frameBufferContext.bitmap->clear(color);
}

void cranked::playdate_graphics_setBackgroundColor(Cranked *cranked, int32 color) {
    cranked->graphics.getCurrentDisplayContext().backgroundColor = LCDSolidColor(color);
}

void cranked::playdate_graphics_setStencil(Cranked *cranked, Bitmap stencil) {
    cranked->graphics.getCurrentDisplayContext().stencilImage = stencil;
}

int32 cranked::playdate_graphics_setDrawMode(Cranked *cranked, int32 mode) {
    auto &context = cranked->graphics.getCurrentDisplayContext();
    auto prev = context.bitmapDrawMode;
    context.bitmapDrawMode = LCDBitmapDrawMode(mode);
    return (int32)prev;
}

void cranked::playdate_graphics_setDrawOffset(Cranked *cranked, int32 dx, int32 dy) {
    cranked->graphics.getCurrentDisplayContext().drawOffset = {dx, dy};
}

void cranked::playdate_graphics_setClipRect(Cranked *cranked, int32 x, int32 y, int32 width, int32 height) {
    cranked->graphics.getCurrentDisplayContext().clipRect = {x, y, width, height};
}

void cranked::playdate_graphics_clearClipRect(Cranked *cranked) {
    cranked->graphics.getCurrentDisplayContext().clipRect = {};
}

void cranked::playdate_graphics_setLineCapStyle(Cranked *cranked, int32 endCapStyle) {
    cranked->graphics.getCurrentDisplayContext().lineEndCapStyle = LCDLineCapStyle(endCapStyle);
}

void cranked::playdate_graphics_setFont(Cranked *cranked, Font font) {
    cranked->graphics.getCurrentDisplayContext().getFont(PDFontVariant::Normal) = font;
}

void cranked::playdate_graphics_setTextTracking(Cranked *cranked, int32 tracking) {
    cranked->graphics.getCurrentDisplayContext().textTracking = tracking;
}

void cranked::playdate_graphics_pushContext(Cranked *cranked, Bitmap target) {
    cranked->graphics.pushContext(target);
}

void cranked::playdate_graphics_popContext(Cranked *cranked) {
    cranked->graphics.popContext();
}

void cranked::playdate_graphics_drawBitmap(Cranked *cranked, Bitmap bitmap, int32 x, int32 y, int32 flip) {
    cranked->graphics.drawBitmap(bitmap, x, y, (LCDBitmapFlip)flip);
}

void cranked::playdate_graphics_tileBitmap(Cranked *cranked, Bitmap bitmap, int32 x, int32 y, int32 width, int32 height, int32 flip) {
    cranked->graphics.drawBitmapTiled(bitmap, x, y, width, height, (LCDBitmapFlip)flip);
}

void cranked::playdate_graphics_drawLine(Cranked *cranked, int32 x1, int32 y1, int32 x2, int32 y2, int32 width, uint32 color) {
    cranked->graphics.drawLine(x1, y1, x2, y2, width, color);
}

void cranked::playdate_graphics_fillTriangle(Cranked *cranked, int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3, uint32 color) {
    cranked->graphics.fillTriangle(x1, y1, x2, y2, x3, y3, color);
}

void cranked::playdate_graphics_drawRect(Cranked *cranked, int32 x, int32 y, int32 width, int32 height, uint32 color) {
    cranked->graphics.drawRect(x, y, width, height, color);
}

void cranked::playdate_graphics_fillRect(Cranked *cranked, int32 x, int32 y, int32 width, int32 height, uint32 color) {
    cranked->graphics.fillRect(x, y, width, height, color);
}

void cranked::playdate_graphics_drawEllipse(Cranked *cranked, int32 x, int32 y, int32 width, int32 height, int32 lineWidth, float startAngle, float endAngle, uint32 color) {
    cranked->graphics.drawEllipse(x, y, width, height, lineWidth, startAngle, endAngle, color, false);
}

void cranked::playdate_graphics_fillEllipse(Cranked *cranked, int32 x, int32 y, int32 width, int32 height, float startAngle, float endAngle, uint32 color) {
    cranked->graphics.drawEllipse(x, y, width, height, 0, startAngle, endAngle, color, true);
}

void cranked::playdate_graphics_drawScaledBitmap(Cranked *cranked, Bitmap bitmap, int32 x, int32 y, float xscale, float yscale) {
    cranked->graphics.drawScaledBitmap(bitmap, x, y, xscale, yscale);
}

int32 cranked::playdate_graphics_drawText(Cranked *cranked, void *text, uint32 len, int32 encoding, int32 x, int32 y) {
    cranked->graphics.drawText(text, (int)len, PDStringEncoding(encoding), x, y);
    return 0; // Todo: Return value?
}

Bitmap cranked::playdate_graphics_newBitmap(Cranked *cranked, int32 width, int32 height, uint32 bgcolor) {
    auto bitmap = cranked->graphics.createBitmap(width, height);
    bitmap->reference();
    bitmap->clear(bgcolor);
    return bitmap;
}

void cranked::playdate_graphics_freeBitmap(Cranked *cranked, Bitmap ptr) {
    ptr->dereference();
}

Bitmap cranked::playdate_graphics_loadBitmap(Cranked *cranked, uint8 *path, cref_t *outerr) {
    try {
        auto image = cranked->graphics.getImage((const char *) path);
        image->reference();
        return image;
    } catch (exception &ex) {
        *outerr = cranked->getEmulatedStringLiteral(ex.what());
        return nullptr;
    }
}

Bitmap cranked::playdate_graphics_copyBitmap(Cranked *cranked, Bitmap bitmap) {
    return cranked->heap.construct<LCDBitmap_32>(*bitmap);
}

void cranked::playdate_graphics_loadIntoBitmap(Cranked *cranked, uint8 *path, Bitmap bitmap, cref_t *outerr) {
    try {
        ResourceRef loaded(cranked->graphics.getImage((const char *) path));
        *bitmap = *loaded;
    } catch (exception &ex) {
        if (outerr)
            *outerr = cranked->getEmulatedStringLiteral(ex.what());
    }
}

void cranked::playdate_graphics_getBitmapData(Cranked *cranked, Bitmap bitmap, int32 *width, int32 *height, int32 *rowbytes, cref_t *mask, cref_t *data) {
    if (width)
        *width = bitmap->width;
    if (height)
        *height = bitmap->height;
    if (rowbytes)
        *rowbytes = 1 + bitmap->width / 8;
    if (data)
        *data = cranked->toVirtualAddress(bitmap->data.data());
    if (mask)
        *mask = cranked->toVirtualAddress(bitmap->mask ? bitmap->mask->data.data() : nullptr);
}

void cranked::playdate_graphics_clearBitmap(Cranked *cranked, Bitmap bitmap, uint32 bgcolor) {
    bitmap->clear(bgcolor);
}

Bitmap cranked::playdate_graphics_rotatedBitmap(Cranked *cranked, Bitmap bitmap, float rotation, float xscale, float yscale, int32 *allocedSize) {
    auto rotated = cranked->graphics.rotateBitmap(bitmap, rotation, 0.5f, 0.5f, xscale, yscale);
    rotated->reference();
    return rotated;
}

BitmapTable cranked::playdate_graphics_newBitmapTable(Cranked *cranked, int32 count, int32 width, int32 height) {
    auto table = cranked->graphics.createBitmapTable(count); // Todo: Do something with size
    table->reference();
    return table;
}

void cranked::playdate_graphics_freeBitmapTable(Cranked *cranked, BitmapTable table) {
    table->dereference();
}

BitmapTable cranked::playdate_graphics_loadBitmapTable(Cranked *cranked, uint8 *path, cref_t *outerr) {
    try {
        auto table = cranked->graphics.getBitmapTable((const char *) path);
        table->reference();
        return table;
    } catch (exception &ex) {
        *outerr = cranked->getEmulatedStringLiteral(ex.what());
        return nullptr;
    }
}

void cranked::playdate_graphics_loadIntoBitmapTable(Cranked *cranked, uint8 *path, BitmapTable table, cref_t *outerr) {
    try {
        ResourceRef loaded(cranked->graphics.getBitmapTable((const char *) path));
        *table = *loaded;
    } catch (exception &ex) {
        *outerr = cranked->getEmulatedStringLiteral(ex.what());
    }
}

Bitmap cranked::playdate_graphics_getTableBitmap(Cranked *cranked, BitmapTable table, int32 idx) {
    return idx < table->bitmaps.size() ? table->bitmaps[idx].get() : nullptr;
}

Font cranked::playdate_graphics_loadFont(Cranked *cranked, uint8 *path, cref_t *outerr) {
    try {
        auto font = cranked->graphics.getFont((const char *) path);
        font->reference();
        return font;
    } catch (exception &ex) {
        *outerr = cranked->getEmulatedStringLiteral(ex.what());
        return nullptr;
    }
}

FontPage cranked::playdate_graphics_getFontPage(Cranked *cranked, Font font, uint32 c) {
    try {
        return font->pages.at((int)c / 256).get();
    } catch (out_of_range &) { // Todo: Don't use exceptions
        return nullptr;
    }
}

LCDFontGlyph_32 *cranked::playdate_graphics_getPageGlyph(Cranked *cranked, FontPage page, uint32 c, cref_t *bitmap, int32 *advance) {
    try {
        auto &glyph = page->glyphs.at((int)c % 256);
        if (bitmap)
            *bitmap = cranked->toVirtualAddress(glyph->bitmap.get());
        if (advance)
            *advance = glyph->advance;
        return glyph.get();
    } catch (out_of_range &) { // Todo: Don't use exceptions
        return nullptr;
    }
}

int32 cranked::playdate_graphics_getGlyphKerning(Cranked *cranked, LCDFontGlyph_32 *glyph, uint32 glyphcode, uint32 nextcode) {
    try {
        return nextcode < 256 ? glyph->shortKerningTable.at((int)nextcode) : glyph->longKerningTable.at((int)nextcode);
    } catch (out_of_range &) { // Todo: Don't use exceptions
        return 0;
    }
}

int32 cranked::playdate_graphics_getTextWidth(Cranked *cranked, Font font, void *text, uint32 len, int32 encoding, int32 tracking) {
    // Todo: Support other encodings
    int size = 0;
    auto string = (const char *) text;
    while (*string) {
        LCDFontGlyph_32 *glyph{};
        char character = *string;
        size += font->glyphWidth;
        try {
            glyph = font->pages.at(character / 256)->glyphs.at(character % 256).get();
        } catch (out_of_range &) {}
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

uint8 *cranked::playdate_graphics_getFrame(Cranked *cranked) {
    return cranked->graphics.frameBuffer->data.data();
}

uint8 *cranked::playdate_graphics_getDisplayFrame(Cranked *cranked) {
    return cranked->graphics.previousFrameBuffer->data.data();
}

Bitmap cranked::playdate_graphics_copyFrameBufferBitmap(Cranked *cranked) {
    return cranked->heap.construct<LCDBitmap_32>(*cranked->graphics.frameBuffer);
}

void cranked::playdate_graphics_markUpdatedRows(Cranked *cranked, int32 start, int32 end) {
    // Not needed, since getFrame() returns the actual frame buffer
}

void cranked::playdate_graphics_display(Cranked *cranked) {
    cranked->graphics.flushDisplayBuffer();
}

void cranked::playdate_graphics_setColorToPattern(Cranked *cranked, uint32 *color, Bitmap bitmap, int32 x, int32 y) {
    // Todo: Does this set color to a pointer which must be freed by the caller? Or just point to the bitmap data?
}

int32 cranked::playdate_graphics_checkMaskCollision(Cranked *cranked, Bitmap bitmap1, int32 x1, int32 y1, int32 flip1, Bitmap bitmap2, int32 x2, int32 y2, int32 flip2, LCDRect_32 rect) {
    return Graphics::checkBitmapMaskCollision(bitmap1, x1, y1, (GraphicsFlip)flip1, bitmap2, x2, y2, (GraphicsFlip)flip2, toRect(rect));
}

void cranked::playdate_graphics_setScreenClipRect(Cranked *cranked, int32 x, int32 y, int32 width, int32 height) {
    cranked->graphics.getCurrentDisplayContext().clipRect = { x, y, width, height };
}

void cranked::playdate_graphics_fillPolygon(Cranked *cranked, int32 nPoints, int32 *coords, uint32 color, int32 fillrule) {
    cranked->graphics.fillPolygon(coords, nPoints, color, (LCDPolygonFillRule)fillrule);
}

uint8 cranked::playdate_graphics_getFontHeight(Cranked *cranked, Font font) {
    return font->glyphHeight;
}

Bitmap cranked::playdate_graphics_getDisplayBufferBitmap(Cranked *cranked) {
    return cranked->graphics.frameBuffer.get();
}

void cranked::playdate_graphics_drawRotatedBitmap(Cranked *cranked, Bitmap bitmap, int32 x, int32 y, float rotation, float centerx, float centery, float xscale, float yscale) {
    cranked->graphics.drawRotatedBitmap(bitmap, x, y, rotation, centerx, centery, xscale, yscale);
}

void cranked::playdate_graphics_setTextLeading(Cranked *cranked, int32 lineHeightAdustment) {
    cranked->graphics.getCurrentDisplayContext().textLeading = lineHeightAdustment;
}

int32 cranked::playdate_graphics_setBitmapMask(Cranked *cranked, Bitmap bitmap, Bitmap mask) {
    bitmap->mask = mask;
    return 0; // Todo: Return value?
}

Bitmap cranked::playdate_graphics_getBitmapMask([[maybe_unused]] Cranked *cranked, Bitmap bitmap) {
    if (!bitmap->mask)
        return nullptr;
    bitmap->mask->reference();
    return bitmap->mask.get();
}

void cranked::playdate_graphics_setStencilImage(Cranked *cranked, Bitmap stencil, int32 tile) {
    cranked->graphics.getCurrentDisplayContext().stencilImage = stencil;
    cranked->graphics.getCurrentDisplayContext().stencilTiled = tile;
}

Font cranked::playdate_graphics_makeFontFromData(Cranked *cranked, LCDFontData_32 *data, int32 wide) {
    return cranked->graphics.getFont((uint8 *) data, wide);
}

int32 cranked::playdate_graphics_getTextTracking(Cranked *cranked) {
    return cranked->graphics.getCurrentDisplayContext().textTracking;
}

void cranked::playdate_graphics_setPixel(Cranked *cranked, int32 x, int32 y, uint32 c) {
    cranked->graphics.drawPixel(x, y, c);
}

int32 cranked::playdate_graphics_getBitmapPixel(Cranked *cranked, Bitmap bitmap, int32 x, int32 y) {
    return (int)bitmap->getPixel(x, y);
}

void cranked::playdate_graphics_getBitmapTableInfo(Cranked *cranked, BitmapTable table, int32 *count, int32 *width) {
    if (count)
        *count = (int)table->bitmaps.size();
    if (width)
        *width = table->bitmaps.empty() ? 0 : table->bitmaps[0]->width;
}

void *cranked::playdate_sys_realloc(Cranked *cranked, void* ptr, uint32 size) {
    if (size == 0) {
        cranked->nativeEngine.freeResource(ptr);
        return nullptr;
    }
    return cranked->heap.reallocate(ptr, size);
}

int32 cranked::playdate_sys_formatString(Cranked *cranked, cref_t *ret, uint8 *fmt, ...) {
    va_list args, args2;
    va_start(args, fmt);
    va_copy(args2, args);
    auto size = vsnprintf(nullptr, 0, (const char *) fmt, args);
    auto buffer = (uint8 *) cranked->heap.allocate(size + 1);
    *ret = cranked->toVirtualAddress(buffer);
    vsprintf((char *) buffer, (const char *) fmt, args2);
    va_end(args);
    va_end(args2);
    return size;
}

// Todo: Should log to logger rather than stdout
// Todo: Args are passed as emulated sizes, which will cause issues when `int` size is different, and such
// Todo: Probably needs conversion when calling vararg functions
void cranked::playdate_sys_logToConsole([[maybe_unused]] Cranked *cranked, uint8 *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf((const char *) fmt, args);
    va_end(args);
    printf("\n");// Doesn't depend on Lua setNewlinePrinted flag
}

void cranked::playdate_sys_error(Cranked *cranked, uint8 *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf((const char *) fmt, args);
    va_end(args);
    printf("\n");// Doesn't depend on Lua setNewlinePrinted flag
    cranked->stop();
}

int32 cranked::playdate_sys_getLanguage([[maybe_unused]] Cranked *cranked) {
    return (int) PDLanguage::English;
}

uint32 cranked::playdate_sys_getCurrentTimeMilliseconds(Cranked *cranked) {
    return cranked->currentMillis;
}

uint32 cranked::playdate_sys_getSecondsSinceEpoch([[maybe_unused]] Cranked *cranked, uint32 *milliseconds) {
    auto millis = duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
    if (milliseconds)
        *milliseconds = uint32(millis % 1000);
    return uint32(millis / 1000);
}

void cranked::playdate_sys_drawFPS(Cranked *cranked, int32 x, int32 y) {
    // Todo: Match the official FPS count better
    auto delta = max(cranked->getFrameDelta(), 1.0f);
    auto string = to_string((int)round(1000.0f / delta));
    if (string.length() < 2)
        string = "0" + string;
    cranked->graphics.fillRect(x - 2, y - 2, 14 * (int)string.size() + 2, 30, LCDSolidColor::White);
    auto context = cranked->graphics.pushContext(cranked->graphics.frameBuffer.get());
    context.bitmapDrawMode = LCDBitmapDrawMode::Copy;
    cranked->graphics.drawText(string.c_str(), (int)string.size(), PDStringEncoding::ASCII, x, y, cranked->graphics.getSystemFont());
    cranked->graphics.popContext();
}

void cranked::playdate_sys_setUpdateCallback(Cranked *cranked, cref_t update, void *userdata) {
    cranked->nativeEngine.setUpdateCallback(update, cranked->toVirtualAddress(userdata));
}

void cranked::playdate_sys_getButtonState(Cranked *cranked, int32 *current, int32 *pushed, int32 *released) {
    if (current)
        *current = cranked->currentInputs;
    if (pushed)
        *pushed = cranked->pressedInputs;
    if (released)
        *released = cranked->releasedInputs;
}

void cranked::playdate_sys_setPeripheralsEnabled(Cranked *cranked, int32 mask) {
    // Not needed
}

void cranked::playdate_sys_getAccelerometer(Cranked *cranked, float *outx, float *outy, float *outz) {
    if (outx)
        *outx = cranked->accelerometerX;
    if (outy)
        *outy = cranked->accelerometerY;
    if (outz)
        *outz = cranked->accelerometerZ;
}

float cranked::playdate_sys_getCrankChange(Cranked *cranked) {
    return cranked->getCrankChange();
}

float cranked::playdate_sys_getCrankAngle(Cranked *cranked) {
    return cranked->crankAngle;
}

int32 cranked::playdate_sys_isCrankDocked(Cranked *cranked) {
    return cranked->crankDocked;
}

int32 cranked::playdate_sys_setCrankSoundsDisabled(Cranked *cranked, int32 flag) {
    auto prev = cranked->crankSoundsEnabled;
    cranked->crankSoundsEnabled = flag;
    return prev;
}

int32 cranked::playdate_sys_getFlipped(Cranked *cranked) {
    return cranked->systemFlipped;
}

void cranked::playdate_sys_setAutoLockDisabled(Cranked *cranked, int32 disable) {
    cranked->autoLockDisabled = disable;
}

void cranked::playdate_sys_setMenuImage(Cranked *cranked, Bitmap bitmap, int32 xOffset) {
    cranked->menu.setImage(bitmap, xOffset);
}

MenuItem cranked::playdate_sys_addMenuItem(Cranked *cranked, uint8 *title, cref_t callback, void *userdata) {
    return cranked->menu.addItem((char *) title, PDMenuItem_32::Type::Button, {}, 0, callback, 0);
}

MenuItem cranked::playdate_sys_addCheckmarkMenuItem(Cranked *cranked, uint8 *title, int32 value, cref_t callback, void *userdata) {
    return cranked->menu.addItem((char *) title, PDMenuItem_32::Type::Checkmark, {}, value, callback, 0);
}

MenuItem cranked::playdate_sys_addOptionsMenuItem(Cranked *cranked, uint8 *title, cref_t *optionTitles, int32 optionsCount, cref_t f, void *userdata) {
    vector<string> options;
    for (int i = 0; i < optionsCount; i++)
        options.emplace_back((char *) cranked->fromVirtualAddress(optionTitles[i]));
    return cranked->menu.addItem((char *) title, PDMenuItem_32::Type::Options, options, 0, f, 0);
}

void cranked::playdate_sys_removeAllMenuItems(Cranked *cranked) {
    cranked->menu.clearItems();
}

void cranked::playdate_sys_removeMenuItem(Cranked *cranked, MenuItem menuItem) {
    cranked->menu.removeItem(menuItem);
}

int32 cranked::playdate_sys_getMenuItemValue(Cranked *cranked, MenuItem menuItem) {
    return menuItem->value;
}

void cranked::playdate_sys_setMenuItemValue(Cranked *cranked, MenuItem menuItem, int32 value) {
    menuItem->value = value;
}

uint8 *cranked::playdate_sys_getMenuItemTitle(Cranked *cranked, MenuItem menuItem) {
    return (uint8 *) menuItem->title.c_str();
}

void cranked::playdate_sys_setMenuItemTitle(Cranked *cranked, MenuItem menuItem, uint8 *title) {
    menuItem->title = (char *)title;
}

void *cranked::playdate_sys_getMenuItemUserdata(Cranked *cranked, MenuItem menuItem) {
    return cranked->fromVirtualAddress(menuItem->userdata);
}

void cranked::playdate_sys_setMenuItemUserdata(Cranked *cranked, MenuItem menuItem, void *ud) {
    menuItem->userdata = cranked->toVirtualAddress(ud);
}

int32 cranked::playdate_sys_getReduceFlashing(Cranked *cranked) {
    return cranked->tryReduceFlashing;
}

float cranked::playdate_sys_getElapsedTime(Cranked *cranked) {
    return (float)duration_cast<chrono::microseconds>((chrono::system_clock::now() - cranked->elapsedTimeStart)).count() / 1000000.0f;
}

void cranked::playdate_sys_resetElapsedTime(Cranked *cranked) {
    cranked->elapsedTimeStart = chrono::system_clock::now();
}

float cranked::playdate_sys_getBatteryPercentage(Cranked *cranked) {
    return cranked->batteryPercentage;
}

float cranked::playdate_sys_getBatteryVoltage(Cranked *cranked) {
    return cranked->batteryVoltage;
}

int32 cranked::playdate_sys_getTimezoneOffset(Cranked *cranked) {
    return cranked->timezoneOffset;
}

int32 cranked::playdate_sys_shouldDisplay24HourTime(Cranked *cranked) {
    return cranked->use24HourTime;
}

void cranked::playdate_sys_convertEpochToDateTime(Cranked *cranked, uint32 epoch, PDDateTime_32 *datetime) {
    // Todo
}

uint32 cranked::playdate_sys_convertDateTimeToEpoch(Cranked *cranked, PDDateTime_32 *datetime) {
    // Todo
    return 0;
}

void cranked::playdate_sys_clearICache(Cranked *cranked) {
    cranked->nativeEngine.flushCaches();
}

void cranked::playdate_sys_setButtonCallback(Cranked *cranked, cref_t cb, void *buttonud, int32 queuesize) {
    cranked->buttonCallback = cb;
    cranked->buttonCallbackUserdata = cranked->toVirtualAddress(buttonud);
    cranked->buttonCallbackQueueSize = queuesize;
}

void cranked::playdate_sys_setSerialMessageCallback(Cranked *cranked, cref_t callback) {
    cranked->serialMessageCallback = callback;
}

int32 cranked::playdate_sys_vaFormatString(Cranked *cranked, cref_t *outstr, uint8 *fmt, ...) {
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
    } catch (bad_alloc &) {
        size = -1;
    } catch (...) {
        free(buffer);
        throw;
    }
    free(buffer);
    return size;
}

int32 cranked::playdate_sys_parseString(Cranked *cranked, uint8 *str, uint8 *fmt, ...) {
    va_list list{};
    va_start(list, fmt);
    auto string = (const char *) str;
    auto fmtStr = (const char *) fmt;
    int stringIndex = 0;
    int formatIndex = 0;
    int stringLen = (int)strlen(string);
    int formatLen = (int)strlen(fmtStr);
    int matches = 0;

    while (formatIndex < formatLen) {
        char c = fmtStr[formatIndex++];

        if (isspace(c)) {
            while (formatIndex < formatLen and isspace(fmtStr[formatIndex]))
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
        c = fmtStr[formatIndex++];

        bool ignored = false;
        if (c == '*') {
            ignored = true;
            if (formatIndex >= formatLen)
                break;
            c = fmtStr[formatIndex++];
        }

        int widthStart = formatIndex - 1;
        int width = 0;
        if (isdigit(c)) {
            char *end{};
            strtol(fmtStr + widthStart, &end, 10);
            width = formatIndex = int(end - fmtStr) - widthStart; // Todo: Verify
            c = fmtStr[formatIndex++];
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
                if (formatIndex < formatLen and fmtStr[formatIndex] == '^') {
                    inverted = true;
                    formatIndex++;
                }
                set<char> set;
                if (formatIndex < formatLen and fmtStr[formatIndex] == ']') {
                    set.emplace(']');
                    formatIndex++;
                }
                while (true) {
                    if (formatIndex >= formatLen) {
                        failed = true;
                        break;
                    }
                    c = fmtStr[formatIndex++];
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
                int64 result{};
                uint64 uresult{};
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
                            *va_arg(list, int8 *) = (int8)result;
                            break;
                        case ArgType::uint8_t:
                            *va_arg(list, uint8 *) = (uint8)uresult;
                            break;
                        case ArgType::int16_t:
                            *va_arg(list, int16 *) = (int16)result;
                            break;
                        case ArgType::uint16_t:
                            *va_arg(list, uint16 *) = (uint16)uresult;
                            break;
                        case ArgType::int32_t:
                            *va_arg(list, int32 *) = (int32)result;
                            break;
                        case ArgType::uint32_t:
                            *va_arg(list, uint32 *) = (uint32)uresult;
                            break;
                        case ArgType::int64_t:
                            *va_arg(list, int64 *) = result;
                            break;
                        case ArgType::uint64_t:
                            *va_arg(list, uint64 *) = uresult;
                            break;
                    }
                }
                stringIndex = int(end - string);
                matches++;
                break;
            }
            case 'n':
                if (!ignored)
                    *va_arg(list, int32 *) = stringIndex;
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
                stringIndex = int(end - string);
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
                stringIndex = int(end - string);
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

int32 cranked::playdate_lua_addFunction(Cranked *cranked, cref_t f, uint8 *name, cref_t *outErr) {
    if (!cranked->getLuaContext()) {
        if (outErr)
            *outErr = cranked->getEmulatedStringLiteral("Lua environment not initialized");
        return false;
    }
    cranked->nativeEngine.pushEmulatedLuaFunction(f);
    cranked->luaEngine.setQualifiedLuaGlobal((const char *) name);
    if (outErr)
        *outErr = 0;
    return true;
}

// Todo: Figure out which version the old registerClass was depreciated, not compatible at present (Before SDK 1.9 at least)
int32 cranked::playdate_lua_registerClass(Cranked *cranked, uint8 *name, lua_reg_32 *reg, lua_val_32 *vals, int32 isstatic, cref_t *outErr) {
    auto context = cranked->getLuaContext();
    if (!context) {
        if (outErr)
            *outErr = cranked->getEmulatedStringLiteral("Lua environment not initialized");
        return false;
    }
    // Todo: Is there a benefit to respecting isstatic?
    lua_newtable(context);
    lua_pushstring(context, "__index");
    lua_pushvalue(context, -2);
    lua_rawset(context, -3);
    lua_pushstring(context, "__name");
    lua_pushstring(context, (char *)name);
    lua_rawset(context, -3);

    while (reg and reg->func and reg->name) {
        lua_pushstring(context, cranked->nativeEngine.fromVirtualAddress<char>(reg->name));
        cranked->nativeEngine.pushEmulatedLuaFunction(reg->func);
        lua_rawset(context, -3);
        reg++;
    }
    while (vals and vals->name) {
        lua_pushstring(context, cranked->nativeEngine.fromVirtualAddress<char>(vals->name));
        switch (vals->type) {
            case 0: // Int
                lua_pushinteger(context, (int)vals->v.intval);
                break;
            case 1: // Float
                lua_pushnumber(context, vals->v.floatval);
                break;
            case 2: // String
                lua_pushstring(context, cranked->nativeEngine.fromVirtualAddress<char>(vals->v.strval));
                break;
            default: // Todo: Error on unknown type?
                lua_pushnil(context);
                break;
        }
        lua_rawset(context, -3);
        vals++;
    }
    cranked->luaEngine.setQualifiedLuaGlobal((char *)name);
    if (outErr)
        *outErr = 0; // Todo: Error conditions (Already exists?)
    return true;
}

void cranked::playdate_lua_pushFunction(Cranked *cranked, cref_t f) {
    if (cranked->getLuaContext())
        cranked->nativeEngine.pushEmulatedLuaFunction(f);
}

int32 cranked::playdate_lua_indexMetatable(Cranked *cranked) {
    return cranked->luaEngine.indexMetatable();
}

void cranked::playdate_lua_stop(Cranked *cranked) {
    cranked->disableUpdateLoop = true;
}

void cranked::playdate_lua_start(Cranked *cranked) {
    cranked->disableUpdateLoop = false;
}

int32 cranked::playdate_lua_getArgCount(Cranked *cranked) {
    if (!cranked->getLuaContext())
        return 0;
    return lua_gettop(cranked->getLuaContext()); // Todo: Is this sufficient? Or should it not change when the stack top moves?
}

int32 cranked::playdate_lua_getArgType(Cranked *cranked, int32 pos, cref_t *outClass) {
    auto context = cranked->getLuaContext();
    if (!context) {
        if (outClass)
            *outClass = 0;
        return 0;
    }
    if (outClass) {
        if (lua_getmetatable(context, pos)) {
            lua_pushstring(context, "__name");
            lua_rawget(context, -2);
            *outClass = cranked->nativeEngine.getEmulatedStringLiteral(lua_tostring(context, -1));
            lua_pop(context, 2);
        } else
            *outClass = 0;
    }
    int type = lua_type(context, pos);
    if (type == LUA_TNUMBER)
        return lua_isinteger(context, pos) ? (int)LuaType::Int : (int)LuaType::Float;
    if (type == LUA_TUSERDATA or type == LUA_TLIGHTUSERDATA)
        return (int)LuaType::Object;
    if (type == LUA_TTHREAD)
        return (int)LuaType::Thread;
    return type;
}

int32 cranked::playdate_lua_argIsNil(Cranked *cranked, int32 pos) {
    if (!cranked->getLuaContext())
        return false;
    return lua_isnoneornil(cranked->getLuaContext(), pos);
}

// Todo: Do the getArg functions convert the argument if the type is different? Or should these be making copies first?

int32 cranked::playdate_lua_getArgBool(Cranked *cranked, int32 pos) {
    if (!cranked->getLuaContext())
        return false;
    return lua_toboolean(cranked->getLuaContext(), pos);
}

int32 cranked::playdate_lua_getArgInt(Cranked *cranked, int32 pos) {
    if (!cranked->getLuaContext())
        return 0;
    return lua_tointeger(cranked->getLuaContext(), pos);
}

float cranked::playdate_lua_getArgFloat(Cranked *cranked, int32 pos) {
    if (!cranked->getLuaContext())
        return 0;
    return lua_tonumber(cranked->getLuaContext(), pos);
}

uint8 *cranked::playdate_lua_getArgString(Cranked *cranked, int32 pos) {
    if (!cranked->getLuaContext())
        return nullptr;
    return (uint8 *) lua_tostring(cranked->getLuaContext(), pos);
}

uint8 *cranked::playdate_lua_getArgBytes(Cranked *cranked, int32 pos, uint32 *outlen) {
    if (!cranked->getLuaContext())
        return nullptr;
    size_t len;
    auto string = lua_tolstring(cranked->getLuaContext(), pos, &len);
    *outlen = len;
    return (uint8 *) string;
}

void *cranked::playdate_lua_getArgObject(Cranked *cranked, int32 pos, uint8 *type, cref_t *outud) {
    auto context = cranked->getLuaContext();
    if (outud)
        *outud = 0;
    if (!context)
        return nullptr;
    if (!lua_getmetatable(context, pos))
        return nullptr;
    lua_pushstring(context, "__name");
    lua_rawget(context, -2);
    auto name = lua_tostring(context, -1);
    bool good = name and string(name) == (char *)type;
    lua_pop(context, 2);
    if (!good)
        return nullptr;
    if (outud)
        *outud = HEAP_ADDRESS + lua_gettop(context);
    return *(void **)lua_touserdata(context, pos);
}

Bitmap cranked::playdate_lua_getBitmap(Cranked *cranked, int32 pos) {
    if (!cranked->getLuaContext())
        return nullptr;
    return LuaVal(cranked->getLuaContext(), pos).asUserdataObject<Bitmap>(); // Todo: Should this add a reference?
}

Sprite cranked::playdate_lua_getSprite(Cranked *cranked, int32 pos) {
    if (!cranked->getLuaContext())
        return nullptr;
    return LuaVal(cranked->getLuaContext(), pos).asUserdataObject<Sprite>(); // Todo: Should this add a reference?
}

void cranked::playdate_lua_pushNil(Cranked *cranked) {
    if (!cranked->getLuaContext())
        return;
    lua_pushnil(cranked->getLuaContext());
}

void cranked::playdate_lua_pushBool(Cranked *cranked, int32 val) {
    if (!cranked->getLuaContext())
        return;
    lua_pushboolean(cranked->getLuaContext(), val);
}

void cranked::playdate_lua_pushInt(Cranked *cranked, int32 val) {
    if (!cranked->getLuaContext())
        return;
    lua_pushinteger(cranked->getLuaContext(), val);
}

void cranked::playdate_lua_pushFloat(Cranked *cranked, float val) {
    if (!cranked->getLuaContext())
        return;
    lua_pushnumber(cranked->getLuaContext(), val);
}

void cranked::playdate_lua_pushString(Cranked *cranked, uint8 *str) {
    if (!cranked->getLuaContext())
        return;
    lua_pushstring(cranked->getLuaContext(), (const char *) str);
}

void cranked::playdate_lua_pushBytes(Cranked *cranked, uint8 *str, uint32 len) {
    if (!cranked->getLuaContext())
        return;
    lua_pushlstring(cranked->getLuaContext(), (const char *) str, len);
}

void cranked::playdate_lua_pushBitmap(Cranked *cranked, Bitmap bitmap) {
    if (!cranked->getLuaContext())
        return;
    cranked->luaEngine.pushResource(bitmap);
}

void cranked::playdate_lua_pushSprite(Cranked *cranked, Sprite sprite) {
    if (!cranked->getLuaContext())
        return;
    cranked->luaEngine.pushResource(sprite);
}

LuaUDObject_32 *cranked::playdate_lua_pushObject(Cranked *cranked, void* obj, uint8 *type, int32 nValues) {
    // Todo: Behavior on missing class? Just return null and push nothing? Push Nil?
    auto context = cranked->getLuaContext();
    if (!context)
        return nullptr;
    auto data = (void **)lua_newuserdatauv(context, sizeof(void *), nValues); // Just store a single pointer in the data block
    *data = obj;
    int pos = lua_gettop(context);
    cranked->luaEngine.getQualifiedLuaGlobal((char *)type);
    lua_setmetatable(context, -2);
    return (LuaUDObject_32 *)((char *)cranked->heap.baseAddress() + pos);
}

LuaUDObject_32 *cranked::playdate_lua_retainObject(Cranked *cranked, LuaUDObject_32 *obj) {
    auto context = cranked->getLuaContext();
    if (!context)
        return nullptr;
    int pos = int((intptr_t)obj - (intptr_t)cranked->heap.baseAddress());
    cranked->luaEngine.preserveLuaValue(pos);
    return obj;
}

void cranked::playdate_lua_releaseObject(Cranked *cranked, LuaUDObject_32 *obj) {
    auto context = cranked->getLuaContext();
    if (!context)
        return;
    int pos = int((intptr_t)obj - (intptr_t)cranked->heap.baseAddress());;
    cranked->luaEngine.releaseLuaValue(pos);
}

void cranked::playdate_lua_setUserValue(Cranked *cranked, LuaUDObject_32 *obj, uint32 slot) {
    auto context = cranked->getLuaContext();
    if (!context)
        return;
    lua_pushvalue(context, -1); // Todo: Verify that this should copy
    lua_setiuservalue(context, int((intptr_t)obj - (intptr_t)cranked->heap.baseAddress()), (int)slot);
}

int32 cranked::playdate_lua_getUserValue(Cranked *cranked, LuaUDObject_32 *obj, uint32 slot) {
    auto context = cranked->getLuaContext();
    if (!context)
        return 0;
    lua_getiuservalue(context, int((intptr_t)obj - (intptr_t)cranked->heap.baseAddress()), (int)slot);
    return lua_gettop(context);
}

void cranked::playdate_lua_callFunction_deprecated(Cranked *cranked, uint8 *name, int32 nargs) {
    if (!cranked->getLuaContext())
        return;
    cranked->luaEngine.getQualifiedLuaGlobal((const char *) name);
    lua_pcall(cranked->getLuaContext(), nargs, LUA_MULTRET, 0);
}

int32 cranked::playdate_lua_callFunction(Cranked *cranked, uint8 *name, int32 nargs, cref_t *outerr) {
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
            *outerr = cranked->getEmulatedStringLiteral(string("Error calling function: ") + cranked->luaEngine.getLuaError(result));
        return 0;
    }
    return 1;
}

void cranked::playdate_json_initEncoder(Cranked *cranked, json_encoder_32 *encoder, cref_t write, void *userdata, int32 pretty) {
    // Encoder API is stored directly after Playdate API, subtract json_encoder_32 struct size from API size to get address
    *encoder = *cranked->fromVirtualAddress<json_encoder_32>(API_ADDRESS + cranked->nativeEngine.getApiSize() - sizeof(json_encoder_32));
    encoder->writeStringFunc = write;
    encoder->userdata = cranked->toVirtualAddress(userdata);
    encoder->pretty = pretty;
}

// Todo: Fix this mess
static int json_decode(Cranked *cranked, json_decoder_32 *functions, json_value_32 *outval, const string &data) {
    struct JsonContext {
        JsonContext(Cranked *emu, json_decoder_32 *decoder, const vector<char> &path) : decoder(*decoder), path(path.begin(), path.end(), emu->heap.allocator<char>()) {}
        json_decoder_32 decoder;
        vheap_vector<char> path;
        bool inArray{};
        int arrayIndex{};
        string lastKey;
    };
    const string rootString = "_root";
    vheap_vector stack(cranked->heap.allocator<JsonContext>());
    nlohmann::json::parser_callback_t cb = [&](int depth, nlohmann::json::parse_event_t event, nlohmann::json &parsed) -> bool {
        switch (event) {
            case nlohmann::json::parse_event_t::object_start:
            case nlohmann::json::parse_event_t::array_start: {
                bool isRoot = stack.empty();
                bool isArray = event == nlohmann::json::parse_event_t::array_start;
                if (isRoot)
                    stack.emplace_back(cranked, functions, vector(rootString.begin(), rootString.end() + 1));
                auto &context = stack.back();
                bool inArray = context.inArray;
                bool willDecode = not inArray;
                if (inArray and context.decoder.shouldDecodeArrayValueAtIndex) {
                    context.arrayIndex++;
                    willDecode = cranked->nativeEngine.invokeEmulatedFunction<int32, ArgType::int32_t, ArgType::ptr_t, ArgType::int32_t>
                            (context.decoder.shouldDecodeArrayValueAtIndex, &context.decoder, context.arrayIndex);
                }
                if (not willDecode)
                    return false;
                if (!isRoot and willDecode) {
                    if (inArray) {
//                        stack.back().arrayIndex++;
                        string path(context.path.begin(), context.path.end());
                        path += "[" + to_string(context.arrayIndex) + "]";
                        stack.emplace_back(cranked, &context.decoder, vector(path.begin(), path.end() + 1)); // Todo: Does this copy the original or the current context?
                    } else {
                        string path(context.path.begin(), context.path.end());
                        path += "." + context.lastKey;
                        stack.emplace_back(cranked, &context.decoder, vector(path.begin(), path.end() + 1));
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
                    auto str = parsed.get<string>();
                    value = cranked->heap.allocate(str.size());
                    memcpy(value, str.data(), str.size());
                }
                stack.pop_back();
                auto &parentContext = stack.back();
                auto sublist = json_value_32{uint8(type), {.tableval = cranked->toVirtualAddress(value)}};
                if (stack.empty()) {
                    *outval = sublist;
                } else {
                    if (parentContext.inArray) {
                        if (parentContext.decoder.didDecodeArrayValue)
                            cranked->nativeEngine.invokeEmulatedFunction<void, ArgType::void_t, ArgType::ptr_t, ArgType::ptr_t, ArgType::struct2_t>
                                    (parentContext.decoder.didDecodeArrayValue, &parentContext.decoder, parentContext.arrayIndex, sublist);
                    } else {
                        auto keyData = vheap_vector(parentContext.lastKey.begin(), parentContext.lastKey.end() + 1, cranked->heap.allocator<char>());
                        if (parentContext.decoder.didDecodeTableValue)
                            cranked->nativeEngine.invokeEmulatedFunction<void, ArgType::void_t, ArgType::ptr_t, ArgType::ptr_t, ArgType::struct2_t>
                                    (parentContext.decoder.didDecodeTableValue, &parentContext.decoder, keyData.data(), sublist);
                    }
                }
//                if (context.decoder.returnString)
//                    stack.back().lastStringResult = ;
//                else
//                    stack.back().lastResult = {uint8(type), {.tableval = cranked->toVirtualAddress(value)}}; // Todo: What is value when didDecodeSublist isn't set?
//                printf("End %i %s\n", (int) event, parsed.dump().c_str());
                break;
            }
            case nlohmann::json::parse_event_t::key: {
                auto &context = stack.back();
                if (context.decoder.returnString)
                    return true;
                auto key = parsed.get<string>();
                auto keyData = vheap_vector(key.begin(), key.end(), cranked->heap.allocator<char>());
                bool shouldDecode = true;
                context.lastKey = key;
                if (context.decoder.shouldDecodeTableValueForKey)
                    shouldDecode = cranked->nativeEngine.invokeEmulatedFunction<int32, ArgType::int32_t, ArgType::ptr_t, ArgType::ptr_t>
                            (context.decoder.shouldDecodeTableValueForKey, &stack.back().decoder, keyData.data());
//                if (shouldDecode)
//                    stack.emplace_back(cranked, functions, vector<char>(keyData.begin(), keyData.end())); // Todo: Does this copy the original or the current context?
//                printf("Key %i %s\n", (int) event, parsed.dump().c_str());
                return shouldDecode;
            }
            case nlohmann::json::parse_event_t::value: {
                auto &context = stack.back();
                if (context.decoder.returnString)
                    return true;
                auto stringValue = vheap_vector(cranked->heap.allocator<char>()); // Todo: Is this expected to be valid after the `didDecode` functions?
                auto parseValue = [&]{
                    json_value_32 value{};
                    if (parsed.is_null())
                        value.type = (int) JsonValueType::Null;
                    else if (parsed.is_boolean()) {
                        value.data.intval = parsed.get<bool>();
                        value.type = (int) (value.data.intval ? JsonValueType::True : JsonValueType::False);
                    } else if (parsed.is_string()) {
                        value.type = (int) JsonValueType::String;
                        auto str = parsed.get<string>();
                        stringValue.resize(str.size());
                        memcpy(stringValue.data(), str.c_str(), str.size());
                        value.data.stringval = cranked->toVirtualAddress(stringValue.data());
                    } else if (parsed.is_number_integer()) {
                        value.data.intval = parsed.get<int>();
                        value.type = (int) JsonValueType::Integer;
                    } else if (parsed.is_number_float()) {
                        value.data.floatval = parsed.get<float>();
                        value.type = (int) JsonValueType::Float;
                    } else
                        throw CrankedError("Invalid JSON value type");
                    return value; // NOLINT: Value is not uninitialized
                };
                if (context.inArray) {
                    bool shouldDecode = true;
                    context.arrayIndex++; // Indices start at 1...
                    if (context.decoder.shouldDecodeArrayValueAtIndex)
                        shouldDecode = cranked->nativeEngine.invokeEmulatedFunction<int32, ArgType::int32_t, ArgType::ptr_t, ArgType::int32_t>
                                (context.decoder.shouldDecodeArrayValueAtIndex, &stack.back().decoder, context.arrayIndex);
                    if (shouldDecode) {
                        if (context.decoder.didDecodeArrayValue)
                            cranked->nativeEngine.invokeEmulatedFunction<void, ArgType::void_t, ArgType::ptr_t, ArgType::int32_t, ArgType::struct2_t>
                                    (context.decoder.didDecodeArrayValue, &stack.back().decoder, context.arrayIndex, parseValue());
                    }
                } else {
                    auto keyData = vheap_vector(context.lastKey.begin(), context.lastKey.end(), cranked->heap.allocator<char>());
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
    } catch (exception &ex) {
        if (functions->decodeError)
            cranked->nativeEngine.invokeEmulatedFunction<void, ArgType::void_t, ArgType::ptr_t, ArgType::uint32_t, ArgType::int32_t>
                    (functions->decodeError, functions, cranked->getEmulatedStringLiteral(ex.what()), -1); // Todo: Is line number retreivable?
        return 0;
    }
}

int32 cranked::playdate_json_decode(Cranked *cranked, json_decoder_32 *functions, json_reader_32 reader, json_value_32 *outval) {
    constexpr int BUFFER_SEGMENT = 512;
    int size = 0;
    vheap_vector buffer(BUFFER_SEGMENT, cranked->heap.allocator<char>());
    while (true) {
        int returned = cranked->nativeEngine.invokeEmulatedFunction<int32, ArgType::int32_t, ArgType::uint32_t, ArgType::ptr_t, ArgType::int32_t>
                (reader.read, reader.userdata, buffer.data() + size, buffer.size() - size);
        if (returned < BUFFER_SEGMENT)
            break;
        size += returned;
        buffer.resize(buffer.size() + BUFFER_SEGMENT);
    }
    return json_decode(cranked, functions, outval, buffer.data());
}

int32 cranked::playdate_json_decodeString(Cranked *cranked, json_decoder_32 *functions, uint8 *jsonString, json_value_32 *outval) {
    return json_decode(cranked, functions, outval, (const char *) jsonString);
}

static void encoderWrite(Cranked *cranked, json_encoder_32 *encoder, cref_t string, int len) {
    cranked->nativeEngine.invokeEmulatedFunction<void, ArgType::void_t, ArgType::uint32_t, ArgType::uint32_t, ArgType::int32_t>
            (encoder->writeStringFunc, encoder->userdata, string, len);
}

static void encoderWrite(Cranked *cranked, json_encoder_32 *encoder, const char *string) {
    encoderWrite(cranked, encoder, cranked->getEmulatedStringLiteral(string), (int)strlen(string));
}

static void encoderWrite(Cranked *cranked, json_encoder_32 *encoder, const string &string) {
    // Use vector rather than heap_string to prevent non-heap addresses for small strings
    auto data = vheap_vector(string.c_str(), string.c_str() + string.size(), cranked->heap.allocator<char>());
    encoderWrite(cranked, encoder, cranked->toVirtualAddress(data.data()), (int)string.length());
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

void cranked::json_encoder_addTableMember(Cranked *cranked, json_encoder_32 *encoder, uint8 *name, int32 len) {
    if (!encoder->startedTable)
        encoderWrite(cranked, encoder, ",");
    if (encoder->pretty) {
        encoderWrite(cranked, encoder, "\n");
        for (int i = 0; i < encoder->depth; i++)
            encoderWrite(cranked, encoder, "\t");
    }
    encoderWrite(cranked, encoder, nlohmann::json(string((const char *) name, len)).dump());
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

void cranked::json_encoder_writeInt(Cranked *cranked, json_encoder_32 *encoder, int32 num) {
    encoderWrite(cranked, encoder, to_string(num));
}

void cranked::json_encoder_writeDouble(Cranked *cranked, json_encoder_32 *encoder, double num) {
    encoderWrite(cranked, encoder, to_string(num));
}

void cranked::json_encoder_writeString(Cranked *cranked, json_encoder_32 *encoder, uint8 *str, int32 len) {
    encoderWrite(cranked, encoder, nlohmann::json(string((const char *) str, len)).dump());
}

uint8 *cranked::playdate_file_geterr(Cranked *cranked) {
    return cranked->fromVirtualAddress<uint8>(cranked->files.lastError); // This conversion is redundant, but simplifies things
}

int32 cranked::playdate_file_listfiles(Cranked *cranked, uint8 *path, cref_t callback, void *userdata, int32 showhidden) {
    vector<string> files;
    if (cranked->files.listFiles((const char *) path, showhidden, files))
        return -1;
    for (auto &file : files)
        cranked->nativeEngine.invokeEmulatedFunction<void, ArgType::ptr_t, ArgType::ptr_t>(callback, file.c_str(), userdata);
    return 0;
}

int32 cranked::playdate_file_stat(Cranked *cranked, uint8 *path, FileStat_32 *stats) {
    return cranked->files.stat((const char *) path, *stats);
}

int32 cranked::playdate_file_mkdir(Cranked *cranked, uint8 *path) {
    return cranked->files.mkdir((const char *) path);
}

int32 cranked::playdate_file_unlink(Cranked *cranked, uint8 *name, int32 recursive) {
    return cranked->files.unlink((const char *) name, recursive);
}

int32 cranked::playdate_file_rename(Cranked *cranked, uint8 *from, uint8 *to) {
    return cranked->files.rename((const char *) from, (const char *) to);
}

File cranked::playdate_file_open(Cranked *cranked, uint8 *name, int32 mode) {
    return cranked->files.open((const char *) name, FileOptions(mode));
}

int32 cranked::playdate_file_close(Cranked *cranked, File file) {
    return cranked->files.close(file);
}

int32 cranked::playdate_file_read(Cranked *cranked, File file, void *buf, uint32 len) {
    return cranked->files.read(file, buf, (int)len);
}

int32 cranked::playdate_file_write(Cranked *cranked, File file, void *buf, uint32 len) {
    return cranked->files.write(file, buf, (int)len);
}

int32 cranked::playdate_file_flush(Cranked *cranked, File file) {
    return cranked->files.flush(file);
}

int32 cranked::playdate_file_tell(Cranked *cranked, File file) {
    return cranked->files.tell(file);
}

int32 cranked::playdate_file_seek(Cranked *cranked, File file, int32 pos, int32 whence) {
    return cranked->files.seek(file, pos, whence);
}

void cranked::playdate_sprite_setAlwaysRedraw(Cranked *cranked, int32 flag) {
    cranked->graphics.alwaysRedrawSprites = flag;
}

void cranked::playdate_sprite_addDirtyRect(Cranked *cranked, LCDRect_32 dirtyRect) {
    // Todo
}

void cranked::playdate_sprite_drawSprites(Cranked *cranked) {
    cranked->graphics.drawSprites();
}

void cranked::playdate_sprite_updateAndDrawSprites(Cranked *cranked) {
    cranked->graphics.updateSprites();
    cranked->graphics.drawSprites();
}

Sprite cranked::playdate_sprite_newSprite(Cranked *cranked) {
    return cranked->nativeEngine.createReferencedResource<Sprite>();
}

void cranked::playdate_sprite_freeSprite(Cranked *cranked, Sprite sprite) {
    cranked->graphics.removeSpriteFromDrawList(sprite); // Just in case, allow deletion
    sprite->dereference();
}

Sprite cranked::playdate_sprite_copy(Cranked *cranked, Sprite sprite) {
    auto copy = sprite->copy();
    copy->reference();
    return copy;
}

void cranked::playdate_sprite_addSprite(Cranked *cranked, Sprite sprite) {
    cranked->graphics.addSpriteToDrawList(sprite);
}

void cranked::playdate_sprite_removeSprite(Cranked *cranked, Sprite sprite) {
    cranked->graphics.removeSpriteFromDrawList(sprite);
}

void cranked::playdate_sprite_removeSprites(Cranked *cranked, cref_t *sprites, int32 count) {
    for (int i = 0; i < count; i++)
        cranked->graphics.removeSpriteFromDrawList(cranked->nativeEngine.fromVirtualAddress<LCDSprite_32>(sprites[i]));
}

void cranked::playdate_sprite_removeAllSprites(Cranked *cranked) {
    cranked->graphics.clearSpriteDrawList();
}

int32 cranked::playdate_sprite_getSpriteCount(Cranked *cranked) {
    return (int32) cranked->graphics.spriteDrawList.size();
}

void cranked::playdate_sprite_setBounds(Cranked *cranked, Sprite sprite, PDRect_32 bounds) {
    sprite->setBounds(toRect(bounds));
}

PDRect_32 cranked::playdate_sprite_getBounds(Cranked *cranked, Sprite sprite) {
    return fromRect(sprite->getBounds());
}

void cranked::playdate_sprite_moveTo(Cranked *cranked, Sprite sprite, float x, float y) {
    sprite->setPosition({ x, y });
}

void cranked::playdate_sprite_moveBy(Cranked *cranked, Sprite sprite, float dx, float dy) {
    sprite->setPosition(sprite->getPosition() + Vec2{ dx, dy });
}

void cranked::playdate_sprite_setImage(Cranked *cranked, Sprite sprite, Bitmap image, int32 flip) {
    sprite->setImage(image, (LCDBitmapFlip)flip);
}

Bitmap cranked::playdate_sprite_getImage(Cranked *cranked, Sprite sprite) {
    return sprite->image.get();
}

void cranked::playdate_sprite_setSize(Cranked *cranked, Sprite s, float width, float height) {
    s->setSize({ width, height });
}

void cranked::playdate_sprite_setZIndex(Cranked *cranked, Sprite sprite, int16 zIndex) {
    sprite->zIndex = zIndex;
}

int16 cranked::playdate_sprite_getZIndex(Cranked *cranked, Sprite sprite) {
    return sprite->zIndex;
}

void cranked::playdate_sprite_setDrawMode(Cranked *cranked, Sprite sprite, int32 mode) {
    sprite->drawMode = (LCDBitmapDrawMode) mode;
}

void cranked::playdate_sprite_setImageFlip(Cranked *cranked, Sprite sprite, int32 flip) {
    sprite->flip = (LCDBitmapFlip) flip;
}

int32 cranked::playdate_sprite_getImageFlip(Cranked *cranked, Sprite sprite) {
    return (int32)sprite->flip;
}

void cranked::playdate_sprite_setStencil(Cranked *cranked, Sprite sprite, Bitmap stencil) {
    sprite->stencil = stencil;
}

void cranked::playdate_sprite_setClipRect(Cranked *cranked, Sprite sprite, LCDRect_32 clipRect) {
    sprite->clipRect = toRect(clipRect);
}

void cranked::playdate_sprite_clearClipRect(Cranked *cranked, Sprite sprite) {
    sprite->clipRect = {};
}

void cranked::playdate_sprite_setClipRectsInRange(Cranked *cranked, LCDRect_32 clipRect, int32 startZ, int32 endZ) {
    cranked->graphics.setSpriteClipRectsInRage(toRect(clipRect), startZ, endZ);
}

void cranked::playdate_sprite_clearClipRectsInRange(Cranked *cranked, int32 startZ, int32 endZ) {
    cranked->graphics.clearSpriteClipRectsInRage(startZ, endZ);
}

void cranked::playdate_sprite_setUpdatesEnabled(Cranked *cranked, Sprite sprite, int32 flag) {
    sprite->updatesEnabled = flag;
}

int32 cranked::playdate_sprite_updatesEnabled(Cranked *cranked, Sprite sprite) {
    return sprite->updatesEnabled;
}

void cranked::playdate_sprite_setCollisionsEnabled(Cranked *cranked, Sprite sprite, int32 flag) {
    sprite->setCollisionsEnabled(flag);
}

int32 cranked::playdate_sprite_collisionsEnabled(Cranked *cranked, Sprite sprite) {
    return sprite->areCollisionsEnabled();
}

void cranked::playdate_sprite_setVisible(Cranked *cranked, Sprite sprite, int32 flag) {
    sprite->visible = flag;
}

int32 cranked::playdate_sprite_isVisible(Cranked *cranked, Sprite sprite) {
    return sprite->visible;
}

void cranked::playdate_sprite_setOpaque(Cranked *cranked, Sprite sprite, int32 flag) {
    sprite->opaque = flag;
}

void cranked::playdate_sprite_markDirty(Cranked *cranked, Sprite sprite) {
    sprite->dirty = true;
}

void cranked::playdate_sprite_setTag(Cranked *cranked, Sprite sprite, uint8 tag) {
    sprite->tag = tag;
}

uint8 cranked::playdate_sprite_getTag(Cranked *cranked, Sprite sprite) {
    return sprite->tag;
}

void cranked::playdate_sprite_setIgnoresDrawOffset(Cranked *cranked, Sprite sprite, int32 flag) {
    sprite->ignoresDrawOffset = flag;
}

void cranked::playdate_sprite_setUpdateFunction(Cranked *cranked, Sprite sprite, cref_t func) {
    sprite->updateFunction = func;
}

void cranked::playdate_sprite_setDrawFunction(Cranked *cranked, Sprite sprite, cref_t func) {
    sprite->drawFunction = func;
}

void cranked::playdate_sprite_getPosition(Cranked *cranked, Sprite sprite, float *x, float *y) {
    auto [posX, posY] = sprite->getPosition();
    if (x)
        *x = posX;
    if (y)
        *y = posY;
}

void cranked::playdate_sprite_resetCollisionWorld(Cranked *cranked) {
    cranked->bump.resetWorld();
}

void cranked::playdate_sprite_setCollideRect(Cranked *cranked, Sprite sprite, PDRect_32 collideRect) {
    sprite->setCollisionRect(toRect(collideRect));
}

PDRect_32 cranked::playdate_sprite_getCollideRect(Cranked *cranked, Sprite sprite) {
    return fromRect(sprite->collideRect);
}

void cranked::playdate_sprite_clearCollideRect(Cranked *cranked, Sprite sprite) {
    sprite->setCollisionRect({});
}

void cranked::playdate_sprite_setCollisionResponseFunction(Cranked *cranked, Sprite sprite, cref_t func) {
    sprite->collideResponseFunction = func;
}

static SpriteCollisionInfo_32 *moveOrCheckCollisions(Cranked *cranked, Sprite sprite, float goalX, float goalY, float *actualX, float *actualY, int32 *len, bool sim) {
    Vec2 goal = { goalX, goalY };
    auto [actual, collisions] = cranked->bump.move(sprite, goal, sim, [&](Sprite a, Sprite b){
        if (sprite->collideResponseFunction)
            return (Bump::ResponseType) cranked->nativeEngine.invokeEmulatedFunction<int32_t, ArgType::int32_t, ArgType::ptr_t, ArgType::ptr_t>
                    (sprite->collideResponseFunction, a, b);
        return Bump::ResponseType::Freeze;
    });
    int count = (int)collisions.size();
    if (len)
        *len = count;
    if (actualX)
        *actualX = actual.x;
    if (actualY)
        *actualY = actual.y;
    if (count == 0)
        return nullptr;
    auto results = (SpriteCollisionInfo_32 *)cranked->heap.allocate(sizeof(SpriteCollisionInfo_32) * count);
    for (int i = 0; i < count; i++) {
        auto &col = collisions[i];
        results[i] = {
            cranked->nativeEngine.toVirtualAddress(col.sprite),
            cranked->nativeEngine.toVirtualAddress(col.other),
            (int8)col.type,
            col.overlaps,
            col.ti,
            col.move.asCollisionPoint(),
            col.normal.asCollisionVector(),
            col.touch.asCollisionPoint(),
            fromRect(col.entityRect),
            fromRect(col.otherRect)
        };
    }
    return results;
}

SpriteCollisionInfo_32 *cranked::playdate_sprite_checkCollisions(Cranked *cranked, Sprite sprite, float goalX, float goalY, float *actualX, float *actualY, int32 *len) {
    return moveOrCheckCollisions(cranked, sprite, goalX, goalY, actualX, actualY, len, true);
}

SpriteCollisionInfo_32 *cranked::playdate_sprite_moveWithCollisions(Cranked *cranked, Sprite sprite, float goalX, float goalY, float *actualX, float *actualY, int32 *len) {
    return moveOrCheckCollisions(cranked, sprite, goalX, goalY, actualX, actualY, len, false);
}

static cref_t *returnSpriteList(Cranked *cranked, const vector<Sprite> &sprites, int32 *len) {
    int count = (int)sprites.size();
    if (len)
        *len = count;
    if (count == 0)
        return nullptr;
    auto results = (cref_t *)cranked->heap.allocate(sizeof(cref_t) * count);
    for (int i = 0; i < count; i++)
        results[i] = cranked->nativeEngine.toVirtualAddress(sprites[i]);
    return results;
}

cref_t *cranked::playdate_sprite_querySpritesAtPoint(Cranked *cranked, float x, float y, int32 *len) {
    return returnSpriteList(cranked, cranked->bump.queryPoint({ x, y }), len);
}

cref_t *cranked::playdate_sprite_querySpritesInRect(Cranked *cranked, float x, float y, float width, float height, int32 *len) {
    return returnSpriteList(cranked, cranked->bump.queryRect({ x, y, width, height }), len);
}

cref_t *cranked::playdate_sprite_querySpritesAlongLine(Cranked *cranked, float x1, float y1, float x2, float y2, int32 *len) {
    return returnSpriteList(cranked, cranked->bump.querySegment({ x1, y1, x2, y2 }), len);
}

SpriteQueryInfo_32 *cranked::playdate_sprite_querySpriteInfoAlongLine(Cranked *cranked, float x1, float y1, float x2, float y2, int32 *len) {
    auto segmentInfo = cranked->bump.querySegmentInfo(LineSeg{x1, y1, x2, y2});
    int count = (int)segmentInfo.size();
    if (len)
        *len = count;
    if (count == 0)
        return nullptr;
    auto results = (SpriteQueryInfo_32 *)cranked->heap.allocate(sizeof(SpriteQueryInfo_32) * count);
    for (int i = 0; i < count; i++) {
        auto &info = segmentInfo[i];
        auto entry = info.seg.start.asCollisionPoint();
        auto exit = info.seg.end.asCollisionPoint();
        results[i] = { cranked->nativeEngine.toVirtualAddress(info.sprite), info.ti1, info.ti2, entry, exit };
    }
    return results;
}

cref_t *cranked::playdate_sprite_overlappingSprites(Cranked *cranked, Sprite sprite, int32 *len) {
    return returnSpriteList(cranked, cranked->bump.getOverlappingSprites(sprite), len);
}

cref_t *cranked::playdate_sprite_allOverlappingSprites(Cranked *cranked, int32 *len) {
    return returnSpriteList(cranked, cranked->bump.getAllOverlappingSprites(), len);
}

void cranked::playdate_sprite_setStencilPattern(Cranked *cranked, Sprite sprite, uint8 *pattern) {
    sprite->stencil = cranked->heap.construct<LCDBitmap_32>(*cranked, 8, 8);
    memcpy(sprite->stencil->data.data(), pattern, 8);
    sprite->stencilTiled = true; // Is this right?
}

void cranked::playdate_sprite_clearStencil(Cranked *cranked, Sprite sprite) {
    sprite->stencil = nullptr;
}

void cranked::playdate_sprite_setUserdata(Cranked *cranked, Sprite sprite, void *userdata) {
    sprite->userdata = cranked->toVirtualAddress(userdata);
}

void *cranked::playdate_sprite_getUserdata(Cranked *cranked, Sprite sprite) {
    return cranked->fromVirtualAddress(sprite->userdata);
}

void cranked::playdate_sprite_setStencilImage(Cranked *cranked, Sprite sprite, Bitmap stencil, int32 tile) {
    sprite->stencil = stencil;
    sprite->stencilTiled = tile;
}

void cranked::playdate_sprite_setCenter(Cranked *cranked, Sprite s, float x, float y) {
    s->setCenter({ x, y });
}

void cranked::playdate_sprite_getCenter(Cranked *cranked, Sprite s, float *x, float *y) {
    if (x)
        *x = s->getCenter().x;
    if (y)
        *y = s->getCenter().y;
}

void cranked::playdate_sound_source_setVolume(Cranked *cranked, SoundSource c, float lvol, float rvol) {
    c->leftVolume = lvol;
    c->rightVolume = rvol;
}

void cranked::playdate_sound_source_getVolume(Cranked *cranked, SoundSource c, float *outl, float *outr) {
    if (outl)
        *outl = c->leftVolume;
    if (outr)
        *outr = c->rightVolume;
}

int32 cranked::playdate_sound_source_isPlaying(Cranked *cranked, SoundSource c) {
    return c->playing;
}

void cranked::playdate_sound_source_setFinishCallback(Cranked *cranked, SoundSource c, cref_t callback, void *userdata) {
    c->completionCallback = callback;
    c->completionCallbackUserdata = Cranked::version < VERSION_2_4_1 ? 0 : cranked->toVirtualAddress(userdata);
}

FilePlayer cranked::playdate_sound_fileplayer_newPlayer(Cranked *cranked) {
    return cranked->audio.allocateReferencedSource<FilePlayer_32>();
}

void cranked::playdate_sound_fileplayer_freePlayer(Cranked *cranked, FilePlayer player) {
    player->dereference();
}

int32 cranked::playdate_sound_fileplayer_loadIntoPlayer(Cranked *cranked, FilePlayer player, uint8 *path) {
    // Todo
    return 0;
}

void cranked::playdate_sound_fileplayer_setBufferLength(Cranked *cranked, FilePlayer player, float bufferLen) {
    // Todo
}

int32 cranked::playdate_sound_fileplayer_play(Cranked *cranked, FilePlayer player, int32 repeat) {
    // Todo
    return 0;
}

int32 cranked::playdate_sound_fileplayer_isPlaying(Cranked *cranked, FilePlayer player) {
    return player->playing; // Todo: Does pausing affect this
}

void cranked::playdate_sound_fileplayer_pause(Cranked *cranked, FilePlayer player) {
    player->paused = true;
}

void cranked::playdate_sound_fileplayer_stop(Cranked *cranked, FilePlayer player) {
    player->playing = false;
    player->sampleOffset = 0;
}

void cranked::playdate_sound_fileplayer_setVolume(Cranked *cranked, FilePlayer player, float left, float right) {
    player->leftVolume = left;
    player->rightVolume = right;
}

void cranked::playdate_sound_fileplayer_getVolume(Cranked *cranked, FilePlayer player, float *left, float *right) {
    if (left)
        *left = player->leftVolume;
    if (right)
        *right = player->rightVolume;
}

float cranked::playdate_sound_fileplayer_getLength(Cranked *cranked, FilePlayer player) {
    return player->file ? framesToSeconds((int) player->file->romData->size()) : 0;
}

void cranked::playdate_sound_fileplayer_setOffset(Cranked *cranked, FilePlayer player, float offset) {
    // Todo
}

void cranked::playdate_sound_fileplayer_setRate(Cranked *cranked, FilePlayer player, float rate) {
    player->rate = rate;
}

void cranked::playdate_sound_fileplayer_setLoopRange(Cranked *cranked, FilePlayer player, float start, float end) {
    player->loopStart = framesFromSeconds(start);
    player->loopEnd = framesFromSeconds(end);
}

int32 cranked::playdate_sound_fileplayer_didUnderrun(Cranked *cranked, FilePlayer player) {
    return player->underran;
}

void cranked::playdate_sound_fileplayer_setFinishCallback(Cranked *cranked, FilePlayer player, cref_t callback, void *userdata) {
    player->completionCallback = callback;
    player->completionCallbackUserdata = Cranked::version < VERSION_2_4_1 ? 0 : cranked->toVirtualAddress(userdata);
}

void cranked::playdate_sound_fileplayer_setLoopCallback(Cranked *cranked, FilePlayer player, cref_t callback, void *userdata) {
    player->loopCallback = callback;
    player->loopCallbackUserdata = Cranked::version < VERSION_2_4_1 ? 0 : cranked->toVirtualAddress(userdata);
}

float cranked::playdate_sound_fileplayer_getOffset(Cranked *cranked, FilePlayer player) {
    return framesToSeconds(player->sampleOffset);
}

float cranked::playdate_sound_fileplayer_getRate(Cranked *cranked, FilePlayer player) {
    return player->rate;
}

void cranked::playdate_sound_fileplayer_setStopOnUnderrun(Cranked *cranked, FilePlayer player, int32 flag) {
    player->stopOnUnderrun = flag;
}
void cranked::playdate_sound_fileplayer_fadeVolume(Cranked *cranked, FilePlayer player, float left, float right, int32 len, cref_t finishCallback, void *userdata) {
    // Todo (userdata was added in VERSION_2_4_1, otherwise use 0)
}

void cranked::playdate_sound_fileplayer_setMP3StreamSource(Cranked *cranked, FilePlayer player, cref_t dataSource, void *userdata, float bufferLen) {
    // Todo
}

AudioSample cranked::playdate_sound_sample_newSampleBuffer(Cranked *cranked, int32 byteCount) {
    return cranked->nativeEngine.createReferencedResource<AudioSample>(byteCount);
}

int32 cranked::playdate_sound_sample_loadIntoSample(Cranked *cranked, AudioSample sample, uint8 *path) {
    // Todo
    return 0;
}

AudioSample cranked::playdate_sound_sample_load(Cranked *cranked, uint8 *path) {
    // Todo
    return nullptr;
}

AudioSample cranked::playdate_sound_sample_newSampleFromData(Cranked *cranked, uint8 *data, int32 format, uint32 sampleRate, int32 byteCount, int32 shouldFreeData) {
    // Todo
    return nullptr;
}

void cranked::playdate_sound_sample_getData(Cranked *cranked, AudioSample sample, cref_t *data, int32 *soundFormat, uint32 *sampleRate, uint32 *bytelength) {
    if (data)
        *data = cranked->toVirtualAddress(sample->data.data());
    if (soundFormat)
        *soundFormat = (int32 )sample->soundFormat;
    if (sampleRate)
        *sampleRate = sample->sampleRate;
    if (bytelength)
        *bytelength = (int32)sample->data.size();
}

void cranked::playdate_sound_sample_freeSample(Cranked *cranked, AudioSample sample) {
    sample->dereference();
}

float cranked::playdate_sound_sample_getLength(Cranked *cranked, AudioSample sample) {
    return framesToSeconds((int) sample->data.size());
}

int32 cranked::playdate_sound_sample_decompress(Cranked *cranked, AudioSample sample) {
    return 0; // Todo
}

SamplePlayer cranked::playdate_sound_sampleplayer_newPlayer(Cranked *cranked) {
    return cranked->audio.allocateReferencedSource<SamplePlayer_32>();
}

void cranked::playdate_sound_sampleplayer_freePlayer(Cranked *cranked, SamplePlayer player) {
    player->dereference();
}

void cranked::playdate_sound_sampleplayer_setSample(Cranked *cranked, SamplePlayer player, AudioSample sample) {
    player->sample = sample;
}

int32 cranked::playdate_sound_sampleplayer_play(Cranked *cranked, SamplePlayer player, int32 repeat, float rate) {
    player->playing = true;
    player->repeat = repeat;
    player->rate = rate;
    return true;
}

int32 cranked::playdate_sound_sampleplayer_isPlaying(Cranked *cranked, SamplePlayer player) {
    return player->playing; // Todo: Does pausing affect this?
}

void cranked::playdate_sound_sampleplayer_stop(Cranked *cranked, SamplePlayer player) {
    player->playing = false;
    player->sampleOffset = 0;
}

void cranked::playdate_sound_sampleplayer_setVolume(Cranked *cranked, SamplePlayer player, float left, float right) {
    player->leftVolume = left;
    player->rightVolume = right;
}

void cranked::playdate_sound_sampleplayer_getVolume(Cranked *cranked, SamplePlayer player, float *left, float *right) {
    if (left)
        *left = player->leftVolume;
    if (right)
        *right = player->rightVolume;
}

float cranked::playdate_sound_sampleplayer_getLength(Cranked *cranked, SamplePlayer player) {
    return player->sample ? framesToSeconds((int) player->sample->data.size()) : 0;
}

void cranked::playdate_sound_sampleplayer_setOffset(Cranked *cranked, SamplePlayer player, float offset) {
    player->sampleOffset = framesFromSeconds(offset);
}

void cranked::playdate_sound_sampleplayer_setRate(Cranked *cranked, SamplePlayer player, float rate) {
    player->rate = rate;
}

void cranked::playdate_sound_sampleplayer_setPlayRange(Cranked *cranked, SamplePlayer player, int32 start, int32 end) {
    player->loopStart = start;
    player->loopEnd = end;
}

void cranked::playdate_sound_sampleplayer_setFinishCallback(Cranked *cranked, SamplePlayer player, cref_t callback, void *userdata) {
    player->completionCallback = callback;
    player->completionCallbackUserdata = cranked->toVirtualAddress(userdata);
}

void cranked::playdate_sound_sampleplayer_setLoopCallback(Cranked *cranked, SamplePlayer player, cref_t callback, void *userdata) {
    player->loopCallback = callback;
    player->loopCallbackUserdata = Cranked::version < VERSION_2_4_1 ? 0 : cranked->toVirtualAddress(userdata);
}

float cranked::playdate_sound_sampleplayer_getOffset(Cranked *cranked, SamplePlayer player) {
    return framesToSeconds(player->sampleOffset);
}

float cranked::playdate_sound_sampleplayer_getRate(Cranked *cranked, SamplePlayer player) {
    return player->rate;
}

void cranked::playdate_sound_sampleplayer_setPaused(Cranked *cranked, SamplePlayer player, int32 flag) {
    player->paused = flag;
}

SynthSignal cranked::playdate_sound_signal_newSignal(Cranked *cranked, cref_t step, cref_t noteOn, cref_t noteOff, cref_t dealloc, void *userdata) {
    // Todo
    return nullptr;
}

void cranked::playdate_sound_signal_freeSignal(Cranked *cranked, SynthSignal signal) {
    signal->dereference();
}

float cranked::playdate_sound_signal_getValue(Cranked *cranked, SynthSignal signal) {
    // Todo
    return 0;
}

void cranked::playdate_sound_signal_setValueScale(Cranked *cranked, SynthSignal signal, float scale) {
    signal->scale = scale;
}

void cranked::playdate_sound_signal_setValueOffset(Cranked *cranked, SynthSignal signal, float offset) {
    signal->offset = framesFromSeconds(offset);
}

uint8 *cranked::playdate_sound_getError(Cranked *cranked) {
    return cranked->fromVirtualAddress<uint8>(cranked->audio.lastError);
}

SynthLFO cranked::playdate_sound_lfo_newLFO(Cranked *cranked, int32 type) {
    return cranked->nativeEngine.createReferencedResource<SynthLFO>((LFOType)type);
}

void cranked::playdate_sound_lfo_freeLFO(Cranked *cranked, SynthLFO lfo) {
    lfo->dereference();
}

void cranked::playdate_sound_lfo_setType(Cranked *cranked, SynthLFO lfo, int32 type) {
    lfo->type = (LFOType)type;
}

void cranked::playdate_sound_lfo_setRate(Cranked *cranked, SynthLFO lfo, float rate) {
    lfo->rate = rate;
}

void cranked::playdate_sound_lfo_setPhase(Cranked *cranked, SynthLFO lfo, float phase) {
    lfo->phase = phase;
}

void cranked::playdate_sound_lfo_setCenter(Cranked *cranked, SynthLFO lfo, float center) {
    lfo->center = center;
}

void cranked::playdate_sound_lfo_setDepth(Cranked *cranked, SynthLFO lfo, float depth) {
    lfo->depth = depth;
}

void cranked::playdate_sound_lfo_setArpeggiation(Cranked *cranked, SynthLFO lfo, int32 nSteps, float *steps) {
    lfo->type = LFOType::Arpeggiator;
    lfo->arpeggiationSteps.clear();
    for (int i = 0; i < nSteps; i++)
        lfo->arpeggiationSteps.push_back(steps[i]);
}

void cranked::playdate_sound_lfo_setFunction(Cranked *cranked, SynthLFO lfo, cref_t lfoFunc, void *userdata, int32 interpolate) {
    lfo->func = lfoFunc;
    lfo->functionUserdata = cranked->toVirtualAddress(userdata);
    lfo->functionInterpolate = interpolate;
    lfo->type = LFOType::Function;
}

void cranked::playdate_sound_lfo_setDelay(Cranked *cranked, SynthLFO lfo, float holdoff, float ramptime) {
    lfo->holdOffSamples = framesFromSeconds(holdoff);
    lfo->rampTimeSamples = framesFromSeconds(ramptime);
}

void cranked::playdate_sound_lfo_setRetrigger(Cranked *cranked, SynthLFO lfo, int32 flag) {
    lfo->reTrigger = flag;
}

float cranked::playdate_sound_lfo_getValue(Cranked *cranked, SynthLFO lfo) {
    // Todo
    return 0;
}

void cranked::playdate_sound_lfo_setGlobal(Cranked *cranked, SynthLFO lfo, int32 global) {
    lfo->global = global;
}

void cranked::playdate_sound_lfo_setStartPhase(Cranked *cranked, SynthLFO  lfo, float phase) {
    lfo->phase = phase;
}

SynthEnvelope cranked::playdate_sound_envelope_newEnvelope(Cranked *cranked, float attack, float decay, float sustain, float release) {
    return cranked->nativeEngine.createReferencedResource<SynthEnvelope>(attack, decay, sustain, release);
}

void cranked::playdate_sound_envelope_freeEnvelope(Cranked *cranked, SynthEnvelope env) {
    env->dereference();
}

void cranked::playdate_sound_envelope_setAttack(Cranked *cranked, SynthEnvelope env, float attack) {
    env->attack = attack;
}

void cranked::playdate_sound_envelope_setDecay(Cranked *cranked, SynthEnvelope env, float decay) {
    env->decay = decay;
}

void cranked::playdate_sound_envelope_setSustain(Cranked *cranked, SynthEnvelope env, float sustain) {
    env->sustain = sustain;
}

void cranked::playdate_sound_envelope_setRelease(Cranked *cranked, SynthEnvelope env, float release) {
    env->release = release;
}

void cranked::playdate_sound_envelope_setLegato(Cranked *cranked, SynthEnvelope env, int32 flag) {
    env->legato = flag;
}

void cranked::playdate_sound_envelope_setRetrigger(Cranked *cranked, SynthEnvelope env, int32 flag) {
    env->reTrigger = flag;
}

float cranked::playdate_sound_envelope_getValue(Cranked *cranked, SynthEnvelope env) {
    // Todo
    return 0;
}

void cranked::playdate_sound_envelope_setCurvature(Cranked *cranked, SynthEnvelope env, float amount) {
    env->curvature = amount;
}

void cranked::playdate_sound_envelope_setVelocitySensitivity(Cranked *cranked, SynthEnvelope env, float velsens) {
    env->velocitySensitivity = velsens;
}

void cranked::playdate_sound_envelope_setRateScaling(Cranked *cranked, SynthEnvelope env, float scaling, float start, float end) {
    env->rateScaling = scaling;
    env->rateStart = start;
    env->rateEnd = end;
}

Synth cranked::playdate_sound_synth_newSynth(Cranked *cranked) {
    return cranked->audio.allocateReferencedSource<PDSynth_32>();
}

void cranked::playdate_sound_synth_freeSynth(Cranked *cranked, Synth synth) {
    synth->dereference();
}

void cranked::playdate_sound_synth_setWaveform(Cranked *cranked, Synth synth, int32 wave) {
    synth->waveform = (SoundWaveform)wave;
}

void cranked::playdate_sound_synth_setGenerator(Cranked *cranked, Synth synth, int32 stereo, cref_t render, cref_t noteOn, cref_t release, cref_t setparam, cref_t dealloc, cref_t copyUserdata, void *userdata) {
    playdate_sound_synth_setGenerator_deprecated(cranked, synth, stereo, render, noteOn, release, setparam, dealloc, userdata);
    synth->generatorCopyUserdataFunc = copyUserdata;
}

Synth cranked::playdate_sound_synth_copy(Cranked *cranked, Synth synth) {
    return nullptr; // Todo
}

void cranked::playdate_sound_synth_setGenerator_deprecated(Cranked *cranked, Synth synth, int32 stereo, cref_t render, cref_t noteOn, cref_t release, cref_t setparam, cref_t dealloc, void *userdata) {
    synth->generatorStereo = stereo;
    synth->generatorRenderFunc = render;
    synth->generatorNoteOnFunc = noteOn;
    synth->generatorReleaseFunc = release;
    synth->generatorSetParameterFunc = setparam;
    synth->generatorDeallocFunc = dealloc;
    synth->generatorUserdata = cranked->toVirtualAddress(userdata);
    synth->generatorCopyUserdataFunc = 0;
}

void cranked::playdate_sound_synth_setSample(Cranked *cranked, Synth synth, AudioSample sample, uint32 sustainStart, uint32 sustainEnd) {
    synth->sample = sample;
    synth->sampleSustainStart = sustainStart;
    synth->sampleSustainEnd = sustainEnd;
}

void cranked::playdate_sound_synth_setAttackTime(Cranked *cranked, Synth synth, float attack) {
    synth->envelope->attack = attack;
}

void cranked::playdate_sound_synth_setDecayTime(Cranked *cranked, Synth synth, float decay) {
    synth->envelope->decay = decay;
}

void cranked::playdate_sound_synth_setSustainLevel(Cranked *cranked, Synth synth, float sustain) {
    synth->envelope->sustain = sustain;
}

void cranked::playdate_sound_synth_setReleaseTime(Cranked *cranked, Synth synth, float release) {
    synth->envelope->release = release;
}

void cranked::playdate_sound_synth_setTranspose(Cranked *cranked, Synth synth, float halfSteps) {
    synth->transposeHalfSteps = halfSteps;
}

void cranked::playdate_sound_synth_setFrequencyModulator(Cranked *cranked, Synth synth, SynthSignalValue mod) {
    synth->frequencyModulator = mod;
}

SynthSignalValue cranked::playdate_sound_synth_getFrequencyModulator(Cranked *cranked, Synth synth) {
    return synth->frequencyModulator.get();
}

void cranked::playdate_sound_synth_setAmplitudeModulator(Cranked *cranked, Synth synth, SynthSignalValue mod) {
    synth->amplitudeModulator = mod;
}

SynthSignalValue cranked::playdate_sound_synth_getAmplitudeModulator(Cranked *cranked, Synth synth) {
    return synth->amplitudeModulator.get();
}

int32 cranked::playdate_sound_synth_getParameterCount(Cranked *cranked, Synth synth) {
    // Todo
    return 0;
}

int32 cranked::playdate_sound_synth_setParameter(Cranked *cranked, Synth synth, int32 parameter, float value) {
    // Todo
    return 0;
}

void cranked::playdate_sound_synth_setParameterModulator(Cranked *cranked, Synth synth, int32 parameter, SynthSignalValue mod) {
    synth->parameterModulators[parameter] = mod;
}

SynthSignalValue cranked::playdate_sound_synth_getParameterModulator(Cranked *cranked, Synth synth, int32 parameter) {
    return synth->parameterModulators[parameter].get();
}

void cranked::playdate_sound_synth_playNote(Cranked *cranked, Synth synth, float freq, float vel, float len, uint32 when) {
    // Todo
}

void cranked::playdate_sound_synth_playMIDINote(Cranked *cranked, Synth synth, float note, float vel, float len, uint32 when) {
    // Todo
}

void cranked::playdate_sound_synth_noteOff(Cranked *cranked, Synth synth, uint32 when) {
    // Todo
}

void cranked::playdate_sound_synth_stop(Cranked *cranked, Synth synth) {
    // Todo
}

void cranked::playdate_sound_synth_setVolume(Cranked *cranked, Synth synth, float left, float right) {
    synth->leftVolume = left;
    synth->rightVolume = right;
}

void cranked::playdate_sound_synth_getVolume(Cranked *cranked, Synth synth, float *left, float *right) {
    if (left)
        *left = synth->leftVolume;
    if (right)
        *right = synth->rightVolume;
}

int32 cranked::playdate_sound_synth_isPlaying(Cranked *cranked, Synth synth) {
    return synth->playing;
}

SynthEnvelope cranked::playdate_sound_synth_getEnvelope(Cranked *cranked, Synth synth) {
    return synth->envelope.get();
}

int32 cranked::playdate_sound_synth_setWavetable(Cranked *cranked, Synth synth, AudioSample sample, int32 log2size, int32 columns, int32 rows) {
    return 0; // Todo
}

ControlSignal cranked::playdate_control_signal_newSignal(Cranked *cranked) {
    return cranked->nativeEngine.createReferencedResource<ControlSignal>();
}

void cranked::playdate_control_signal_freeSignal(Cranked *cranked, ControlSignal signal) {
    signal->dereference();
}

void cranked::playdate_control_signal_clearEvents(Cranked *cranked, ControlSignal control) {
    control->events.clear();
}

void cranked::playdate_control_signal_addEvent(Cranked *cranked, ControlSignal control, int32 step, float value, int32 interpolate) {
    control->events.insert(pair(step, ControlSignal_32::Event{value, (bool)interpolate}));
}

void cranked::playdate_control_signal_removeEvent(Cranked *cranked, ControlSignal control, int32 step) {
    control->events.erase(step);
}

int32 cranked::playdate_control_signal_getMIDIControllerNumber(Cranked *cranked, ControlSignal control) {
    return control->controllerNumber;
}

SynthInstrument cranked::playdate_sound_instrument_newInstrument(Cranked *cranked) {
    return cranked->audio.allocateReferencedSource<PDSynthInstrument_32>();
}

void cranked::playdate_sound_instrument_freeInstrument(Cranked *cranked, SynthInstrument inst) {
    inst->dereference();
}

int32 cranked::playdate_sound_instrument_addVoice(Cranked *cranked, SynthInstrument inst, Synth synth, float rangeStart, float rangeEnd, float transpose) {
    // Todo: Check if already in an instrument or channel
    eraseByEquivalentKey(inst->voices, synth);
    synth->instrumentStartFrequency = rangeStart;
    synth->instrumentEndFrequency = rangeEnd;
    synth->instrumentTranspose = transpose;
    return true;
}

Synth cranked::playdate_sound_instrument_playNote(Cranked *cranked, SynthInstrument inst, float frequency, float vel, float len, uint32 when) {
    // Todo
    return nullptr;
}

Synth cranked::playdate_sound_instrument_playMIDINote(Cranked *cranked, SynthInstrument inst, float note, float vel, float len, uint32 when) {
    // Todo
    return nullptr;
}

void cranked::playdate_sound_instrument_setPitchBend(Cranked *cranked, SynthInstrument inst, float bend) {
    inst->pitchBend = bend;
}

void cranked::playdate_sound_instrument_setPitchBendRange(Cranked *cranked, SynthInstrument inst, float halfSteps) {
    inst->pitchBendRangeHalfSteps = halfSteps;
}

void cranked::playdate_sound_instrument_setTranspose(Cranked *cranked, SynthInstrument inst, float halfSteps) {
    inst->transposeHalfSteps = halfSteps;
}

void cranked::playdate_sound_instrument_noteOff(Cranked *cranked, SynthInstrument inst, float note, uint32 when) {
    // Todo
}

void cranked::playdate_sound_instrument_allNotesOff(Cranked *cranked, SynthInstrument inst, uint32 when) {
    // Todo
}

void cranked::playdate_sound_instrument_setVolume(Cranked *cranked, SynthInstrument inst, float left, float right) {
    inst->leftVolume = left;
    inst->rightVolume = right;
}

void cranked::playdate_sound_instrument_getVolume(Cranked *cranked, SynthInstrument inst, float *left, float *right) {
    if (left)
        *left = inst->leftVolume;
    if (right)
        *right = inst->rightVolume;
}

int32 cranked::playdate_sound_instrument_activeVoiceCount(Cranked *cranked, SynthInstrument inst) {
    // Todo
    return 0;
}

SequenceTrack cranked::playdate_sound_track_newTrack(Cranked *cranked) {
    return cranked->audio.allocateReferencedSource<SequenceTrack_32>();
}

void cranked::playdate_sound_track_freeTrack(Cranked *cranked, SequenceTrack track) {
    track->dereference();
}

void cranked::playdate_sound_track_setInstrument(Cranked *cranked, SequenceTrack track, SynthInstrument inst) {
    track->instrument = inst;
}

SynthInstrument cranked::playdate_sound_track_getInstrument(Cranked *cranked, SequenceTrack track) {
    return track->instrument.get();
}

void cranked::playdate_sound_track_addNoteEvent(Cranked *cranked, SequenceTrack track, uint32 step, uint32 len, float note, float velocity) {
    // Todo
}

void cranked::playdate_sound_track_removeNoteEvent(Cranked *cranked, SequenceTrack track, uint32 step, float note) {
    // Todo
}

void cranked::playdate_sound_track_clearNotes(Cranked *cranked, SequenceTrack track) {
    // Todo
}

int32 cranked::playdate_sound_track_getControlSignalCount(Cranked *cranked, SequenceTrack track) {
    // Todo
    return 0;
}

ControlSignal cranked::playdate_sound_track_getControlSignal(Cranked *cranked, SequenceTrack track, int32 idx) {
    // Todo
    return nullptr;
}

void cranked::playdate_sound_track_clearControlEvents(Cranked *cranked, SequenceTrack track) {
    // Todo
}

int32 cranked::playdate_sound_track_getPolyphony(Cranked *cranked, SequenceTrack track) {
    // Todo
    return 0;
}

int32 cranked::playdate_sound_track_activeVoiceCount(Cranked *cranked, SequenceTrack track) {
    // Todo
    return 0;
}

void cranked::playdate_sound_track_setMuted(Cranked *cranked, SequenceTrack track, int32 mute) {
    track->muted = mute;
}

uint32 cranked::playdate_sound_track_getLength(Cranked *cranked, SequenceTrack track) {
    // Todo
    return 0;
}

int32 cranked::playdate_sound_track_getIndexForStep(Cranked *cranked, SequenceTrack track, uint32 step) {
    // Todo
    return 0;
}

int32 cranked::playdate_sound_track_getNoteAtIndex(Cranked *cranked, SequenceTrack track, int32 index, uint32 *outStep, uint32 *outLen, float *outNote, float *outVelocity) {
    // Todo
    return 0;
}

ControlSignal cranked::playdate_sound_track_getSignalForController(Cranked *cranked, SequenceTrack track, int32 controller, int32 create) {
    // Todo
    return nullptr;
}

SoundSequence cranked::playdate_sound_sequence_newSequence(Cranked *cranked) {
    return cranked->audio.allocateReferencedSource<SoundSequence_32>();
}

void cranked::playdate_sound_sequence_freeSequence(Cranked *cranked, SoundSequence sequence) {
    sequence->dereference();
}
int32 cranked::playdate_sound_sequence_loadMIDIFile(Cranked *cranked, SoundSequence seq, uint8 *path) {
    // Todo
    return 0;
}

uint32 cranked::playdate_sound_sequence_getTime(Cranked *cranked, SoundSequence seq) {
    return seq->time;
}

void cranked::playdate_sound_sequence_setTime(Cranked *cranked, SoundSequence seq, uint32 time) {
    seq->time = (int)time;
}

void cranked::playdate_sound_sequence_setLoops(Cranked *cranked, SoundSequence seq, int32 loopstart, int32 loopend, int32 loops) {
    seq->currentLoop = 0;
    seq->loopStart = loopstart;
    seq->loopEnd = loopend;
    seq->loops = loops;
}

float cranked::playdate_sound_sequence_getTempo(Cranked *cranked, SoundSequence seq) {
    return seq->tempo;
}

int32 cranked::playdate_sound_sequence_getTempo_deprecated(Cranked *cranked, SoundSequence seq) {
    return (int32) seq->tempo;
}

void cranked::playdate_sound_sequence_setTempo(Cranked *cranked, SoundSequence seq, float stepsPerSecond) {
    seq->tempo = stepsPerSecond;
}

void cranked::playdate_sound_sequence_setTempo_int(Cranked *cranked, SoundSequence seq, int32 stepsPerSecond) {
    seq->tempo = (float)stepsPerSecond;
}

int32 cranked::playdate_sound_sequence_getTrackCount(Cranked *cranked, SoundSequence seq) {
    // Todo
    return 0;
}

SequenceTrack cranked::playdate_sound_sequence_addTrack(Cranked *cranked, SoundSequence seq) {
    // Todo
    return nullptr;
}

SequenceTrack cranked::playdate_sound_sequence_getTrackAtIndex(Cranked *cranked, SoundSequence seq, uint32 track) {
    // Todo
    return nullptr;
}

void cranked::playdate_sound_sequence_setTrackAtIndex(Cranked *cranked, SoundSequence seq, SequenceTrack track, uint32 idx) {
    // Todo
}

void cranked::playdate_sound_sequence_allNotesOff(Cranked *cranked, SoundSequence seq) {
    // Todo
}

int32 cranked::playdate_sound_sequence_isPlaying(Cranked *cranked, SoundSequence seq) {
    return seq->playing;
}

uint32 cranked::playdate_sound_sequence_getLength(Cranked *cranked, SoundSequence seq) {
    // Todo
    return 0;
}

void cranked::playdate_sound_sequence_play(Cranked *cranked, SoundSequence seq, cref_t finishCallback, void *userdata) {
    // Todo
}

void cranked::playdate_sound_sequence_stop(Cranked *cranked, SoundSequence seq) {
    // Todo
}

int32 cranked::playdate_sound_sequence_getCurrentStep(Cranked *cranked, SoundSequence seq, int32 *timeOffset) {
    // Todo
    return 0;
}

void cranked::playdate_sound_sequence_setCurrentStep(Cranked *cranked, SoundSequence seq, int32 step, int32 timeOffset, int32 playNotes) {
    // Todo
}

TwoPoleFilter cranked::playdate_sound_effect_twopolefilter_newFilter(Cranked *cranked) {
    return cranked->nativeEngine.createReferencedResource<TwoPoleFilter>();
}

void cranked::playdate_sound_effect_twopolefilter_freeFilter(Cranked *cranked, TwoPoleFilter filter) {
    filter->dereference();
}

void cranked::playdate_sound_effect_twopolefilter_setType(Cranked *cranked, TwoPoleFilter filter, int32 type) {
    filter->type = (TwoPoleFilterType) type;
}

void cranked::playdate_sound_effect_twopolefilter_setFrequency(Cranked *cranked, TwoPoleFilter filter, float frequency) {
    filter->frequency = frequency;
}

void cranked::playdate_sound_effect_twopolefilter_setFrequencyModulator(Cranked *cranked, TwoPoleFilter filter, SynthSignalValue signal) {
    filter->frequencyModulator = signal;
}

SynthSignalValue cranked::playdate_sound_effect_twopolefilter_getFrequencyModulator(Cranked *cranked, TwoPoleFilter filter) {
    return filter->frequencyModulator.get();
}

void cranked::playdate_sound_effect_twopolefilter_setGain(Cranked *cranked, TwoPoleFilter filter, float gain) {
    filter->gain = gain;
}

void cranked::playdate_sound_effect_twopolefilter_setResonance(Cranked *cranked, TwoPoleFilter filter, float resonance) {
    filter->resonance = resonance;
}

void cranked::playdate_sound_effect_twopolefilter_setResonanceModulator(Cranked *cranked, TwoPoleFilter filter, SynthSignalValue signal) {
    filter->resonanceModulator = signal;
}

SynthSignalValue cranked::playdate_sound_effect_twopolefilter_getResonanceModulator(Cranked *cranked, TwoPoleFilter filter) {
    return filter->resonanceModulator.get();
}

OnePoleFilter cranked::playdate_sound_effect_onepolefilter_newFilter(Cranked *cranked) {
    return cranked->nativeEngine.createReferencedResource<OnePoleFilter>();
}

void cranked::playdate_sound_effect_onepolefilter_freeFilter(Cranked *cranked, OnePoleFilter filter) {
    filter->dereference();
}

void cranked::playdate_sound_effect_onepolefilter_setParameter(Cranked *cranked, OnePoleFilter filter, float parameter) {
    filter->cutoffFrequency = parameter;
}

void cranked::playdate_sound_effect_onepolefilter_setParameterModulator(Cranked *cranked, OnePoleFilter filter, SynthSignalValue signal) {
    filter->cutoffFrequencyModulator = signal;
}

SynthSignalValue cranked::playdate_sound_effect_onepolefilter_getParameterModulator(Cranked *cranked, OnePoleFilter filter) {
    return filter->cutoffFrequencyModulator.get();
}

BitCrusher cranked::playdate_sound_effect_bitcrusher_newBitCrusher(Cranked *cranked) {
    return cranked->nativeEngine.createReferencedResource<BitCrusher>();
}

void cranked::playdate_sound_effect_bitcrusher_freeBitCrusher(Cranked *cranked, BitCrusher filter) {
    filter->dereference();
}

void cranked::playdate_sound_effect_bitcrusher_setAmount(Cranked *cranked, BitCrusher filter, float amount) {
    filter->amount = amount;
}

void cranked::playdate_sound_effect_bitcrusher_setAmountModulator(Cranked *cranked, BitCrusher filter, SynthSignalValue signal) {
    filter->amountModulator = signal;
}

SynthSignalValue cranked::playdate_sound_effect_bitcrusher_getAmountModulator(Cranked *cranked, BitCrusher filter) {
    return filter->amountModulator.get();
}

void cranked::playdate_sound_effect_bitcrusher_setUndersampling(Cranked *cranked, BitCrusher filter, float undersampling) {
    filter->undersampling = undersampling;
}

void cranked::playdate_sound_effect_bitcrusher_setUndersampleModulator(Cranked *cranked, BitCrusher filter, SynthSignalValue signal) {
    filter->undersamplingModulator = signal;
}

SynthSignalValue cranked::playdate_sound_effect_bitcrusher_getUndersampleModulator(Cranked *cranked, BitCrusher filter) {
    return filter->undersamplingModulator.get();
}

RingModulator cranked::playdate_sound_effect_ringmodulator_newRingmod(Cranked *cranked) {
    return cranked->nativeEngine.createReferencedResource<RingModulator>();
}

void cranked::playdate_sound_effect_ringmodulator_freeRingmod(Cranked *cranked, RingModulator filter) {
    filter->dereference();
}

void cranked::playdate_sound_effect_ringmodulator_setFrequency(Cranked *cranked, RingModulator filter, float frequency) {
    filter->frequency = frequency;
}

void cranked::playdate_sound_effect_ringmodulator_setFrequencyModulator(Cranked *cranked, RingModulator filter, SynthSignalValue signal) {
    filter->frequencyModulator = signal;
}

SynthSignalValue cranked::playdate_sound_effect_ringmodulator_getFrequencyModulator(Cranked *cranked, RingModulator filter) {
    return filter->frequencyModulator.get();
}

DelayLine cranked::playdate_sound_effect_delayline_newDelayLine(Cranked *cranked, int32 length, int32 stereo) {
    return cranked->nativeEngine.createReferencedResource<DelayLine>((int) length, (bool) stereo);
}

void cranked::playdate_sound_effect_delayline_freeDelayLine(Cranked *cranked, DelayLine filter) {
    filter->dereference();
}

void cranked::playdate_sound_effect_delayline_setLength(Cranked *cranked, DelayLine d, int32 frames) {
    d->data.clear();
    d->data.resize((d->stereo ? 2 : 1) * frames);
}

void cranked::playdate_sound_effect_delayline_setFeedback(Cranked *cranked, DelayLine d, float fb) {
    d->feedback = fb;
}

DelayLineTap cranked::playdate_sound_effect_delayline_addTap(Cranked *cranked, DelayLine d, int32 delay) {
    return cranked->audio.allocateReferencedSource<DelayLineTap_32>(d, (int) delay);
}

void cranked::playdate_sound_effect_delayline_freeTap(Cranked *cranked, DelayLineTap tap) {
    tap->dereference();
}

void cranked::playdate_sound_effect_delayline_setTapDelay(Cranked *cranked, DelayLineTap t, int32 frames) {
    t->delayFrames = frames;
}

void cranked::playdate_sound_effect_delayline_setTapDelayModulator(Cranked *cranked, DelayLineTap t, SynthSignalValue mod) {
    t->delayModulator = mod;
}

SynthSignalValue cranked::playdate_sound_effect_delayline_getTapDelayModulator(Cranked *cranked, DelayLineTap t) {
    return t->delayModulator.get();
}

void cranked::playdate_sound_effect_delayline_setTapChannelsFlipped(Cranked *cranked, DelayLineTap t, int32 flip) {
    t->channelsFlipped = flip;
}

Overdrive cranked::playdate_sound_effect_overdrive_newOverdrive(Cranked *cranked) {
    return cranked->nativeEngine.createReferencedResource<Overdrive>();
}

void cranked::playdate_sound_effect_overdrive_freeOverdrive(Cranked *cranked, Overdrive filter) {
    filter->dereference();
}

void cranked::playdate_sound_effect_overdrive_setGain(Cranked *cranked, Overdrive o, float gain) {
    o->gain = gain;
}

void cranked::playdate_sound_effect_overdrive_setLimit(Cranked *cranked, Overdrive o, float limit) {
    o->limit = limit;
}

void cranked::playdate_sound_effect_overdrive_setLimitModulator(Cranked *cranked, Overdrive o, SynthSignalValue mod) {
    o->limitModulator = mod;
}

SynthSignalValue cranked::playdate_sound_effect_overdrive_getLimitModulator(Cranked *cranked, Overdrive o) {
    return o->limitModulator.get();
}

void cranked::playdate_sound_effect_overdrive_setOffset(Cranked *cranked, Overdrive o, float offset) {
    o->offset = offset;
}

void cranked::playdate_sound_effect_overdrive_setOffsetModulator(Cranked *cranked, Overdrive o, SynthSignalValue mod) {
    o->offsetModulator = mod;
}

SynthSignalValue cranked::playdate_sound_effect_overdrive_getOffsetModulator(Cranked *cranked, Overdrive o) {
    return o->offsetModulator.get();
}

SoundEffect cranked::playdate_sound_effect_newEffect(Cranked *cranked, cref_t proc, void *userdata) {
    // Todo
    return nullptr;
}

void cranked::playdate_sound_effect_freeEffect(Cranked *cranked, SoundEffect effect) {
    effect->dereference();
}

void cranked::playdate_sound_effect_setMix(Cranked *cranked, SoundEffect effect, float level) {
    effect->mixLevel = level;
}

void cranked::playdate_sound_effect_setMixModulator(Cranked *cranked, SoundEffect effect, SynthSignalValue signal) {
    effect->mixModulator = signal;
}

SynthSignalValue cranked::playdate_sound_effect_getMixModulator(Cranked *cranked, SoundEffect effect) {
    return effect->mixModulator.get();
}

void cranked::playdate_sound_effect_setUserdata(Cranked *cranked, SoundEffect effect, void *userdata) {
    effect->userdata = cranked->toVirtualAddress(userdata);
}

void *cranked::playdate_sound_effect_getUserdata(Cranked *cranked, SoundEffect effect) {
    return cranked->fromVirtualAddress(effect->userdata);
}

SoundChannel cranked::playdate_sound_channel_newChannel(Cranked *cranked) {
    auto channel = cranked->audio.allocateChannel();
    channel->reference();
    return channel;
}

void cranked::playdate_sound_channel_freeChannel(Cranked *cranked, SoundChannel channel) {
    channel->dereference();
}

int32 cranked::playdate_sound_channel_addSource(Cranked *cranked, SoundChannel channel, SoundSource source) {
    if (find(channel->sources.begin(), channel->sources.end(), source) != channel->sources.end())
        return false;
    channel->sources.emplace(source);
    return true;
}

int32 cranked::playdate_sound_channel_removeSource(Cranked *cranked, SoundChannel channel, SoundSource source) {
    return eraseByEquivalentKey(channel->sources, source);
}

SoundSource cranked::playdate_sound_channel_addCallbackSource(Cranked *cranked, SoundChannel channel, cref_t callback, void *context, int32 stereo) {
    // Todo
    return nullptr;
}

void cranked::playdate_sound_channel_addEffect(Cranked *cranked, SoundChannel channel, SoundEffect effect) {
    channel->effects.emplace(effect);
}

void cranked::playdate_sound_channel_removeEffect(Cranked *cranked, SoundChannel channel, SoundEffect effect) {
    eraseByEquivalentKey(channel->effects, effect);
}

void cranked::playdate_sound_channel_setVolume(Cranked *cranked, SoundChannel channel, float volume) {
    channel->volume = volume;
}

float cranked::playdate_sound_channel_getVolume(Cranked *cranked, SoundChannel channel) {
    return channel->volume;
}

void cranked::playdate_sound_channel_setVolumeModulator(Cranked *cranked, SoundChannel channel, SynthSignalValue mod) {
    channel->volumeModulator = mod;
}

SynthSignalValue cranked::playdate_sound_channel_getVolumeModulator(Cranked *cranked, SoundChannel channel) {
    return channel->volumeModulator.get();
}

void cranked::playdate_sound_channel_setPan(Cranked *cranked, SoundChannel channel, float pan) {
    channel->pan = pan;
}

void cranked::playdate_sound_channel_setPanModulator(Cranked *cranked, SoundChannel channel, SynthSignalValue mod) {
    channel->panModulator = mod;
}

SynthSignalValue cranked::playdate_sound_channel_getPanModulator(Cranked *cranked, SoundChannel channel) {
    return channel->panModulator.get();
}

SynthSignalValue cranked::playdate_sound_channel_getDryLevelSignal(Cranked *cranked, SoundChannel channel) {
    // Todo
    return nullptr;
}

SynthSignalValue cranked::playdate_sound_channel_getWetLevelSignal(Cranked *cranked, SoundChannel channel) {
    // Todo
    return nullptr;
}

uint32 cranked::playdate_sound_getCurrentTime(Cranked *cranked) {
    return cranked->audio.sampleTime;
}

SoundSource cranked::playdate_sound_addSource(Cranked *cranked, cref_t callback, void *context, int32 stereo) {
    // Todo
    return nullptr;
}

SoundChannel cranked::playdate_sound_getDefaultChannel(Cranked *cranked) {
    return cranked->audio.mainChannel.get();
}

int32 cranked::playdate_sound_addChannel(Cranked *cranked, SoundChannel channel) {
    if (channel == cranked->audio.mainChannel)
        return false;
    auto &activeChannels = cranked->audio.activeChannels;
    bool success = !activeChannels.contains(channel);
    activeChannels.emplace(channel);
    return success;
}

int32 cranked::playdate_sound_removeChannel(Cranked *cranked, SoundChannel channel) {
    auto &activeChannels = cranked->audio.activeChannels;
    bool success = activeChannels.contains(channel);
    eraseByEquivalentKey(activeChannels, channel);
    return success;
}

int32 cranked::playdate_sound_setMicCallback(Cranked *cranked, cref_t callback, void *context, int32 forceInternal) {
    cranked->audio.micCallback = callback;
    cranked->audio.micCallbackUserdata = cranked->toVirtualAddress(context);
    cranked->audio.micCallbackSource = (MicSource)forceInternal;
    return (int)MicSource::Autodetect; // Todo
}

void cranked::playdate_sound_getHeadphoneState(Cranked *cranked, int32 *headphone, int32 *headsetmic, cref_t changeCallback) {
    *headphone = cranked->audio.headphonesConnected;
    *headsetmic = cranked->audio.headsetMicConnected;
    cranked->audio.headsetStateCallback = changeCallback;
}

void cranked::playdate_sound_setOutputsActive(Cranked *cranked, int32 headphone, int32 speaker) {
    cranked->audio.headphoneOutputActive = headphone;
    cranked->audio.speakerOutputActive = speaker;
}

int32 cranked::playdate_sound_removeSource(Cranked *cranked, SoundSource source) {
    if (eraseByEquivalentKey(cranked->audio.mainChannel->sources, source))
        return true;
    for (auto &channel : cranked->audio.activeChannels)
        if (eraseByEquivalentKey(channel->sources, source))
            return true;
    return false;
}

int32 cranked::playdate_display_getWidth(Cranked *cranked) {
    return DISPLAY_WIDTH / cranked->graphics.displayScale;
}

int32 cranked::playdate_display_getHeight(Cranked *cranked) {
    return DISPLAY_HEIGHT / cranked->graphics.displayScale;
}

void cranked::playdate_display_setRefreshRate(Cranked *cranked, float rate) {
    cranked->graphics.framerate = rate;
}

void cranked::playdate_display_setInverted(Cranked *cranked, int32 flag) {
    cranked->graphics.displayInverted = flag;
}

void cranked::playdate_display_setScale(Cranked *cranked, uint32 s) {
    if (s == 1 or s == 2 or s == 4 or s == 8)
        cranked->graphics.displayScale = (int32)s;
    else
        cranked->graphics.displayScale = 1;
}

void cranked::playdate_display_setMosaic(Cranked *cranked, uint32 x, uint32 y) {
    cranked->graphics.displayMosaic = {(int)x & 0x3, (int)y & 3};
}

void cranked::playdate_display_setFlipped(Cranked *cranked, int32 x, int32 y) {
    cranked->graphics.displayFlippedX = x;
    cranked->graphics.displayFlippedY = y;
}

void cranked::playdate_display_setOffset(Cranked *cranked, int32 x, int32 y) {
    cranked->graphics.displayOffset = {x, y};
}

int32 cranked::playdate_scoreboards_addScore(Cranked *cranked, uint8 *boardId, uint32 value, cref_t callback) {
    // Todo
    return 0;
}

int32 cranked::playdate_scoreboards_getPersonalBest(Cranked *cranked, uint8 *boardId, cref_t callback) {
    // Todo
    return 0;
}

void cranked::playdate_scoreboards_freeScore(Cranked *cranked, PDScore_32 *score) {
    // Todo
}

int32 cranked::playdate_scoreboards_getScoreboards(Cranked *cranked, cref_t callback) {
    // Todo
    return 0;
}

void cranked::playdate_scoreboards_freeBoardsList(Cranked *cranked, PDBoardsList_32 *boardsList) {
    // Todo
}

int32 cranked::playdate_scoreboards_getScores(Cranked *cranked, uint8 *boardId, cref_t callback) {
    // Todo
    return 0;
}

void cranked::playdate_scoreboards_freeScoresList(Cranked *cranked, PDScoresList_32 *scoresList) {
    // Todo
}

// NOLINTEND(*-non-const-parameter)
