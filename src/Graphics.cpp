#include "Graphics.hpp"
#include "Emulator.hpp"
#include "gen/SystemFont.hpp"

Graphics::Graphics(class Emulator *emulator) : emulator(emulator), heap(emulator->heap), systemFontSource(Rom::readFont(SYSTEM_FONT, sizeof(SYSTEM_FONT))) {}

inline LCDBitmap_32::LCDBitmap_32(int width, int height, class Graphics *graphics)
        : width(width), height(height), graphics(graphics), data(vheap_vector<uint8_t>(width * height, graphics->heap.allocator<uint8_t>())), mask(nullptr) {}

LCDBitmap_32::LCDBitmap_32(const LCDBitmap_32 &other)
        : width(other.width), height(other.height), data(other.data), graphics(other.graphics), mask(other.mask ? graphics->heap.construct<LCDBitmap_32>(*other.mask) : nullptr) {}

LCDBitmap_32::LCDBitmap_32(LCDBitmap_32 &&other) noexcept : width(other.width), height(other.height), data(std::move(other.data)), mask(other.mask), graphics(other.graphics) {
    if (other.mask)
        other.mask = nullptr;
}

LCDBitmap_32::~LCDBitmap_32() {
    if (mask)
        graphics->heap.destruct(mask);
}

LCDBitmap_32& LCDBitmap_32::operator=(const LCDBitmap_32 &other) {
    if (&other == this)
        return *this;
    if (mask)
        graphics->heap.destruct(mask);
    data = other.data;
    mask = other.mask ? graphics->heap.construct<LCDBitmap_32>(*other.mask) : nullptr;
    return *this;
}

LCDFontPage_32::LCDFontPage_32(Graphics *graphics) : glyphs(vheap_map<int, LCDFontGlyph_32>(graphics->heap.pairAllocator<int, LCDFontGlyph_32>())) {}

LCDFont_32::LCDFont_32(Graphics *graphics, int tracking, int glyphWidth, int glyphHeight)
        : tracking(tracking), glyphWidth(glyphWidth), glyphHeight(glyphHeight), pages(vheap_map<int, LCDFontPage_32>(graphics->heap.pairAllocator<int, LCDFontPage_32>())) {}

LCDBitmapTable_32::LCDBitmapTable_32(Graphics *graphics, int cellsPerRow) : cellsPerRow(cellsPerRow), graphics(graphics), bitmaps(graphics->heap.allocator<LCDBitmap_32>()) {}

void LCDBitmap_32::drawPixel(int x, int y, LCDColor color) {
    if (x < 0 or x >= width or y < 0 or y >= height)
        return;
    // Todo: Is pattern offset affected by draw offset?
    auto c = color.color;
    if (color.pattern >= 4) {
        auto row = y % 8;
        auto column = x % 8;
        auto word = graphics->emulator->virtualRead<uint8_t>(color.pattern + row);
        auto maskWord = graphics->emulator->virtualRead<uint8_t>(color.pattern + row + 8);
        if (maskWord & (0x80 >> column))
            c = word & (0x80 >> column) ? LCDSolidColor::White : LCDSolidColor::Black;
        else
            c = LCDSolidColor::Clear;
    }

    // Todo: Is this even right?
//        if (c == LCDSolidColor::Clear and mask)
//            mask->setBufferPixel(x, y, false);
    if (c == LCDSolidColor::Clear)
        return;

    if (c == LCDSolidColor::XOR)
        setBufferPixel(x, y, not getBufferPixel(x, y));
    else
        setBufferPixel(x, y, c == LCDSolidColor::White);
}

void Graphics::popContext() {
    if (displayContextStack.empty())
        return;
    auto &context = displayContextStack.back();
    // Free any preserved Lua references (Safe for non-preserved values, as well)
    emulator->releaseLuaReference(context.focusedImage);
    emulator->releaseLuaReference(context.font);
    emulator->releaseLuaReference(context.stencilImage);
    if (context.bitmap != frameBuffer)
        emulator->releaseLuaReference(context.bitmap);
    displayContextStack.pop_back();
}

