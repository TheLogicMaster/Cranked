#pragma once

#include "PlaydateTypes.hpp"

#include <string>
#include <vector>

// Virtual heap strings must be manually freed (To avoid needing emulator reference)
struct PDMenuItem_32 {
    enum class Type {
        Button,
        Checkmark,
        Options,
    } type;
    std::vector<const char *> options;
    const char *title;
    int value;
    cref_t emulatedCallback;
    cref_t userdata;
};

class Menu {
public:
    static constexpr int MAX_ITEMS = 3;

    explicit Menu(class Emulator *emulator);
    void reset();
    void update();
    void render();

    void setImage(LCDBitmap_32 *bitmap, int xOffset);
    PDMenuItem_32 *addItem(const std::string &title, PDMenuItem_32::Type type, const std::vector<std::string> &options, int value, cref_t emulatedCallback, int luaCallback);
    void removeItem(PDMenuItem_32 *item);
    void clearItems();
    void setItemTitle(PDMenuItem_32 *item, const char *title);
    void assertHasItem(PDMenuItem_32 *item);

    class Emulator *emulator;
    bool isOpen{};
    std::vector<PDMenuItem_32 *> items{MAX_ITEMS};
    LCDBitmap_32 *image{};
    int imageXOffset{};
};
