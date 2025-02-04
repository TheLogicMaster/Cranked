#include "Graphics.hpp"
#include "Cranked.hpp"

using namespace cranked;

Graphics::Graphics(Cranked &cranked)
        : cranked(cranked), heap(cranked.heap), systemFontSources(Rom::readSystemFont("Asheville-Sans-14-Light.pft"), Rom::readSystemFont("Asheville-Sans-14-Bold.pft"), Rom::readSystemFont("Asheville-Sans-14-Light-Oblique.pft")) {}

GrayscaleImage::GrayscaleImage(Bitmap bitmap, float alpha) : GrayscaleImage(bitmap->width, bitmap->height) {
    for (int y = 0; y < bitmap->height; y++) {
        for (int x = 0; x < bitmap->width; x++) {
            if (bitmap->getBufferPixel(x, y))
                data[y][x] = alpha;
        }
    }
}

void GrayscaleImage::copyInto(Bitmap bitmap, float threshold) const {
    if (width() != bitmap->width or height() != bitmap->height)
        throw CrankedError("Image size mismatch");
    for (int y = 0; y < bitmap->height; y++) {
        for (int x = 0; x < bitmap->width; x++) {
            bitmap->setBufferPixel(x, y, data[y][x] >= threshold);
        }
    }
}

void GrayscaleImage::diagonalLines(Bitmap bitmap, int xPhase, int yPhase) const {
    for (int y = 0; y < bitmap->height; y++)
        for (int x = 0; x < bitmap->width; x++)
            bitmap->setBufferPixel(x, y, data[y][x] >= (float)((x + y + xPhase + yPhase) % 4) / 4.0f); // Todo: Is this correct for phase values?
}

void GrayscaleImage::verticalLines(Bitmap bitmap, int xPhase) const {
    // Todo: This works slightly differently than official implementation (Probably rounding)
    for (int x = 0; x < bitmap->width; x++) {
        float threshold = (float)((x + xPhase) % 4) / 4.0f;
        for (int y = 0; y < bitmap->height; y++)
            bitmap->setBufferPixel(x, y, data[y][x] >= threshold);
    }
}

void GrayscaleImage::horizontalLines(Bitmap bitmap, int yPhase) const {
    // Todo: This works slightly differently than official implementation (Probably rounding)
    for (int y = 0; y < bitmap->height; y++) {
        float threshold = (float)((y + yPhase) % 4) / 4.0f;
        for (int x = 0; x < bitmap->width; x++)
            bitmap->setBufferPixel(x, y, data[y][x] >= threshold);
    }
}

void GrayscaleImage::screenDoor(Bitmap bitmap, int xPhase, int yPhase) const {
    // Todo: This works differently than official implementation
    for (int y = 0; y < bitmap->height; y++)
        for (int x = 0; x < bitmap->width; x++)
            bitmap->setBufferPixel(x, y, data[y][x] >= (float)((x + xPhase) % 2 + (y + yPhase) % 2) / 4.0f);
}

void GrayscaleImage::bayer2x2(Bitmap bitmap, int xPhase, int yPhase) const {
    constexpr BayerTable<2> table{};
    for (int y = 0; y < height(); y++)
        for (int x = 0; x < width(); x++)
            bitmap->setBufferPixel(x, y, data[y][x] >= table.get(x + xPhase, y + yPhase));
}

void GrayscaleImage::bayer4x4(Bitmap bitmap, int xPhase, int yPhase) const {
    constexpr BayerTable<4> table{};
    for (int y = 0; y < height(); y++)
        for (int x = 0; x < width(); x++)
            bitmap->setBufferPixel(x, y, data[y][x] >= table.get(x + xPhase, y + yPhase));
}

void GrayscaleImage::bayer8x8(Bitmap bitmap, int xPhase, int yPhase) const {
    constexpr BayerTable<8> table{};
    for (int y = 0; y < height(); y++)
        for (int x = 0; x < width(); x++)
            bitmap->setBufferPixel(x, y, data[y][x] >= table.get(x + xPhase, y + yPhase));
}

void GrayscaleImage::floydSteinberg(Bitmap bitmap) {
    for (int y = 0; y < height(); y++) {
        for (int x = 0; x < width(); x++) {
            float old = data[y][x];
            bool val = old > 0.5f;
            auto newVal = (float)val;
            bitmap->setBufferPixel(x, y, val);
            float error = old - newVal;
            diffuseError(x + 1, y, error, 7 / 16.0f);
            diffuseError(x - 1, y + 1, error, 3 / 16.0f);
            diffuseError(x + 0, y + 1, error, 5 / 16.0f);
            diffuseError(x + 1, y + 1, error, 1 / 16.0f);
        }
    }
}

void GrayscaleImage::burkes(Bitmap bitmap) {
    for (int y = 0; y < height(); y++) {
        for (int x = 0; x < width(); x++) {
            float old = data[y][x];
            bool val = old > 0.5f;
            auto newVal = (float)val;
            bitmap->setBufferPixel(x, y, val);
            float error = old - newVal;
            diffuseError(x + 1, y, error, 8 / 32.0f);
            diffuseError(x + 2, y, error, 4 / 32.0f);
            diffuseError(x - 2, y + 1, error, 2 / 32.0f);
            diffuseError(x - 1, y + 1, error, 4 / 32.0f);
            diffuseError(x + 0, y + 1, error, 8 / 32.0f);
            diffuseError(x + 1, y + 1, error, 4 / 32.0f);
            diffuseError(x + 2, y + 1, error, 2 / 32.0f);
        }
    }
}

void GrayscaleImage::atkinson(Bitmap bitmap) {
    for (int y = 0; y < height(); y++) {
        for (int x = 0; x < width(); x++) {
            float old = data[y][x];
            bool val = old > 0.5f;
            auto newVal = (float)val;
            bitmap->setBufferPixel(x, y, val);
            float error = old - newVal;
            diffuseError(x + 1, y, error, 1 / 8.0f);
            diffuseError(x + 2, y, error, 1 / 8.0f);
            diffuseError(x - 1, y + 1, error, 1 / 8.0f);
            diffuseError(x + 0, y + 1, error, 1 / 8.0f);
            diffuseError(x + 1, y + 1, error, 1 / 8.0f);
            diffuseError(x + 0, y + 2, error, 1 / 8.0f);
        }
    }
}

