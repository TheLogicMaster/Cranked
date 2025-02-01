#pragma once

#include "gen/PlaydateAPI.hpp"
#include "Rom.hpp"
#include "HeapAllocator.hpp"
#include "Geometry.hpp"

namespace cranked {

    class Graphics;

    class Cranked;

    class FontFamily;

    union LCDColor {
        LCDColor(uint32 value) : pattern(value) {} // NOLINT(*-explicit-constructor)
        LCDColor(LCDSolidColor value) : color(value) {} // NOLINT(*-explicit-constructor)

        LCDSolidColor color;
        cref_t pattern;
    };

    struct PatternColor {
        array<uint8, 16> pattern;
    };

    struct DitherColor {
        DitherType type;
        float alpha;
    };

    using Color = variant<LCDColor, PatternColor, DitherColor>;

    template<int N>
    class BayerTable {
    public:
        constexpr BayerTable() : table() {
            for (int i = 0; i < N * N; i++) {
                const int q = i ^ i >> 3;
                table[i] = (float)((i & 4) >> 2 | (q & 4) >> 1 | (i & 2) << 1 | (q & 2) << 2 | (i & 1) << 4 | (q & 1) << 5) / (float)(N * N);
            }
        }

        [[nodiscard]] constexpr float get(int x, int y) const {
            return table[y % N * N + x % N];
        }

    private:
        float table[N * N];
    };

    class GrayscaleImage {
    public:
        GrayscaleImage(int width, int height) {
            data.resize(height);
            for (auto &row : data)
                row.resize(width);
        }

        explicit GrayscaleImage(Bitmap bitmap, float alpha = 1);

        [[nodiscard]] int height() const {
            return (int)data.size();
        }

        [[nodiscard]] int width() const {
            return height() == 0 ? 0 : (int)data[0].size();
        }

        vector<float> &operator[](int row) {
            return data[row];
        }

        void copyInto(Bitmap bitmap, float threshold = numeric_limits<float>::epsilon()) const;

        void diagonalLines(Bitmap bitmap, int xPhase, int yPhase) const;

        void verticalLines(Bitmap bitmap, int xPhase) const;

        void horizontalLines(Bitmap bitmap, int yPhase) const;

        void screenDoor(Bitmap bitmap, int xPhase, int yPhase) const;

        void bayer2x2(Bitmap bitmap, int xPhase, int yPhase) const;

        void bayer4x4(Bitmap bitmap, int xPhase, int yPhase) const;

        void bayer8x8(Bitmap bitmap, int xPhase, int yPhase) const;

        void floydSteinberg(Bitmap bitmap);

        void burkes(Bitmap bitmap);

        void atkinson(Bitmap bitmap);

        void dither(Bitmap bitmap, DitherType type, int xPhase, int yPhase);

    private:
        void diffuseError(int x, int y, float error, float weight) {
            if (x < 0 or x >= width() or y < 0 or y >= height())
                return;
            data[y][x] += error * weight;
        }

        vector<vector<float>> data;
    };

    struct GlyphInstance {
        FontGlyph glyph;
        IntRect bounds;
        char32_t character;
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

        void drawPixel(int x, int y, const Color &color);

        [[nodiscard]] LCDSolidColor getPixel(int x, int y) const;

        void clear(const Color &color);

        void drawVerticalLine(int x, int y1, int y2, const Color &color);

        void drawHorizontalLine(int x1, int x2, int y, const Color &color);

        void drawRect(int x, int y, int width, int height, const Color &color);

        void fillRect(int x, int y, int width, int height, const Color &color);

        void drawBitmap(Bitmap bitmap, int x, int y, BitmapDrawMode mode = {}, GraphicsFlip flip = {}, optional<IntRect> sourceRect = {});

        IntRect drawText(int x, int y, string_view text, const FontFamily &fonts, Font font = nullptr, StringEncoding encoding = PDStringEncoding::UFT8, IntVec2 size = {}, TextWrap wrap = {}, TextAlign align = {}, BitmapDrawMode mode = {}, int tracking = 0, int leadingAdjust = 0, int charCount = 0);

        [[nodiscard]] vector<uint8> toRGB() const;

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
        void setImage(Bitmap bitmap, LCDBitmapFlip bitmapFlip = GraphicsFlip::Unflipped, float xScale = 1, float yScale = 1);

