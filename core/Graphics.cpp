#include "Graphics.hpp"
#include "Cranked.hpp"

using namespace cranked;

Graphics::Graphics(Cranked &cranked)
        : cranked(cranked), heap(cranked.heap), systemFontSources(Rom::readSystemFont("Asheville-Sans-14-Light.pft"), Rom::readSystemFont("Asheville-Sans-14-Bold.pft"), Rom::readSystemFont("Asheville-Sans-14-Light-Oblique.pft")) {}

LCDVideoPlayer_32::LCDVideoPlayer_32(Cranked &cranked, float frameRate, IntVec2 size) : NativeResource(cranked), frameRate(frameRate), size(size) {}

LCDBitmap_32::LCDBitmap_32(Cranked &cranked, int width, int height)
        : NativeResource(cranked), width(width), height(height), data(vheap_vector(width * height, cranked.heap.allocator<uint8>())), mask(nullptr) {}

LCDBitmap_32::LCDBitmap_32(const LCDBitmap_32 &other)
        : NativeResource(other.cranked), width(other.width), height(other.height), data(other.data), mask(other.mask ? cranked.heap.construct<LCDBitmap_32>(*other.mask) : nullptr) {}

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
        : NativeResource(bitmap->cranked), advance(advance), shortKerningTable(shortKerningTable), longKerningTable(longKerningTable), bitmap(bitmap) {}

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

[[nodiscard]] IntVec2 LCDBitmapTable_32::getBitmapSize() const {
    if (!bitmaps.empty() and bitmaps[0])
        return { bitmaps[0]->width, bitmaps[0]->height };
    return {};
}

LCDTileMap_32::LCDTileMap_32(Cranked &cranked) : NativeResource(cranked) {}

// Todo: These pixel functions should be used less and live in the header

tuple<int, int> LCDBitmap_32::getBufferPixelLocation(int x, int y) const {
    if (x < 0 or x >= width or y < 0 or y >= height)
        return { 0, 0 };
    int offset = 7 - x % 8;
    auto stride = (int) ceil((float) width / 8);
    return { y * stride + x / 8, 1 << offset };
}

bool LCDBitmap_32::getBufferPixel(int x, int y) const {
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

LCDSolidColor LCDBitmap_32::getPixel(int x, int y) const {
    bool pixel = getBufferPixel(x, y);
    if (inverted)
        pixel = not pixel;
    return mask and not mask->getBufferPixel(x, y) ? LCDSolidColor::Clear : LCDSolidColor(pixel);
}

// Todo: Could be more efficient using memset and such (Patterns add complexity)
void LCDBitmap_32::clear(LCDColor color) {
    if (color.color == LCDSolidColor::Clear) {
        if (!mask)
            mask = cranked.heap.construct<LCDBitmap_32>(cranked, width, height);
        else
            mask->clear(color);
    }
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

// Todo: Can be more efficient
void LCDBitmap_32::drawBitmap(Bitmap image, int x, int y, int w, int h, int sourceX, int sourceY) {
    if (w < 0)
        w = image->width;
    if (h < 0)
        h = image->height;
    w = min(min(w, width - x), image->width - sourceX);
    h = min(min(h, height - y), image->height - sourceY);
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            drawPixel(j, i, image->getPixel(x, y));
        }
    }
}

[[nodiscard]] vector<uint8> LCDBitmap_32::toRGB() const {
    vector<uint8_t> frame(width * height);
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
            ((uint32 *)frame.data())[i * width + j] = getPixel(j, i) == LCDSolidColor::White ? cranked.graphics.displayBufferOnColor : cranked.graphics.displayBufferOffColor;
    return frame;
}

LCDSprite_32::LCDSprite_32(Cranked &cranked) : NativeResource(cranked) {
    cranked.graphics.allocatedSprites.emplace(this);
}

LCDSprite_32::~LCDSprite_32() {
    cranked.luaEngine.clearResourceValue(this);
    cranked.graphics.allocatedSprites.erase(this);
}

void LCDSprite_32::updateCollisionWorld() {
    cranked.bump.updateSprite(this);
}

void LCDSprite_32::updateLuaPosition() {
    if (!cranked.luaEngine.isLoaded())
        return;
    LuaValGuard value(cranked.luaEngine.getResourceValue(this));
    if (value.val.isNil())
        return;
    auto pos = getPosition();
    value.val.setFloatField("x", pos.x);
    value.val.setFloatField("y", pos.y);
}

