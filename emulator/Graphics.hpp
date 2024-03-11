#pragma once

#include "gen/PlaydateAPI.hpp"
#include "Constants.hpp"
#include "Rom.hpp"
#include "HeapAllocator.hpp"

#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include <optional>

class Graphics;
class Emulator;

struct Vec2 {
    int x, y;

    inline Vec2 operator +(const Vec2 other) const {
        return { x + other.x, y + other.y };
    }

    inline Vec2 operator -(const Vec2 other) const {
        return { x - other.x, y - other.y };
    }

    inline Vec2 operator -() const {
        return { -x, -y };
    }
};

struct Rect {
    inline bool contains(Vec2 point) {
        return point.x >= pos.x and point.x < pos.x + size.x and point.y >= pos.y and point.y < pos.y + size.y;
    }

    inline Rect operator +(const Vec2 offset) const {
        return { pos.x + offset.x, pos.y + offset.y, size.x, size.y };
    }

    inline Rect operator -(const Vec2 offset) const {
        return { pos.x - offset.x, pos.y - offset.y, size.x, size.y };
    }

    inline Rect intersection(const Rect &other) {
        Rect rect{};
        if (other.pos.x < pos.x) {
            rect.pos.x = other.pos.x;
            rect.size.x = std::max(0, std::min(other.size.x, size.x - (pos.x - other.pos.x)));
        } else {
            rect.pos.x = pos.x;
            rect.size.x = std::max(0, std::min(size.x, other.size.x - (other.pos.x - pos.x)));
        }
        if (other.pos.y < pos.y) {
            rect.pos.y = other.pos.y;
            rect.size.y = std::max(0, std::min(other.size.y, size.y - (pos.y - other.pos.y)));
        } else {
            rect.pos.y = pos.y;
            rect.size.y = std::max(0, std::min(size.y, other.size.y - (other.pos.y - pos.y)));
        }
        return rect;
    }

    Vec2 pos;
    Vec2 size;
};

struct Transform {
    // Todo

    float m11, m12, m21, m22, tx, ty;
};

union LCDColor {
    inline LCDColor(uint32_t value) : pattern(value) {} // NOLINT: Intentional implicit conversion
    inline LCDColor(LCDSolidColor value) : color(value) {} // NOLINT: Intentional implicit conversion

    LCDSolidColor color;
    cref_t pattern;
};

struct LCDVideoPlayer_32 {

};

/**
 * The operations implemented here aren't affected by draw offsets or clipping rects (Except for display buffer writes, which are affected by display offsets)
 */
struct LCDBitmap_32 {
    LCDBitmap_32(const LCDBitmap_32 &other);

    LCDBitmap_32(LCDBitmap_32 &&other) noexcept;

    LCDBitmap_32(int width, int height, Graphics *graphics);

    ~LCDBitmap_32();

    LCDBitmap_32& operator=(const LCDBitmap_32 &other);

    bool getBufferPixel(int x, int y);

    void setBufferPixel(int x, int y, bool color);

    void drawPixel(int x, int y, LCDColor color);

    LCDSolidColor getPixel(int x, int y);

    void clear(LCDColor color);

    void drawLine(int x1, int y1, int x2, int y2, LCDColor color);

    // Todo: Can be more efficient since it's done in horizontal segments
    void fillRect(int x, int y, int w, int h, LCDColor color);

    // Todo: Rotate bitmaps using three shears method: https://gautamnagrawal.medium.com/rotating-image-by-any-angle-shear-transformation-using-only-numpy-d28d16eb5076

    int width;
    int height;
    bool inverted{};
    vheap_vector<uint8_t> data;
    Graphics *graphics;
    LCDBitmap_32 *mask; // Always owns mask
};