void Graphics::drawRect(int x, int y, int width, int height, LCDColor color) {
    auto &context = getCurrentDisplayContext();
    auto shift = 2 * context.lineWidth;
    if (context.strokeLocation == StrokeLocation::Inside) {
        x++;
        y++;
        width -= shift;
        height -= shift;
    } else if (context.strokeLocation == StrokeLocation::Outside) {
        x--;
        y--;
        width += shift;
        height += shift;
    }
    drawLine(x, y, x + width, y, context.lineWidth, color);
    drawLine(x, y, x, y + height, context.lineWidth, color);
    drawLine(x + width, y, x + width, y + height, context.lineWidth, color);
    drawLine(x, y + height, x + width, y + height, context.lineWidth, color);
}

void Graphics::drawText(const void* text, int len, [[maybe_unused]] PDStringEncoding encoding, int x, int y, LCDFont_32 *font) {
    // Todo: Support encodings
    // Todo: Kerning
    const char *string = (const char *) text;
    auto &context = getCurrentDisplayContext();
    if (!font)
        font = context.font ? context.font : systemFont;
    for (int i = 0; i < len; i++) {
        char character = string[i];
        int pageIndex = 0; // Todo: Temp ascii
        try {
            auto &page = font->pages.at(pageIndex);
            auto &glyph = page.glyphs.at(character);
            drawBitmap(&glyph.bitmap, x, y, LCDBitmapFlip::Unflipped);
            x += glyph.advance;
        } catch (std::out_of_range &ignored) {}
    }
}

void Graphics::fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, LCDColor color) {
    int a, b, y, last;
    if (y1 > y2) {
        std::swap(y1, y2);
        std::swap(x1, x2);
    }
    if (y2 > y3) {
        std::swap(y2, y3);
        std::swap(x2, x3);
    }
    if (y1 > y2) {
        std::swap(y1, y2);
        std::swap(x1, x2);
    }
    if (y1 == y3) {
        a = b = x1;
        if (x2 < a)
            a = x2;
        else if (x2 > b)
            b = x2;
        if (x3 < a)
            a = x3;
        else if (x3 > b)
            b = x3;
        drawLine(a, y1, b + 1, y1, 1, color);
        return;
    }
    int dx12 = x2 - x1, dy12 = y2 - y1;
    int dx13 = x3 - x1, dy13 = y3 - y1;
    int dx23 = x3 - x2, dy23 = y3 - y2;
    int sa = 0, sb = 0;
    last = y2 == y3 ? y2 : y2 - 1;
    for (y = y1; y <= last; y++) {
        a = x1 + sa / dy12;
        b = x1 + sb / dy13;
        sa += dx12;
        sb += dx13;
        if (a > b)
            std::swap(a, b);
        drawLine(a, y, b + 1, y, 1, color);
    }
    sa = dx23 * (y - y2);
    sb = dx13 * (y - y1);
    for (; y <= last; y++) {
        a = x1 + sa / dy23;
        b = x1 + sb / dy13;
        sa += dx12;
        sb += dx13;
        if (a > b)
            std::swap(a, b);
        drawLine(a, y, b + 1, y, 1, color);
    }
}

