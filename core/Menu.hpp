#pragma once

#include "PlaydateTypes.hpp"
#include "NativeResource.hpp"
#include "HeapAllocator.hpp"

#include <string>
#include <vector>

namespace cranked {

    class Cranked;

    class Menu;

// Todo: Cleanup with emulator reference
// Virtual heap strings must be manually freed (To avoid needing emulator reference)
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
        int32_t value{};
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

        void setImage(LCDBitmap_32 *bitmap, int xOffset);

        PDMenuItem_32 *addItem(const std::string &title, PDMenuItem_32::Type type, const std::vector<std::string> &options, int value, cref_t emulatedCallback, int luaCallback);

        void removeItem(PDMenuItem_32 *item);

        void clearItems();

        int findItem(PDMenuItem_32 *item);

        Cranked &cranked;
        bool isOpen{};
        std::vector<ResourcePtr<PDMenuItem_32>> items{MAX_ITEMS};
        ResourcePtr<LCDBitmap_32> image;
        int imageXOffset{};
    };

}
