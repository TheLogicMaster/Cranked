#include "Menu.hpp"
#include "Emulator.hpp"

Menu::Menu(class Emulator *emulator) : emulator(emulator) {}

void Menu::reset() {
    isOpen = false;
    image = nullptr;
    imageXOffset = 0;
    for (auto &item : items)
        item = nullptr;
}

void Menu::update() {
    if (emulator->pressedInputs & int(PDButtons::Menu)) {
        if (isOpen) {
            emulator->invokeLuaCallback("gameWillResume");
            if (emulator->nativeEventCallback)
                emulator->invokeEmulatedFunction<int32_t, ArgType::int32_t, ArgType::uint32_t, ArgType::int32_t, ArgType::uint32_t>
                        (emulator->nativeEventCallback, API_ADDRESS, (int32_t) PDSystemEvent::Resume, uint32_t(0));
        } else {
            emulator->invokeLuaCallback("gameWillPause");
            if (emulator->nativeEventCallback)
                emulator->invokeEmulatedFunction<int32_t, ArgType::int32_t, ArgType::uint32_t, ArgType::int32_t, ArgType::uint32_t>
                        (emulator->nativeEventCallback, API_ADDRESS, (int32_t) PDSystemEvent::Pause, uint32_t(0));
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
    if (image) {
        emulator->releaseLuaReference(image);
        emulator->heap.destruct(image);
        image = nullptr;
    }
    if (bitmap)
        image = emulator->heap.construct<LCDBitmap_32>(*bitmap); // Todo: Is making a copy okay?
}

PDMenuItem_32 *Menu::addItem(const std::string &title, PDMenuItem_32::Type type, const std::vector<std::string> &options, int value, cref_t emulatedCallback, int luaCallback) {
    int index = -1;
    for (int i = 0; i < MAX_ITEMS; i++)
        if (!items[i]) {
            index = i;
            break;
        }
    if (index == -1)
        throw std::runtime_error("No free menu item slots");

    auto item = emulator->heap.construct<PDMenuItem_32>();
    item->type = type;
    item->title = emulator->heap.allocateString(title.c_str());
    for (auto &option : options)
        item->options.push_back(emulator->heap.allocateString(option.c_str()));
    item->value = value;
    item->emulatedCallback = emulatedCallback;
    if (luaCallback) {
        emulator->getQualifiedLuaGlobal("cranked.menuCallbacks");
        lua_pushvalue(emulator->getLuaContext(), luaCallback);
        lua_seti(emulator->getLuaContext(), -2, index + 1);
        lua_pop(emulator->getLuaContext(), 1);
    }
    return item;
}

void Menu::removeItem(PDMenuItem_32 *item) {
    if (!item)
        return;
    for (int i = 0; i < MAX_ITEMS; i++) {
        auto &it = items[i];
        if (it == item) {
            emulator->heap.free((void *) item->title);
            for (auto option: item->options)
                emulator->heap.free((void *) option);
            emulator->heap.destruct(item);
            if (emulator->hasLua) {
                emulator->getQualifiedLuaGlobal("cranked.menuCallbacks");
                lua_pushnil(emulator->getLuaContext());
                lua_seti(emulator->getLuaContext(), -2, i + 1);
                lua_pop(emulator->getLuaContext(), 1);
            }
            it = nullptr;
            break;
        }
    }
}

void Menu::clearItems() {
    for (auto item : items)
        removeItem(item);
}

void Menu::setItemTitle(PDMenuItem_32 *item, const char *title) {
    emulator->heap.free((void *) item->title);
    item->title = emulator->heap.allocateString(title);
}

void Menu::assertHasItem(PDMenuItem_32 *item) {
    if (std::find(items.begin(), items.end(), item) == items.end())
        throw std::runtime_error("Invalid menu item");
}