void Graphics::drawEllipse(int rectX, int rectY, int width, int height, int lineWidth, float startAngle, float endAngle, LCDColor color, bool filled) {
    int xc = rectX + width / 2;
    int yc = rectY + height / 2;
    int rx = width / 2;
    int ry = height / 2;
    double dx, dy, d1, d2;
    int x = 0, y = ry;
    d1 = ry * ry - rx * rx * ry + 0.25 * rx * rx;
    dx = 2 * ry * ry * x;
    dy = 2 * rx * rx * y;
    auto draw = [&] {
        double q1 = 90 - 180 / std::numbers::pi * atan2(y, x);
        double q2 = 360 - q1;
        double q3 = 180 + q1;
        double q4 = 180 - q1;
        if (filled) {
            if (q1 >= startAngle and q1 <= endAngle)
                drawLine(xc, yc - y, xc + x, yc - y, 1, color);
            if (q2 >= startAngle and q2 <= endAngle)
                drawLine(xc, yc - y, xc - x, yc - y, 1, color);
            if (q3 >= startAngle and q3 <= endAngle)
                drawLine(xc, yc + y, xc - x, yc + y, 1, color);
            if (q4 >= startAngle and q4 <= endAngle)
                drawLine(xc, yc + y, xc + x, yc + y, 1, color);
        } else if (lineWidth == 0) {
            if (q1 >= startAngle and q1 <= endAngle)
                drawPixel(xc + x, yc - y, color);
            if (q2 >= startAngle and q2 <= endAngle)
                drawPixel(xc - x, yc - y, color);
            if (q3 >= startAngle and q3 <= endAngle)
                drawPixel(xc - x, yc + y, color);
            if (q4 >= startAngle and q4 <= endAngle)
                drawPixel(xc + x, yc + y, color);
        } else {
            if (q1 >= startAngle and q1 <= endAngle)
                for (int i = 0; i < lineWidth; i++) {
                    drawPixel(xc + x + i, yc - y, color);
                    drawPixel(xc + x, yc - y - i, color);
                }
            if (q2 >= startAngle and q2 <= endAngle)
                for (int i = 0; i < lineWidth; i++) {
                    drawPixel(xc - x - i, yc - y, color);
                    drawPixel(xc - x, yc - y - i, color);
                }
            if (q3 >= startAngle and q3 <= endAngle)
                for (int i = 0; i < lineWidth; i++) {
                    drawPixel(xc - x - i, yc + y, color);
                    drawPixel(xc - x, yc + y + i, color);
                }
            if (q4 >= startAngle and q4 <= endAngle)
                for (int i = 0; i < lineWidth; i++) {
                    drawPixel(xc + x + i, yc + y, color);
                    drawPixel(xc + x, yc + y + i, color);
                }
        }
    };
    while (dx < dy) {
        draw();
        if (d1 < 0) {
            x++;
            dx += 2 * ry * ry;
            d1 += dx + ry * ry;
        } else {
            x++;
            y--;
            dx += 2 * ry * ry;
            dy -= 2 * rx * rx;
            d1 += dx - dy + ry * ry;
        }
    }
    d2 = ry * ry * (x + 0.5) * (x + 0.5) + rx * rx * (y - 1) * (y - 1) - rx * rx * ry * ry;
    while (y >= 0) {
        draw();
        if (d2 > 0) {
            y--;
            dy -= 2 * rx * rx;
            d2 += rx * rx - dy;
        } else {
            y--;
            x++;
            dx += 2 * ry * ry;
            dy -= 2 * rx * rx;
            d2 += dx - dy + rx * rx;
        }
    }
}

LCDBitmapTable_32 *Graphics::getBitmapTable(const std::string &path) {
    auto &bitmapTable = loadedBitmapTables[path];
    if (bitmapTable.cells.empty())
        bitmapTable = emulator->rom->getImageTable(path);
    auto table = heap.construct<LCDBitmapTable_32>(this, bitmapTable.cellsPerRow);
    allocatedBitmapTables.emplace(table);
    table->bitmaps.reserve(bitmapTable.cells.size());
    for (auto &bitmap : bitmapTable.cells)
        table->bitmaps.emplace_back(*getImage(bitmap));
    return table;
}

LCDBitmap_32 *Graphics::getImage(const std::string &path) {
    auto &image = loadedImages[path];
    if (image.cell.data.empty())
        image = emulator->rom->getImage(path);
    auto bitmap = getImage(image.cell);
    allocatedBitmaps.emplace(bitmap);
    return bitmap;
}

LCDFont_32 *Graphics::getFont(const std::string &path) {
    auto &font = loadedFonts[path];
    if (font)
        return font;
    font = getFont(emulator->rom->getFont(path));
    return font;
}