void GrayscaleImage::dither(Bitmap bitmap, DitherType type, int xPhase = 0, int yPhase = 0) {
    switch (type) {
        case DitherType::DiagonalLine:
            diagonalLines(bitmap, xPhase, yPhase);
        break;
        case DitherType::VerticalLine:
            verticalLines(bitmap, xPhase);
        break;
        case DitherType::HorizontalLine:
            horizontalLines(bitmap, yPhase);
        break;
        case DitherType::Screen:
            screenDoor(bitmap, xPhase, yPhase);
        break;
        case DitherType::Bayer2x2:
            bayer2x2(bitmap, xPhase, yPhase);
        break;
        case DitherType::Bayer4x4:
            bayer4x4(bitmap, xPhase, yPhase);
        break;
        case DitherType::Bayer8x8:
            bayer8x8(bitmap, xPhase, yPhase);
        break;
        case DitherType::FloydSteinberg:
            floydSteinberg(bitmap);
        break;
        case DitherType::Burkes:
            burkes(bitmap);
        break;
        case DitherType::Atkinson:
            atkinson(bitmap);
        break;
        default:
        case DitherType::None:
            break;
    }
}

LCDVideoPlayer_32::LCDVideoPlayer_32(Cranked &cranked, float frameRate, IntVec2 size) : NativeResource(cranked, ResourceType::VideoPlayer, this), frameRate(frameRate), size(size) {}

LCDBitmap_32::LCDBitmap_32(Cranked &cranked, int width, int height)
        : NativeResource(cranked, ResourceType::Bitmap, this), width(width), height(height), data(vheap_vector(ceilDiv(width, 8) * height, cranked.heap.allocator<uint8>())), mask(nullptr) {}

LCDBitmap_32::LCDBitmap_32(const LCDBitmap_32 &other)
        : NativeResource(other.cranked, ResourceType::Bitmap, this), width(other.width), height(other.height), data(other.data), mask(other.mask ? cranked.heap.construct<LCDBitmap_32>(*other.mask) : nullptr) {}

LCDBitmap_32& LCDBitmap_32::operator=(const LCDBitmap_32 &other) {
    if (&other == this)
        return *this;
    width = other.width;
    height = other.height;
    data = other.data;
    mask = other.mask ? cranked.heap.construct<LCDBitmap_32>(*other.mask) : nullptr;
    return *this;
}

LCDFontGlyph_32::LCDFontGlyph_32(Bitmap bitmap, int advance, const map<int, int8> &kerningTable)
        : NativeResource(bitmap->cranked, ResourceType::FontGlyph, this), advance(advance), kerningTable(kerningTable), bitmap(bitmap) {}

LCDFontPage_32::LCDFontPage_32(Cranked &cranked) : NativeResource(cranked, ResourceType::FontPage, this) {}

LCDFont_32::LCDFont_32(Cranked &cranked, int tracking, int glyphWidth, int glyphHeight)
        : NativeResource(cranked, ResourceType::Font, this), tracking(tracking), leading(), glyphWidth(glyphWidth), glyphHeight(glyphHeight) {}

LCDBitmapTable_32::LCDBitmapTable_32(Cranked &cranked, int cellsPerRow) : NativeResource(cranked, ResourceType::BitmapTable, this), cellsPerRow(cellsPerRow) {}

LCDBitmapTable_32& LCDBitmapTable_32::operator=(const LCDBitmapTable_32 &other) {
    if (&other == this)
        return *this;
    bitmaps = other.bitmaps;
    cellsPerRow = other.cellsPerRow;
    return *this;
}

LCDTileMap_32::LCDTileMap_32(Cranked &cranked) : NativeResource(cranked, ResourceType::TileMap, this) {}

[[nodiscard]] IntVec2 LCDTileMap_32::getCellSize() const {
    if (table and !table->bitmaps.empty() and table->bitmaps[0])
        return { table->bitmaps[0]->width, table->bitmaps[0]->height };
    return {};
}

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

void LCDBitmap_32::drawPixel(int x, int y, const Color &color) {
    if (x < 0 or x >= width or y < 0 or y >= height)
        return;
    // Todo: Is pattern offset affected by draw offset?
    const uint8 *pattern{};
    LCDSolidColor c{};
    if (holds_alternative<LCDColor>(color)) {
        if (cref_t val = get<LCDColor>(color).pattern; val >= 4)
            pattern = (uint8 *)cranked.fromVirtualAddress(val);
        else
            c = (LCDSolidColor)val;
    } else if (holds_alternative<PatternColor>(color)) {
        pattern = get<PatternColor>(color).pattern.data();
    } else if (holds_alternative<DitherColor>(color)) {
        // Todo
    }
    if (pattern) {
        auto row = y % 8;
        auto column = x % 8;
        auto word = pattern[row];
        auto maskWord = pattern[row + 8];
        if (maskWord & 0x80 >> column)
            c = word & 0x80 >> column ? LCDSolidColor::White : LCDSolidColor::Black;
        else
            c = LCDSolidColor::Clear;
    }

    // Todo: Is this even right?
    if (mask)
        mask->setBufferPixel(x, y, c != LCDSolidColor::Clear);

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
void LCDBitmap_32::clear(const Color &color) {
    if (auto c = get_if<LCDColor>(&color); c->color == LCDSolidColor::Clear) {
        if (!mask)
            mask = cranked.heap.construct<LCDBitmap_32>(cranked, width, height);
        else {
            for (int y = 0; y < height; y++)
                for (int x = 0; x < width; x++)
                    mask->setBufferPixel(x, y, false);
        }
    }
    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++)
            drawPixel(x, y, color);
}

