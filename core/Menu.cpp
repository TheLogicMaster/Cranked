#include "Menu.hpp"
#include "Cranked.hpp"

using namespace cranked;

Menu::Menu(Cranked *cranked) : cranked(cranked) {}

void Menu::reset() {
    isOpen = false;
    image = nullptr;
    imageXOffset = 0;
    for (auto &item : items)
        item = nullptr;
}

void Menu::update() {
    if (cranked->pressedInputs & int(PDButtons::Menu)) {
        if (isOpen) {
            cranked->invokeLuaCallback("gameWillResume");
            if (cranked->nativeEventCallback)
                cranked->invokeEmulatedFunction<int32_t, ArgType::int32_t, ArgType::uint32_t, ArgType::int32_t, ArgType::uint32_t>
                        (cranked->nativeEventCallback, API_ADDRESS, (int32_t) PDSystemEvent::Resume, uint32_t(0));
        } else {
            cranked->invokeLuaCallback("gameWillPause");
            if (cranked->nativeEventCallback)
                cranked->invokeEmulatedFunction<int32_t, ArgType::int32_t, ArgType::uint32_t, ArgType::int32_t, ArgType::uint32_t>
                        (cranked->nativeEventCallback, API_ADDRESS, (int32_t) PDSystemEvent::Pause, uint32_t(0));
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
        cranked->releaseLuaReference(image);
        cranked->heap.destruct(image);
        image = nullptr;
    }
    if (bitmap)
        image = cranked->heap.construct<LCDBitmap_32>(*bitmap); // Todo: Is making a copy okay?
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

    auto item = cranked->heap.construct<PDMenuItem_32>();
    item->type = type;
    item->title = cranked->heap.allocateString(title.c_str());
    for (auto &option : options)
        item->options.push_back(cranked->heap.allocateString(option.c_str()));
    item->value = value;
    item->emulatedCallback = emulatedCallback;
    if (luaCallback) {
        cranked->getQualifiedLuaGlobal("cranked.menuCallbacks");
        lua_pushvalue(cranked->getLuaContext(), luaCallback);
        lua_seti(cranked->getLuaContext(), -2, index + 1);
        lua_pop(cranked->getLuaContext(), 1);
    }
    return item;
}

void Menu::removeItem(PDMenuItem_32 *item) {
    if (!item)
        return;
    for (int i = 0; i < MAX_ITEMS; i++) {
        auto &it = items[i];
        if (it == item) {
            cranked->heap.free((void *) item->title);
            for (auto option: item->options)
                cranked->heap.free((void *) option);
            cranked->heap.destruct(item);
            if (cranked->hasLua) {
                cranked->getQualifiedLuaGlobal("cranked.menuCallbacks");
                lua_pushnil(cranked->getLuaContext());
                lua_seti(cranked->getLuaContext(), -2, i + 1);
                lua_pop(cranked->getLuaContext(), 1);
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
    cranked->heap.free((void *) item->title);
    item->title = cranked->heap.allocateString(title);
}

void Menu::assertHasItem(PDMenuItem_32 *item) {
    if (std::find(items.begin(), items.end(), item) == items.end())
        throw std::runtime_error("Invalid menu item");
}
