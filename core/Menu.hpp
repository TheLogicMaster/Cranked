#pragma once

#include "PlaydateTypes.hpp"
#include "HeapAllocator.hpp"

namespace cranked {

    class Cranked;

    class Menu;

    struct PDMenuItem_32 : NativeResource {
        enum class Type {
            Button,
            Checkmark,
            Options,
        };

        PDMenuItem_32(Cranked &cranked, Type type);
        ~PDMenuItem_32() override = default;

        Type type{};

        vheap_vector<vheap_string> options;
        vheap_string title{};
        int32 value{};
        cref_t emulatedCallback{};
        cref_t userdata{};
    };

    class Menu {
    public:
        static constexpr int MAX_ITEMS = 3;

        explicit Menu(Cranked &cranked);

        void reset();

        void update();

        void render();

        void setImage(Bitmap bitmap, int xOffset);

        MenuItem addItem(const string &title, PDMenuItem_32::Type type, const vector<string> &options, int value, cref_t emulatedCallback, int luaCallback);

        void removeItem(MenuItem item);

        void clearItems();

        int findItem(MenuItem item);

        Cranked &cranked;
        bool isOpen{};
        MenuItemRef items[MAX_ITEMS];
        BitmapRef image;
        int imageXOffset{};
    };

}
