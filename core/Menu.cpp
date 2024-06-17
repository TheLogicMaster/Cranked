#include "Menu.hpp"
#include "Cranked.hpp"

using namespace cranked;

PDMenuItem_32::PDMenuItem_32(Cranked &cranked, Type type)
        : NativeResource(cranked), type(type), options(cranked.heap.allocator<vheap_string>()), title(cranked.heap.allocator<char>()) {}

Menu::Menu(Cranked &cranked) : cranked(cranked) {}

void Menu::reset() {
    isOpen = false;
    image = nullptr;
    imageXOffset = 0;
    for (auto &item : items)
        item.reset();
    image.reset();
}

void Menu::update() {
    if (cranked.pressedInputs & int(PDButtons::Menu)) {
        if (isOpen) {
            cranked.luaEngine.invokeLuaCallback("gameWillResume");
            if (cranked.nativeEngine.isLoaded())
                cranked.nativeEngine.invokeEventCallback(PDSystemEvent::Resume, 0);
        } else {
            cranked.luaEngine.invokeLuaCallback("gameWillPause");
            if (cranked.nativeEngine.isLoaded())
                cranked.nativeEngine.invokeEventCallback(PDSystemEvent::Pause, 0);
        }
        isOpen = not isOpen;
    }
}

void Menu::render() {
    // Todo: Render over the final RGB buffer directly, potentially dithering and animating like the official implementation
    // Todo: Lock screen should probably also be handled by menu
}

void Menu::setImage(LCDBitmap_32 *bitmap, int xOffset) {
    this->imageXOffset = xOffset;
    image = bitmap ? cranked.heap.construct<LCDBitmap_32>(*bitmap) : nullptr; // Todo: Copy or just reference?
}

PDMenuItem_32 *Menu::addItem(const std::string &title, PDMenuItem_32::Type type, const std::vector<std::string> &options, int value, cref_t emulatedCallback, int luaCallback) {
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
        cranked.luaEngine.getQualifiedLuaGlobal("cranked.menuCallbacks");
        lua_pushvalue(cranked.getLuaContext(), luaCallback);
        lua_seti(cranked.getLuaContext(), -2, index + 1);
        lua_pop(cranked.getLuaContext(), 1);
    }
    return item;
}

void Menu::removeItem(PDMenuItem_32 *item) {
    int index = findItem(item);
    if (index < 0)
        return;
    if (cranked.luaEngine.isLoaded()) {
        cranked.luaEngine.getQualifiedLuaGlobal("cranked.menuCallbacks");
        lua_pushnil(cranked.getLuaContext());
        lua_seti(cranked.getLuaContext(), -2, index + 1);
        lua_pop(cranked.getLuaContext(), 1);
    }
    item->dereference();
    items[index] = nullptr;
}

void Menu::clearItems() {
    for (auto &item : std::vector(items))
        removeItem(item.get());
}

int Menu::findItem(PDMenuItem_32 *item) {
    auto found = std::find(items.begin(), items.end(), item);
    if (found == items.end())
        return -1;
    return int(found - items.begin());
}