struct LCDSprite_32 {
    PDRect_32 bounds{};
    LCDBitmap_32 *image{}; // Todo: Does the sprite own this?
    float centerX{}, centerY{};
    bool visible{};
    bool updatesEnabled{};
    bool dontRedrawOnImageChange{};
    bool ignoresDrawOffset{};
    bool opaque{};
    bool collisionsEnabled{};
    PDRect_32 collideRect{};
    int16_t zIndex{};
    Rect size{};
    Rect pos{};
    Rect scale{};
    Rect clipRect{};
    uint8_t tag{};
    LCDBitmapDrawMode drawMode{};
    LCDBitmapFlip flip{};
    LCDBitmap_32 *stencil{};
    bool stencilTiled{};
    uint8_t stencilPattern[8]{};
    cref_t updateFunction{};
    cref_t drawFunction{};
    cref_t collideResponseFunction{};
    cref_t userdata{};
    bool dirty{};
    std::vector<LCDRect_32> dirtyRects;
};

struct LCDBitmapTable_32 {
    LCDBitmapTable_32(Graphics *graphics, int cellsPerRow);

    LCDBitmapTable_32& operator=(const LCDBitmapTable_32 &other);

    Graphics *graphics;
    int cellsPerRow{};
    vheap_vector<LCDBitmap_32> bitmaps; // Todo: Is this supposed to make copies and own the bitmaps? Lua API needs reference to stored bitmap...
};

struct LCDFontGlyph_32 {
    explicit LCDFontGlyph_32(LCDBitmap_32 &&bitmap, int advance, const std::map<int, int8_t> &shortKerningTable, const std::map<int, int8_t> &longKerningTable);

    int advance;
    std::map<int, int8_t> shortKerningTable;
    std::map<int, int8_t> longKerningTable; // Page 0 entries
    LCDBitmap_32 bitmap;
};

struct LCDFontPage_32 {
    explicit LCDFontPage_32(Graphics *graphics);

    vheap_map<int, LCDFontGlyph_32> glyphs;
};

struct LCDFont_32 {
    explicit LCDFont_32(Graphics *graphics, int tracking, int glyphWidth, int glyphHeight);

    int tracking;
    int glyphWidth;
    int glyphHeight;
    vheap_map<int, LCDFontPage_32> pages;
};

struct DisplayContext { // Todo: Lua native resources used here should be saved into a `set` table to prevent garbage collection until popped/reset
    LCDBitmap_32 *bitmap;
    Vec2 drawOffset;
    Rect clipRect; // In world-space (Offset by drawOffset)
    LCDSolidColor drawingColor;
    LCDSolidColor backgroundColor;
    int lineWidth;
    LCDLineCapStyle lineEndCapStype;
    StrokeLocation strokeLocation; // Only used by drawRect
    LCDBitmap_32 *stencilImage;
    bool stencilTiled;
    uint8_t stencilPattern[8]; // Todo: This is only used when stencilImage isn't set? `setPattern` disables it, so probably need a flag to determine current color
    bool usingStencil;
    LCDBitmap_32 *focusedImage; // Todo: Is this global or context specific?
    LCDBitmapDrawMode bitmapDrawMode;
    LCDFont_32 *font;
    LCDPolygonFillRule polygonFillRule;
    int textTracking;
    int textLeading;
};

class Graphics {
public:
    explicit Graphics(Emulator *emulator);

    void init();
    void reset();
    void update();
    void flushDisplayBuffer();

    inline DisplayContext &getCurrentDisplayContext() {
        return displayContextStack.empty() ? frameBufferContext : displayContextStack.back();
    }

    inline LCDBitmap_32 *getTargetBitmap() {
        auto &context = getCurrentDisplayContext();
        return context.focusedImage ? context.focusedImage : context.bitmap;
    }

    void pushContext(LCDBitmap_32 *target);

    void popContext();

    void drawPixel(int x, int y, LCDColor color, bool ignoreOffset = false);

    LCDSolidColor getPixel(int x, int y, bool ignoreOffset = false);

    void drawLine(int x1, int y1, int x2, int y2, int width, LCDColor color);

    void drawRect(int x, int y, int width, int height, LCDColor color);

    void fillRect(int x, int y, int width, int height, LCDColor color);

