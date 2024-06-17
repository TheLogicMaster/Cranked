#pragma once

#include "gen/PlaydateAPI.hpp"
#include "Constants.hpp"
#include "Rom.hpp"
#include "HeapAllocator.hpp"
#include "NativeResource.hpp"

#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include <optional>

namespace cranked {

    class Graphics;

    class Cranked;

    template<typename T>
    struct Vector2 {
        T x, y;

        template<typename S>
        [[nodiscard]] inline Vector2<S> as() const {
            return Vector2<S>((S) x, (S) y);
        }

        inline Vector2 operator+(const Vector2 other) const {
            return {x + other.x, y + other.y};
        }

        inline Vector2 operator-(const Vector2 other) const {
            return {x - other.x, y - other.y};
        }

        inline Vector2 operator-() const {
            return {-x, -y};
        }
    };

    typedef Vector2<int32_t> IntVec2;
    typedef Vector2<float> Vec2;

    template<typename T>
    struct Rectangle {
        template<typename S>
        [[nodiscard]] inline Rectangle<S> as() const {
            return Rectangle<S>(pos.template as<S>(), size.template as<S>());
        }

        template<typename S>
        [[nodiscard]] inline bool contains(Vector2<S> point) const {
            return point.x >= pos.x and point.x < pos.x + size.x and point.y >= pos.y and point.y < pos.y + size.y;
        }

        template<typename S>
        inline Rectangle operator+(const Vector2<S> offset) const {
            return {pos.x + offset.x, pos.y + offset.y, size.x, size.y};
        }

        template<typename S>
        inline Rectangle operator-(const Vector2<S> offset) const {
            return {pos.x - offset.x, pos.y - offset.y, size.x, size.y};
        }

        template<typename S>
        inline Rectangle intersection(const Rectangle<S> &other) {
            Rectangle rect{};
            if (other.pos.x < pos.x) {
                rect.pos.x = other.pos.x;
                rect.size.x = std::max(0.0f, std::min(other.size.x, size.x - (pos.x - other.pos.x)));
            } else {
                rect.pos.x = pos.x;
                rect.size.x = std::max(0.0f, std::min(size.x, other.size.x - (other.pos.x - pos.x)));
            }
            if (other.pos.y < pos.y) {
                rect.pos.y = other.pos.y;
                rect.size.y = std::max(0.0f, std::min(other.size.y, size.y - (pos.y - other.pos.y)));
            } else {
                rect.pos.y = pos.y;
                rect.size.y = std::max(0.0f, std::min(size.y, other.size.y - (other.pos.y - pos.y)));
            }
            return rect;
        }

        Vector2<T> pos;
        Vector2<T> size;
    };

    typedef Rectangle<int32_t> IntRect;
    typedef Rectangle<float> Rect;

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

    struct LCDVideoPlayer_32 : NativeResource {
        explicit LCDVideoPlayer_32(Cranked &cranked);

        LCDVideoPlayer_32(const LCDVideoPlayer_32 &other) = delete;
        LCDVideoPlayer_32(LCDVideoPlayer_32 &&other) = delete;
        ~LCDVideoPlayer_32() override = default;
    };

/**
 * The operations implemented here aren't affected by draw offsets or clipping rects (Except for display buffer writes, which are affected by display offsets)
 */
    struct LCDBitmap_32 : NativeResource {
        LCDBitmap_32(Cranked &cranked, int width, int height);

        LCDBitmap_32(const LCDBitmap_32 &other);

        LCDBitmap_32(LCDBitmap_32 &&other) = delete;

        ~LCDBitmap_32() override = default;

        LCDBitmap_32 &operator=(const LCDBitmap_32 &other);

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
        ResourcePtr<LCDBitmap_32> mask;
    };

    struct LCDSprite_32 : NativeResource {
        explicit LCDSprite_32(Cranked &cranked);

        LCDSprite_32(const LCDSprite_32 &other) = delete;
        LCDSprite_32(LCDSprite_32 &&other) = delete;
        ~LCDSprite_32() override;

        PDRect_32 bounds{};
        ResourcePtr<LCDBitmap_32> image{};
        float centerX{}, centerY{};
        bool visible{};
        bool updatesEnabled{};
        bool dontRedrawOnImageChange{};
        bool ignoresDrawOffset{};
        bool opaque{};
        bool collisionsEnabled{};
        PDRect_32 collideRect{};
        int16_t zIndex{};
        Vec2 size{};
        Vec2 pos{};
        Rect scale{};
        Rect clipRect{};
        uint8_t tag{};
        LCDBitmapDrawMode drawMode{};
        LCDBitmapFlip flip{};
        ResourcePtr<LCDBitmap_32> stencil{};
        bool stencilTiled{};
        uint8_t stencilPattern[8]{};
        cref_t updateFunction{};
        cref_t drawFunction{};
        cref_t collideResponseFunction{};
        cref_t userdata{};
        bool dirty{};
        std::vector<LCDRect_32> dirtyRects;
    };

    struct LCDBitmapTable_32 : NativeResource {
        LCDBitmapTable_32(Cranked &cranked, int cellsPerRow);

        LCDBitmapTable_32(const LCDBitmapTable_32 &other) = default;
        LCDBitmapTable_32(LCDBitmapTable_32 &&other) = delete;
        ~LCDBitmapTable_32() override = default;

        LCDBitmapTable_32 &operator=(const LCDBitmapTable_32 &other);

        int cellsPerRow{};
        std::vector<ResourcePtr<LCDBitmap_32>> bitmaps;
    };

    struct LCDFontGlyph_32 : NativeResource {
        explicit LCDFontGlyph_32(LCDBitmap_32 *bitmap, int advance, const std::map<int, int8_t> &shortKerningTable, const std::map<int, int8_t> &longKerningTable);

