#pragma once

#include "gen/PlaydateAPI.hpp"
#include "Rom.hpp"
#include "HeapAllocator.hpp"
#include "NativeResource.hpp"
#include "Geometry.hpp"

namespace cranked {

    class Graphics;

    class Cranked;

    union LCDColor {
        LCDColor(uint32 value) : pattern(value) {} // NOLINT(*-explicit-constructor)
        LCDColor(LCDSolidColor value) : color(value) {} // NOLINT(*-explicit-constructor)

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

        tuple<int, int> getBufferPixelLocation(int x, int y);

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
        vheap_vector<uint8> data;
        BitmapRef mask;
    };

    struct LCDSprite_32 : NativeResource {
        explicit LCDSprite_32(Cranked &cranked);

        LCDSprite_32(const LCDSprite_32 &other) = delete;
        LCDSprite_32(LCDSprite_32 &&other) = delete;
        ~LCDSprite_32() override;

        Rect getWorldCollideRect() {
            return { bounds.pos + collideRect.pos, collideRect.size };
        }

        void updateCollisionWorld();

        void setCollisionsEnabled(bool enabled) {
            collisionsEnabled = enabled;
            updateCollisionWorld();
        }

        /// Sets the collision rect relative to top-left corner of bounds
        void setCollisionRect(Rect rect) {
            collideRect = rect;
            updateCollisionWorld();
        }

        /// Sets the bounding rect in screen space (Before draw offset, and size is overridden if an image is present)
        void setBounds(Rect rect) {
            bounds = rect;
            if (image) // Todo: Is this correct?
                bounds.size = { (float)image->width, (float)image->height };
            updateCollisionWorld();
        }

        /// Sets the position based on the specified center
        void setPosition(Vec2 pos) {
            bounds.pos = pos - getCenterOffset();
            updateCollisionWorld();
        }

        /// Gets the position based on the specified center
        [[nodiscard]] Vec2 getPosition() const {
            return bounds.pos + getCenterOffset();
        }

        /// Sets the image and adjusts the bounds to maintain the same position
        void setImage(Bitmap bitmap, LCDBitmapFlip bitmapFlip) {
            // Todo: Figure out how this relates to stencil
            if (!dontRedrawOnImageChange)
                dirty = true;
            if (bitmap and !bitmap->mask)
                opaque = true;
            image = bitmap;
            flip = bitmapFlip;
            Vec2 pos = getPosition();
            if (bitmap)
                bounds.size = { (float)bitmap->width, (float)bitmap->height };
            setPosition(pos);
        }

        /// Returns the offset from the world space top-left corner that is considered the sprite's position
        [[nodiscard]] Vec2 getCenterOffset() const {
            return { center.x * bounds.size.x, center.y * bounds.size.y };
        }

        void draw();

        void update();

        Sprite copy();

        Rect bounds{};
        BitmapRef image{};
        Vec2 center{ 0.5f, 0.5f };
        bool visible = true;
        bool updatesEnabled = true;
        bool dontRedrawOnImageChange{};
        bool ignoresDrawOffset{};
        bool opaque{};
        bool collisionsEnabled = true;
        Rect collideRect{};
        uint32 groupMask{}; // Todo: Only present on Lua side, just filters after Bump output? Or filter passed into bump calls?
        int16 zIndex{};
        Rect scale{};
        IntRect clipRect{};
        uint8 tag{};
        LCDBitmapDrawMode drawMode{};
        LCDBitmapFlip flip{};
        BitmapRef stencil{};
        bool stencilTiled{};
        cref_t updateFunction{};
        cref_t drawFunction{};
        cref_t collideResponseFunction{};
        cref_t userdata{};
        bool dirty = true;
        vector<LCDRect_32> dirtyRects;
    };

    struct LCDBitmapTable_32 : NativeResource {
        LCDBitmapTable_32(Cranked &cranked, int cellsPerRow);

        LCDBitmapTable_32(const LCDBitmapTable_32 &other) = default;
        LCDBitmapTable_32(LCDBitmapTable_32 &&other) = delete;
        ~LCDBitmapTable_32() override = default;

        LCDBitmapTable_32 &operator=(const LCDBitmapTable_32 &other);

        int cellsPerRow{};
        vector<BitmapRef> bitmaps;
    };

    struct LCDFontGlyph_32 : NativeResource {
        explicit LCDFontGlyph_32(Bitmap bitmap, int advance, const map<int, int8> &shortKerningTable, const map<int, int8> &longKerningTable);

        LCDFontGlyph_32(const LCDFontGlyph_32 &other) = delete;
        LCDFontGlyph_32(LCDFontGlyph_32 &&other) = delete;
        ~LCDFontGlyph_32() override = default;

        int advance;
        map<int, int8> shortKerningTable; // Page 0 entries
        map<int, int8> longKerningTable;
        BitmapRef bitmap;
    };

    struct LCDFontPage_32 : NativeResource {
        explicit LCDFontPage_32(Cranked &cranked);

        LCDFontPage_32(const LCDFontPage_32 &other) = delete;
        LCDFontPage_32(LCDFontPage_32 &&other) = delete;
        ~LCDFontPage_32() override = default;

        unordered_map<int, FontGlyphRef> glyphs;
    };

    struct LCDFont_32 : NativeResource {
        explicit LCDFont_32(Cranked &cranked, int tracking, int glyphWidth, int glyphHeight);

        LCDFont_32(const LCDFont_32 &other) = delete;
        LCDFont_32(LCDFont_32 &&other) = delete;
        ~LCDFont_32() override = default;

