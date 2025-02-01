#include "Menu.hpp"
#include "Cranked.hpp"

using namespace cranked;

PDMenuItem_32::PDMenuItem_32(Cranked &cranked, Type type)
        : NativeResource(cranked, ResourceType::MenuItem, this), type(type), options(cranked.heap.allocator<vheap_string>()), title(cranked.heap.allocator<char>()) {}

Menu::Menu(Cranked &cranked) : cranked(cranked) {}

void Menu::init() {
    buffer = cranked.graphics.createBitmap(DISPLAY_WIDTH, DISPLAY_HEIGHT);
}

void Menu::reset() {
    isOpen = false;
    imageXOffset = 0;
    itemCount = 0;
    selectedIndex = 0;
    for (auto &item : items)
        item.reset();
    image.reset();
    buffer.reset();
}

void Menu::update() {
    if (cranked.pressedInputs & (int)PDButtons::Menu) {
        if (isOpen)
            hideMenu();
        else
            showMenu();
    }
    if (isOpen) {
        if (cranked.pressedInputs & (int)PDButtons::Up)
            selectedIndex = max(0, selectedIndex - 1);
        else if (cranked.pressedInputs & (int)PDButtons::Down)
            selectedIndex = min(itemCount - 1, selectedIndex + 1);
        else if (cranked.pressedInputs & (int)PDButtons::A) {
            if (MenuItem item = items[selectedIndex]) {
                item->interactedWith = true;
                switch (item->type) {
                    case PDMenuItem_32::Type::Button:
                        hideMenu();
                        break;
                    case PDMenuItem_32::Type::Checkmark:
                        item->value = !item->value;
                        break;
                    case PDMenuItem_32::Type::Options:
                        item->value = (item->value + 1) % max(1, (int)item->options.size());
                        break;
                }
            }
        }
    }
}

void Menu::render() {
    ZoneScoped;
    if (not isOpen)
        return;
    buffer->clear(LCDSolidColor::Clear);
    if (image)
        buffer->drawBitmap(image, imageXOffset, 0);
    buffer->fillRect(DISPLAY_WIDTH / 2, 0, DISPLAY_WIDTH / 2, DISPLAY_HEIGHT, LCDSolidColor::White);
    buffer->drawRect(DISPLAY_WIDTH / 2, 0, DISPLAY_WIDTH / 2, DISPLAY_HEIGHT, LCDSolidColor::Black);

    int index = 0;
    for (auto &item : items) {
        if (not item)
            continue;
        bool selected = index == selectedIndex;
        if (selected)
            buffer->fillRect(DISPLAY_WIDTH / 2, index * 20, DISPLAY_WIDTH / 2, 20, LCDSolidColor::Black);
        BitmapDrawMode mode = selected ? LCDBitmapDrawMode::Inverted : LCDBitmapDrawMode::Copy;
        IntRect bounds = buffer->drawText(DISPLAY_WIDTH / 2 + 10, index * 20, item->title, cranked.graphics.systemFonts, nullptr, {}, {}, {}, {}, mode);
        if (item->type == PDMenuItem_32::Type::Options and item->value < (int)item->options.size())
            buffer->drawText(bounds.pos.x + bounds.size.x, index * 20, item->options[item->value], cranked.graphics.systemFonts, nullptr, {}, {}, {}, {}, mode);
        index++;
    }

    auto &displayBuffer = cranked.graphics.displayBufferRGBA;
    auto onColor = cranked.graphics.displayBufferOnColor;
    auto offColor = cranked.graphics.displayBufferOffColor;
    bool nativeEndian = cranked.graphics.displayBufferNativeEndian;
    for (int y = 0; y < DISPLAY_HEIGHT; y++) {
        for (int x = 0; x < DISPLAY_WIDTH; x++) {
            if (auto pixel = buffer->getPixel(x, y); pixel != LCDSolidColor::Clear) {
                auto color = pixel == LCDSolidColor::Black ? offColor : onColor;
                if (nativeEndian)
                    displayBuffer[y][x] = color;
                else
                    writeUint32BE((uint8 *) &displayBuffer[y][x], color);
            }
        }
    }
}

