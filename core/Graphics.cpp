#include "Graphics.hpp"
#include "Cranked.hpp"
#include "gen/SystemFont.hpp"

using namespace cranked;

Graphics::Graphics(Cranked &cranked)
        : cranked(cranked), heap(cranked.heap), systemFontSource(Rom::readFont(SYSTEM_FONT, sizeof(SYSTEM_FONT))) {}

LCDVideoPlayer_32::LCDVideoPlayer_32(Cranked &cranked) : NativeResource(cranked) {}

LCDBitmap_32::LCDBitmap_32(Cranked &cranked, int width, int height)
        : NativeResource(cranked), width(width), height(height), data(vheap_vector<uint8>(width * height, cranked.heap.allocator<uint8>())), mask(nullptr) {}

LCDBitmap_32::LCDBitmap_32(const LCDBitmap_32 &other)
        : NativeResource(other), width(other.width), height(other.height), data(other.data), mask(other.mask ? cranked.heap.construct<LCDBitmap_32>(*other.mask) : nullptr) {}

LCDBitmap_32& LCDBitmap_32::operator=(const LCDBitmap_32 &other) {
    if (&other == this)
        return *this;
    width = other.width;
    height = other.height;
    data = other.data;
    mask = other.mask ? cranked.heap.construct<LCDBitmap_32>(*other.mask) : nullptr;
    return *this;
}

LCDFontGlyph_32::LCDFontGlyph_32(Bitmap bitmap, int advance, const map<int, int8> &shortKerningTable, const map<int, int8> &longKerningTable)
        : NativeResource(bitmap->cranked), bitmap(bitmap), advance(advance), shortKerningTable(shortKerningTable), longKerningTable(longKerningTable) {}

LCDFontPage_32::LCDFontPage_32(Cranked &cranked) : NativeResource(cranked) {}

LCDFont_32::LCDFont_32(Cranked &cranked, int tracking, int glyphWidth, int glyphHeight)
        : NativeResource(cranked), tracking(tracking), glyphWidth(glyphWidth), glyphHeight(glyphHeight) {}

LCDBitmapTable_32::LCDBitmapTable_32(Cranked &cranked, int cellsPerRow) : NativeResource(cranked), cellsPerRow(cellsPerRow) {}

LCDBitmapTable_32& LCDBitmapTable_32::operator=(const LCDBitmapTable_32 &other) {
    if (&other == this)
        return *this;
    bitmaps = other.bitmaps;
    cellsPerRow = other.cellsPerRow;
    return *this;
}

bool LCDBitmap_32::getBufferPixel(int x, int y) {
    if (x < 0 or x >= width or y < 0 or y >= height)
        return false;
    int offset = 7 - x % 8;
    auto stride = (int) ceil((float) width / 8);
    return data[y * stride + x / 8] & (1 << offset);
}

void LCDBitmap_32::setBufferPixel(int x, int y, bool color) {
    if (x < 0 or x >= width or y < 0 or y >= height)
        return;
    int offset = 7 - x % 8;
    auto stride = (int) ceil((float) width / 8);
    auto &word = data[y * stride + x / 8];
    word &= ~(1 << offset);
    word |= color << offset;
}