        void setTileMap(TileMap tileMap);

        /// Returns the offset from the world space top-left corner to the sprite's considered position
        [[nodiscard]] Vec2 getCenterOffset() const {
            return { center.x * bounds.size.x, center.y * bounds.size.y };
        }

        /// Returns the offset from the world space collision rect position to the sprite's considered position
        [[nodiscard]] Vec2 getCollisionRectToCenterOffset() const {
            return getCenterOffset() - collideRect.pos;
        }

        void updateLuaPosition();

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
        explicit LCDFontGlyph_32(Bitmap bitmap, int advance, const map<int, int8> &kerningTable);

        int advance;
        map<int, int8> kerningTable;
        BitmapRef bitmap;
    };

    struct LCDFontPage_32 : NativeResource {
        explicit LCDFontPage_32(Cranked &cranked);

        unordered_map<int, FontGlyphRef> glyphs;
    };

    struct LCDFont_32 : NativeResource {
        explicit LCDFont_32(Cranked &cranked, int tracking, int glyphWidth, int glyphHeight);

        int tracking; // Space between characters
        int leading; // Space between lines
        int glyphWidth;
        int glyphHeight;
        unordered_map<int, FontPageRef> pages;
    };

    struct FontFamily {
        [[nodiscard]] Font getFont(PDFontVariant variant) const {
            switch (variant) {
                case PDFontVariant::Bold:
                    return bold;
                case PDFontVariant::Italic:
                    return italic;
                default:
                    return regular;
            }
        }

        void setFont(PDFontVariant variant, Font font) {
            switch (variant) {
                case PDFontVariant::Bold:
                    bold = font;;
                    break;
                case PDFontVariant::Italic:
                    italic = font;
                    break;
                default:
                    regular = font;
                    break;
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
            color = LCDSolidColor::Black;
            focusedImage.reset();
            fonts.reset();
        }

        [[nodiscard]] Font getFont(PDFontVariant variant) const {
            return fonts.getFont(variant);
        }

        void setFont(PDFontVariant variant, Font font) {
            fonts.setFont(variant, font);
        }

        [[nodiscard]] Bitmap getTargetBitmap() const {
            return focusedImage ? focusedImage : bitmap;
        }

        BitmapRef bitmap;
        IntVec2 drawOffset{};
        IntRect clipRect{0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT}; // In world-space (Offset by drawOffset)
        Color color = LCDSolidColor::Black;
        LCDSolidColor drawingColor{}; // Copy of last set solid color for Lua getColor
        LCDSolidColor backgroundColor = LCDSolidColor::White;
        BitmapRef stencil{};
        bool stencilTiled{};
        int lineWidth = 1;
        LCDLineCapStyle lineEndCapStyle{};
        StrokeLocation strokeLocation{}; // Only used by drawRect
        BitmapRef focusedImage; // Todo: Is this global or context specific?
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

        DisplayContext &getContext() {
            return displayContextStack.empty() ? frameBufferContext : displayContextStack.back();
        }

        DisplayContext &pushContext(Bitmap target);

        void popContext();

        void drawPixel(int x, int y, const Color &color, bool ignoreOffset = false);

        LCDSolidColor getPixel(int x, int y, bool ignoreOffset = false);

        void drawLine(int x1, int y1, int x2, int y2, const Color &color);

        void drawLine(int x1, int y1, int x2, int y2, int width, const Color &color);

        void drawRect(int x, int y, int width, int height, const Color &color);

        void fillRect(int x, int y, int width, int height, const Color &color);

        void drawRoundRect(int x, int y, int width, int height, int radius, const Color &color);

        void fillRoundRect(int x, int y, int width, int height, int radius, const Color &color);

        void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, const Color &color);

        vector<GlyphInstance> layoutText(int x, int y, string_view text, const FontFamily &fonts, Font font = nullptr, StringEncoding encoding = PDStringEncoding::UFT8, IntVec2 size = {}, TextWrap wrap = {}, TextAlign align = {}, int tracking = 0, int leadingAdjust = 0, int charCount = 0);

        void drawText(int x, int y, string_view text, const FontFamily &fonts, Font font = nullptr, StringEncoding encoding = PDStringEncoding::UFT8, IntVec2 size = {}, TextWrap wrap = {}, TextAlign align = {}, int tracking = 0, int leadingAdjust = 0, int charCount = 0);