void Menu::showMenu() {
    if (isOpen)
        return;
    isOpen = true;
    if (cranked.luaEngine.isLoaded())
        cranked.luaEngine.invokeLuaCallback("gameWillPause");
    if (cranked.nativeEngine.isLoaded())
        cranked.nativeEngine.invokeEventCallback(PDSystemEvent::Pause, 0);
}

void Menu::hideMenu() {
    if (not isOpen)
        return;
    isOpen = false;

    for (int i = 0; i < MAX_ITEMS; i++) {
        auto &item = items[i];
        if (!item or not item->interactedWith)
            continue;
        item->interactedWith = false;
        if (item->emulatedCallback)
            cranked.nativeEngine.invokeEmulatedFunction<void, ArgType::void_t, ArgType::ptr_t>(item->emulatedCallback, item->userdata);
        if (!cranked.luaEngine.getQualifiedLuaGlobal("cranked.menuCallbacks"))
            throw std::runtime_error("Lua menuCallbacks table not found");
        lua_geti(cranked.getLuaContext(), -1, i + 1);
        if (item->type == PDMenuItem_32::Type::Checkmark)
            lua_pushboolean(cranked.getLuaContext(), item->value);
        else if (item->type == PDMenuItem_32::Type::Options)
            lua_pushstring(cranked.getLuaContext(), item->value < (int)item->options.size() ? item->options[item->value].c_str() : nullptr);
        lua_call(cranked.getLuaContext(), item->type == PDMenuItem_32::Type::Button ? 0 : 1, 0); // Todo: Protected?
        lua_pop(cranked.getLuaContext(), 1);
    }

    if (cranked.luaEngine.isLoaded())
        cranked.luaEngine.invokeLuaCallback("gameWillResume");
    if (cranked.nativeEngine.isLoaded())
        cranked.nativeEngine.invokeEventCallback(PDSystemEvent::Resume, 0);
}

void Menu::setImage(LCDBitmap_32 *bitmap, int xOffset) {
    this->imageXOffset = xOffset;
    image = bitmap ? cranked.heap.construct<LCDBitmap_32>(*bitmap) : nullptr; // Todo: Copy or just reference?
}

MenuItem Menu::addItem(const string &title, PDMenuItem_32::Type type, const vector<string> &options, int value, cref_t emulatedCallback, int luaCallback) {
    int index = -1;
    for (int i = 0; i < MAX_ITEMS; i++)
        if (!items[i]) {
            index = i;
            break;
        }
    if (index == -1)
        throw CrankedError("No free menu item slots");

    auto item = cranked.heap.construct<PDMenuItem_32>(cranked, type);
    item->title = cranked.heap.allocateString(title.c_str());
    for (auto &option : options)
        item->options.emplace_back(option, cranked.heap.allocator<char>());
    item->value = value;
    item->emulatedCallback = emulatedCallback;
    if (luaCallback) {
        if (!cranked.luaEngine.getQualifiedLuaGlobal("cranked.menuCallbacks"))
            throw std::runtime_error("Lua menuCallbacks table not found");
        lua_pushvalue(cranked.getLuaContext(), luaCallback);
        lua_seti(cranked.getLuaContext(), -2, index + 1);
        lua_pop(cranked.getLuaContext(), 1);
    }
    items[index] = item;
    itemCount++;
    return item;
}

void Menu::removeItem(MenuItem item) {
    int index = findItem(item);
    if (index < 0)
        return;
    if (cranked.luaEngine.isLoaded()) {
        if (!cranked.luaEngine.getQualifiedLuaGlobal("cranked.menuCallbacks"))
            throw std::runtime_error("Lua menuCallbacks table not found");
        lua_pushnil(cranked.getLuaContext());
        lua_seti(cranked.getLuaContext(), -2, index + 1);
        lua_pop(cranked.getLuaContext(), 1);
    }
    items[index] = nullptr;
    itemCount--;
}

void Menu::clearItems() {
    for (auto &item : items)
        removeItem(item);
    itemCount = 0;
}

int Menu::findItem(MenuItem item) {
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (items[i] == item)
            return i;
    }
    return -1;
}

MenuItem Menu::getSelectedItem() {
    int index = -1;
    for (auto &item : items) {
        if (!item)
            continue;
        if (selectedIndex == index)
            return item;
        index++;
    }
    return nullptr;
}