void LCDBitmap_32::drawPixel(int x, int y, LCDColor color) {
    if (x < 0 or x >= width or y < 0 or y >= height)
        return;
    // Todo: Is pattern offset affected by draw offset?
    auto c = color.color;
    if (color.pattern >= 4) {
        auto row = y % 8;
        auto column = x % 8;
        auto word = cranked.virtualRead<uint8>(color.pattern + row);
        auto maskWord = cranked.virtualRead<uint8>(color.pattern + row + 8);
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

LCDSolidColor LCDBitmap_32::getPixel(int x, int y) {
    bool pixel = getBufferPixel(x, y);
    if (inverted)
        pixel = not pixel;
    return mask and not mask->getBufferPixel(x, y) ? LCDSolidColor::Clear : LCDSolidColor(pixel);
}

// Todo: Could be more efficient using memset and such (Patterns add complexity)
void LCDBitmap_32::clear(LCDColor color) {
    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++)
            drawPixel(x, y, color);
}

void LCDBitmap_32::drawLine(int x1, int y1, int x2, int y2, LCDColor color) {
    // Todo: Line ending styles
    // Todo: Draws slightly differently from official implementation, steps differently
    bool vertical = abs(y2 - y1) > abs(x2 - x1);
    if (vertical) {
        swap(x1, y1);
        swap(x2, y2);
    }
    if (x1 > x2) {
        swap(x1, x2);
        swap(y1, y2);
    }
    int dx = x2 - x1;
    int dy = abs(y2 - y1);
    int err = dx / 2;
    int yStep = y1 < y2 ? 1 : -1;
    for (; x1 <= x2; x1++) {
        if (vertical)
            drawPixel(y1, x1, color);
        else
            drawPixel(x1, y1, color);
        err -= dy;
        if (err < 0) { // Todo: Maybe err check should happen before first draw
            y1 += yStep;
            err += dx;
            if (vertical)
                drawPixel(y1, x1, color);
            else
                drawPixel(x1, y1, color);
        }
    }
}

// Todo: Can be more efficient since it's done in horizontal segments
void LCDBitmap_32::fillRect(int x, int y, int w, int h, LCDColor color) {
    for (int i = 0; i < h; i++) {
        if (y + i >= height)
            break;
        for (int j = 0; j < w; j++) {
            if (x + j >= width)
                break;
            drawPixel(x, y, color);
        }
    }
}

LCDSprite_32::LCDSprite_32(Cranked &cranked) : NativeResource(cranked) {
    cranked.graphics.allocatedSprites.emplace(this);
}

LCDSprite_32::~LCDSprite_32() {
    eraseByEquivalentKey(cranked.graphics.spriteDrawList, this);
    cranked.graphics.allocatedSprites.erase(this);
}

void LCDSprite_32::updateCollisionWorld() {
    cranked.bump.updateSprite(this);
}

void LCDSprite_32::draw() {
    if (!visible)
        return;
    Vec2 drawPos = bounds.pos;// - Vec2{center.x * bounds.size.x, center.y * bounds.size.y};
    if (image and clipRect) {
        // Todo: Respect clip rect (Probably adding clip rect support to drawing function in addition to context clip rect)
        cranked.graphics.drawBitmap(image.get(), (int)drawPos.x, (int)drawPos.y, flip, ignoresDrawOffset);
    } else if (drawFunction)
        cranked.nativeEngine.invokeEmulatedFunction<void, ArgType::void_t, ArgType::ptr_t, ArgType::struct4f_t, ArgType::struct4f_t>
                (drawFunction, this, fromRect(Rect{ drawPos, bounds.size }), PDRect_32{ 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT }); // Todo: Dirty rect
}

void LCDSprite_32::update() {
    if (!updatesEnabled)
        return;
    if (updateFunction)
        cranked.nativeEngine.invokeEmulatedFunction<void, ArgType::void_t, ArgType::ptr_t>(updateFunction, this);
}

void Graphics::pushContext(Bitmap target) {
    displayContextStack.emplace_back(target ? target : frameBuffer.get());
}

void Graphics::popContext() {
    if (displayContextStack.empty())
        return;
    displayContextStack.pop_back();
}

void Graphics::drawPixel(int x, int y, LCDColor color, bool ignoreOffset) {
    auto &context = getCurrentDisplayContext();
    auto pos = ignoreOffset ? IntVec2{x, y} : context.drawOffset.as<int32>() + IntVec2{x, y};
    if (!context.clipRect.contains(pos))
        return;
    context.bitmap->drawPixel(pos.x, pos.y, color);
}

LCDSolidColor Graphics::getPixel(int x, int y, bool ignoreOffset) {
    auto &context = getCurrentDisplayContext();
    auto pos = ignoreOffset ? IntVec2{x, y} : context.drawOffset.as<int32>() + IntVec2{x, y};
    return context.bitmap->getPixel(pos.x, pos.y);
}

void Graphics::drawLine(int x1, int y1, int x2, int y2, int width, LCDColor color) {
    auto &context = getCurrentDisplayContext();
    auto start = IntVec2{x1, y1} + context.drawOffset;
    auto end = IntVec2{x2, y2} + context.drawOffset;
    // Todo: Clipping
    // Todo: Width
    // Todo: Line caps
    context.bitmap->drawLine(start.x, start.y, end.x, end.y, color);
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

void Graphics::fillRect(int x, int y, int width, int height, LCDColor color) {
    for (int i = 0; i < height; i++)
        drawLine(x, y + i, x + width - 1, y + i, 1, color);
}

void Graphics::drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, LCDColor color) {
    auto lineWidth = getCurrentDisplayContext().lineWidth;
    drawLine(x1, y1, x2, y2, lineWidth, color);
    drawLine(x1, y1, x3, y3, lineWidth, color);
    drawLine(x2, y2, x3, y3, lineWidth, color);
}

void Graphics::drawBitmap(Bitmap bitmap, int x, int y, LCDBitmapFlip flip, bool ignoreOffset, optional<IntRect> sourceRect) {
    auto &context = getCurrentDisplayContext();
    auto mode = context.bitmapDrawMode;
    bool flipY = flip == LCDBitmapFlip::FlippedY or flip == LCDBitmapFlip::FlippedXY;
    bool flipX = flip == LCDBitmapFlip::FlippedX or flip == LCDBitmapFlip::FlippedXY;
    if (sourceRect)
        sourceRect = *sourceRect - IntVec2{x, y};
    for (int i = 0; i < bitmap->height; i++)
        for (int j = 0; j < bitmap->width; j++) {
            auto pixel = bitmap->getPixel(j, i);
            int pixelX = x + (flipX ? bitmap->width - 1 - j : j);
            int pixelY = y + (flipY ? bitmap->height - 1 - i : i);
            if (pixel == LCDSolidColor::Clear)
                continue;
            if (sourceRect and sourceRect->contains(IntVec2{pixelX, pixelY}))
                continue;
            switch (mode) {
                case LCDBitmapDrawMode::Copy:
                    drawPixel(pixelX, pixelY, pixel, ignoreOffset);
                    break;
                case LCDBitmapDrawMode::WhiteTransparent:
                    if (pixel != LCDSolidColor::White)
                        drawPixel(pixelX, pixelY, pixel, ignoreOffset);
                    break;
                case LCDBitmapDrawMode::BlackTransparent:
                    if (pixel != LCDSolidColor::Black)
                        drawPixel(pixelX, pixelY, pixel, ignoreOffset);
                    break;
                case LCDBitmapDrawMode::FillWhite:
                    drawPixel(pixelX, pixelY, LCDSolidColor::White, ignoreOffset);
                    break;
                case LCDBitmapDrawMode::FillBlack:
                    drawPixel(pixelX, pixelY, LCDSolidColor::Black, ignoreOffset);
                    break;
                case LCDBitmapDrawMode::XOR:
                    drawPixel(pixelX, pixelY, getPixel(pixelX, pixelY, ignoreOffset) == pixel ? LCDSolidColor::Black : LCDSolidColor::White, ignoreOffset);
                    break;
                case LCDBitmapDrawMode::NXOR:
                    drawPixel(pixelX, pixelY, getPixel(pixelX, pixelY, ignoreOffset) != pixel ? LCDSolidColor::Black : LCDSolidColor::White, ignoreOffset);
                    break;
                case LCDBitmapDrawMode::Inverted:
                    drawPixel(pixelX, pixelY, pixel == LCDSolidColor::White ? LCDSolidColor::Black : LCDSolidColor::White, ignoreOffset);
                    break;
            }
        }
}

void Graphics::drawText(const void* text, int len, PDStringEncoding encoding, int x, int y, Font font) {
    // Todo: Support encodings
    // Todo: Kerning
    const char *string = (const char *) text;
    auto &context = getCurrentDisplayContext();
    if (!font)
        font = (context.font ? context.font : systemFont).get();
    for (int i = 0; i < len; i++) {
        char character = string[i];
        int pageIndex = 0; // Todo: Temp ascii
        try {
            auto &page = font->pages.at(pageIndex);
            auto &glyph = *page->glyphs.at(character);
            drawBitmap(glyph.bitmap.get(), x, y, LCDBitmapFlip::Unflipped);
            x += glyph.advance + font->tracking;
        } catch (out_of_range &ignored) {} // Todo: Prevent exceptions?
    }
}

void Graphics::fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, LCDColor color) {
    int a, b, y, last;
    if (y1 > y2) {
        swap(y1, y2);
        swap(x1, x2);
    }
    if (y2 > y3) {
        swap(y2, y3);
        swap(x2, x3);
    }
    if (y1 > y2) {
        swap(y1, y2);
        swap(x1, x2);
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
            swap(a, b);
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
            swap(a, b);
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
        double q1 = 90 - 180 / PI * atan2(y, x);
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

void Graphics::updateSprites() {
    // Todo: Supposed to be updated in sorted order?
    for (auto &sprite : spriteDrawList) {
        sprite->update();
    }
}

void Graphics::drawSprites() {
    auto &context = getCurrentDisplayContext();
    context.bitmap->clear(context.backgroundColor);
    // Todo: Dirty rect support
    // Todo: Sort by Z then added order (Stable sort?)
    for (auto &sprite : spriteDrawList) {
        sprite->draw();
    }
}

BitmapTable Graphics::getBitmapTable(const string &path) {
    auto bitmapTable = cranked.rom->getImageTable(path);
    auto table = heap.construct<LCDBitmapTable_32>(cranked, bitmapTable.cellsPerRow);
    table->bitmaps.reserve(bitmapTable.cells.size());
    for (auto &bitmap : bitmapTable.cells)
        table->bitmaps.emplace_back(getImage(bitmap));
    return table;
}

Bitmap Graphics::getImage(const string &path) {
    auto image = cranked.rom->getImage(path);
    return getImage(image.cell);
}

Bitmap Graphics::getImage(const Rom::ImageCell &source) {
    auto bitmap = heap.construct<LCDBitmap_32>(cranked, source.width, source.height);
    auto stride = (int) ceil((float) source.width / 8);
    auto readBitmapData = [&](const uint8 *src, uint8 *dest){
        for (int i = 0; i < source.height; i++)
            for (int j = 0; j < source.width; j++)
                if (src[i * source.width + j])
                    dest[stride * i + j / 8] |= 0x80 >> (j % 8);
    };
    readBitmapData(source.data.data(), bitmap->data.data());
    if (!source.mask.empty()) {
        bitmap->mask = heap.construct<LCDBitmap_32>(cranked, source.width, source.height);
        readBitmapData(source.mask.data(), bitmap->mask->data.data());
    }
    return bitmap;
}

Font Graphics::getFont(const string &path) {
    return getFont(cranked.rom->getFont(path));
}

Font Graphics::getFont(const Rom::Font &source) {
    auto font = heap.construct<LCDFont_32>(cranked, source.tracking, source.glyphWidth, source.glyphHeight);
    for (auto &pageEntry : source.glyphs) {
        auto &page = font->pages[pageEntry.first] = heap.construct<LCDFontPage_32>(cranked);
        for (auto &glyphEntry : pageEntry.second) {
            auto &glyph = glyphEntry.second;
            page->glyphs[glyphEntry.first] = heap.construct<LCDFontGlyph_32>(getImage(glyph.cell), glyph.advance, glyph.shortKerningTable, glyph.longKerningTable);
        }
    }
    return font;
}

Font Graphics::getFont(const uint8 *data, bool wide) {
    auto fontData = Rom::readFontData((uint8 *) data, wide);
    auto font = getFont(fontData);
    return font;
}

void Graphics::init() {
    frameBuffer = createBitmap(DISPLAY_BUFFER_WIDTH, DISPLAY_HEIGHT);
    previousFrameBuffer = createBitmap(DISPLAY_BUFFER_WIDTH, DISPLAY_HEIGHT);
    frameBufferContext = DisplayContext(frameBuffer.get());
    systemFont = getFont(systemFontSource);
}

void Graphics::reset() {
    frameBuffer.reset();
    previousFrameBuffer.reset();
    systemFont.reset();

    for (auto &context : displayContextStack)
        context.reset();
    displayContextStack.clear();

    frameBufferContext.reset();

    for (SpriteRef &sprite : vector(spriteDrawList.begin(), spriteDrawList.end()))
        sprite.reset();
    spriteDrawList.clear();

    allocatedSprites.clear();

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
                writeUint32BE((uint8 *) &displayBufferRGBA[i][j], color);
        }
    }

    memcpy(previousFrameBuffer->data.data(), frameBuffer->data.data(), frameBuffer->data.size());

    cranked.menu.render();
}