        IntVec2 measureText(string_view text, const FontFamily &fonts, Font font = nullptr, StringEncoding encoding = PDStringEncoding::UFT8, IntVec2 size = {}, TextWrap wrap = {}, TextAlign align = {}, int tracking = 0, int leadingAdjust = 0, int charCount = 0);

        const char *getLocalizedText(const char *key, PDLanguage language);

        void fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, const Color &color);

        void drawEllipse(int rectX, int rectY, int width, int height, int lineWidth, float startAngle, float endAngle, const Color &color, bool filled);

        void fillPolygon(int32 *coords, int32 points, const Color &color, PolygonFillRule fillType);

        void drawPolygon(int32 *coords, int32 points, const Color &color);

        void drawBitmap(Bitmap bitmap, int x, int y, GraphicsFlip flip = GraphicsFlip::Unflipped, bool ignoreOffset = false, optional<IntRect> sourceRect = {});

        void drawBitmapTiled(Bitmap bitmap, int x, int y, int width, int height, GraphicsFlip flip);

        void drawScaledBitmap(Bitmap bitmap, int x, int y, float xScale, float yScale);

        Bitmap scaleBitmap(Bitmap bitmap, float xScale, float yScale);

        void drawRotatedBitmap(Bitmap bitmap, int x, int y, float angle, float centerX, float centerY, float xScale, float yScale);

        Bitmap rotateBitmap(Bitmap bitmap, float angle, float centerX, float centerY, float xScale, float yScale);

        void drawTransformedBitmap(Bitmap bitmap, const Transform &transform, int x, int y);

        Bitmap transformBitmap(Bitmap bitmap, const Transform &transform);

        void drawSampledBitmap(Bitmap image, int x, int y, int width, int height, float centerX, float centerY, float dxx, float dyx, float dxy, float dyy, float dx, float dy, float z, float tiltAngle, bool tile);

        void drawBlurredBitmap(Bitmap bitmap, int x, int y, int radius, int numPasses, DitherType type, GraphicsFlip flip, int xPhase, int yPhase);

        Bitmap blurredBitmap(Bitmap bitmap, int radius, int numPasses, DitherType type, bool padEdges, int xPhase, int yPhase);

        void drawFadedBitmap(Bitmap bitmap, int x, int y, float alpha, DitherType type);

        Bitmap fadedBitmap(Bitmap bitmap, float alpha, DitherType type);

        Bitmap invertedBitmap(Bitmap bitmap);

        Bitmap blendedBitmap(Bitmap bitmap, Bitmap other, float alpha, DitherType type);

        Bitmap vcrPauseFilteredBitmap(Bitmap bitmap);

        void drawTileMap(TileMap tilemap, int x, int y, bool ignoreOffset = false, optional<IntRect> sourceRect = {});

        static bool checkBitmapMaskCollision(Bitmap bitmap1, int x1, int y1, GraphicsFlip flip1, Bitmap bitmap2, int x2, int y2, GraphicsFlip flip2, IntRect rect);

        void updateSprites();

        void drawSprites();

        void addSpriteDirtyRect(IntRect rect);

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
        Rom::Font systemFontSources[3]{};
        FontFamily systemFonts;
        uint32 displayBufferRGBA[DISPLAY_HEIGHT][DISPLAY_WIDTH]{};
        bool displayBufferNativeEndian = false;
        uint32 displayBufferOnColor = 0xB0AEA7FF;
        uint32 displayBufferOffColor = 0x302E27FF;
        BitmapRef frameBuffer, previousFrameBuffer;
        DisplayContext frameBufferContext{ frameBuffer };
        vector<DisplayContext> displayContextStack;
        map<PDLanguage, Rom::StringTable> localizedStrings;
        IntVec2 displayOffset{};
        int32 displayScale{};
        bool displayInverted{};
        bool displayFlippedX{}, displayFlippedY{};
        IntVec2 displayMosaic{};
        float framerate{};
        bool alwaysRedrawSprites{};
        unordered_set<Sprite> allocatedSprites;
        vector<SpriteRef> spriteDrawList;
        vector<IntRect> spriteDirtyRects;
    };

}