LCDFont_32 *Graphics::getFont(const Rom::Font &source) {
    auto font = heap.construct<LCDFont_32>(this, source.tracking, source.glyphWidth, source.glyphHeight);
    for (auto &pageEntry : source.glyphs) {
        auto &page = (*font->pages.emplace(std::pair{pageEntry.first, LCDFontPage_32{this}}).first).second;
        for (auto &glyphEntry : pageEntry.second) {
            auto &glyph = glyphEntry.second;
            page.glyphs.emplace(std::pair{glyphEntry.first, LCDFontGlyph_32{std::move(*getImage(glyph.cell)), glyph.advance, glyph.shortKerningTable, glyph.longKerningTable}});
        }
    }
    return font;
}

LCDFont_32 *Graphics::getFont(const uint8_t *data, bool wide) {
    auto fontData = Rom::readFontData((uint8_t *) data, wide);
    auto font = getFont(fontData);
    allocatedFonts.emplace(font);
    return font;
}

void Graphics::init() {
    frameBuffer = heap.construct<LCDBitmap_32>(DISPLAY_BUFFER_WIDTH, DISPLAY_HEIGHT, this);
    previousFrameBuffer = heap.construct<LCDBitmap_32>(DISPLAY_BUFFER_WIDTH, DISPLAY_HEIGHT, this);
    frameBufferContext.bitmap = frameBuffer;
    systemFont = getFont(systemFontSource);
}

void Graphics::reset() {
    // Don't bother freeing resources since heap will be reset

    frameBuffer = nullptr;
    previousFrameBuffer = nullptr;
    systemFont = nullptr;

    allocatedBitmaps.clear();
    allocatedBitmapTables.clear();
    loadedFonts.clear();
    allocatedSprites.clear();
    spriteDrawList.clear();
    loadedImages.clear();

    displayContextStack.clear();
    frameBufferContext = {.bitmap = frameBuffer, .backgroundColor = LCDSolidColor::White, .lineWidth = 1};

    memset(displayBufferRGBA, 0, sizeof(displayBufferRGBA));
    displayOffset = {};
    displayScale = 1;
    displayInverted = false;
    displayFlippedX = displayFlippedY = false;
    displayMosaic = {};
    framerate = 20;
}

void Graphics::update() {
    frameBufferContext.clipRect = {0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT};
    while (!displayContextStack.empty())
        popContext();
}

void Graphics::flushDisplayBuffer() {
    // Todo: Mosaic (Could it be implemented as a box blur? Compare to blur effect)

    for (int i = 0; i < DISPLAY_HEIGHT; i++) {
        int y = (displayFlippedY ? DISPLAY_HEIGHT - 1 - i : i) - displayOffset.y;
        if (y >= 0 and y < DISPLAY_HEIGHT) // Check before truncating to avoid affecting display offset
            y /= displayScale;
        for (int j = 0; j < DISPLAY_WIDTH; j++) {
            int x = (displayFlippedX ? DISPLAY_WIDTH - 1 - j : j) - displayOffset.x;
            if (x >= 0 and x < DISPLAY_WIDTH)
                x /= displayScale;
            auto color = frameBuffer->getBufferPixel(x, y) ? displayBufferOnColor : displayBufferOffColor;
            if (y < 0 or y >= DISPLAY_HEIGHT / displayScale or x < 0 or x >= DISPLAY_WIDTH / displayScale)
                color = frameBufferContext.backgroundColor == LCDSolidColor::White ? displayBufferOnColor : displayBufferOffColor;
            if (displayBufferNativeEndian)
                displayBufferRGBA[i][j] = color;
            else
                writeUint32BE((uint8_t *) &displayBufferRGBA[i][j], color);
        }
    }

    memcpy(previousFrameBuffer->data.data(), frameBuffer->data.data(), frameBuffer->data.size());

    emulator->menu.render();
}

bool Graphics::handleFree(void *ptr) {
    if (allocatedBitmapTables.contains((LCDBitmapTable_32 *) ptr)) {
        freeBitmapTable((LCDBitmapTable_32 *) ptr); // Todo: Are bitmap tables meant to be freed with realloc?
        return true;
    } else if (allocatedFonts.contains((LCDFont_32 *) ptr)) {
        allocatedFonts.erase((LCDFont_32 *) ptr);
        heap.destruct((LCDFont_32 *) ptr);
        return true;
    }
    return false;
}
