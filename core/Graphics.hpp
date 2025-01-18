#pragma once

#include "gen/PlaydateAPI.hpp"
#include "Rom.hpp"
#include "HeapAllocator.hpp"
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

    struct LCDVideoPlayer_32 final : NativeResource {
        explicit LCDVideoPlayer_32(Cranked &cranked, float frameRate, IntVec2 size);

        float frameRate;
        IntVec2 size;
        vector<BitmapRef> frames;
        BitmapRef target;
        int currentFrame{}; // Todo: Is this meant to be the previously rendered frame?
        cref_t lastError{};
    };

/**
 * The operations implemented here aren't affected by draw offsets or clipping rects (Except for display buffer writes, which are affected by display offsets)
 */
    struct LCDBitmap_32 final : NativeResource {
        LCDBitmap_32(Cranked &cranked, int width, int height);

        LCDBitmap_32(const LCDBitmap_32 &other);

        LCDBitmap_32 &operator=(const LCDBitmap_32 &other);

        [[nodiscard]] tuple<int, int> getBufferPixelLocation(int x, int y) const;

        [[nodiscard]] bool getBufferPixel(int x, int y) const;

        void setBufferPixel(int x, int y, bool color);

        void drawPixel(int x, int y, LCDColor color);

        [[nodiscard]] LCDSolidColor getPixel(int x, int y) const;

        void clear(LCDColor color);

        void drawLine(int x1, int y1, int x2, int y2, LCDColor color);

        // Todo: Can be more efficient since it's done in horizontal segments
        void fillRect(int x, int y, int w, int h, LCDColor color);

        void drawBitmap(Bitmap image, int x, int y, int w = -1, int h = -1, int sourceX = 0, int sourceY = 0); // Todo: Support more options

        [[nodiscard]] vector<uint8> toRGB() const;

        // Todo: Rotate bitmaps using three shears method: https://gautamnagrawal.medium.com/rotating-image-by-any-angle-shear-transformation-using-only-numpy-d28d16eb5076

        int width;
        int height;
        bool inverted{};
        vheap_vector<uint8> data;
        BitmapRef mask;
    };

    // Todo: Protect position/bounds and update Lua x/y when moved
    struct LCDSprite_32 : NativeResource {
        friend Graphics;

        explicit LCDSprite_32(Cranked &cranked);

        ~LCDSprite_32() override;

        [[nodiscard]] Rect getWorldCollideRect() const {
            return { bounds.pos + collideRect.pos, collideRect.size };
        }

        void updateCollisionWorld();

        void setCollisionsEnabled(bool enabled) {
            collisionsEnabled = enabled;
            updateCollisionWorld();
        }

        [[nodiscard]] bool areCollisionsEnabled() const {
            return collisionsEnabled;
        }

        /// Sets the collision rect relative to top-left corner of bounds
        void setCollisionRect(Rect rect) {
            collideRect = rect;
            updateCollisionWorld();
        }

        /// Sets the bounding rect in screen space (Before draw offset, and size is overridden if an image is present)
        void setBounds(Rect rect) {
            bounds = rect;
            if (image)
                bounds.size = { (float)image->width, (float)image->height };
            updateCollisionWorld();
            updateLuaPosition();
        }

        [[nodiscard]] Rect getBounds() const {
            return bounds;
        }

        /// Sets the position based on the specified center
        void setPosition(Vec2 pos) {
            bounds.pos = pos - getCenterOffset();
            updateCollisionWorld();
            updateLuaPosition();
        }

        /// Gets the position based on the specified center
        [[nodiscard]] Vec2 getPosition() const {
            return bounds.pos + getCenterOffset();
        }

        void setCenter(Vec2 newCenter) {
            Vec2 pos = getPosition();
            center = newCenter;
            setPosition(pos);
        }

        [[nodiscard]] Vec2 getCenter() const {
            return center;
        }

        void setSize(Vec2 size) {
            Vec2 pos = getPosition();
            bounds.size = size;
            setPosition(pos);
        }

        [[nodiscard]] Vec2 getSize() const {
            return bounds.size;
        }

        /// Sets the image and adjusts the bounds to maintain the same position
        void setImage(Bitmap bitmap, LCDBitmapFlip bitmapFlip, float xScale = 1, float yScale = 1) {
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

        /// Returns the offset from the world space top-left corner to the sprite's considered position
        [[nodiscard]] Vec2 getCenterOffset() const {
            return { center.x * bounds.size.x, center.y * bounds.size.y };
        }

        /// Returns the offset from the world space collision rect position to the sprite's considered position
        [[nodiscard]] Vec2 getCollisionRectToCenterOffset() const {
            return getCenterOffset() - collideRect.pos;
        }

        void updateLuaPosition();

        void setImage(Bitmap bitmap) {
            image = bitmap;
            collideRectFlip = GraphicsFlip::Unflipped;
        }

        void draw();

        void update();

        Sprite copy();

        BitmapRef image{};
        TileMapRef tileMap{}; // Todo: Behavior compared to image (Replaces image?)
        bool visible = true;
        bool updatesEnabled = true;
        bool dontRedrawOnImageChange{};
        bool ignoresDrawOffset{};
        bool opaque{};
        Rect collideRect{};
        GraphicsFlip collideRectFlip{}; // Todo
        uint32 groupMask{}; // Only used by Lua
        uint32 collidesWithGroupMask{};
        int16 zIndex{};
        Vec2 scale{ 1.0f, 1.0f };
        float rotation{};
        IntRect clipRect{};
        uint8 tag{};
        BitmapDrawMode drawMode{};
        GraphicsFlip flip{};
        BitmapRef stencil{};
        bool stencilTiled{};
        cref_t updateFunction{};
        cref_t drawFunction{};
        cref_t collideResponseFunction{};
        cref_t userdata{};
        bool dirty = true;
        vector<LCDRect_32> dirtyRects;

    private:
        bool collisionsEnabled = true;
        Rect bounds{};
        Vec2 center{ 0.5f, 0.5f };
        bool inDrawList{}; // Only set from Graphics
    };

    struct LCDBitmapTable_32 : NativeResource {
        LCDBitmapTable_32(Cranked &cranked, int cellsPerRow);

        LCDBitmapTable_32 &operator=(const LCDBitmapTable_32 &other);

        int cellsPerRow{};
        vector<BitmapRef> bitmaps;
    };

    struct LCDTileMap_32 : NativeResource {
        explicit LCDTileMap_32(Cranked &cranked);

        [[nodiscard]] int getHeight() const {
            return (int)tiles.size() / width;
        }

        [[nodiscard]] IntVec2 getCellSize() const;

        BitmapTableRef table;
        int width{};
        vector<int> tiles;
    };

    struct LCDFontGlyph_32 : NativeResource {
        explicit LCDFontGlyph_32(Bitmap bitmap, int advance, const map<int, int8> &shortKerningTable, const map<int, int8> &longKerningTable);

        int advance;
        map<int, int8> shortKerningTable; // Page 0 entries
        map<int, int8> longKerningTable;
        BitmapRef bitmap;
    };

    struct LCDFontPage_32 : NativeResource {
        explicit LCDFontPage_32(Cranked &cranked);

        unordered_map<int, FontGlyphRef> glyphs;
    };

    struct LCDFont_32 : NativeResource {
        explicit LCDFont_32(Cranked &cranked, int tracking, int glyphWidth, int glyphHeight);

        int tracking;
        int glyphWidth;
        int glyphHeight;
        unordered_map<int, FontPageRef> pages;
    };

    struct FontFamily {
        FontRef &getFont(PDFontVariant variant) {
            switch (variant) {
                case PDFontVariant::Bold:
                    return bold;
                case PDFontVariant::Italic:
                    return italic;
                default:
                    return regular;
            }
        }

        void reset() {
            regular.reset();
            bold.reset();
            italic.reset();
        }

        FontRef regular{};
        FontRef bold{};
        FontRef italic{};
    };

    struct DisplayContext {
        explicit DisplayContext(Bitmap bitmap, FontFamily fonts = {});

        void reset() {
            bitmap.reset();
            stencilImage.reset();
            focusedImage.reset();
            fonts.reset();
        }

        FontRef &getFont(PDFontVariant variant) {
            return fonts.getFont(variant);
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
        bool usingDither{};
        DitherType ditherPattern{};
        float ditherAlpha{};
        LCDBitmapDrawMode bitmapDrawMode{};
        FontFamily fonts;
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

        void drawText(const void *text, int len, PDStringEncoding encoding, int x, int y, Font font = nullptr);

        void fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, LCDColor color);

        void drawEllipse(int rectX, int rectY, int width, int height, int lineWidth, float startAngle, float endAngle, LCDColor color, bool filled);

        void fillPolygon(int32 *coords, int32 points, LCDColor color, LCDPolygonFillRule fillType);

        void drawPolygon(int32 *coords, int32 points, LCDColor color);

        void drawBitmap(Bitmap bitmap, int x, int y, LCDBitmapFlip flip = GraphicsFlip::Unflipped, bool ignoreOffset = false, optional<IntRect> sourceRect = {});

        void drawBitmapTiled(Bitmap bitmap, int x, int y, int width, int height, LCDBitmapFlip flip);

        void drawScaledBitmap(Bitmap bitmap, int x, int y, float xScale, float yScale);

        Bitmap scaleBitmap(Bitmap bitmap, float xScale, float yScale);

        void drawRotatedBitmap(Bitmap bitmap, int x, int y, float angle, float centerX, float centerY, float xScale, float yScale);

        Bitmap rotateBitmap(Bitmap bitmap, float angle, float centerX, float centerY, float xScale, float yScale);

        void drawBlurredBitmap(Bitmap bitmap, int x, int y, float radius, int numPasses, DitherType type, GraphicsFlip flip, int xPhase, int yPhase);

        Bitmap blurredBitmap(Bitmap bitmap, float radius, int numPasses, DitherType type, bool padEdges, int xPhase, int yPhase);

        void drawFadedBitmap(Bitmap bitmap, int x, int y, float alpha, DitherType type);

        Bitmap fadedBitmap(Bitmap bitmap, float alpha, DitherType type);

        Bitmap invertedBitmap(Bitmap bitmap);

        Bitmap blendedBitmap(Bitmap bitmap, Bitmap other, float alpha, DitherType type);

        Bitmap vcrPauseFilteredBitmap(Bitmap bitmap);

        void drawTileMap(TileMap tilemap, int x, int y, bool ignoreOffset = false, optional<IntRect> sourceRect = {});

        IntVec2 measureText(const char *text, const FontFamily &fonts, int leadingAdjustment = 0);

        static bool checkBitmapMaskCollision(Bitmap bitmap1, int x1, int y1, GraphicsFlip flip1, Bitmap bitmap2, int x2, int y2, GraphicsFlip flip2, IntRect rect);

        void updateSprites();

        void drawSprites();

        void setSpriteClipRectsInRage(IntRect rect, int startZ, int endZ) const {
            for (auto sprite : allocatedSprites) {
                if (sprite->zIndex >= startZ and sprite->zIndex <= endZ)
                    sprite->clipRect = rect;
            }
        }

        void clearSpriteClipRectsInRage(int startZ, int endZ) const {
            for (auto sprite : allocatedSprites) {
                if (sprite->zIndex >= startZ and sprite->zIndex <= endZ)
                    sprite->clipRect = {};
            }
        }

        void addSpriteToDrawList(Sprite sprite);

        void removeSpriteFromDrawList(Sprite sprite);

        void clearSpriteDrawList();

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

        VideoPlayer getVideoPlayer(const string &path);

        Bitmap getImage(const string &path);

        Bitmap getImage(const Rom::ImageCell &source);

        Font getFont(const string &path);

        Font getFont(const Rom::Font &source);

        Font getFont(const uint8 *data, bool wide);

        Font getSystemFont(PDFontVariant variant = PDFontVariant::Normal);

        Cranked &cranked;
        HeapAllocator &heap;
        Rom::Font systemFontSources[3]{}; // Todo: Convert to a FontFamily structure in PlaydateTypes
        FontFamily systemFonts;
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
        vector<SpriteRef> spriteDrawList;
    };

}