void LCDSprite_32::draw() {
    if (!visible or !inDrawList)
        return;
    Vec2 drawPos = bounds.pos;
    // Todo: Support tilemap
    Rect dirtyRect = { -bounds.pos, { DISPLAY_WIDTH, DISPLAY_HEIGHT } }; // Todo
    if (image) {
        // Todo: Respect scaling, rotation, clip rect (Probably adding clip rect support to drawing function in addition to context clip rect)
        cranked.graphics.drawBitmap(image.get(), (int)drawPos.x, (int)drawPos.y, flip, ignoresDrawOffset);
    } else if (drawFunction)
        cranked.nativeEngine.invokeEmulatedFunction<void, ArgType::void_t, ArgType::ptr_t, ArgType::struct4f_t, ArgType::struct4f_t>
                (drawFunction, this, fromRect(Rect{ drawPos, bounds.size }), fromRect(dirtyRect)); // Todo: Correct args?
    else if (LuaValGuard value{ cranked.luaEngine.getResourceValue(this) }; value.val.isTable()) {
        if (auto callback = value.val.getFieldRaw("draw"); callback.isFunction()) {
            cranked.luaEngine.pushResource(this);
            cranked.luaEngine.pushFloat(dirtyRect.pos.x);
            cranked.luaEngine.pushFloat(dirtyRect.pos.y);
            cranked.luaEngine.pushFloat(dirtyRect.size.x);
            cranked.luaEngine.pushFloat(dirtyRect.size.y);
            if (int result = lua_pcall(cranked.getLuaContext(), 5, 0, 0); result != LUA_OK) {
                string errStr = lua_isstring(cranked.getLuaContext(), -1) ? lua_tostring(cranked.getLuaContext(), -1) : "";
                lua_pop(cranked.getLuaContext(), 1);
                throw CrankedError("Exception in Sprite draw: {}, ({})", cranked.luaEngine.getLuaError(result), errStr);
            }
        } else
            lua_pop(cranked.getLuaContext(), 1);
    }
}

void LCDSprite_32::update() {
    if (!updatesEnabled or !inDrawList)
        return;
    if (updateFunction)
        cranked.nativeEngine.invokeEmulatedFunction<void, ArgType::void_t, ArgType::ptr_t>(updateFunction, this);
    else if (LuaValGuard value{ cranked.luaEngine.getResourceValue(this) }; value.val.isTable()) {
        if (auto callback = value.val.getFieldRaw("update"); callback.isFunction()) {
            bool isUpdateFunc; // Guard against playdate.graphics.sprite.update being found and causing C stack overflow
            {
                if (!cranked.luaEngine.getQualifiedLuaGlobal("playdate.graphics.sprite.update"))
                    throw CrankedError("Missing Sprite table");
                LuaValGuard updateFunc { LuaVal(cranked.getLuaContext()) };
                isUpdateFunc = lua_compare(cranked.getLuaContext(), callback, updateFunc.val, LUA_OPEQ);
            }
            if (isUpdateFunc) {
                lua_pop(cranked.getLuaContext(), 1);
                return;
            }
            cranked.luaEngine.pushResource(this);
            if (int result = lua_pcall(cranked.getLuaContext(), 1, 0, 0); result != LUA_OK) {
                string errStr = lua_isstring(cranked.getLuaContext(), -1) ? lua_tostring(cranked.getLuaContext(), -1) : "";
                lua_pop(cranked.getLuaContext(), 1);
                throw CrankedError("Exception in Sprite update: {}, ({})", cranked.luaEngine.getLuaError(result), errStr);
            }
        } else
            lua_pop(cranked.getLuaContext(), 1);
    }
}

Sprite LCDSprite_32::copy() {
    // Todo: What should not be copied?
    auto other = cranked.heap.construct<LCDSprite_32>(cranked);
    other->bounds = bounds;
    other->image = image;
    other->tileMap = tileMap;
    other->center = center;
    other->visible = visible;
    other->updatesEnabled = updatesEnabled;
    other->dontRedrawOnImageChange = dontRedrawOnImageChange;
    other->ignoresDrawOffset = ignoresDrawOffset;
    other->opaque = opaque;
    other->collisionsEnabled = collisionsEnabled;
    other->collideRect = collideRect;
    other->collideRectFlip = collideRectFlip;
    other->groupMask = groupMask;
    other->collidesWithGroupMask = collidesWithGroupMask;
    other->zIndex = zIndex;
    other->scale = scale;
    other->rotation = rotation;
    other->clipRect = clipRect;
    other->tag = tag;
    other->drawMode = drawMode;
    other->flip = flip;
    other->stencil = stencil;
    other->stencilTiled = stencilTiled;
    other->updateFunction = updateFunction;
    other->drawFunction = drawFunction;
    other->collideResponseFunction = collideResponseFunction;
    other->userdata = userdata;
    return other;
}