void LCDBitmap_32::drawVerticalLine(int x, int y1, int y2, const Color &color) {
    for (int y = y1; y < y2; y++)
        drawPixel(x, y, color);
}

void LCDBitmap_32::drawHorizontalLine(int x1, int x2, int y, const Color &color) {
    for (int x = x1; x < x2; x++)
        drawPixel(x, y, color);
}

void LCDBitmap_32::drawRect(int x, int y, int width, int height, const Color &color) {
    // Todo: A bit off with line thickness, needs to extend to meet perpendicular lines
    drawVerticalLine(x, y, y + height - 1, color);
    drawVerticalLine(x + width - 1, y, y + height - 1, color);
    drawHorizontalLine(x + 1, x + width - 2, y, color);
    drawHorizontalLine(x + 1, x + width - 2, y + height - 1, color);
}

void LCDBitmap_32::fillRect(int x, int y, int width, int height, const Color &color) {
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
            drawPixel(x + j, y + i, color);
}

// Todo: Can be more efficient
void LCDBitmap_32::drawBitmap(Bitmap bitmap, int x, int y, BitmapDrawMode mode, GraphicsFlip flip, optional<IntRect> sourceRect) {
    if (!bitmap)
        return;
    bool flipY = flip == LCDBitmapFlip::FlipY or flip == LCDBitmapFlip::FlipXY;
    bool flipX = flip == LCDBitmapFlip::FlipX or flip == LCDBitmapFlip::FlipXY;
    IntRect bounds = sourceRect ? *sourceRect : IntRect{ {}, bitmap->width, bitmap->height };
    int endX = bounds.pos.x + bounds.size.x;
    int endY = bounds.pos.y + bounds.size.y;
    for (int i = bounds.pos.y; i < endY; i++)
        for (int j = bounds.pos.y; j < endX; j++) {
            auto pixel = bitmap->getPixel(j, i);
            int pixelX = x + (flipX ? bounds.size.x - 1 - j : j);
            int pixelY = y + (flipY ? bounds.size.y - 1 - i : i);
            if (pixel == LCDSolidColor::Clear)
                continue;
            switch (mode) {
                case LCDBitmapDrawMode::Copy:
                    drawPixel(pixelX, pixelY, pixel);
                    break;
                case LCDBitmapDrawMode::WhiteTransparent:
                    if (pixel != LCDSolidColor::White)
                        drawPixel(pixelX, pixelY, pixel);
                    break;
                case LCDBitmapDrawMode::BlackTransparent:
                    if (pixel != LCDSolidColor::Black)
                        drawPixel(pixelX, pixelY, pixel);
                    break;
                case LCDBitmapDrawMode::FillWhite:
                    drawPixel(pixelX, pixelY, LCDSolidColor::White);
                    break;
                case LCDBitmapDrawMode::FillBlack:
                    drawPixel(pixelX, pixelY, LCDSolidColor::Black);
                    break;
                case LCDBitmapDrawMode::XOR:
                    drawPixel(pixelX, pixelY, getPixel(pixelX, pixelY) == pixel ? LCDSolidColor::Black : LCDSolidColor::White);
                    break;
                case LCDBitmapDrawMode::NXOR:
                    drawPixel(pixelX, pixelY, getPixel(pixelX, pixelY) != pixel ? LCDSolidColor::Black : LCDSolidColor::White);
                    break;
                case LCDBitmapDrawMode::Inverted:
                    drawPixel(pixelX, pixelY, pixel == LCDSolidColor::White ? LCDSolidColor::Black : LCDSolidColor::White);
                    break;
            }
        }
}

IntRect LCDBitmap_32::drawText(int x, int y, string_view text, const FontFamily &fonts, Font font, StringEncoding encoding, IntVec2 size, TextWrap wrap, TextAlign align, BitmapDrawMode mode, int tracking, int leadingAdjust, int charCount) {
    int maxX = x, maxY = y;
    for (auto &[glyph, bounds, character] : cranked.graphics.layoutText(x, y, text, fonts, font, encoding, size, wrap, align, tracking, leadingAdjust, charCount)) {
        drawBitmap(glyph->bitmap, bounds.pos.x, bounds.pos.y, mode);
        maxX = max(maxX, bounds.pos.x + bounds.size.x);
        maxY = max(maxY, bounds.pos.y + bounds.size.y);
    }
    return { {x, y}, { maxX - x, maxY - y } };
}

[[nodiscard]] vector<uint8> LCDBitmap_32::toRGB() const {
    vector<uint8_t> frame(width * height);
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
            ((uint32 *)frame.data())[i * width + j] = getPixel(j, i) == LCDSolidColor::White ? cranked.graphics.displayBufferOnColor : cranked.graphics.displayBufferOffColor;
    return frame;
}

LCDSprite_32::LCDSprite_32(Cranked &cranked) : NativeResource(cranked, ResourceType::Sprite, this) {
    cranked.graphics.allocatedSprites.emplace(this);
}

LCDSprite_32::~LCDSprite_32() {
    cranked.luaEngine.clearResourceValue(this);
    cranked.graphics.allocatedSprites.erase(this);
}

void LCDSprite_32::updateCollisionWorld() {
    cranked.bump.updateSprite(this);
}

void LCDSprite_32::setImage(Bitmap bitmap, LCDBitmapFlip bitmapFlip, float xScale, float yScale) {
    // Todo: Figure out how this relates to stencil
    if (!dontRedrawOnImageChange)
        dirty = true;
    if (bitmap and !bitmap->mask)
        opaque = true;
    image = bitmap;
    flip = bitmapFlip;
    scale = { xScale, yScale };
    if (bitmap)
        setSize({ (float)bitmap->width, (float)bitmap->height });
}