    inline void drawRoundRect(int x, int y, int width, int height, int radius, LCDColor color) {
        // Todo
    }

    inline void fillRoundRect(int x, int y, int width, int height, int radius, LCDColor color) {
        // Todo
    }

    void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, LCDColor color);

    void drawBitmap(LCDBitmap_32 *bitmap, int x, int y, LCDBitmapFlip flip, bool ignoreOffset = false, std::optional<Rect> sourceRect = {});

    void drawText(const void* text, int len, PDStringEncoding encoding, int x, int y, LCDFont_32 *font = nullptr);

    void fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, LCDColor color);

    void drawEllipse(int rectX, int rectY, int width, int height, int lineWidth, float startAngle, float endAngle, LCDColor color, bool filled);

    inline LCDSprite_32 *allocateSprite() {
        auto sprite = heap.construct<LCDSprite_32>();
        allocatedSprites.emplace(sprite);
        return sprite;
    }

    inline void freeSprite(LCDSprite_32 *sprite) {
        heap.destruct(sprite);
        allocatedSprites.erase(sprite);
    }

    inline LCDBitmap_32 *allocateBitmap(int width, int height) {
        auto bitmap = heap.construct<LCDBitmap_32>(width, height, this);
        allocatedBitmaps.emplace(bitmap);
        return bitmap;
    }

    inline void freeBitmap(LCDBitmap_32 *bitmap) {
        heap.destruct(bitmap);
        allocatedBitmaps.erase(bitmap);
    }

    inline LCDBitmapTable_32 *allocateBitmapTable(int count) {
        auto table = heap.construct<LCDBitmapTable_32>(this, count);
        allocatedBitmapTables.emplace(table);
        return table;
    }

    inline void freeBitmapTable(LCDBitmapTable_32 *table) {
        allocatedBitmapTables.erase(table);
        heap.destruct(table);
    }

    LCDBitmapTable_32 *getBitmapTable(const std::string &path);

    LCDBitmap_32 *getImage(const std::string &path);

    // Doesn't add to allocatedBitmaps
    LCDBitmap_32 *getImage(const Rom::ImageCell &source);

    LCDFont_32 *getFont(const std::string &path);

    // Doesn't add to loadedFonts or allocatedFonts
    LCDFont_32 *getFont(const Rom::Font &source);

    LCDFont_32 *getFont(const uint8_t *data, bool wide);

    bool handleFree(void *ptr);

    Emulator *emulator;
    HeapAllocator &heap;
    Rom::Font systemFontSource;
    LCDFont_32 *systemFont{};
    uint32_t displayBufferRGBA[DISPLAY_HEIGHT][DISPLAY_WIDTH]{};
    bool displayBufferNativeEndian = false;
    uint32_t displayBufferOnColor = 0xB0AEA7FF;
    uint32_t displayBufferOffColor = 0x302E27FF;
    LCDBitmap_32 *frameBuffer{}, *previousFrameBuffer{};
    DisplayContext frameBufferContext{};
    std::vector<DisplayContext> displayContextStack;
    Vec2 displayOffset{};
    int displayScale{};
    bool displayInverted{};
    bool displayFlippedX{}, displayFlippedY{};
    Vec2 displayMosaic{};
    float framerate{};
    bool alwaysRedrawSprites{};
    std::unordered_set<LCDSprite_32 *> allocatedSprites;
    std::vector<LCDSprite_32 *> spriteDrawList;
    std::unordered_map<std::string, Rom::Image> loadedImages;
    std::unordered_set<LCDBitmap_32 *> allocatedBitmaps;
    std::unordered_map<std::string, Rom::ImageTable> loadedBitmapTables;
    std::unordered_set<LCDBitmapTable_32 *> allocatedBitmapTables;
    std::unordered_map<std::string, LCDFont_32 *> loadedFonts;
    std::unordered_set<LCDFont_32 *> allocatedFonts; // Only used when allocating from raw data in C
};