DisplayContext &Graphics::pushContext(Bitmap target) {
    // Todo: Should this copy the existing context?
    return displayContextStack.emplace_back(target ? target : frameBuffer.get());
}

void Graphics::popContext() {
    if (displayContextStack.empty())
        return;
    displayContextStack.pop_back();
}

void Graphics::drawPixel(int x, int y, LCDColor color, bool ignoreOffset) {
    ZoneScoped;
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

void Graphics::drawRoundRect(int x, int y, int width, int height, int radius, LCDColor color) {
    // Todo
    drawRect(x, y, width, height, color);
}

void Graphics::fillRoundRect(int x, int y, int width, int height, int radius, LCDColor color) {
    // Todo
    fillRect(x, y, width, height, color);
}

void Graphics::drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, LCDColor color) {
    auto lineWidth = getCurrentDisplayContext().lineWidth;
    drawLine(x1, y1, x2, y2, lineWidth, color);
    drawLine(x1, y1, x3, y3, lineWidth, color);
    drawLine(x2, y2, x3, y3, lineWidth, color);
}

void Graphics::drawBitmap(Bitmap bitmap, int x, int y, LCDBitmapFlip flip, bool ignoreOffset, optional<IntRect> sourceRect) {
    ZoneScoped;
    // Todo: This could be heavly optimized with bitwise operations
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

void Graphics::drawBitmapTiled(Bitmap bitmap, int x, int y, int width, int height, LCDBitmapFlip flip) {
    int right = x + width, bottom = y + height;
    int bitmapWidth = bitmap->width, bitmapHeight = bitmap->height;
    if (bitmap->data.empty()) // Prevent infinite loop
        return;
    for (int i = y; i < bottom; i += bitmapHeight) {
        for (int j = x; j < right; j += bitmapWidth) {
            bool truncate = i + bitmapHeight > bottom or j + bitmapWidth > right;
            optional<IntRect> sourceRect = truncate ? IntRect{ min(bitmapWidth, right - j), min(bitmapHeight, bottom - i) } : optional<IntRect>{};
            drawBitmap(bitmap, j, i, flip, false, sourceRect);
        }
    }
}

void Graphics::drawText(const void* text, int len, PDStringEncoding encoding, int x, int y, Font font) {
    ZoneScoped;
    // Todo: Support encodings
    // Todo: Kerning
    const char *string = (const char *) text;
    auto &context = getCurrentDisplayContext();
    if (!font)
        font = (context.font ? context.font : getSystemFont()).get();
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

void Graphics::fillPolygon(int32 *coords, int32 points, LCDColor color, LCDPolygonFillRule fillType) {

}

void Graphics::drawPolygon(int32 *coords, int32 points, LCDColor color) {
    if (!points)
        return;
    for (int i = 0; i < points - 1; i++)
        drawLine(coords[i * 2 + 0], coords[i * 2 + 1], coords[i * 2 + 2], coords[i * 2 + 3], getCurrentDisplayContext().lineWidth, color);
    drawLine(coords[0], coords[1], coords[points * 2 - 2], coords[points * 2 - 1], getCurrentDisplayContext().lineWidth, color);
}

void Graphics::drawScaledBitmap(Bitmap bitmap, int x, int y, float xScale, float yScale) {
    // Todo: Making a copy each time is probably not particularly efficient
    BitmapRef ref = scaleBitmap(bitmap, xScale, yScale);
    drawBitmap(ref.get(), x, y, GraphicsFlip::Unflipped);
}

Bitmap Graphics::scaleBitmap(Bitmap bitmap, float xScale, float yScale) {
    // Todo: Should any filtering be done?
    float xInv = 1 / xScale, yInv = 1 / yScale;
    int width = (int)((float)bitmap->width * xScale), height = (int)((float)bitmap->height * yScale);
    auto scaled = heap.construct<LCDBitmap_32>(cranked, width, height);
    if (bitmap->mask)
        scaled->mask = scaleBitmap(bitmap->mask.get(), xScale, yScale);
    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++)
            scaled->setBufferPixel(x, y, bitmap->getBufferPixel((int)((float)x * xInv), (int)((float)y * yInv)));
    return scaled;
}

void Graphics::drawRotatedBitmap(Bitmap bitmap, int x, int y, float angle, float centerX, float centerY, float xScale, float yScale) {
    // Todo: Making a copy each time is probably not particularly efficient
    BitmapRef ref = rotateBitmap(bitmap, angle, centerX, centerY, xScale, yScale);
    drawBitmap(ref.get(), x, y, GraphicsFlip::Unflipped);
}

Bitmap Graphics::rotateBitmap(Bitmap bitmap, float angle, float centerX, float centerY, float xScale, float yScale) {
    auto rotated = heap.construct<LCDBitmap_32>(cranked, bitmap->width, bitmap->height); // Actual size will be different
    if (bitmap->mask)
        rotated->mask = rotateBitmap(bitmap->mask.get(), angle, centerX, centerY, xScale, yScale);
    // Todo
    return rotated;
}

void Graphics::drawBlurredBitmap(Bitmap bitmap, int x, int y, float radius, int numPasses, DitherType type, GraphicsFlip flip, int xPhase, int yPhase) {
    BitmapRef ref = blurredBitmap(bitmap, radius, numPasses, type, false, xPhase, yPhase);
    drawBitmap(ref.get(), x, y, flip);
}

Bitmap Graphics::blurredBitmap(Bitmap bitmap, float radius, int numPasses, DitherType type, bool padEdges, int xPhase, int yPhase) {
    auto blurred = heap.construct<LCDBitmap_32>(cranked, bitmap->width, bitmap->height);
    // Todo
    return blurred;
}

void Graphics::drawFadedBitmap(Bitmap bitmap, int x, int y, float alpha, DitherType type) {
    BitmapRef ref = fadedBitmap(bitmap, alpha, type);
    drawBitmap(ref.get(), x, y, GraphicsFlip::Unflipped);
}

Bitmap Graphics::fadedBitmap(Bitmap bitmap, float alpha, DitherType type) {
    auto faded = heap.construct<LCDBitmap_32>(cranked, bitmap->width, bitmap->height);
    // Todo
    return faded;
}

Bitmap Graphics::invertedBitmap(Bitmap bitmap) {
    auto inverted = heap.construct<LCDBitmap_32>(cranked, bitmap->width, bitmap->height);
    for (int i = 0; i < (int)bitmap->data.size(); i++)
        inverted->data[i] = ~bitmap->data[i]; // Affects row padding bits, which doesn't matter
    inverted->mask = bitmap->mask ? heap.construct<LCDBitmap_32>(*bitmap->mask) : nullptr;
    return inverted;
}

Bitmap Graphics::blendedBitmap(Bitmap bitmap, Bitmap other, float alpha, DitherType type) {
    auto blended = heap.construct<LCDBitmap_32>(cranked, bitmap->width, bitmap->height); // Size is probably either intersection or bitmap's
    // Todo
    return blended;
}

Bitmap Graphics::vcrPauseFilteredBitmap(Bitmap bitmap) {
    auto filtered = heap.construct<LCDBitmap_32>(cranked, bitmap->width, bitmap->height);
    // Todo
    return filtered;
}

void Graphics::drawTileMap(TileMap tilemap, int x, int y, bool ignoreOffset, optional<IntRect> sourceRect) {
    // Todo
}

bool Graphics::checkBitmapMaskCollision(Bitmap bitmap1, int x1, int y1, GraphicsFlip flip1, Bitmap bitmap2, int x2, int y2, GraphicsFlip flip2, IntRect rect) {
    // Todo: This could be optimized with bitwise operations
    IntRect rect1 = { x1, y1, bitmap1->width, bitmap1->height };
    IntRect rect2 = { x2, y2, bitmap2->width, bitmap2->height };
    bool flip1X = (int)flip1 & 0b01, flip1Y = (int)flip1 & 0b10;
    bool flip2X = (int)flip2 & 0b01, flip2Y = (int)flip2 & 0b10;
    const IntRect intersection = rect1.intersection(rect2).intersection(rect);
    if (!intersection)
        return false;
    if (!bitmap1->mask or !bitmap2->mask)
        return true;
    for (int y = intersection.pos.y; y < intersection.pos.y + intersection.size.y; y++) {
        int t1y = flip1Y ? bitmap1->height - 1 - (y - y1) : y - y1;
        int t2y = flip2Y ? bitmap2->height - 1 - (y - y2) : y - y2;
        for (int x = intersection.pos.x; x < intersection.pos.x + intersection.size.x; x++) {
            int t1x = flip1X ? bitmap1->width - 1 - (x - x1) : x - x1;
            int t2x = flip2X ? bitmap2->width - 1 - (x - x2) : x - x2;
            if (bitmap1->mask->getBufferPixel(t1x, t1y) and bitmap2->mask->getBufferPixel(t2x, t2y))
                return true;
        }
    }
    return false;
}

void Graphics::updateSprites() {
    // Todo: Supposed to be updated in sorted order?
    for (auto &sprite : vector(spriteDrawList)) { // Guard against modifications in callback
        sprite->update();
    }
}

void Graphics::drawSprites() {
    auto &context = getCurrentDisplayContext();
    context.bitmap->clear(context.backgroundColor);
    // Todo: Dirty rect support
    auto sprites = vector(spriteDrawList);
    stable_sort(sprites.begin(), sprites.end(), [](const SpriteRef &a, const SpriteRef &b) { return a->zIndex < b->zIndex; }); // Todo: Does this need to be a stable sort within same Z layer?
    for (auto &sprite : sprites) {
        sprite->draw();
    }
}

void Graphics::addSpriteToDrawList(Sprite sprite) {
    spriteDrawList.emplace_back(sprite);
    cranked.bump.updateSprite(sprite);
    sprite->inDrawList = true;
}

void Graphics::removeSpriteFromDrawList(Sprite sprite) {
    eraseByEquivalentValue(spriteDrawList, sprite);
    cranked.bump.removeSprite(sprite);
    sprite->inDrawList = false;
}

void Graphics::clearSpriteDrawList() {
    for (auto &sprite : spriteDrawList) {
        cranked.bump.removeSprite(sprite.get());
        sprite->inDrawList = false;
    }
    spriteDrawList.clear();
}

BitmapTable Graphics::getBitmapTable(const string &path) {
    auto bitmapTable = cranked.rom->getImageTable(path);
    auto table = heap.construct<LCDBitmapTable_32>(cranked, bitmapTable.cellsPerRow);
    table->bitmaps.reserve(bitmapTable.cells.size());
    for (auto &bitmap : bitmapTable.cells)
        table->bitmaps.emplace_back(getImage(bitmap));
    return table;
}

VideoPlayer Graphics::getVideoPlayer(const string &path) {
    auto video = cranked.rom->getVideo(path);
    auto stride = (int) ceil((float) video.width / 8);
    auto player = heap.construct<LCDVideoPlayer_32>(cranked, video.framerate, IntVec2{ video.width, video.height });
    for (auto data : video.frames) {
        auto bitmap = heap.construct<LCDBitmap_32>(cranked, video.width, video.height);
        for (int i = 0; i < video.height; i++)
            for (int j = 0; j < video.width; j++)
                if (data[i * video.width + j])
                    bitmap->data[stride * i + j / 8] |= 0x80 >> (j % 8);
        player->frames.emplace_back(bitmap);
    }
    return player;
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

Font Graphics::getSystemFont(PDFontVariant variant) {
    return systemFonts[(int)variant].get();
}

void Graphics::init() {
    frameBuffer = createBitmap(DISPLAY_BUFFER_WIDTH, DISPLAY_HEIGHT);
    previousFrameBuffer = createBitmap(DISPLAY_BUFFER_WIDTH, DISPLAY_HEIGHT);
    frameBufferContext = DisplayContext(frameBuffer.get());
    for (int i = 0; i < 3; i++)
        systemFonts[i] = getFont(systemFontSources[i]);
}

void Graphics::reset() {
    frameBuffer.reset();
    previousFrameBuffer.reset();
    for (auto &font : systemFonts)
        font.reset();

    for (auto &context : displayContextStack)
        context.reset();
    displayContextStack.clear();

    frameBufferContext.reset();

    for (SpriteRef &sprite : vector(spriteDrawList))
        sprite.reset();
    spriteDrawList.clear();

    allocatedSprites.clear();

    memset(displayBufferRGBA, 0, sizeof(displayBufferRGBA));
    displayOffset = {};
    displayScale = 1;
    displayInverted = false;
    displayFlippedX = displayFlippedY = false;
    displayMosaic = {};
    framerate = 30;
}

void Graphics::update() {
    ZoneScoped;
    frameBufferContext.clipRect = {0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT};
    // Todo: Is this supposed to be done here?
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

    FrameImage(displayBufferRGBA, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0, false);
}