        LCDFontGlyph_32(const LCDFontGlyph_32 &other) = delete;
        LCDFontGlyph_32(LCDFontGlyph_32 &&other) = delete;
        ~LCDFontGlyph_32() override = default;

        int advance;
        std::map<int, int8_t> shortKerningTable;
        std::map<int, int8_t> longKerningTable; // Page 0 entries
        ResourcePtr<LCDBitmap_32> bitmap;
    };

    struct LCDFontPage_32 : NativeResource {
        explicit LCDFontPage_32(Cranked &cranked);

        LCDFontPage_32(const LCDFontPage_32 &other) = delete;
        LCDFontPage_32(LCDFontPage_32 &&other) = delete;
        ~LCDFontPage_32() override = default;

        std::unordered_map<int, ResourcePtr<LCDFontGlyph_32>> glyphs;
    };

    struct LCDFont_32 : NativeResource {
        explicit LCDFont_32(Cranked &cranked, int tracking, int glyphWidth, int glyphHeight);

        LCDFont_32(const LCDFont_32 &other) = delete;
        LCDFont_32(LCDFont_32 &&other) = delete;
        ~LCDFont_32() override = default;

        int tracking;
        int glyphWidth;
        int glyphHeight;
        std::unordered_map<int, ResourcePtr<LCDFontPage_32>> pages;
    };

    struct DisplayContext { // Todo: Lua native resources used here should be saved into a `set` table to prevent garbage collection until popped/reset
        explicit DisplayContext(LCDBitmap_32 *bitmap) : bitmap(bitmap) {}

        DisplayContext(const DisplayContext &other) = default;
        DisplayContext(DisplayContext &&other) = delete;
        DisplayContext &operator=(const DisplayContext &other) = default;

        void reset() {
            bitmap.reset();
            stencilImage.reset();
            focusedImage.reset();
            font.reset();
        }

        ResourcePtr<LCDBitmap_32> bitmap;
        IntVec2 drawOffset{};
        IntRect clipRect{}; // In world-space (Offset by drawOffset)
        LCDSolidColor drawingColor{};
        LCDSolidColor backgroundColor = LCDSolidColor::White;
        int lineWidth = 1;
        LCDLineCapStyle lineEndCapStyle{};
        StrokeLocation strokeLocation{}; // Only used by drawRect
        ResourcePtr<LCDBitmap_32> stencilImage{};
        bool stencilTiled{};
        uint8_t stencilPattern[8]{}; // Todo: This is only used when stencilImage isn't set? `setPattern` disables it, so probably need a flag to determine current color
        bool usingStencil{};
        ResourcePtr<LCDBitmap_32> focusedImage; // Todo: Is this global or context specific?
        LCDBitmapDrawMode bitmapDrawMode{};
        ResourcePtr<LCDFont_32> font{};
        LCDPolygonFillRule polygonFillRule{};
        int textTracking{};
        int textLeading{};
    };

    class Graphics {
    public:
        explicit Graphics(Cranked &cranked);

        void init();

        void reset();

        void update();

        void flushDisplayBuffer();

        inline DisplayContext &getCurrentDisplayContext() {
            return displayContextStack.empty() ? frameBufferContext : displayContextStack.back();
        }

        inline LCDBitmap_32 *getTargetBitmap() {
            auto &context = getCurrentDisplayContext();
            return (context.focusedImage ? context.focusedImage : context.bitmap).get();
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

        void drawBitmap(LCDBitmap_32 *bitmap, int x, int y, LCDBitmapFlip flip, bool ignoreOffset = false,
                        std::optional<IntRect> sourceRect = {});

        void drawText(const void *text, int len, PDStringEncoding encoding, int x, int y, LCDFont_32 *font = nullptr);

        void fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, LCDColor color);

        void drawEllipse(int rectX, int rectY, int width, int height, int lineWidth, float startAngle, float endAngle,
                         LCDColor color, bool filled);

        inline LCDSprite_32 *createSprite() {
            return heap.construct<LCDSprite_32>(cranked);
        }

        inline LCDBitmap_32 *createBitmap(int width, int height) {
            return heap.construct<LCDBitmap_32>(cranked, width, height);
        }

        inline LCDBitmapTable_32 *createBitmapTable(int count) {
            return heap.construct<LCDBitmapTable_32>(cranked, count);
        }

        LCDBitmapTable_32 *getBitmapTable(const std::string &path);

        LCDBitmap_32 *getImage(const std::string &path);

        LCDBitmap_32 *getImage(const Rom::ImageCell &source);

        LCDFont_32 *getFont(const std::string &path);

        LCDFont_32 *getFont(const Rom::Font &source);

        LCDFont_32 *getFont(const uint8_t *data, bool wide);

        Cranked &cranked;
        HeapAllocator &heap;
        Rom::Font systemFontSource;
        ResourcePtr<LCDFont_32> systemFont;
        uint32_t displayBufferRGBA[DISPLAY_HEIGHT][DISPLAY_WIDTH]{};
        bool displayBufferNativeEndian = false;
        uint32_t displayBufferOnColor = 0xB0AEA7FF;
        uint32_t displayBufferOffColor = 0x302E27FF;
        ResourcePtr<LCDBitmap_32> frameBuffer, previousFrameBuffer;
        DisplayContext frameBufferContext{frameBuffer.get()};
        std::vector<DisplayContext> displayContextStack;
        IntVec2 displayOffset{};
        int32_t displayScale{};
        bool displayInverted{};
        bool displayFlippedX{}, displayFlippedY{};
        IntVec2 displayMosaic{};
        float framerate{};
        bool alwaysRedrawSprites{};
        std::vector<ResourcePtr<LCDSprite_32>> spriteDrawList;
    };

}