void LCDSprite_32::setTileMap(TileMap tileMap) {
    if (!dontRedrawOnImageChange)
        dirty = true;
    opaque = true; // Todo: Opaque?
    image = nullptr;
    flip = GraphicsFlip::Unflipped;
    scale = { 1, 1 };
    if (tileMap)
        setSize(IntVec2{ tileMap->width * tileMap->getCellSize().x, tileMap->getHeight() * tileMap->getCellSize().x }.as<float>());
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
    value.val.setFloatField("width", bounds.size.x);
    value.val.setFloatField("height", bounds.size.y);
}

void LCDSprite_32::draw() {
    if (!visible or !inDrawList)
        return;
    Vec2 drawPos = bounds.pos;
    Rect dirtyRect = { -bounds.pos, { DISPLAY_WIDTH, DISPLAY_HEIGHT } }; // Todo (Pass in union of all dirty rects as parameter?)

    auto &context = cranked.graphics.getContext();
    auto drawOffset = context.drawOffset;
    ScopeExitHelper cleanupDrawOffset{ [=, &context]{ context.drawOffset = drawOffset; } };
    context.drawOffset += drawPos.as<int32>();

    if (image) {
        // Todo: Respect scaling, rotation, clip rect (Probably adding clip rect support to drawing function in addition to context clip rect, or like drawOffset)
        cranked.graphics.drawBitmap(image, 0, 0, flip, ignoresDrawOffset);
    } else if (tileMap)
        cranked.graphics.drawTileMap(tileMap, 0, 0, ignoresDrawOffset);
    else if (drawFunction)
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
    else if (cranked.luaEngine.isLoaded()) {
        if (LuaValGuard value{ cranked.luaEngine.getResourceValue(this) }; value.val.isTable()) {
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

DisplayContext::DisplayContext(Bitmap bitmap, FontFamily fonts) : bitmap(bitmap), fonts(std::move(fonts)) {}

DisplayContext &Graphics::pushContext(Bitmap target) {
    // Todo: Should this copy the existing context? Do fonts always default to system fonts?
    auto &ctx = displayContextStack.emplace_back(getContext());
    if (target) {
        ctx.bitmap = target;
        ctx.clipRect = {0, 0, target->width, target->height };
    }
    return ctx;
}

void Graphics::popContext() {
    if (displayContextStack.empty())
        return;
    displayContextStack.pop_back();
}

void Graphics::drawPixel(int x, int y, const Color &color, bool ignoreOffset) {
    ZoneScoped;
    auto &context = getContext();
    auto pos = ignoreOffset ? IntVec2{x, y} : context.drawOffset + IntVec2{x, y};
    if (!context.clipRect.contains(pos))
        return;
    if (context.stencil) {
        int stencilX = pos.x, stencilY = pos.y;
        if (stencilX < 0 or stencilY < 0) return;
        if (context.stencilTiled) {
            stencilX %= context.stencil->width;
            stencilY %= context.stencil->height;
        } else if (x >= context.stencil->width or y >= context.stencil->height)
            return;
        if (context.stencil->getPixel(stencilX, stencilY) == LCDSolidColor::Black)
            return;
    }
    context.getTargetBitmap()->drawPixel(pos.x, pos.y, color);
}

LCDSolidColor Graphics::getPixel(int x, int y, bool ignoreOffset) {
    auto &context = getContext();
    auto pos = ignoreOffset ? IntVec2{x, y} : context.drawOffset + IntVec2{x, y};
    return context.getTargetBitmap()->getPixel(pos.x, pos.y);
}

// Todo: Specialized horizontal/vertical variants for efficiency
void Graphics::drawLine(int x1, int y1, int x2, int y2, const Color &color) {
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

void Graphics::drawLine(int x1, int y1, int x2, int y2, int width, const Color &color) {
    // Todo: Line caps
    // Todo: Draws slightly differently from official implementation, steps differently
    // Todo: A DDA algorithm for thickness would be better
    Vec2 tangent = IntVec2{ y2 - y1, x2 - x1 }.as<float>().normalized();
    if (tangent.isInvalid())
        tangent = {};
    int steps = (int)ceilf((float)width / 2.0f);
    for (int i = 0; i < steps; i++) {
        Vec2 offset = (tangent * i);
        int xOff = (int)round(offset.x);
        int yOff = (int)round(offset.y);
        drawLine(x1 + xOff, y1 + yOff, x2 + xOff, y2 + yOff, color);
        if (i > 0 and (steps % 2 == 0 or i < width - 1))
            drawLine(x1 - xOff, y1 - yOff, x2 - xOff, y2 - yOff, color);
    }
}

void Graphics::drawRect(int x, int y, int width, int height, const Color &color) {
    auto &context = getContext();
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

void Graphics::fillRect(int x, int y, int width, int height, const Color &color) {
    for (int i = 0; i < height; i++)
        drawLine(x, y + i, x + width - 1, y + i, 1, color);
}

void Graphics::drawRoundRect(int x, int y, int width, int height, int radius, const Color &color) {
    // Todo: Support line thickness

    radius = min(radius, min(width, height) / 2);

    auto drawCorner = [&](int xOff, int yOff, int corner) {
        int f = 1 - radius;
        int dx = 1, dy = -2 * radius;
        int cx = 0, cy = radius;
        int x0 = x + xOff, y0 = y + yOff;
        while (cx < cy) {
            if (f >= 0) {
                cy--;
                dy += 2;
                f += dy;
            }
            cx++;
            dx += 2;
            f += dx;
            if (corner == 2) {
                drawPixel(x0 + cx, y0 + cy, color);
                drawPixel(x0 + cy, y0 + cx, color);
            } else if (corner == 1) {
                drawPixel(x0 + cx, y0 - cy, color);
                drawPixel(x0 + cy, y0 - cx, color);
            } else if (corner == 3) {
                drawPixel(x0 - cx, y0 + cy, color);
                drawPixel(x0 - cy, y0 + cx, color);
            } else if (corner == 0) {
                drawPixel(x0 - cx, y0 - cy, color);
                drawPixel(x0 - cy, y0 - cx, color);
            }
        }
    };

    drawLine(x + radius, y, x + width - radius, y, color);
    drawLine(x + radius, y + height - 1, x + width - radius, y + height - 1, color);
    drawLine(x, y + radius, x, y + height - radius, color);
    drawLine(x + width - 1, y + radius, x + width - 1, y + height - radius, color);

    drawCorner(radius, radius, 0);
    drawCorner(width - radius - 1, radius, 1);
    drawCorner(width - radius - 1, height - radius - 1, 2);
    drawCorner(radius, height - radius - 1, 3);
}

void Graphics::fillRoundRect(int x, int y, int width, int height, int radius, const Color &color) {
    radius = min(radius, min(width, height) / 2);

    auto fillCorners = [&](int x0, int y0, int corner, int delta) {
        int f = 1 - radius;
        int dx = 1, dy = -2 * radius;
        int cx = 0, cy = radius;
        int px = cx, py = cy;
        while (cx < cy) {
            if (f >= 0) {
                cy--;
                dy += 2;
                f += dy;
            }
            cx++;
            dx += 2;
            f += dx;
            if (corner == 0) {
                drawLine(x0 + cx, y0 - cy, x0 + cx, y0 + cy + delta, color);
                drawLine(x0 + py, y0 - px, x0 + py, y0 + px + delta, color);
            } else {
                drawLine(x0 - cx, y0 - cy, x0 - cx, y0 + cy + delta, color);
                drawLine(x0 - py, y0 - px, x0 - py, y0 + px + delta, color);
            }
            py = cy;
            px = cx;
        }
    };

    fillRect(x + radius, y, width - 2 * radius, height, color);
    fillCorners(x + width - radius - 1, y + radius, 0, height - 2 * radius - 1);
    fillCorners(x + radius, y + radius, 1, height - 2 * radius - 1);
}

void Graphics::drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, const Color &color) {
    auto lineWidth = getContext().lineWidth;
    drawLine(x1, y1, x2, y2, lineWidth, color);
    drawLine(x1, y1, x3, y3, lineWidth, color);
    drawLine(x2, y2, x3, y3, lineWidth, color);
}

void Graphics::drawBitmap(Bitmap bitmap, int x, int y, LCDBitmapFlip flip, bool ignoreOffset, optional<IntRect> sourceRect) {
    ZoneScoped;
    // Todo: This could be heavily optimized with bitwise operations
    auto &context = getContext();
    auto mode = context.bitmapDrawMode;
    bool flipY = flip == LCDBitmapFlip::FlipY or flip == LCDBitmapFlip::FlipXY;
    bool flipX = flip == LCDBitmapFlip::FlipX or flip == LCDBitmapFlip::FlipXY;
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

vector<GlyphInstance> Graphics::layoutText(int x, int y, string_view text, const FontFamily &fonts, Font font, StringEncoding encoding, IntVec2 size, TextWrap wrap, TextAlign align, int tracking, int leadingAdjust, int charCount) {
    auto &ctx = getContext();
    const bool useStyling = !font;
    const int wrapX = size.x ? x + size.x : 0; // Todo: Wrap at screen if bounds not set?
    const int clipY = size.y ? y + size.y : 0; // Todo: Set clip rect for bitmap drawing rather than chopping off text which doesn't fit?

    u32string characters;
    characters.reserve(text.size());
    switch (encoding) {
        case PDStringEncoding::UFT8:
            unicode::utf8::decode(text.data(), text.size(), characters);
            break;
        case PDStringEncoding::LE16Bit:
            unicode::utf16::decode((char16_t *)text.data(), text.size() / 2, characters);
            break;
        case PDStringEncoding::ASCII:
            default:
                for (char c : text)
                    characters.push_back(c);
            break;
    }
    if (charCount > 0 and (int)characters.size() > charCount)
        characters.resize(charCount);

    vector<GlyphInstance> glyphs;

    int currentX = x;
    int currentY = y;
    char32_t prevChar{};
    FontVariant variant{};
    Font currentFont = font ? font : fonts.regular.get();
    vector<GlyphInstance> line;

    auto dumpLine = [&] {
        if (!line.empty()) {
            for (int i = (int)line.size() - 1; i >= 0; i--) {
                if (auto &[glyph, bounds, character] = line[i]; wrapX > 0 and bounds.pos.x + glyph->advance > wrapX)
                    line.erase(line.begin() + i);
                else
                    break;
            }
            int adjust = align == TextAlign::Left ? 0 : align == TextAlign::Center ? line.back().bounds.pos.x - x : wrapX <= 0 ? 0 : wrapX - line.back().bounds.pos.x;
            for (auto &[glyph, position, character] : line)
                glyphs.emplace_back(glyph, position + IntVec2{ adjust, 0 }, character);
        }
        currentX = x;
        currentY += currentFont->leading + ctx.textLeading + leadingAdjust;
        line.clear();
    };

    for (char32_t c : characters) {
        char32 lastChar = prevChar;
        prevChar = c;

        if (useStyling and (c == '*' or c == '_')) {
            variant = variant != FontVariant::Normal ? FontVariant::Normal : c == '*' ? FontVariant::Bold : FontVariant::Italic;
            currentFont = fonts.getFont(variant);
            if (lastChar != c)
                continue;
        }

        if (c == '\n') {
            dumpLine();
            continue;
        }

        if (!currentFont)
            continue;

        FontGlyph glyph;
        try {
            glyph = currentFont->pages.at((int)(c / 256))->glyphs.at((int)(c % 256));
        } catch (out_of_range &) {
            continue;
        }

        int kerning = 0;
        if (auto it = glyph->kerningTable.find((int)lastChar); it != glyph->kerningTable.end()) // Todo: Is this correct or backwards?
            kerning = (short)it->second;

        IntVec2 pos{ currentX + kerning, currentY };
        int finalX = pos.x + glyph->advance + currentFont->tracking + tracking + ctx.textTracking; // Todo: Tracking from C API may be meant to replace font tracking
        if (wrapX > 0 and pos.x > x and finalX > wrapX) {
            if (wrap == TextWrap::Character) {
                dumpLine();
            } else if (wrap == TextWrap::Word) {
                vector<GlyphInstance> wrapped;
                if (!unicode::is_white_space(c)) {
                    for (int i = (int)line.size() - 1; i > 0; i--) {
                        if (unicode::is_white_space(line[i].character))
                            break;
                        line.push_back(line[i]);
                        line.pop_back();
                    }
                }
                dumpLine();
                line.insert(line.end(), wrapped.begin(), wrapped.end());
            }
        }

        if (clipY > 0 and currentY > clipY)
            break;

        line.emplace_back(glyph, IntRect{ pos, { currentFont->glyphWidth, currentFont->glyphHeight } }, c);
        currentX = finalX;
    }

    dumpLine();

    return glyphs;
}

void Graphics::drawText(int x, int y, string_view text, const FontFamily &fonts, Font font, StringEncoding encoding, IntVec2 size, TextWrap wrap, TextAlign align, int tracking, int leadingAdjust, int charCount) {
    for (auto &[glyph, bounds, character] : layoutText(x, y, text, fonts, font, encoding, size, wrap, align, tracking, leadingAdjust, charCount))
        drawBitmap(glyph->bitmap, bounds.pos.x, bounds.pos.y, LCDBitmapFlip::Unflipped);
}

IntVec2 Graphics::measureText(string_view text, const FontFamily &fonts, Font font, StringEncoding encoding, IntVec2 size, TextWrap wrap, TextAlign align, int tracking, int leadingAdjust, int charCount) {
    int maxX{}, maxY{};
    for (auto &[glyph, bounds, character] : layoutText(0, 0, text, fonts, font, encoding, size, wrap, align, tracking, leadingAdjust, charCount)) {
        maxX = max(bounds.pos.x + bounds.size.x, maxX);
        maxY = max(bounds.pos.y + bounds.size.y, maxY);
    }
    return { maxX, maxY };
}

const char *Graphics::getLocalizedText(const char *key, PDLanguage language) {
    auto &table = localizedStrings[language];
    if (auto it = table.strings.find(key); it != table.strings.end())
        return it->second.c_str();
    return nullptr;
}

void Graphics::fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, const Color &color) {
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
    for (; y <= y3; y++) {
        a = x2 + sa / dy23;
        b = x1 + sb / dy13;
        sa += dx23;
        sb += dx13;
        if (a > b)
            swap(a, b);
        drawLine(a, y, b + 1, y, 1, color);
    }
}

void Graphics::drawEllipse(int rectX, int rectY, int width, int height, int lineWidth, float startAngle, float endAngle, const Color &color, bool filled) {
    // Todo: The angle checks need normalization
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

void Graphics::fillPolygon(int32 *coords, int32 points, const Color &color, LCDPolygonFillRule fillType) {
    // Todo: Support non-zero rule mode
    if (points < 2) return;
    vector<int> nodes;
    int yMin = coords[1], yMax = coords[1];
    for (int i = 1; i < points; i++) {
        int y = coords[i * 2 + 1];
        yMin = min(yMin, y);
        yMax = max(yMax, y);
    }
    for (int y = yMin; y <= yMax; y++) {
        nodes.clear();
        int j = points - 1;
        for (int i = 0; i < points; i++) {
            int ix = coords[i * 2 + 0], iy = coords[i * 2 + 1];
            int jx = coords[j * 2 + 0], jy = coords[j * 2 + 1];
            if (iy < y and jy >= y or jy < y and iy >= y)
                nodes.push_back(ix + (y - iy) * (jx - ix) / (jy - iy));
            j = i;
        }
        sort(nodes.begin(), nodes.end());
        for (int i = 0; i < (int)nodes.size() - 1; i += 2) {
            for (int x = nodes[i]; x <= nodes[i + 1]; x++)
                drawPixel(x, y, color);
        }
    }
}

void Graphics::drawPolygon(int32 *coords, int32 points, const Color &color) {
    if (!points)
        return;
    for (int i = 0; i < points - 1; i++)
        drawLine(coords[i * 2 + 0], coords[i * 2 + 1], coords[i * 2 + 2], coords[i * 2 + 3], getContext().lineWidth, color);
    drawLine(coords[0], coords[1], coords[points * 2 - 2], coords[points * 2 - 1], getContext().lineWidth, color);
}

void Graphics::drawScaledBitmap(Bitmap bitmap, int x, int y, float xScale, float yScale) {
    // Todo: Making a copy each time is probably not particularly efficient
    BitmapRef ref = scaleBitmap(bitmap, xScale, yScale);
    drawBitmap(ref, x, y, GraphicsFlip::Unflipped);
}

Bitmap Graphics::scaleBitmap(Bitmap bitmap, float xScale, float yScale) {
    // Todo: Should any filtering be done?
    float xInv = 1 / xScale, yInv = 1 / yScale;
    int width = (int)((float)bitmap->width * xScale), height = (int)((float)bitmap->height * yScale);
    auto scaled = heap.construct<LCDBitmap_32>(cranked, width, height);
    if (bitmap->mask)
        scaled->mask = scaleBitmap(bitmap->mask, xScale, yScale);
    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++)
            scaled->setBufferPixel(x, y, bitmap->getBufferPixel((int)((float)x * xInv), (int)((float)y * yInv)));
    return scaled;
}

void Graphics::drawRotatedBitmap(Bitmap bitmap, int x, int y, float angle, float centerX, float centerY, float xScale, float yScale) {
    // Todo: Making a copy each time is probably not particularly efficient
    BitmapRef ref = rotateBitmap(bitmap, angle, centerX, centerY, xScale, yScale);
    // Todo: Image position is slightly off
    drawBitmap(ref, x - (int)(centerX * (float)ref->width), y - (int)(centerY * (float)ref->height), GraphicsFlip::Unflipped);
}

Bitmap Graphics::rotateBitmap(Bitmap bitmap, float angle, float centerX, float centerY, float xScale, float yScale) {
    // Todo: Optimizations for axis-aligned rotations
    // Todo: Rotate bitmaps using three shears method?: https://gautamnagrawal.medium.com/rotating-image-by-any-angle-shear-transformation-using-only-numpy-d28d16eb5076
    Vec2 offset{ centerX * (float)bitmap->width, centerY * (float)bitmap->height };
    // Todo: Image position is slightly off
    auto affine =
        Transform::translate(offset)
        * Transform::rotate(angle * numbers::pi_v<float> / 180)
        * Transform::translate(-offset)
        * Transform::scale(xScale, yScale);
    return transformBitmap(bitmap, affine);
}

void Graphics::drawTransformedBitmap(Bitmap bitmap, const Transform &transform, int x, int y) {
    // Todo: Making a copy each time is probably not particularly efficient
    BitmapRef ref = transformBitmap(bitmap, transform);
    drawBitmap(ref, x - ref->width / 2, y - ref->height / 2, GraphicsFlip::Unflipped); // Coordinates are centered
}

Bitmap Graphics::transformBitmap(Bitmap bitmap, const Transform &transform) {
    vector corners {
        transform * IntVec2{0, 0},
        transform * IntVec2{bitmap->width, 0},
        transform * IntVec2{0, bitmap->height},
        transform * IntVec2{bitmap->width, bitmap->height}
    };
    IntVec2 minBound = corners[0].as<int32>(), maxBound = corners[0].as<int32>();
    for (auto [x, y] : corners) {
        minBound = { min(minBound.x, x), min(minBound.y, y) };
        maxBound = { max(maxBound.x, x), max(maxBound.y, y) };
    }

    auto transformed = heap.construct<LCDBitmap_32>(cranked, maxBound.x + 1, maxBound.y + 1);
    if (bitmap->mask)
        transformed->mask = transformBitmap(bitmap->mask, transform);

    Transform inv = transform.invert();

    for (int y = minBound.y; y <= maxBound.y; y++) {
        for (int x = minBound.x; x <= maxBound.x; x++) {
            IntVec2 point{ x, y };
            IntVec2 srcPoint = inv * point;
            if (srcPoint.x < 0 or srcPoint.y < 0 or srcPoint.x >= bitmap->width or srcPoint.y >= bitmap->height)
                continue;
            transformed->setBufferPixel(x, y, bitmap->getBufferPixel(srcPoint.x, srcPoint.y));
        }
    }

    return transformed;
}

void Graphics::drawSampledBitmap(Bitmap image, int x, int y, int width, int height, float centerX, float centerY, float dxx, float dyx, float dxy, float dyy, float dx, float dy, float z, float tiltAngle, bool tile) {
    // Todo: How is this actually supposed to work? Take Z and tilt to apply perspective, apply affine, then texture?
    // Todo: Does not respect bitmap drawing modes
    Transform affine{ dxx, dxy, dx, dyy, dyx, dy };
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            IntVec2 point{ j + x, i + y };
            IntVec2 srcPoint = affine * point;
            if (tile) {
                srcPoint.x = srcPoint.x % image->width;
                if (srcPoint.x < 0)
                    srcPoint.x += image->width;
                srcPoint.y = srcPoint.y % image->height;
                if (srcPoint.y < 0)
                    srcPoint.y += image->height;
            } else if (srcPoint.x < 0 or srcPoint.y < 0 or srcPoint.x >= image->width or srcPoint.y >= image->height)
                continue;
            drawPixel(point.x, point.y, image->getBufferPixel(srcPoint.x, srcPoint.y));
        }
    }
}

void Graphics::drawBlurredBitmap(Bitmap bitmap, int x, int y, int radius, int numPasses, DitherType type, GraphicsFlip flip, int xPhase, int yPhase) {
    // Todo: Making a copy each time is probably not particularly efficient
    BitmapRef ref = blurredBitmap(bitmap, radius, numPasses, type, false, xPhase, yPhase);
    drawBitmap(ref, x, y, flip);
}

Bitmap Graphics::blurredBitmap(Bitmap bitmap, int radius, int numPasses, DitherType type, bool padEdges, int xPhase, int yPhase) {
    // Todo: Use moving window or something for performance
    // Todo: Support multiple passes and edge padding
    // Todo: No diffusion option is a blank image?
    // Todo: This is probably using the mask image incorrectly
    auto blurred = heap.construct<LCDBitmap_32>(*bitmap);
    if (bitmap->mask) {
        blurred->mask = blurredBitmap(bitmap->mask, radius, numPasses, type, padEdges, xPhase, yPhase);
        return blurred;
    }
    GrayscaleImage grayscale{ bitmap->width, bitmap->height };
    for (int y = 0; y < bitmap->height; y++)
        for (int x = 0; x < bitmap->width; x++)
            grayscale[y][x] = (float)bitmap->getBufferPixel(x, y);
    GrayscaleImage grayscaleBlurred{ bitmap->width, bitmap->height };
    for (int y = 0; y < bitmap->height; y++) {
        for (int x = 0; x < bitmap->width; x++) {
            float sum{};
            for (int i = -radius; i <= radius; i++) {
                for (int j = -radius; j <= radius; j++) {
                    int px = x + j, py = y + i;
                    if (px >= 0 and py >= 0 and px < grayscale.width() and py < grayscale.height())
                        sum += grayscale[py][px];
                }
            }
            grayscaleBlurred[y][x] = sum / (((float)radius * 2 + 1) * ((float)radius * 2 + 1));
        }
    }
    grayscaleBlurred.dither(blurred, type, xPhase, yPhase);
    return blurred;
}

void Graphics::drawFadedBitmap(Bitmap bitmap, int x, int y, float alpha, DitherType type) {
    // Todo: Making a copy each time is probably not particularly efficient
    BitmapRef ref = fadedBitmap(bitmap, alpha, type);
    drawBitmap(ref, x, y, GraphicsFlip::Unflipped);
}

Bitmap Graphics::fadedBitmap(Bitmap bitmap, float alpha, DitherType type) {
    // Todo: Supposed to act on mask if present, should do same operation to both or just mask?
    auto faded = heap.construct<LCDBitmap_32>(*bitmap);
    if (bitmap->mask) {
        faded->mask = fadedBitmap(bitmap->mask, alpha, type);
        return faded;
    }
    GrayscaleImage grayscale{ bitmap, alpha };
    grayscale.dither(faded, type);
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
    auto blended = heap.construct<LCDBitmap_32>(cranked, max(bitmap->width, other->width), max(bitmap->height, other->height));
    GrayscaleImage grayscale{ blended->width, blended->height };
    float otherAlpha = 1 - alpha;
    for (int y = 0; y < bitmap->height; y++)
        for (int x = 0; x < bitmap->width; x++)
            grayscale[y][x] = (float)bitmap->getBufferPixel(x, y) * alpha;
    for (int y = 0; y < other->height; y++)
        for (int x = 0; x < other->width; x++)
            grayscale[y][x] += (float)other->getBufferPixel(x, y) * otherAlpha;
    grayscale.dither(blended, type);
    return blended;
}

Bitmap Graphics::vcrPauseFilteredBitmap(Bitmap bitmap) {
    auto filtered = heap.construct<LCDBitmap_32>(cranked, bitmap->width, bitmap->height);
    // Todo
    return filtered;
}

void Graphics::drawTileMap(TileMap tilemap, int x, int y, bool ignoreOffset, optional<IntRect> sourceRect) {
    if (not tilemap->table)
        return;
    auto cellSize = tilemap->getCellSize();
    for (int i = 0; i < tilemap->getHeight(); i++) {
        int drawY = y + cellSize.y * i;
        for (int j = 0; j < tilemap->width; j++) {
            int index = tilemap->tiles[i * tilemap->width + j];
            if (index < 0 or index >= tilemap->table->bitmaps.size())
                continue;
            drawBitmap(tilemap->table->bitmaps[index], x + cellSize.x * j, drawY, GraphicsFlip::Unflipped, ignoreOffset); // Todo: Support sourceRect
        }
    }
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
    auto &context = getContext();
    context.bitmap->clear(LCDColor(context.backgroundColor));
    // Todo: Dirty rect support
    auto sprites = vector(spriteDrawList);
    stable_sort(sprites.begin(), sprites.end(), [](const SpriteRef &a, const SpriteRef &b) { return a->zIndex < b->zIndex; }); // Todo: Does this need to be a stable sort within same Z layer?
    for (auto &sprite : sprites) {
        sprite->draw();
    }
    spriteDirtyRects.clear();
}

void Graphics::addSpriteDirtyRect(IntRect rect) {
    spriteDirtyRects.emplace_back(rect);
}

void Graphics::addSpriteToDrawList(Sprite sprite) {
    if (containsEquivalentValue(spriteDrawList, sprite))
        return;
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
        cranked.bump.removeSprite(sprite);
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
    auto stride = ceilDiv(video.width, 8);
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
    auto stride = ceilDiv(source.width, 8);
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
            page->glyphs[glyphEntry.first] = heap.construct<LCDFontGlyph_32>(getImage(glyph.cell), glyph.advance, glyph.kerningTable);
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
    return systemFonts.getFont(variant);
}

void Graphics::init() {
    frameBuffer = createBitmap(DISPLAY_BUFFER_WIDTH, DISPLAY_HEIGHT);
    previousFrameBuffer = createBitmap(DISPLAY_BUFFER_WIDTH, DISPLAY_HEIGHT);
    for (int i = 0; i < 3; i++)
        systemFonts.setFont((PDFontVariant) i, getFont(systemFontSources[i]));
    frameBufferContext = DisplayContext(frameBuffer, systemFonts);
    localizedStrings = cranked.rom->getStrings();
}

void Graphics::reset() {
    frameBuffer.reset();
    previousFrameBuffer.reset();
    systemFonts.reset();
    localizedStrings.clear();

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
    displayContextStack.clear();
    frameBufferContext.focusedImage = nullptr;
    frameBufferContext.clipRect = {0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT};
    frameBufferContext.stencil = nullptr;
}

void Graphics::flushDisplayBuffer() {
    ZoneScoped;
    // Todo: Mosaic (Could it be implemented as a box blur? Compare to blur effect)

    for (int i = 0; i < DISPLAY_HEIGHT; i++) {
        int y = (displayFlippedY ? DISPLAY_HEIGHT - 1 - i : i) - displayOffset.y;
        if (y >= 0 and y < DISPLAY_HEIGHT) // Check before truncating to avoid affecting display offset
            y /= displayScale;
        for (int j = 0; j < DISPLAY_WIDTH; j++) {
            int x = (displayFlippedX ? DISPLAY_WIDTH - 1 - j : j) - displayOffset.x;
            if (x >= 0 and x < DISPLAY_WIDTH)
                x /= displayScale;
            auto color = frameBuffer->getBufferPixel(x, y) ^ displayInverted ? displayBufferOnColor : displayBufferOffColor;
            if (y < 0 or y >= DISPLAY_HEIGHT / displayScale or x < 0 or x >= DISPLAY_WIDTH / displayScale)
                color = frameBufferContext.backgroundColor == LCDSolidColor::White ? displayBufferOnColor : displayBufferOffColor;
            if (displayBufferNativeEndian)
                displayBufferRGBA[i][j] = color;
            else
                writeUint32BE((uint8 *) &displayBufferRGBA[i][j], color);
        }
    }

    // Todo: This shouldn't swap for menu
    memcpy(previousFrameBuffer->data.data(), frameBuffer->data.data(), frameBuffer->data.size()); // Todo: A swap would be more efficient

    cranked.menu.render();
}