        int tracking;
        int glyphWidth;
        int glyphHeight;
        unordered_map<int, FontPageRef> pages;
    };

    struct DisplayContext { // Todo: Lua native resources used here should be saved into a `set` table to prevent garbage collection until popped/reset
        explicit DisplayContext(Bitmap bitmap) : bitmap(bitmap) {}

        DisplayContext(const DisplayContext &other) = default;
        DisplayContext(DisplayContext &&other) = delete;
        DisplayContext &operator=(const DisplayContext &other) = default;

        void reset() {
            bitmap.reset();
            stencilImage.reset();
            focusedImage.reset();
            font.reset();
            boldFont.reset();
            italicFont.reset();
        }

        FontRef &getFont(PDFontVariant variant) {
            switch (variant) {
                case PDFontVariant::Bold:
                    return boldFont;
                case PDFontVariant::Italic:
                    return italicFont;
                default:
                    return font;
            }
        }

        BitmapRef bitmap;
        IntVec2 drawOffset{};
        IntRect clipRect{0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT}; // In world-space (Offset by drawOffset)
        LCDSolidColor drawingColor{};
        LCDSolidColor backgroundColor = LCDSolidColor::White;
        int lineWidth = 1;
        LCDLineCapStyle lineEndCapStyle{};
        StrokeLocation strokeLocation{}; // Only used by drawRect
        BitmapRef stencilImage{};
        bool stencilTiled{};
        uint8 stencilPattern[8]{}; // Todo: This is only used when stencilImage isn't set? `setPattern` disables it, so probably need a flag to determine current color
        bool usingStencil{};
        BitmapRef focusedImage; // Todo: Is this global or context specific?
        LCDBitmapDrawMode bitmapDrawMode{};
        FontRef font{};
        FontRef boldFont{};
        FontRef italicFont{};
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

        DisplayContext &getCurrentDisplayContext() {
            return displayContextStack.empty() ? frameBufferContext : displayContextStack.back();
        }

        Bitmap getTargetBitmap() {
            auto &context = getCurrentDisplayContext();
            return (context.focusedImage ? context.focusedImage : context.bitmap).get();
        }

        DisplayContext &pushContext(Bitmap target);

        void popContext();

        void drawPixel(int x, int y, LCDColor color, bool ignoreOffset = false);

        LCDSolidColor getPixel(int x, int y, bool ignoreOffset = false);

        void drawLine(int x1, int y1, int x2, int y2, int width, LCDColor color);

        void drawRect(int x, int y, int width, int height, LCDColor color);

        void fillRect(int x, int y, int width, int height, LCDColor color);

        void drawRoundRect(int x, int y, int width, int height, int radius, LCDColor color);

        void fillRoundRect(int x, int y, int width, int height, int radius, LCDColor color);

        void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, LCDColor color);

        void drawBitmap(Bitmap bitmap, int x, int y, LCDBitmapFlip flip, bool ignoreOffset = false, optional<IntRect> sourceRect = {});

        void drawBitmapTiled(Bitmap bitmap, int x, int y, int width, int height, LCDBitmapFlip flip);

        void drawText(const void *text, int len, PDStringEncoding encoding, int x, int y, Font font = nullptr);

        void fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, LCDColor color);

        void drawEllipse(int rectX, int rectY, int width, int height, int lineWidth, float startAngle, float endAngle, LCDColor color, bool filled);

        void fillPolygon(int32 *coords, int32 points, LCDColor color, LCDPolygonFillRule fillType);

        void drawPolygon(int32 *coords, int32 points, LCDColor color);

        void drawScaledBitmap(Bitmap bitmap, int x, int y, float xScale, float yScale);

        Bitmap scaleBitmap(Bitmap bitmap, float xScale, float yScale);

        void drawRotatedBitmap(Bitmap bitmap, int x, int y, float angle, float centerX, float centerY, float xScale, float yScale);

        Bitmap rotateBitmap(Bitmap bitmap, float angle, float centerX, float centerY, float xScale, float yScale);

        void updateSprites();

        void drawSprites();

        Sprite createSprite() {
            return heap.construct<LCDSprite_32>(cranked);
        }

        Bitmap createBitmap(int width, int height) {
            return heap.construct<LCDBitmap_32>(cranked, width, height);
        }

        BitmapTable createBitmapTable(int count) {
            return heap.construct<LCDBitmapTable_32>(cranked, count);
        }

        BitmapTable getBitmapTable(const string &path);

        Bitmap getImage(const string &path);

        Bitmap getImage(const Rom::ImageCell &source);

        Font getFont(const string &path);

        Font getFont(const Rom::Font &source);

        Font getFont(const uint8 *data, bool wide);

        Cranked &cranked;
        HeapAllocator &heap;
        Rom::Font systemFontSource;
        FontRef systemFont;
        uint32 displayBufferRGBA[DISPLAY_HEIGHT][DISPLAY_WIDTH]{};
        bool displayBufferNativeEndian = false;
        uint32 displayBufferOnColor = 0xB0AEA7FF;
        uint32 displayBufferOffColor = 0x302E27FF;
        BitmapRef frameBuffer, previousFrameBuffer;
        DisplayContext frameBufferContext{ frameBuffer.get() };
        vector<DisplayContext> displayContextStack;
        IntVec2 displayOffset{};
        int32 displayScale{};
        bool displayInverted{};
        bool displayFlippedX{}, displayFlippedY{};
        IntVec2 displayMosaic{};
        float framerate{};
        bool alwaysRedrawSprites{};
        unordered_set<Sprite> allocatedSprites;
        unordered_set<SpriteRef> spriteDrawList;
    };

}
