#include "LuaRuntime.hpp"
#include "gen/LuaRuntimeSource.hpp"

using namespace cranked;

static LuaRet import_lua(Cranked *cranked, const char *name) {
    if (cranked->luaEngine.loadedLuaFiles.contains(name))
        return 0;
    for (auto &file: cranked->rom->pdzFiles)
        if (file.type == Rom::FileType::LUAC and file.name == name) { // Name gets extension removed at compile time
            luaL_loadbuffer(cranked->getLuaContext(), (char *) file.data.data(), file.data.size(), file.name.c_str());
            if (lua_pcall(cranked->getLuaContext(), 0, 0, 0) != LUA_OK) {
                luaL_error(cranked->getLuaContext(), "Failed to load module `%s`: %s", name, lua_tostring(cranked->getLuaContext(), -1));
                return 1;
            }
            cranked->luaEngine.loadedLuaFiles.emplace(name);
            return 0;
        }
    luaL_error(cranked->getLuaContext(), "Failed to load module: %s", name);
    return 1;
}

static LuaRet table_create_lua(lua_State *context, int arrayCount, int hashCount) {
    lua_createtable(context, arrayCount, hashCount);
    return 1;
}

static int playdate_apiVersion_lua(lua_State *luaContext) {
    lua_pushinteger(luaContext, 20000); // Todo: Determine mapping from semantic version to integer
    lua_pushinteger(luaContext, 10000);
    return 2;
}

static void playdate_wait_lua(Cranked *cranked, int millis) {
    cranked->suspendUpdateLoopUntil = chrono::system_clock::now() + chrono::milliseconds(millis);
}

static void playdate_stop_lua(Cranked *cranked) {
    cranked->disableUpdateLoop = true;
}

static void playdate_start_lua(Cranked *cranked) {
    cranked->disableUpdateLoop = false;
}

static int playdate_getSystemLanguage_lua(Cranked *cranked) {
    return int(cranked->systemLanguage);
}

static LuaRet playdate_getCrankChange_lua(Cranked *cranked) {
    auto change = cranked->getCrankChange();
    lua_pushnumber(cranked->getLuaContext(), change);
    lua_pushnumber(cranked->getLuaContext(), change); // Todo: Apply `acceleration` scaling (Maybe scale up a bit after a certain threshold)
    return 2;
}

static void playdate_setCrankSoundsDisabled_lua(Cranked *cranked, bool disabled) {
    cranked->crankSoundsEnabled = not disabled;
}

static PDButtons buttonStringToEnum(const string &button) {
    if (button == "left")
        return PDButtons::Left;
    else if (button == "right")
        return PDButtons::Right;
    else if (button == "up")
        return PDButtons::Up;
    else if (button == "down")
        return PDButtons::Down;
    else if (button == "b")
        return PDButtons::B;
    else if (button == "a")
        return PDButtons::A;
    else
        return PDButtons(0);
}

static bool playdate_buttonIsPressed_lua(Cranked *cranked, LuaVal button) {
    if (button.isInt())
        return button.asInt() & cranked->currentInputs;
    auto btnString = button.asString();
    if (!btnString)
        return false;
    return cranked->currentInputs & int(buttonStringToEnum(btnString));
}

static bool playdate_buttonJustPressed_lua(Cranked *cranked, LuaVal button) {
    if (button.isInt())
        return button.asInt() & cranked->pressedInputs;
    auto btnString = button.asString();
    if (!btnString)
        return false;
    return cranked->pressedInputs & int(buttonStringToEnum(btnString));
}

static bool playdate_buttonJustReleased_lua(Cranked *cranked, LuaVal button) {
    if (button.isInt())
        return button.asInt() & cranked->releasedInputs;
    auto btnString = button.asString();
    if (!btnString)
        return false;
    return cranked->releasedInputs & int(buttonStringToEnum(btnString));
}

static LuaRet playdate_getButtonState_lua(Cranked *cranked) {
    lua_pushinteger(cranked->getLuaContext(), cranked->currentInputs);
    lua_pushinteger(cranked->getLuaContext(), cranked->pressedInputs);
    lua_pushinteger(cranked->getLuaContext(), cranked->releasedInputs);
    return 3;
}

static LuaRet playdate_file_modtime_lua(lua_State *context, const char *path) {
    FileStat_32 stat{}; // Official implementation returns garbage on non-existent file, so don't worry about error checking
    Cranked::fromLuaContext(context)->files.stat(path, stat);
    lua_createtable(context, 0, 6);
    LuaVal table(context, lua_gettop(context));
    table.setIntField("year", stat.m_year);
    table.setIntField("month", stat.m_month);
    table.setIntField("day", stat.m_day);
    table.setIntField("hour", stat.m_hour);
    table.setIntField("minute", stat.m_minute);
    table.setIntField("second", stat.m_second);
    return 1;
}

static int playdate_file_getSize_lua(Cranked *context, const char *path) {
    FileStat_32 stat{};
    if (context->files.stat(path, stat))
        return -1;
    return (int) stat.size;
}

static LuaRet playdate_file_getType_lua(lua_State *context, const char *path) {
    auto type = Cranked::fromLuaContext(context)->files.getType(path);
    if (type)
        lua_pushstring(context, type);
    else
        lua_pushnil(context);
    return 1;
}

static bool playdate_file_isdir_lua(Cranked *context, const char *path) {
    FileStat_32 stat{};
    context->files.stat(path, stat);
    return stat.isdir;
}

static LuaRet playdate_file_listFiles_lua(lua_State *context, const char *path, bool showHidden) {
    vector<string> files;
    if (Cranked::fromLuaContext(context)->files.listFiles(path, showHidden, files)) {
        lua_pushnil(context);
        return 1;
    }
    lua_createtable(context, (int) files.size(), 0);
    LuaVal table(context, lua_gettop(context));
    for (int i = 0; i < files.size(); i++)
        table.setStringElement(i + 1, files[i].c_str());
    return 1;
}

static bool playdate_file_delete_lua(Cranked *context, const char *path, bool recursive) {
    return !context->files.unlink(path, recursive);
}

static bool playdate_file_exists_lua(Cranked *context, const char *path) {
    return context->files.exists(path);
}

static LuaRet playdate_file_open_lua(lua_State *context, const char *path, int mode) {
    constexpr int kFileRead = 3;
    constexpr int kFileWrite = 4;
    constexpr int kFileAppend = 8;
    FileOptions openMode;
    switch (mode) {
        case kFileWrite:
            openMode = FileOptions::Write;
            break;
        case kFileAppend:
            openMode = FileOptions::Append;
            break;
        case kFileRead:
        default:
            openMode = FileOptions::ReadDataFallback;
    }
    auto cranked = Cranked::fromLuaContext(context);
    auto file = cranked->files.open(path, openMode);
    if (!file) {
        lua_pushnil(context);
        lua_pushstring(context, cranked->fromVirtualAddress<const char>(cranked->files.lastError));
        return 2;
    }
    pushFile(file);
    return 1;
}

static void playdate_file_close_lua(lua_State *context, File file) {
    Cranked::fromLuaContext(context)->files.close(file);
}

static LuaRet playdate_file_write_lua(Cranked *context, File file, const char *string) {
    auto result = context->files.write(file, (void *) string, (int) strlen(string));
    if (result < 0) {
        lua_pushinteger(context->getLuaContext(), 0);
        lua_pushstring(context->getLuaContext(), context->fromVirtualAddress<const char>(context->files.lastError));
        return 2;
    }
    lua_pushinteger(context->getLuaContext(), result);
    return 1;
}

static LuaRet playdate_file_read_lua(Cranked *context, File file, int bytes) {
    vector<uint8> buffer(bytes);
    auto result = context->files.read(file, (void *) buffer.data(), bytes);
    if (result < 0) {
        lua_pushnil(context->getLuaContext());
        lua_pushstring(context->getLuaContext(), context->fromVirtualAddress<const char>(context->files.lastError));
        return 2;
    }
    lua_pushlstring(context->getLuaContext(), (const char *) buffer.data(), result);
    lua_pushinteger(context->getLuaContext(), result);
    return 2;
}

static LuaRet playdate_file_readline_lua(lua_State *context, File file) {
    auto result = Cranked::fromLuaContext(context)->files.readline(file);
    if (result.empty())
        lua_pushnil(context);
    else
        lua_pushstring(context, result.c_str());
    return 1;
}

static int playdate_file_tell_lua(Cranked *context, File file) {
    return context->files.tell(file); // Todo: Not sure about error handling
}

static void playdate_file_seek_lua(Cranked *context, File file, int offset) {
    context->files.seek(file, offset, SEEK_SET);
}

static LuaRet json_decode_lua(lua_State *context, const char *str) {
    function<void(const nlohmann::json&)> createTable;
    createTable = [&](const nlohmann::json &json){
        for (auto &entry : json.items()) {
            auto &value = entry.value();
            if (value.is_null())
                lua_pushnil(context);
            else if (value.is_boolean())
                lua_pushboolean(context, value.get<bool>());
            else if (value.is_number_integer())
                lua_pushinteger(context, value.get<int>());
            else if (value.is_number_float())
                lua_pushnumber(context, value.get<float>());
            else if (value.is_string())
                lua_pushstring(context, value.get<string>().c_str());
            else
                createTable(value);
            if (json.is_array())
                lua_seti(context, -2, stoi(entry.key()));
            else
                lua_setfield(context, -2, entry.key().c_str());
        }
    };
    createTable(nlohmann::json(str));
    return 1;
}

static LuaRet json_decodeFile_lua(lua_State *context, File file) {
    auto cranked = Cranked::fromLuaContext(context);
    int size = 0;
    constexpr int BUFFER_SEGMENT = 512;
    vector<char> buffer;
    while (true) {
        auto returned = cranked->files.read(file, buffer.data() + size, (int) buffer.size() - size);
        if (returned < 0)
            return luaL_error(context, cranked->fromVirtualAddress<char>(cranked->files.lastError));
        if (returned < BUFFER_SEGMENT)
            break;
        size += returned;
        buffer.resize(buffer.size() + BUFFER_SEGMENT);
    }
    return json_decode_lua(context, buffer.data());
}

static string jsonEncodeTable(lua_State *context, LuaVal table, bool pretty) {
    function<void(nlohmann::json *)> encodeTable;
    encodeTable = [&](nlohmann::json *jsonValue){
        lua_pushnil(context);
        while (lua_next(context, -2)) {
            lua_pushvalue(context, -2);
            nlohmann::json *newValue;
            if (lua_isinteger(context, -1))
                newValue = &(*jsonValue)[lua_tointeger(context, -1)];
            else
                newValue = &(*jsonValue)[lua_tostring(context, -1)];
            if (lua_isinteger(context, -2))
                *newValue = lua_tointeger(context, -2);
            else if (lua_isboolean(context, -2))
                *newValue = (bool) lua_toboolean(context, -2);
            else if (lua_isnumber(context, -2))
                *newValue = lua_tonumber(context, -2);
            else if (lua_isstring(context, -2))
                *newValue = lua_tostring(context, -2);
            else if (lua_isnil(context, -2))
                *newValue = nullptr;
            else {
                lua_pushvalue(context, -2);
                encodeTable(newValue);
            }
            lua_pop(context, 2);
        }
        lua_pop(context, 1);
    };
    nlohmann::json json;
    lua_pushvalue(context, table);
    encodeTable(&json);
    return json.dump(pretty ? 4 : 0);
}

static string *json_encode_lua(lua_State *context, LuaVal table) {
    return new string(jsonEncodeTable(context, table, false));
}

static string *json_encodePretty_lua(lua_State *context, LuaVal table) {
    return new string(jsonEncodeTable(context, table, true));
}

static void json_encodeToFile_lua(lua_State *context, File file, LuaVal pretty, LuaVal table) {
    bool isPretty = false;
    if (!pretty.isTable())
        table = pretty;
    else
        isPretty = pretty.asBool();
    playdate_file_write_lua(Cranked::fromLuaContext(context), file, jsonEncodeTable(context, table, isPretty).c_str());
}

static LuaRet json_null_tostring_lua(lua_State *context) {
    lua_pushstring(context, "null");
    return 1;
}

static void playdate_datastore_write_lua(lua_State *context, LuaVal table, LuaVal filename, LuaVal pretty) {
    bool isPretty = false;
    string name = "data";
    if (filename.isString()) {
        name = filename.asString();
        isPretty = pretty.asBool();
    } else
        isPretty = filename.asBool();
    name += ".json";
    auto cranked = Cranked::fromLuaContext(context);
    auto file = cranked->files.open(name.c_str(), FileOptions::Write);
    playdate_file_write_lua(Cranked::fromLuaContext(context), file, jsonEncodeTable(context, table, isPretty).c_str());
    cranked->files.close(file);
}

static LuaRet playdate_datastore_read_lua(Cranked *context, LuaVal filename) {
    string name(filename.isString() ? filename.asString() : "data");
    name += ".json";
    if (!context->files.exists(name)) {
        lua_pushnil(context->getLuaContext());
        return 1;
    }
    auto file = context->files.open(name.c_str(), FileOptions::ReadData);
    json_decodeFile_lua(context->getLuaContext(), file);
    context->files.close(file);
    return 1;
}

static bool playdate_datastore_delete_lua(Cranked *context, LuaVal filename) {
    string name(filename.isString() ? filename.asString() : "data");
    name += ".json";
    return context->files.unlink(name, false) == 0;
}

static void playdate_datastore_writeImage_lua(Cranked *cranked, LuaVal image, const char *path) {
    // Todo
}

static LuaRet playdate_datastore_readImage_lua(Cranked *cranked, const char *path) {
    string stringPath(path);
    if (stringPath.find('/') == string::npos)
        stringPath = "images/" + stringPath;
    try {
        pushImage(cranked->graphics.getImage(stringPath));
    } catch (exception &ex) {
        lua_pushnil(cranked->getLuaContext());
    }
    return 1;
}

static LuaRet playdate_graphics_image_new_lua(Cranked *cranked, LuaVal width, int height, int bgcolor) {
    Bitmap image;
    if (width.isString()) {
        auto path = width.asString();
        try {
            image = cranked->graphics.getImage(path);
        } catch (exception &ex) {
            lua_pushnil(cranked->getLuaContext());
            return 1;
        }
    } else {
        image = cranked->graphics.createBitmap(width, height);
        image->clear(bgcolor);
    }
    pushImage(image);
    return 1;
}

static void playdate_graphics_image_gc_lua(Cranked *cranked, LuaVal image) {
    releaseUserdataResource(image);
}

static LuaRet playdate_graphics_image_load_lua(Cranked *cranked, Bitmap image, const char *path) {
    try {
        BitmapRef loaded(cranked->graphics.getImage((const char *) path));
        *image = *loaded;
    } catch (exception &ex) {
        lua_pushboolean(cranked->getLuaContext(), false);
        lua_pushstring(cranked->getLuaContext(), ex.what());
        return 2;
    }
    lua_pushboolean(cranked->getLuaContext(), true);
    return 1;
}

static LuaRet playdate_graphics_image_copy_lua(Cranked *cranked, Bitmap image) {
    pushImage(cranked->heap.construct<LCDBitmap_32>(*image));
    return 1;
}

static LuaRet playdate_graphics_image_getSize_lua(Cranked *cranked, Bitmap image) {
    lua_pushinteger(cranked->getLuaContext(), image->width);
    lua_pushinteger(cranked->getLuaContext(), image->height);
    return 2;
}

static LuaRet playdate_graphics_imageSizeAtPath_lua(Cranked *cranked, const char *path) {
    // Todo: Error handling? Data image files?
    auto image = cranked->rom->getImage(path);
    lua_pushinteger(cranked->getLuaContext(), image.width);
    lua_pushinteger(cranked->getLuaContext(), image.height);
    return 2;
}

static void playdate_graphics_image_draw_lua(Cranked *cranked, Bitmap image, LuaVal arg1) {
    int x, y;
    LCDBitmapFlip flip{};
    optional<IntRect> sourceRect;
    LuaVal flipVal(cranked->getLuaContext(), arg1 + 1);
    if (arg1.isTable()) {
        x = arg1.getIntField("x");
        y = arg1.getIntField("y");
    } else {
        x = arg1.asInt();
        y = (arg1 + 1).asInt();
        flipVal = arg1 + 2;
    }
    if (!flipVal.isNil()) {
        flip = flipVal.isString() ? bitmapFlipFromString(flipVal.asString()) : LCDBitmapFlip(flipVal.asInt());
        LuaVal rectVal = flipVal + 1;
        if (rectVal.isTable())
            sourceRect = rectVal.asIntRect();
        else if (!rectVal.isNil())
            sourceRect = IntRect{ rectVal.asInt(), (rectVal + 1).asInt(), (rectVal + 2).asInt(), (rectVal + 3).asInt() };
    }
    cranked->graphics.drawBitmap(image, x, y, flip, false, sourceRect);
}

static void playdate_graphics_image_drawIgnoringOffset_lua(Cranked *cranked, Bitmap image, LuaVal x, LuaVal y, LuaVal flip) {
    int drawX, drawY;
    LCDBitmapFlip flipped{};
    if (x.isTable()) {
        drawX = x.getIntField("x");
        drawY = x.getIntField("y");
        flip = y;
    } else {
        drawX = x.asInt();
        drawY = y.asInt();
    }
    if (!flip.isNil())
        flipped = flip.isString() ? bitmapFlipFromString(flip.asString()) : LCDBitmapFlip(flip.asInt());
    cranked->graphics.drawBitmap(image, x, y, flipped, true);
}

static void playdate_graphics_image_clear_lua(Bitmap image, int color) { // Todo: Verify parameter offsets are correct without context
    image->clear(LCDSolidColor(color));
}

static int playdate_graphics_image_sample_lua(Bitmap image, int x, int y) {
    return (int) image->getPixel(x, y);
}

static void playdate_graphics_image_drawRotated_lua(Cranked *cranked, Bitmap image, int x, int y, float angle, LuaVal scale, LuaVal yScale) {
    // Coordinates are centered
    // Todo
}

static LuaRet playdate_graphics_image_rotatedImage_lua(Cranked *cranked, Bitmap image, float angle, LuaVal scale, LuaVal yScale) {
    // Todo
    return 0;
}

static void playdate_graphics_image_drawScaled_lua(Cranked *cranked, Bitmap image, int x, int y, float scale, LuaVal yScale) {
    // Todo
}

static LuaRet playdate_graphics_image_scaledImage_lua(Cranked *cranked, Bitmap image, float scale, LuaVal yScale) {
    // Todo
    return 0;
}

static void playdate_graphics_image_drawWithTransform_lua(Cranked *cranked, Bitmap image, LuaVal xform, int x, int y) {
    // Coordinates are centered
    // Todo
}

static LuaRet playdate_graphics_image_transformedImage_lua(Cranked *cranked, Bitmap image) {
    // Todo
    return 0;
}

static void playdate_graphics_image_drawSampled_lua(Cranked *cranked, Bitmap image, int x, int y, int width, int height, float centerX, float centerY, float dxx, float dyx, float dxy, float dyy, float dx, float dy, float z, float tiltAngle, bool tile) {
    // Basically `Mode 7`
    // Todo
}

static void playdate_graphics_image_setMaskImage_lua(Cranked *cranked, Bitmap image, Bitmap maskImage) {
    image->mask = cranked->heap.construct<LCDBitmap_32>(*maskImage);
}

static LuaRet playdate_graphics_image_getMaskImage_lua(Cranked *cranked, Bitmap image) {
    pushImage(image);
    return 1;
}

static void playdate_graphics_image_addMask_lua(Cranked *cranked, Bitmap image, bool opaque) {
    image->mask = cranked->heap.construct<LCDBitmap_32>(*cranked, image->width, image->height);
}

static void playdate_graphics_image_removeMask_lua(Cranked *cranked, Bitmap image) {
    image->mask = nullptr;
}

static bool playdate_graphics_image_hasMask_lua(Cranked *cranked, Bitmap image) {
    return image->mask;
}

static void playdate_graphics_image_clearMask_lua(Cranked *cranked, Bitmap image, bool opaque) {
    if (image->mask)
        image->mask->clear(opaque ? LCDSolidColor::White : LCDSolidColor::Black);
}

static void playdate_graphics_image_drawTiled_lua(Cranked *cranked, Bitmap image, LuaVal arg1) {
    // Todo
}

static void playdate_graphics_image_drawBlurred_lua(Cranked *cranked, Bitmap image, int x, int y, float radius, int numPasses, DitherType ditherType, LuaVal arg1, LuaVal arg2, LuaVal arg3) {
    // Todo
}

static void playdate_graphics_image_drawFaded_lua(Cranked *cranked, Bitmap image, int x, int y, float alpha, DitherType ditherType) {
    // Todo
}

static void playdate_graphics_image_setInverted_lua(Cranked *cranked, Bitmap image, bool flag) {
    image->inverted = flag;
}

static LuaRet playdate_graphics_image_invertedImage_lua(Cranked *cranked, Bitmap image) {
    // Todo: Does this modify the mask?
    return 0;
}

static LuaRet playdate_graphics_image_blendWithImage_lua(Cranked *cranked, Bitmap self, Bitmap image, float alpha, DitherType ditherType) {
    // Todo
    return 0;
}

static LuaRet playdate_graphics_image_blurredImage_lua(Cranked *cranked, Bitmap image, float radius, int numPasses, DitherType ditherType, bool padEdges, int xPhase, int yPhase) {
    // Todo
    return 0;
}

static LuaRet playdate_graphics_image_fadedImage_lua(Cranked *cranked, Bitmap image, float alpha, DitherType ditherType) {
    // Todo
    return 0;
}

static LuaRet playdate_graphics_image_vcrPauseFilterImage_lua(Cranked *cranked, Bitmap image) {
    // Todo
    return 0;
}

static void playdate_graphics_pushContext_lua(Cranked *cranked, LuaVal imageVal) {
    auto image = imageVal.isUserdataObject() ? imageVal.asUserdataObject<LCDBitmap_32>() : nullptr;
    if (image)
        cranked->luaEngine.preserveLuaReference(image, imageVal);
    cranked->graphics.pushContext(image);
}

static void playdate_graphics_clear_lua(Cranked *cranked, LuaVal color) {
    cranked->graphics.frameBuffer->clear(color.isNil() ? cranked->graphics.getCurrentDisplayContext().backgroundColor : (LCDSolidColor) color.asInt());
}

static bool playdate_graphics_checkAlphaCollision_lua(Cranked *cranked, Bitmap image1, int x1, int y1, LuaVal flip1, Bitmap image2, int x2, int y2, LuaVal flip2) {
    // Todo
    return false;
}

static void playdate_graphics_setColor_lua(Cranked *cranked, LCDSolidColor color) {
    // Todo: Disable pattern/stencil stuff
    cranked->graphics.getCurrentDisplayContext().drawingColor = color;
}

static LCDSolidColor playdate_graphics_getColor_lua(Cranked *cranked) {
    return cranked->graphics.getCurrentDisplayContext().drawingColor;
}

static LCDSolidColor playdate_graphics_getBackgroundColor_lua(Cranked *cranked) {
    return cranked->graphics.getCurrentDisplayContext().backgroundColor;
}

static void playdate_graphics_setPattern_lua(Cranked *cranked, LuaVal pattern, int x, int y) {
    // Todo
}

static void playdate_graphics_setDitherPattern_lua(Cranked *cranked, float alpha, DitherType ditherType) {
    // Todo
}

static void playdate_graphics_drawLine_lua(Cranked *cranked, LuaVal x1, int y1, int x2, int y2) {
    auto &context = cranked->graphics.getCurrentDisplayContext();
    if (x1.isTable())
        cranked->graphics.drawLine(x1.getIntField("x1"), x1.getIntField("y1"), x1.getIntField("x2"), x1.getIntField("y2"), context.lineWidth, context.drawingColor);
    else
        cranked->graphics.drawLine(x1.asInt(), y1, x2, y2, context.lineWidth, context.drawingColor);
}

static void playdate_graphics_setLineCapStyle_lua(Cranked *cranked, LCDLineCapStyle style) {
    cranked->graphics.getCurrentDisplayContext().lineEndCapStyle = style;
}

static void playdate_graphics_drawPixel_lua(Cranked *cranked, LuaVal x, int y) {
    if (x.isTable())
        cranked->graphics.drawPixel(x.getIntField("x"), x.getIntField("y"), cranked->graphics.getCurrentDisplayContext().drawingColor);
    else
        cranked->graphics.drawPixel(x.asInt(), y, cranked->graphics.getCurrentDisplayContext().drawingColor);
}

static void playdate_graphics_drawRect_lua(Cranked *cranked, LuaVal x, int y, int w, int h) {
    auto &context = cranked->graphics.getCurrentDisplayContext();
    if (x.isTable())
        cranked->graphics.drawRect(x.getIntField("x"), x.getIntField("y"), x.getIntField("width"), x.getIntField("height"), context.drawingColor);
    else
        cranked->graphics.drawRect(x.asInt(), y, w, h, context.drawingColor);
}

static void playdate_graphics_fillRect_lua(Cranked *cranked, LuaVal x, int y, int w, int h) {
    auto &context = cranked->graphics.getCurrentDisplayContext();
    if (x.isTable())
        cranked->graphics.fillRect(x.getIntField("x"), x.getIntField("y"), x.getIntField("width"), x.getIntField("height"), context.drawingColor);
    else
        cranked->graphics.fillRect(x.asInt(), y, w, h, context.drawingColor);
}

static void playdate_graphics_drawRoundRect_lua(Cranked *cranked, LuaVal x, int y, int w, int h, int radius) {
    auto &context = cranked->graphics.getCurrentDisplayContext();
    if (x.isTable())
        cranked->graphics.drawRoundRect(x.getIntField("x"), x.getIntField("y"), x.getIntField("width"), x.getIntField("height"), y, context.drawingColor);
    else
        cranked->graphics.drawRoundRect(x.asInt(), y, w, h, radius, context.drawingColor);
}

static void playdate_graphics_fillRoundRect_lua(Cranked *cranked, LuaVal x, int y, int w, int h, int radius) {
    auto &context = cranked->graphics.getCurrentDisplayContext();
    if (x.isTable())
        cranked->graphics.fillRoundRect(x.getIntField("x"), x.getIntField("y"), x.getIntField("width"), x.getIntField("height"), y, context.drawingColor);
    else
        cranked->graphics.fillRoundRect(x.asInt(), y, w, h, radius, context.drawingColor);
}

static void drawEllipse(Cranked *cranked, LuaVal arg1, bool filled) {
    auto &context = cranked->graphics.getCurrentDisplayContext();
    IntRect rect{};
    LuaVal startAngleArg = arg1 + 4;
    if (arg1.isTable()) {
        rect = arg1.asIntRect();
        startAngleArg = arg1 + 1;
    } else
        rect = {arg1.asInt(), (arg1 + 1).asInt(), (arg1 + 2).asInt(), (arg1 + 3).asInt()};
    float startAngle = startAngleArg.asFloat();
    float endAngle = (startAngleArg + 1).isNil() ? 360 : (startAngleArg + 1).asFloat();
    cranked->graphics.drawEllipse(rect.pos.x, rect.pos.y, rect.size.x, rect.size.y, context.lineWidth, startAngle, endAngle, context.drawingColor, filled);
}

static void playdate_graphics_drawEllipseInRect_lua(Cranked *cranked, LuaVal arg1) {
    drawEllipse(cranked, arg1, false);
}

static void playdate_graphics_fillEllipseInRect_lua(Cranked *cranked, LuaVal arg1) {
    drawEllipse(cranked, arg1, true);
}

static void playdate_graphics_drawPolygon_lua(Cranked *cranked, LuaVal arg1) {
    // Todo
}

static void playdate_graphics_fillPolygon_lua(Cranked *cranked, LuaVal arg1) {
    // Todo
}

static void playdate_graphics_setPolygonFillRule_lua(Cranked *cranked, LCDPolygonFillRule rule) {
    cranked->graphics.getCurrentDisplayContext().polygonFillRule = rule;
}

static void playdate_graphics_drawTriangle_lua(Cranked *cranked, int x1, int y1, int x2, int y2, int x3, int y3) {
    cranked->graphics.drawTriangle(x1, y1, x2, y2, x3, y3, cranked->graphics.getCurrentDisplayContext().drawingColor);
}

static void playdate_graphics_fillTriangle_lua(Cranked *cranked, int x1, int y1, int x2, int y2, int x3, int y3) {
    cranked->graphics.fillTriangle(x1, y1, x2, y2, x3, y3, cranked->graphics.getCurrentDisplayContext().drawingColor);
}

static LuaRet playdate_graphics_perlin_lua(Cranked *cranked, float x, float y, float z, float repeat, int octaves, float persistence) {
    // Todo
    return 0;
}

static LuaRet playdate_graphics_perlinArray_lua(Cranked *cranked, int count, float x, float dx, LuaVal next) {
    // Todo
    return 0;
}

static void playdate_graphics_setClipRect_lua(Cranked *cranked, LuaVal arg1, int y, int width, int height) {
    if (arg1.isTable())
        cranked->graphics.getCurrentDisplayContext().clipRect = arg1.asIntRect();
    else
        cranked->graphics.getCurrentDisplayContext().clipRect = {arg1.asInt(), y, width, height};
}

static LuaRet playdate_graphics_getClipRect_lua(Cranked *cranked) {
    pushRect(cranked, cranked->graphics.getCurrentDisplayContext().clipRect);
    return 1;
}

static void playdate_graphics_setScreenClipRect_lua(Cranked *cranked, LuaVal arg1, int y, int width, int height) {
    auto &displayContext = cranked->graphics.getCurrentDisplayContext();
    auto rect = arg1.isTable() ? arg1.asIntRect() : IntRect{arg1.asInt(), y, width, height};
    displayContext.clipRect = rect + displayContext.drawOffset; // Todo: Verify offset direction
}

static LuaRet playdate_graphics_getScreenClipRect_lua(Cranked *cranked) {
    auto &displayContext = cranked->graphics.getCurrentDisplayContext();
    pushRect(cranked, displayContext.clipRect - displayContext.drawOffset); // Todo: Verify offset direction
    return 1;
}

static void playdate_graphics_setStencilImage_lua(Cranked *cranked, LuaVal image, bool tile) {
    // Todo
}

static void playdate_graphics_setStencilPattern_lua(Cranked *cranked, LuaVal arg1) {
    // Todo
}

static void playdate_graphics_clearStencil_lua(Cranked *cranked) {
    // Todo
}

static LCDBitmapDrawMode playdate_graphics_getImageDrawMode_lua(Cranked *cranked) {
    return cranked->graphics.getCurrentDisplayContext().bitmapDrawMode;
}

static void playdate_graphics_setLineWidth_lua(Cranked *cranked, int width) {
    cranked->graphics.getCurrentDisplayContext().lineWidth = width;
}

static int playdate_graphics_getLineWidth_lua(Cranked *cranked) {
    return cranked->graphics.getCurrentDisplayContext().lineWidth;
}

static void playdate_graphics_setStrokeLocation_lua(Cranked *cranked, StrokeLocation location) {
    cranked->graphics.getCurrentDisplayContext().strokeLocation = location;
}

static StrokeLocation playdate_graphics_getStrokeLocation_lua(Cranked *cranked) {
    return cranked->graphics.getCurrentDisplayContext().strokeLocation;
}

static void playdate_graphics_lockFocus_lua(Cranked *cranked, LuaVal imageVal) {
    cranked->graphics.getCurrentDisplayContext().focusedImage = imageVal.asUserdataObject<LCDBitmap_32>();
}

static void playdate_graphics_unlockFocus_lua(Cranked *cranked) {
    cranked->graphics.getCurrentDisplayContext().focusedImage = nullptr;
}

static LuaRet playdate_graphics_getDrawOffset_lua(Cranked *cranked) {
    auto &offset = cranked->graphics.getCurrentDisplayContext().drawOffset;
    lua_pushinteger(cranked->getLuaContext(), offset.x);
    lua_pushinteger(cranked->getLuaContext(), offset.y);
    return 2;
}

static LuaRet playdate_graphics_getDisplayImage_lua(Cranked *cranked) {
    pushImage(cranked->heap.construct<LCDBitmap_32>(*cranked->graphics.frameBuffer));
    return 1;
}

static LuaRet playdate_graphics_getWorkingImage_lua(Cranked *cranked) {
    pushImage(cranked->heap.construct<LCDBitmap_32>(*cranked->graphics.previousFrameBuffer));
    return 1;
}

static LuaRet playdate_getSecondsSinceEpoch_lua(lua_State *context) {
    lua_pushinteger(context, (int) duration_cast<chrono::seconds>(chrono::system_clock::now().time_since_epoch()).count());
    lua_pushinteger(context, (int) duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count());
    return 2;
}

static LuaRet playdate_getTime_lua(Cranked *cranked) {
    // Todo: Timezone offset
    pushTime(cranked, chrono::system_clock::now());
    return 1;
}

static LuaRet playdate_getGMTTime_lua(Cranked *cranked) {
    pushTime(cranked, chrono::system_clock::now());
    return 1;
}

static LuaRet playdate_epochFromTime_lua(Cranked *cranked, LuaVal time) {
    // Todo: Timezone offset
    auto point = time.asTime();
    auto secondsPoint = chrono::floor<chrono::seconds>(point);
    auto millis = point - secondsPoint;
    lua_pushinteger(cranked->getLuaContext(), (int) duration_cast<chrono::seconds>(secondsPoint.time_since_epoch()).count());
    lua_pushinteger(cranked->getLuaContext(), (int) duration_cast<chrono::milliseconds>(millis).count());
    return 2;
}

static LuaRet playdate_epochFromGMTTime_lua(Cranked *cranked, LuaVal time) {
    auto point = time.asTime();
    auto secondsPoint = chrono::floor<chrono::seconds>(point);
    auto millis = point - secondsPoint;
    lua_pushinteger(cranked->getLuaContext(), (int) duration_cast<chrono::seconds>(secondsPoint.time_since_epoch()).count());
    lua_pushinteger(cranked->getLuaContext(), (int) duration_cast<chrono::milliseconds>(millis).count());
    return 2;
}

static LuaRet playdate_timeFromEpoch_lua(Cranked *cranked, int seconds, int milliseconds) {
    // Todo: Timezone offset
    chrono::system_clock::time_point time;
    time += chrono::seconds(seconds);
    time += chrono::milliseconds(milliseconds);
    pushTime(cranked, time);
    return 1;
}

static LuaRet playdate_GMTTimeFromEpoch_lua(Cranked *cranked, int seconds, int milliseconds) {
    chrono::system_clock::time_point time;
    time += chrono::seconds(seconds);
    time += chrono::milliseconds(milliseconds);
    pushTime(cranked, time);
    return 1;
}

static int playdate_getFPS_lua(Cranked *cranked) {
    auto delta = duration_cast<chrono::milliseconds>((chrono::system_clock::now() - cranked->lastFrameTime)).count();
    return int((float) delta / 1000);
}

static LuaRet playdate_getStats_lua(Cranked *cranked) {
    // Todo
    auto stats = pushTable(cranked);
    stats.setFloatField("kernel", 0.f);
    stats.setFloatField("game", 0.f);
    stats.setFloatField("audio", 0.f);
    return 1;
}

static void playdate_setStatsInterval_lua(Cranked *cranked, int seconds) {
    cranked->statsInterval = seconds;
}

static LuaRet playdate_getPowerStatus_lua(Cranked *cranked) {
    auto status = pushTable(cranked);
    status.setBoolField("charging", false);
    status.setBoolField("USB", false);
    status.setBoolField("screws", false);
    return 1;
}

static LuaRet playdate_getSystemMenu_lua(Cranked *cranked) {
    pushTable(cranked); // Just use an empty table
    cranked->luaEngine.getQualifiedLuaGlobal("playdate.menu");
    lua_setmetatable(cranked->getLuaContext(), -2);
    return 1;
}

static void playdate_setMenuImage_lua(Cranked *cranked, LuaVal image, int xOffset) {
    auto bitmap = image.asUserdataObject<LCDBitmap_32>();
    cranked->luaEngine.preserveLuaReference(bitmap, image);
    cranked->menu.setImage(bitmap, xOffset);
}

static LuaRet playdate_menu_addMenuItem_lua(Cranked *cranked, LuaVal menu, const char *title, LuaVal callback) {
    // Items are all owned by Menu, but assertions should mostly protect against memory corruption after removing items with dangling userdata pointers
    try {
        pushMenuItem(cranked->menu.addItem(title, PDMenuItem_32::Type::Button, {}, 0, 0, callback));
    } catch (exception &ex) {
        lua_pushnil(cranked->getLuaContext());
        lua_pushstring(cranked->getLuaContext(), ex.what());
        return 2;
    }
    return 1;
}

static LuaRet playdate_menu_addCheckmarkMenuItem_lua(Cranked *cranked, LuaVal menu, const char *title, LuaVal arg2, LuaVal arg3) {
    int initialValue{};
    int callback;
    if (arg2.isBool()) {
        initialValue = arg2.asBool();
        callback = arg3;
    } else
        callback = arg2;
    try {
        pushMenuItem(cranked->menu.addItem(title, PDMenuItem_32::Type::Checkmark, {}, initialValue, 0, callback));
    } catch (exception &ex) {
        lua_pushnil(cranked->getLuaContext());
        lua_pushstring(cranked->getLuaContext(), ex.what());
        return 2;
    }
    return 1;
}

static LuaRet playdate_menu_addOptionsMenuItem_lua(Cranked *cranked, LuaVal menu, const char *title, LuaVal optionsVal, LuaVal arg3, LuaVal arg4) {
    int initialValue{};
    int callback;
    auto count = lua_rawlen(cranked->getLuaContext(), optionsVal);
    vector<string> options(count);
    for (int i = 0; i < count; i++) {
        lua_geti(cranked->getLuaContext(), optionsVal, i + 1);
        options[i] = lua_tostring(cranked->getLuaContext(), -1);
        lua_pop(cranked->getLuaContext(), 1);
    }
    if (arg3.isString()) {
        auto value = arg3.asString();
        for (int i = 0; i < options.size(); i++)
            if (options[i] == value) {
                initialValue = i;
                break;
            }
        callback = arg4;
    } else
        callback = arg3;
    try {
        pushMenuItem(cranked->menu.addItem(title, PDMenuItem_32::Type::Options, options, initialValue, 0, callback));
    } catch (exception &ex) {
        lua_pushnil(cranked->getLuaContext());
        lua_pushstring(cranked->getLuaContext(), ex.what());
        return 2;
    }
    return 1;
}

static LuaRet playdate_menu_getMenuItems_lua(Cranked *cranked, LuaVal menu) {
    auto items = pushTable(cranked);
    int i = 1;
    for (auto &item : cranked->menu.items) {
        if (!item)
            continue;
        pushMenuItem(item.get());
        lua_seti(cranked->getLuaContext(), -2, i);
        i++;
    }
    return 1;
}

static void playdate_menu_removeMenuItem_lua(Cranked *cranked, LuaVal menu, MenuItem item) {
    cranked->menu.removeItem(item);
}

static void playdate_menu_removeAllMenuItems_lua(Cranked *cranked, LuaVal menu) {
    cranked->menu.clearItems();
}

static void playdate_menu_item_setCallback_lua(Cranked *cranked, MenuItem item, LuaVal callback) {
    // Todo: This needs to call a function on the item instead
    cranked->luaEngine.getQualifiedLuaGlobal("cranked.menuCallbacks");
    lua_pushvalue(cranked->getLuaContext(), callback);
    lua_seti(cranked->getLuaContext(), -2, cranked->menu.findItem(item) + 1);
    lua_pop(cranked->getLuaContext(), 1);
}

static void playdate_menu_item_setTitle_lua(Cranked *cranked, MenuItem item, const char *title) {
    item->title = title;
}

static LuaRet playdate_menu_item_getTitle_lua(Cranked *cranked, MenuItem item) {
    lua_pushstring(cranked->getLuaContext(), item->title.c_str());
    return 1;
}

static void playdate_menu_item_setValue_lua(Cranked *cranked, MenuItem item, LuaVal value) {
    if (value.isString()) {
        string string(value.asString());
        auto found = find(item->options.begin(), item->options.end(), string.c_str());
        item->value = found == item->options.end() ? 0 : int(found - item->options.begin());
    } else if (value.isBool())
        item->value = value.asBool();
    else
        item->value = value.asInt();
}

static LuaRet playdate_menu_item_getValue_lua(Cranked *cranked, MenuItem item) {
    if (item->type == PDMenuItem_32::Type::Checkmark)
        lua_pushboolean(cranked->getLuaContext(), item->value);
    else if (item->type == PDMenuItem_32::Type::Options) {
        if (item->value >= 0 and item->value < item->options.size())
            lua_pushstring(cranked->getLuaContext(), item->options[item->value].c_str());
        else
            lua_pushnil(cranked->getLuaContext()); // Todo: Is this right for invalid values?
    } else
        lua_pushnil(cranked->getLuaContext()); // Todo: Is this right for button items?
    return 1;
}

static void playdate_menu_item_newindex_lua(Cranked *cranked, MenuItem item, LuaVal keyArg, LuaVal value) {
    if (!keyArg.isString())
        return;
    string key(keyArg.asString());
    if (key == "value")
        playdate_menu_item_setValue_lua(cranked, item, value);
    else if (key == "title")
        playdate_menu_item_setTitle_lua(cranked, item, value.asString());
}

static LuaRet playdate_menu_item_index_lua(Cranked *cranked, MenuItem item, LuaVal keyArg) {
    if (!keyArg.isString()) {
        lua_pushnil(cranked->getLuaContext());
        return 1;
    }
    string key(keyArg.asString());
    if (key == "value")
        return playdate_menu_item_getValue_lua(cranked, item);
    else if (key == "title")
        return playdate_menu_item_getTitle_lua(cranked, item);
    else {
        cranked->luaEngine.getQualifiedLuaGlobal("playdate.menu.item");
        lua_getfield(cranked->getLuaContext(), -1, key.c_str());
        return 1;
    }
}

static float playdate_display_getRefreshRate_lua(Cranked *cranked) {
    return cranked->graphics.framerate;
}

static LuaRet playdate_display_getSize_lua(Cranked *cranked) {
    lua_pushinteger(cranked->getLuaContext(), DISPLAY_WIDTH / cranked->graphics.displayScale);
    lua_pushinteger(cranked->getLuaContext(), DISPLAY_HEIGHT / cranked->graphics.displayScale);
    return 2;
}

static LuaRet playdate_display_getRect_lua(Cranked *cranked) {
    pushRect(cranked, IntRect {0, 0, DISPLAY_WIDTH / cranked->graphics.displayScale, DISPLAY_HEIGHT / cranked->graphics.displayScale}); // Todo: Includes offset?
    return 1;
}

static int playdate_display_getScale_lua(Cranked *cranked) {
    return cranked->graphics.displayScale;
}

static bool playdate_display_getInverted_lua(Cranked *cranked) {
    return cranked->graphics.displayInverted;
}

static LuaRet playdate_display_getMosaic_lua(Cranked *cranked) {
    lua_pushinteger(cranked->getLuaContext(), cranked->graphics.displayMosaic.x);
    lua_pushinteger(cranked->getLuaContext(), cranked->graphics.displayMosaic.y);
    return 2;
}

static LuaRet playdate_display_getOffset_lua(Cranked *cranked) {
    lua_pushinteger(cranked->getLuaContext(), cranked->graphics.displayOffset.x);
    lua_pushinteger(cranked->getLuaContext(), cranked->graphics.displayOffset.y);
    return 2;
}

static void playdate_display_loadImage_lua(Cranked *cranked, const char *path) {
    // Ignores image size requirement
    BitmapRef image(cranked->graphics.getImage(path));
    for (int i = 0; i < image->height; i++)
        for (int j = 0; j < image->width; j++)
            cranked->graphics.frameBuffer->drawPixel(j, i, image->getPixel(j, i));
}

static LuaRet playdate_file_load_lua(Cranked *cranked, const char *path, LuaVal env) {
    // Todo: Resolve data files?
    // Todo: Should be added to Rom pdz list for loading assets?
    luaL_loadfile(cranked->getLuaContext(), path);
    if (!env.isNil()) {
        lua_pushvalue(cranked->getLuaContext(), env);
        lua_setupvalue(cranked->getLuaContext(), -2, 1);
    }
    return 1;
}

static LuaRet playdate_file_run_lua(Cranked *cranked, const char *path, LuaVal env) {
    playdate_file_load_lua(cranked, path, env);
    lua_call(cranked->getLuaContext(), 0, LUA_MULTRET);
    return lua_gettop(cranked->getLuaContext());
}

static LuaRet playdate_graphics_imagetable_new_lua(Cranked *cranked, LuaVal arg1, LuaVal arg2, LuaVal arg3) {
    // Todo
    return 0;
}

static LuaRet playdate_graphics_imagetable_getImage_lua(Cranked *cranked, BitmapTable table, LuaVal arg1, LuaVal arg2) {
    // Todo
    return 0;
}

static void playdate_graphics_imagetable_setImage_lua(Cranked *cranked, BitmapTable table, int n, LuaVal image) {
    // Todo
}

static LuaRet playdate_graphics_imagetable_load_lua(Cranked *cranked, BitmapTable table, const char *path) {
    // Todo
    return 0;
}

static int playdate_graphics_imagetable_getLength_lua(Cranked *cranked, BitmapTable table) {
    // Todo
    return 0;
}

static LuaRet playdate_graphics_imagetable_getSize_lua(Cranked *cranked, BitmapTable table) {
    // Todo
    return 0;
}

static void playdate_graphics_imagetable_drawImage_lua(Cranked *cranked, BitmapTable table, int n, int x, int y, LuaVal flip) {
    // Todo
}

static void playdate_graphics_imagetable_gc_lua(Cranked *cranked, LuaVal table) {
    // Todo
}

static LuaRet playdate_graphics_imagetable_index_lua(Cranked *cranked, LuaVal table, LuaVal key) {
    // Todo
    return 0;
}

static void playdate_graphics_imagetable_newindex_lua(Cranked *cranked, LuaVal table, LuaVal key, LuaVal value) {
    // Todo
}

static LuaRet playdate_graphics_tilemap_new_lua(Cranked *cranked) {
    // Todo
    return 0;
}

static void playdate_graphics_tilemap_gc_lua(Cranked *cranked, LuaVal tilemap) {
    // Todo
}

static void playdate_graphics_tilemap_setImageTable_lua(Cranked *cranked, LuaVal tilemap, LuaVal table) {
    // Todo
}

static void playdate_graphics_tilemap_setTiles_lua(Cranked *cranked, LuaVal tilemap, LuaVal data, int width) {
    // Todo
}

static LuaRet playdate_graphics_tilemap_getTiles_lua(Cranked *cranked, LuaVal tilemap) {
    // Todo
    return 0;
}

static LuaRet playdate_graphics_tilemap_draw_lua(Cranked *cranked, LuaVal tilemap, int x, int y, LuaVal sourceRect) {
    // Todo
    return 0;
}

static LuaRet playdate_graphics_tilemap_drawIgnoringOffset_lua(Cranked *cranked, LuaVal tilemap, int x, int y, LuaVal sourceRect) {
    // Todo
    return 0;
}

static void playdate_graphics_tilemap_setTileAtPosition_lua(Cranked *cranked, LuaVal tilemap, int x, int y, int index) {
    // Todo
}

static LuaRet playdate_graphics_tilemap_getTileAtPosition_lua(Cranked *cranked, LuaVal tilemap, int x, int y, int index) {
    // Todo
    return 0;
}

static void playdate_graphics_tilemap_setSize_lua(Cranked *cranked, LuaVal tilemap, int width, int height) {
    // Todo
}

static LuaRet playdate_graphics_tilemap_getSize_lua(Cranked *cranked, LuaVal tilemap) {
    // Todo
    return 0;
}

static LuaRet playdate_graphics_tilemap_getPixelSize_lua(Cranked *cranked, LuaVal tilemap) {
    // Todo
    return 0;
}

static LuaRet playdate_graphics_tilemap_getTileSize_lua(Cranked *cranked, LuaVal tilemap) {
    // Todo
    return 0;
}

static LuaRet playdate_graphics_tilemap_getCollisionRects_lua(Cranked *cranked, LuaVal tilemap, LuaVal emptyIDs) {
    // Todo
    return 0;
}

static void playdate_graphics_setFont_lua(Cranked *cranked, LuaVal font, LuaVal variant) {
    // Todo
}

static LuaRet playdate_graphics_getFont_lua(Cranked *cranked, LuaVal variant) {
    // Todo
    return 0;
}

static void playdate_graphics_setFontFamily_lua(Cranked *cranked, LuaVal fontFamily) {
    // Todo
}

static void playdate_graphics_setFontTracking_lua(Cranked *cranked, int pixels) {
    // Todo
}

static int playdate_graphics_getFontTracking_lua(Cranked *cranked) {
    // Todo
    return 0;
}

static LuaRet playdate_graphics_getSystemFont_lua(Cranked *cranked, LuaVal variant) {
    // Todo
    return 0;
}

static void playdate_graphics_drawText_lua(Cranked *cranked, const char *text, int x, int y, int leadingAdjustment) {
    // Todo: Encoding, leading adjustment
    cranked->graphics.drawText(text, (int) strlen(text), PDStringEncoding::ASCII, x, y);
}

static void playdate_graphics_drawLocalizedText_lua(Cranked *cranked, const char *key, int x, int y, LuaVal language, int leadingAdjustment) {
    // Todo
}

static LuaRet playdate_graphics_getLocalizedText_lua(Cranked *cranked, const char *key, LuaVal language) {
    // Todo
    return 0;
}

static LuaRet playdate_graphics_getTextSize_lua(Cranked *cranked, const char *str, LuaVal fontFamily, int leadingAdjustment) {
    // Todo
    return 0;
}

static LuaRet playdate_graphics_font_new_lua(Cranked *cranked, const char *path) {
    // Todo
    return 0;
}

static LuaRet playdate_graphics_font_newFamily_lua(Cranked *cranked, LuaVal fontPaths) {
    // Todo
    return 0;
}

static void playdate_graphics_font_drawText_lua(Cranked *cranked, Font font, int x, int y, int loadingAdjustment) {
    // Todo
}

static int playdate_graphics_font_getHeight_lua(Cranked *cranked, Font font) {
    // Todo
    return 0;
}

static int playdate_graphics_font_getTextWidth_lua(Cranked *cranked, Font font, const char *text) {
    // Todo
    return 0;
}

static void playdate_graphics_font_setTracking_lua(Cranked *cranked, Font font, int pixels) {
    // Todo
}

static int playdate_graphics_font_getTracking_lua(Cranked *cranked, Font font) {
    // Todo
    return 0;
}

static void playdate_graphics_font_setLeading_lua(Cranked *cranked, Font font, int pixels) {
    // Todo
}

static int playdate_graphics_font_getLeading_lua(Cranked *cranked, Font font) {
    // Todo
    return 0;
}

static LuaRet playdate_graphics_font_getGlyph_lua(Cranked *cranked, Font font, LuaVal character) {
    // Todo
    return 0;
}

static LuaRet playdate_graphics_sprite_new_lua(Cranked *context) {
    pushSprite(context->graphics.createSprite());
    return 1;
}

static void playdate_graphics_sprite_gc_lua(Cranked *cranked, LuaVal sprite) {
    releaseUserdataResource(sprite);
}

static LuaRet playdate_graphics_sprite_index_lua(Cranked *cranked, LuaVal sprite) {
    // Todo
    return 0;
}

static LuaRet playdate_graphics_sprite_update_lua(Cranked *cranked, LuaVal sprite) {
    // Todo
    return 0;
}

static void playdate_graphics_sprite_setImage_lua(Cranked *cranked, Sprite sprite, LuaVal image, LuaVal flip, float scale, LuaVal yScale) {
    // Todo
}

static LuaRet playdate_graphics_sprite_getImage_lua(Cranked *cranked, LuaVal sprite) {
    // Todo
    return 0;
}

static void playdate_graphics_sprite_add_lua(Cranked *cranked, Sprite sprite) {
    // Todo
}

static void playdate_graphics_sprite_remove_lua(Cranked *cranked, Sprite sprite) {
    // Todo
}

static LuaRet playdate_graphics_sprite_getPosition_lua(Cranked *cranked, Sprite sprite) {
    // Todo
    return 0;
}

static void playdate_graphics_sprite_setCenter_lua(Cranked *cranked, Sprite sprite, float x, float y) {
    // Todo
}

static LuaRet playdate_graphics_sprite_getCenter_lua(Cranked *cranked, Sprite sprite) {
    // Todo
    return 0;
}

static LuaRet playdate_graphics_sprite_getCenterPoint_lua(Cranked *cranked, Sprite sprite) {
    // Todo
    return 0;
}
static LuaRet playdate_graphics_sprite_getSize_lua(Cranked *cranked, Sprite sprite) {
    // Todo
    return 0;
}

static void playdate_graphics_sprite_setScale_lua(Cranked *cranked, Sprite sprite, float scale, LuaVal yScale) {
    // Todo
}

static LuaRet playdate_graphics_sprite_getScale_lua(Cranked *cranked, Sprite sprite) {
    // Todo
    return 0;
}

static void playdate_graphics_sprite_setRotation_lua(Cranked *cranked, Sprite sprite, float angle, LuaVal scale, LuaVal yScale) {
    // Todo
}

static LuaRet playdate_graphics_sprite_getRotation_lua(Cranked *cranked, Sprite sprite) {
    // Todo
    return 0;
}

static LuaRet playdate_graphics_sprite_copy_lua(Cranked *cranked, Sprite sprite) {
    // Todo
    return 0;
}

static void playdate_graphics_sprite_setImageFlip_lua(Cranked *cranked, Sprite sprite, LuaVal flip, LuaVal flipCollideRect) {
    // Todo
}

static void playdate_graphics_sprite_setBounds_lua(Cranked *cranked, Sprite sprite, LuaVal arg1, LuaVal arg2, LuaVal arg3, LuaVal arg4) {
    // Todo
}

static LuaRet playdate_graphics_sprite_getBounds_lua(Cranked *cranked, Sprite sprite) {
    // Todo
    return 0;
}

static LuaRet playdate_graphics_sprite_getBoundsRect_lua(Cranked *cranked, Sprite sprite) {
    // Todo
    return 0;
}

static bool playdate_graphics_sprite_isOpaque_lua(Cranked *cranked, Sprite sprite) {
    // Todo
    return false;
}

static void playdate_graphics_sprite_setTilemap_lua(Cranked *cranked, Sprite sprite, LuaVal tilemap) {
    // Todo
}

static void playdate_graphics_sprite_setClipRect_lua(Cranked *cranked, Sprite sprite, LuaVal arg1, LuaVal arg2, LuaVal arg3, LuaVal arg4) {
    // Todo
}

static void playdate_graphics_sprite_setClipRectsInRange_lua(Cranked *cranked, LuaVal arg1, LuaVal arg2, LuaVal arg3, LuaVal arg4, LuaVal arg5, LuaVal arg6) {
    // Todo
}

static void playdate_graphics_sprite_setStencilImage_lua(Cranked *cranked, Sprite sprite, LuaVal stencil, bool tile) {
    // Todo
}

static void playdate_graphics_sprite_setStencilPattern_lua(Cranked *cranked, Sprite sprite, LuaVal arg1, LuaVal arg2) {
    // Todo
}

static bool playdate_graphics_sprite_getAlwaysRedraw_lua(Cranked *cranked) {
    // Todo
    return false;
}

void playdate_sprite_addDirtyRect_lua(Cranked *cranked, Sprite sprite, int x, int y, int width, int height) {
    // Todo
}

static void playdate_graphics_sprite_setRedrawsOnImageChange_lua(Cranked *cranked, Sprite sprite, bool flag) {
    // Todo
}

static LuaRet playdate_graphics_sprite_getAllSprites_lua(Cranked *cranked) {
    // Todo
    return 0;
}

static void playdate_graphics_sprite_removeSprites_lua(Cranked *cranked, LuaVal spriteArray) {
    // Todo
}

static void playdate_graphics_sprite_setCollideRect_lua(Cranked *cranked, Sprite sprite, LuaVal arg1, LuaVal arg2, LuaVal arg3, LuaVal arg4) {
    // Todo
}

static LuaRet playdate_graphics_sprite_getCollideRect_lua(Cranked *cranked, Sprite sprite) {
    // Todo
    return 0;
}

static LuaRet playdate_graphics_sprite_getCollideBounds_lua(Cranked *cranked, Sprite sprite) {
    // Todo
    return 0;
}

static LuaRet playdate_graphics_sprite_overlappingSprites_lua(Cranked *cranked, Sprite sprite) {
    // Todo
    return 0;
}

static LuaRet playdate_graphics_sprite_allOverlappingSprites_lua(Cranked *cranked) {
    // Todo
    return 0;
}

static bool playdate_graphics_sprite_alphaCollision_lua(Cranked *cranked, Sprite sprite, Sprite anotherSprite) {
    // Todo
    return false;
}

static void playdate_graphics_sprite_setGroups_lua(Cranked *cranked, Sprite sprite, LuaVal groups) {
    // Todo
}

static void playdate_graphics_sprite_setCollidesWithGroups_lua(Cranked *cranked, Sprite sprite, LuaVal groups) {
    // Todo
}

static void playdate_graphics_sprite_setGroupMask_lua(Cranked *cranked, Sprite sprite, uint32 mask) {
    // Todo
}

static uint32 playdate_graphics_sprite_getGroupMask_lua(Cranked *cranked, Sprite sprite) {
    // Todo
    return 0;
}

static void playdate_graphics_sprite_setCollidesWithGroupsMask_lua(Cranked *cranked, Sprite sprite, uint32 mask) {
    // Todo
}

static uint32 playdate_graphics_sprite_getCollidesWithGroupsMask_lua(Cranked *cranked, Sprite sprite) {
    // Todo
    return 0;
}

static void playdate_graphics_sprite_resetGroupMask_lua(Cranked *cranked, Sprite sprite) {
    // Todo
}

static void playdate_graphics_sprite_resetCollidesWithGroupsMask_lua(Cranked *cranked, Sprite sprite) {
    // Todo
}

static LuaRet playdate_graphics_sprite_moveWithCollisions_lua(Cranked *cranked, Sprite sprite, LuaVal arg1, LuaVal arg2) {
    // Todo
    return 0;
}

static LuaRet playdate_graphics_sprite_checkCollisions_lua(Cranked *cranked, Sprite sprite, LuaVal arg1, LuaVal arg2) {
    // Todo
    return 0;
}

static LuaRet playdate_graphics_sprite_querySpritesAtPoint_lua(Cranked *cranked, LuaVal arg1, LuaVal arg2) {
    // Todo
    return 0;
}

static LuaRet playdate_graphics_sprite_querySpritesInRect_lua(Cranked *cranked, LuaVal arg1, LuaVal arg2, LuaVal arg3, LuaVal arg4) {
    // Todo
    return 0;
}

static LuaRet playdate_graphics_sprite_querySpritesAlongLine_lua(Cranked *cranked, LuaVal arg1, LuaVal arg2, LuaVal arg3, LuaVal arg4) {
    // Todo
    return 0;
}

static LuaRet playdate_graphics_sprite_querySpriteInfoAlongLine_lua(Cranked *cranked, LuaVal arg1, LuaVal arg2, LuaVal arg3, LuaVal arg4) {
    // Todo
    return 0;
}

static LuaRet playdate_graphics_video_new_lua(Cranked *cranked, const char *path) {
    // Todo
    return 0;
}

static void playdate_graphics_video_gc_lua(Cranked *cranked, LuaVal video) {
    // Todo
}

static LuaRet playdate_graphics_video_getSize_lua(Cranked *cranked, VideoPlayer video) {
    // Todo
    return 0;
}

static int playdate_graphics_video_getFrameCount_lua(Cranked *cranked, VideoPlayer video) {
    // Todo
    return 0;
}

static int playdate_graphics_video_getFrameRate_lua(Cranked *cranked, VideoPlayer video) {
    // Todo
    return 0;
}

static void playdate_graphics_video_setContext_lua(Cranked *cranked, VideoPlayer video, LuaVal image) {
    // Todo
}

static LuaRet playdate_graphics_video_getContext_lua(Cranked *cranked, VideoPlayer video) {
    // Todo
    return 0;
}

static void playdate_graphics_video_useScreenContext_lua(Cranked *cranked, VideoPlayer video) {
    // Todo
}

static void playdate_graphics_video_renderFrame_lua(Cranked *cranked, VideoPlayer video, int number) {
    // Todo
}

static LuaRet playdate_pathfinder_graph_new_lua(Cranked *cranked, LuaVal nodeCount, LuaVal coordinates) {
    // Todo
    return 0;
}

static void playdate_pathfinder_graph_gc_lua(Cranked *cranked, LuaVal graph) {
    // Todo
}

static LuaRet playdate_pathfinder_graph_new2DGrid_lua(Cranked *cranked, LuaVal graph, int width, int height, bool allowDiagonals, LuaVal includedNodes) {
    // Todo
    return 0;
}

static LuaRet playdate_pathfinder_graph_addNewNode_lua(Cranked *cranked, LuaVal graph, int id, LuaVal x, LuaVal y, LuaVal connectedNodes, LuaVal weights, bool addReciprocalConnections) {
    // Todo
    return 0;
}

static LuaRet playdate_pathfinder_graph_addNewNodes_lua(Cranked *cranked, LuaVal graph, int count) {
    // Todo
    return 0;
}

static void playdate_pathfinder_graph_addNode_lua(Cranked *cranked, LuaVal graph, LuaVal node, LuaVal connectedNodes, LuaVal weights, bool addReciprocalConnections) {
    // Todo
}

static void playdate_pathfinder_graph_addNodes_lua(Cranked *cranked, LuaVal graph, LuaVal nodes) {
    // Todo
}

static LuaRet playdate_pathfinder_graph_allNodes_lua(Cranked *cranked, LuaVal graph) {
    // Todo
    return 0;
}

static void playdate_pathfinder_graph_removeNode_lua(Cranked *cranked, LuaVal graph, LuaVal node) {
    // Todo
}

static LuaRet playdate_pathfinder_graph_removeNodeWithXY_lua(Cranked *cranked, LuaVal graph, int x, int y) {
    // Todo
    return 0;
}

static LuaRet playdate_pathfinder_graph_removeNodeWithID_lua(Cranked *cranked, LuaVal graph, LuaVal id) {
    // Todo
    return 0;
}

static LuaRet playdate_pathfinder_graph_nodeWithID_lua(Cranked *cranked, LuaVal graph, LuaVal id) {
    // Todo
    return 0;
}

static LuaRet playdate_pathfinder_graph_nodeWithXY_lua(Cranked *cranked, LuaVal graph, int x, int y) {
    // Todo
    return 0;
}

static void playdate_pathfinder_graph_addConnections_lua(Cranked *cranked, LuaVal graph, LuaVal connections) {
    // Todo
}

static void playdate_pathfinder_graph_addConnectionToNodeWithID_lua(Cranked *cranked, LuaVal graph, LuaVal from, LuaVal to, LuaVal weights, bool addReciprocalConnections) {
    // Todo
}

static void playdate_pathfinder_graph_removeAllConnections_lua(Cranked *cranked, LuaVal graph) {
    // Todo
}

static void playdate_pathfinder_graph_removeAllConnectionsFromNodeWithID_lua(Cranked *cranked, LuaVal graph, LuaVal id, bool removeIncoming) {
    // Todo
}

static LuaRet playdate_pathfinder_graph_findPath_lua(Cranked *cranked, LuaVal graph, LuaVal startNode, LuaVal goalNode, LuaVal heuristic, bool findPathToGoalAdjacentNodes) {
    // Todo
    return 0;
}

static LuaRet playdate_pathfinder_graph_findPathWithIDs_lua(Cranked *cranked, LuaVal graph, LuaVal startID, LuaVal goalID, LuaVal heuristic, bool findPathToGoalAdjacentNodes) {
    // Todo
    return 0;
}

static void playdate_pathfinder_graph_setXYForNodeWithID_lua(Cranked *cranked, LuaVal graph, LuaVal id, int x, int y) {
    // Todo
}

static void playdate_pathfinder_node_gc_lua(Cranked *cranked, LuaVal self) {
    // Todo
}

static LuaRet playdate_pathfinder_node_index_lua(Cranked *cranked, LuaVal self, LuaVal key) {
    // Todo
    return 0;
}

static void playdate_pathfinder_node_newindex_lua(Cranked *cranked, LuaVal self, LuaVal key, LuaVal value) {
    // Todo
}

static void playdate_pathfinder_node_addConnection_lua(Cranked *cranked, LuaVal self, LuaVal node, LuaVal weight, bool addReciprocalConnection) {
    // Todo
}

static void playdate_pathfinder_node_addConnections_lua(Cranked *cranked, LuaVal self, LuaVal nodes, LuaVal weights, bool addReciprocalConnection) {
    // Todo
}

static void playdate_pathfinder_node_addConnectionToNodeWithXY_lua(Cranked *cranked, LuaVal self, int x, int y, LuaVal weight, bool addReciprocalConnection) {
    // Todo
}

static LuaRet playdate_pathfinder_node_connectedNodes_lua(Cranked *cranked, LuaVal self) {
    // Todo
    return 0;
}

static void playdate_pathfinder_node_removeConnection_lua(Cranked *cranked, LuaVal self, LuaVal node, bool removeReciprocal) {
    // Todo
}

static void playdate_pathfinder_node_removeAllConnections_lua(Cranked *cranked, LuaVal self, bool removeIncoming) {
    // Todo
}

static void playdate_pathfinder_node_setXY_lua(Cranked *cranked, LuaVal self, int x, int y) {
    // Todo
}

static int playdate_sound_getSampleRate_lua(Cranked *cranked) {
    return 44100;
}

static LuaRet playdate_sound_playingSources_lua(Cranked *cranked) {
    // Todo
    return 0;
}

static void playdate_sound_addEffect_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_removeEffect_lua(Cranked *cranked) {
    // Todo
}

static bool playdate_sound_getHeadphoneState_lua(Cranked *cranked) {
    // Todo
    return false;
}

static void playdate_sound_resetTime_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_sampleplayer_new_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_sampleplayer_gc_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_sampleplayer_copy_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_sampleplayer_play_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_sampleplayer_playAt_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_sampleplayer_setVolume_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_sampleplayer_getVolume_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_sampleplayer_setLoopCallback_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_sampleplayer_setFinishCallback_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_sampleplayer_getSample_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_sampleplayer_setRateMod_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_fileplayer_new_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_fileplayer_gc_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_fileplayer_load_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_fileplayer_play_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_fileplayer_setFinishCallback_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_fileplayer_setLoopRange_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_fileplayer_setLoopCallback_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_fileplayer_setBufferSize_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_fileplayer_setRateMod_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_fileplayer_setVolume_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_fileplayer_getVolume_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_sample_new_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_sample_gc_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_sample_getSubsample_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_sample_load_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_sample_getSampleRate_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_sample_getFormat_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_sample_play_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_sample_playAt_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_sample_save_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_channel_new_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_channel_gc_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_channel_remove_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_channel_setVolume_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_channel_getVolume_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_synth_new_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_synth_gc_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_synth_copy_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_synth_playNote_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_synth_playMIDINote_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_synth_setADSR_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_synth_setEnvelopeCurvature_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_synth_getEnvelope_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_synth_setFinishCallback_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_synth_setLegato_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_synth_setVolume_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_synth_getVolume_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_synth_setWaveform_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_signal_setOffset_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_signal_setScale_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_lfo_new_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_lfo_gc_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_lfo_setArpeggio_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_envelope_new_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_envelope_gc_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_envelope_setScale_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_envelope_setOffset_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_envelope_trigger_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_envelope_setGlobal_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_effect_bitcrusher_new_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_effect_bitcrusher_gc_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_effect_bitcrusher_setMix_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_effect_bitcrusher_setMixMod_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_effect_bitcrusher_setAmountModulator_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_effect_bitcrusher_setUndersampleModulator_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_effect_ringmodulator_new_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_effect_ringmodulator_gc_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_effect_ringmodulator_setMix_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_effect_ringmodulator_setMixMod_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_effect_ringmodulator_setFrequencyModulator_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_effect_onepolefilter_new_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_effect_onepolefilter_gc_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_effect_onepolefilter_setMix_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_effect_onepolefilter_setMixMod_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_effect_onepolefilter_setParameterModulator_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_effect_twopolefilter_new_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_effect_twopolefilter_gc_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_effect_twopolefilter_setMix_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_effect_twopolefilter_setMixMod_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_effect_twopolefilter_setFrequencyModulator_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_effect_twopolefilter_setResonanceModulator_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_effect_overdrive_new_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_effect_overdrive_gc_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_effect_overdrive_setMix_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_effect_overdrive_setMixMod_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_effect_overdrive_setLimitModulator_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_effect_overdrive_setOffsetModulator_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_delayline_new_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_delayline_gc_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_delayline_setMix_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_delayline_setMixMod_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_delayline_addTap_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_delaylinetap_gc_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_delaylinetap_setDelayMod_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_delaylinetap_setVolume_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_delaylinetap_getVolume_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_sequence_new_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_sequence_gc_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_sequence_play_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_sequence_goToStep_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_sequence_getCurrentStep_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_sequence_setLoops_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_sequence_addTrack_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_sequence_setTrackAtIndex_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_sequence_getTrackAtIndex_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_track_new_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_track_gc_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_track_addNote_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_track_setNotes_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_track_getNotes_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_track_getNotesActive_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_track_setInstrument_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_track_getInstrument_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_track_removeNote_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_track_addControlSignal_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_track_getControlSignals_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_instrument_new_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_instrument_gc_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_instrument_addVoice_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_instrument_playNote_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_instrument_playMIDINote_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_instrument_noteOff_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_instrument_setVolume_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_instrument_getVolume_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_controlsignal_new_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_controlsignal_gc_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_controlsignal_addEvent_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_controlsignal_clearEvents_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_controlsignal_setControllerType_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_controlsignal_getControllerType_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_micinput_recordToSample_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_micinput_stopRecording_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_micinput_startListening_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_micinput_stopListening_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_micinput_getLevel_lua(Cranked *cranked) {
    // Todo
}

static void playdate_sound_micinput_getSource_lua(Cranked *cranked) {
    // Todo
}

void LuaEngine::registerLuaGlobals() {
    // Global import function for loading modules
    lua_pushcfunction(getContext(), luaNativeWrapper<import_lua>);
    lua_setglobal(getContext(), "import");

    // Add custom `table` functions (Most are implemented in Lua directly)
    lua_getglobal(getContext(), "table");
    setLuaFunction(getContext(), "create", luaNativeWrapper<table_create_lua>);
    lua_pop(getContext(), 1);

    { // No idea why this isn't on playdate API table
        auto json = LuaApiHelper(getContext(), "json", true);
        json.setWrappedFunction<json_decode_lua>("decode");
        json.setWrappedFunction<json_decodeFile_lua>("decodeFile");
        json.setWrappedFunction<json_encode_lua>("encode");
        json.setWrappedFunction<json_encodePretty_lua>("encodePretty");
        json.setWrappedFunction<json_encodeToFile_lua>("encodeToFile");

        { // Both functions just return "null"
            auto jsonNull = LuaApiHelper(getContext(), "null");
            jsonNull.setWrappedFunction<json_null_tostring_lua>("__call");
            jsonNull.setWrappedFunction<json_null_tostring_lua>("__tostring");
        }
    }

    {
        auto playdate = LuaApiHelper(getContext(), "playdate", true);
        playdate.setInteger("kButtonLeft", 1 << 0);
        playdate.setInteger("kButtonRight", 1 << 1);
        playdate.setInteger("kButtonUp", 1 << 2);
        playdate.setInteger("kButtonDown", 1 << 3);
        playdate.setInteger("kButtonB", 1 << 4);
        playdate.setInteger("kButtonA", 1 << 5);
        playdate.setInteger("isSimulator", 0); // Totally legit hardware
        playdate.setWrappedFunction<playdate_apiVersion_lua>("apiVersion");
        playdate.setWrappedFunction<playdate_wait_lua>("wait");
        playdate.setWrappedFunction<playdate_stop_lua>("stop");
        playdate.setWrappedFunction<playdate_start_lua>("start");
        playdate.setWrappedFunction<playdate_getSystemLanguage_lua>("getSystemLanguage");
        playdate.setWrappedFunction<playdate_sys_getReduceFlashing>("getReduceFlashing");
        playdate.setWrappedFunction<playdate_sys_getFlipped>("getFlipped");
        playdate.setWrappedFunction<playdate_buttonIsPressed_lua>("buttonIsPressed");
        playdate.setWrappedFunction<playdate_buttonJustPressed_lua>("buttonJustPressed");
        playdate.setWrappedFunction<playdate_buttonJustReleased_lua>("buttonJustReleased");
        playdate.setWrappedFunction<playdate_getButtonState_lua>("getButtonState");
        playdate.setWrappedFunction<playdate_sys_isCrankDocked>("isCrankDocked");
        playdate.setWrappedFunction<playdate_sys_getCrankAngle>("getCrankPosition");
        playdate.setWrappedFunction<playdate_getCrankChange_lua>("getCrankChange");
        playdate.setWrappedFunction<playdate_setCrankSoundsDisabled_lua>("setCrankSoundsDisabled");
        playdate.setWrappedFunction<playdate_sys_getCurrentTimeMilliseconds>("getCurrentTimeMilliseconds");
        playdate.setWrappedFunction<playdate_sys_setAutoLockDisabled>("setAutoLockDisabled");
        playdate.setWrappedFunction<playdate_sys_getCurrentTimeMilliseconds>("getCurrentTimeMilliseconds");
        playdate.setWrappedFunction<playdate_sys_resetElapsedTime>("resetElapsedTime");
        playdate.setWrappedFunction<playdate_sys_getElapsedTime>("getElapsedTime");
        playdate.setWrappedFunction<playdate_getSecondsSinceEpoch_lua>("getSecondsSinceEpoch");
        playdate.setWrappedFunction<playdate_getTime_lua>("getTime");
        playdate.setWrappedFunction<playdate_getGMTTime_lua>("getGMTTime");
        playdate.setWrappedFunction<playdate_epochFromTime_lua>("epochFromTime");
        playdate.setWrappedFunction<playdate_epochFromGMTTime_lua>("epochFromGMTTime");
        playdate.setWrappedFunction<playdate_timeFromEpoch_lua>("timeFromEpoch");
        playdate.setWrappedFunction<playdate_GMTTimeFromEpoch_lua>("GMTTimeFromEpoch");
        playdate.setWrappedFunction<playdate_sys_shouldDisplay24HourTime>("shouldDisplay24HourTime");
        playdate.setWrappedFunction<playdate_sys_drawFPS>("drawFPS");
        playdate.setWrappedFunction<playdate_getFPS_lua>("getFPS");
        playdate.setWrappedFunction<playdate_getStats_lua>("getStats");
        playdate.setWrappedFunction<playdate_setStatsInterval_lua>("setStatsInterval");
        playdate.setWrappedFunction<playdate_getSystemMenu_lua>("getSystemMenu");
        playdate.setWrappedFunction<playdate_getPowerStatus_lua>("getPowerStatus");
        playdate.setWrappedFunction<playdate_sys_getBatteryPercentage>("getBatteryPercentage");
        playdate.setWrappedFunction<playdate_sys_getBatteryVoltage>("getBatteryVoltage");
        playdate.setWrappedFunction<playdate_setMenuImage_lua>("setMenuImage");

        // Set playdate.metadata table
        lua_newtable(getContext());
        for (auto &entry: cranked.rom->manifest) {
            lua_pushstring(getContext(), entry.second.c_str());
            lua_setfield(getContext(), -2, entry.first.c_str());
        }
        lua_setfield(getContext(), -2, "metadata");

        { // Implemented in Lua
            auto inputHandlers = LuaApiHelper(getContext(), "inputHandlers");
            inputHandlers.setString("__name", "playdate.inputHandlers");
        }

        {
            auto menuApi = LuaApiHelper(getContext(), "menu");
            menuApi.setString("__name", "playdate.menu");
            menuApi.setWrappedFunction<playdate_menu_addMenuItem_lua>("addMenuItem");
            menuApi.setWrappedFunction<playdate_menu_addCheckmarkMenuItem_lua>("addCheckmarkMenuItem");
            menuApi.setWrappedFunction<playdate_menu_addOptionsMenuItem_lua>("addOptionsMenuItem");
            menuApi.setWrappedFunction<playdate_menu_getMenuItems_lua>("getMenuItems");
            menuApi.setWrappedFunction<playdate_menu_removeMenuItem_lua>("removeMenuItem");
            menuApi.setWrappedFunction<playdate_menu_removeAllMenuItems_lua>("removeAllMenuItems");

            {
                auto menuItem = LuaApiHelper(getContext(), "item");
                menuItem.setString("__name", "playdate.menu.item");
                menuItem.setWrappedFunction<playdate_menu_item_index_lua>("__index");
                menuItem.setWrappedFunction<playdate_menu_item_newindex_lua>("__newindex");
                menuItem.setWrappedFunction<playdate_menu_item_setCallback_lua>("setCallback");
                menuItem.setWrappedFunction<playdate_menu_item_setTitle_lua>("setTitle");
                menuItem.setWrappedFunction<playdate_menu_item_getTitle_lua>("getTitle");
                menuItem.setWrappedFunction<playdate_menu_item_setValue_lua>("setValue");
                menuItem.setWrappedFunction<playdate_menu_item_getValue_lua>("getValue");
            }
        }

        {
            auto display = LuaApiHelper(getContext(), "display");
            display.setSelfIndex();
            display.setWrappedFunction<playdate_display_setRefreshRate>("setRefreshRate");
            display.setWrappedFunction<playdate_display_getRefreshRate_lua>("getRefreshRate");
            display.setWrappedFunction<playdate_graphics_display>("flush");
            display.setWrappedFunction<playdate_display_getHeight>("getHeight");
            display.setWrappedFunction<playdate_display_getWidth>("getWidth");
            display.setWrappedFunction<playdate_display_getSize_lua>("getSize");
            display.setWrappedFunction<playdate_display_getRect_lua>("getRect");
            display.setWrappedFunction<playdate_display_setScale>("setScale");
            display.setWrappedFunction<playdate_display_getScale_lua>("getScale");
            display.setWrappedFunction<playdate_display_setInverted>("setInverted");
            display.setWrappedFunction<playdate_display_getInverted_lua>("getInverted");
            display.setWrappedFunction<playdate_display_setMosaic>("setMosaic");
            display.setWrappedFunction<playdate_display_getMosaic_lua>("getMosaic");
            display.setWrappedFunction<playdate_display_setMosaic>("setOffset");
            display.setWrappedFunction<playdate_display_getOffset_lua>("getOffset");
            display.setWrappedFunction<playdate_display_setFlipped>("setFlipped");
            display.setWrappedFunction<playdate_display_loadImage_lua>("loadImage");
        }

        {
            auto datastore = LuaApiHelper(getContext(), "datastore");
            datastore.setSelfIndex();
            datastore.setString("__name", "playdate.datastore");
            datastore.setWrappedFunction<playdate_datastore_write_lua>("write");
            datastore.setWrappedFunction<playdate_datastore_read_lua>("read");
            datastore.setWrappedFunction<playdate_datastore_delete_lua>("delete");
            datastore.setWrappedFunction<playdate_datastore_writeImage_lua>("writeImage");
            datastore.setWrappedFunction<playdate_datastore_readImage_lua>("readImage");
        }

        {
            auto file = LuaApiHelper(getContext(), "file");
            file.setSelfIndex();
            file.setInteger("kSeekSet", 0);
            file.setInteger("kSeekFromCurrent", 1);
            file.setInteger("kSeekFromEnd", 2);
            file.setInteger("kFileRead", 3);
            file.setInteger("kFileWrite", 4);
            file.setInteger("kFileAppend", 8);
            file.setWrappedFunction<playdate_file_open_lua>("open");
            file.setWrappedFunction<playdate_file_listFiles_lua>("listFiles");
            file.setWrappedFunction<playdate_file_exists_lua>("exists");
            file.setWrappedFunction<playdate_file_isdir_lua>("isdir");
            file.setWrappedFunction<playdate_file_mkdir>("mkdir");
            file.setWrappedFunction<playdate_file_delete_lua>("delete");
            file.setWrappedFunction<playdate_file_getSize_lua>("getSize");
            file.setWrappedFunction<playdate_file_getType_lua>("getType");
            file.setWrappedFunction<playdate_file_modtime_lua>("modtime");
            file.setWrappedFunction<playdate_file_rename>("rename");
            file.setWrappedFunction<playdate_file_load_lua>("load");
            file.setWrappedFunction<playdate_file_run_lua>("run");

            {
                auto fileFile = LuaApiHelper(getContext(), "file");
                fileFile.setSelfIndex();
                fileFile.setString("__name", "playdate.file.file");
                fileFile.setWrappedFunction<playdate_file_close_lua>("__gc");
                fileFile.setWrappedFunction<playdate_file_close_lua>("close");
                fileFile.setWrappedFunction<playdate_file_write_lua>("write");
                fileFile.setWrappedFunction<playdate_file_flush>("flush");
                fileFile.setWrappedFunction<playdate_file_readline_lua>("readline");
                fileFile.setWrappedFunction<playdate_file_read_lua>("read");
                fileFile.setWrappedFunction<playdate_file_seek_lua>("seek");
                fileFile.setWrappedFunction<playdate_file_tell_lua>("tell");
            }
        }

        { // Primarily implemented in Lua as tables (Official implementation is presumably done in C for performance, using all userdata)
            auto geometry = LuaApiHelper(getContext(), "geometry");
            geometry.setSelfIndex();
            geometry.setInteger("kUnflipped", 0);
            geometry.setInteger("kFlippedX", 1);
            geometry.setInteger("kFlippedY", 2);
            geometry.setInteger("kFlippedXY", 3);

            {
                auto rect = LuaApiHelper(getContext(), "rect");
                rect.setString("__name", "playdate.geometry.rect");
            }

            {
                auto point = LuaApiHelper(getContext(), "point");
                point.setSelfIndex();
                point.setString("__name", "playdate.geometry.point");
            }

            {
                auto polygon = LuaApiHelper(getContext(), "polygon");
                polygon.setSelfIndex();
                polygon.setString("__name", "playdate.geometry.polygon");
            }

            {
                auto arc = LuaApiHelper(getContext(), "arc");
                arc.setSelfIndex();
                arc.setString("__name", "playdate.geometry.arc");
            }

            {
                auto size = LuaApiHelper(getContext(), "size");
                size.setSelfIndex();
                size.setString("__name", "playdate.geometry.size");
            }

            {
                auto lineSegment = LuaApiHelper(getContext(), "lineSegment");
                lineSegment.setSelfIndex();
                lineSegment.setString("__name", "playdate.geometry.lineSegment");
            }

            {
                auto vector2D = LuaApiHelper(getContext(), "vector2D");
                vector2D.setString("__name", "playdate.geometry.vector2D");
            }

            {
                auto affineTransform = LuaApiHelper(getContext(), "affineTransform");
                affineTransform.setSelfIndex();
                affineTransform.setString("__name", "playdate.geometry.affineTransform");
            }
        }

        {
            auto gfx = LuaApiHelper(getContext(), "graphics");
            gfx.setSelfIndex();
            gfx.setInteger("kColorBlack", 0);
            gfx.setInteger("kColorWhite", 1);
            gfx.setInteger("kColorClear", 2);
            gfx.setInteger("kColorXOR", 3);
            gfx.setInteger("kDrawModeCopy", 0);
            gfx.setInteger("kDrawModeWhiteTransparent", 1);
            gfx.setInteger("kDrawModeBlackTransparent", 2);
            gfx.setInteger("kDrawModeFillWhite", 3);
            gfx.setInteger("kDrawModeFillBlack", 4);
            gfx.setInteger("kDrawModeXOR", 5);
            gfx.setInteger("kDrawModeNXOR", 6);
            gfx.setInteger("kDrawModeInverted", 7);
            gfx.setInteger("kImageUnflipped", 0);
            gfx.setInteger("kImageFlippedX", 1);
            gfx.setInteger("kImageFlippedY", 2);
            gfx.setInteger("kImageFlippedXY", 3);
            gfx.setInteger("kLineCapStyleButt", 0);
            gfx.setInteger("kLineCapStyleSquare", 1);
            gfx.setInteger("kLineCapStyleRound", 2);
            gfx.setInteger("kPolygonFillNonZero", 0);
            gfx.setInteger("kPolygonFillEvenOdd", 1);
            gfx.setInteger("kStrokeCentered", 0);
            gfx.setInteger("kStrokeInside", 1);
            gfx.setInteger("kStrokeOutside", 2);
            gfx.setWrappedFunction<playdate_graphics_pushContext_lua>("pushContext");
            gfx.setWrappedFunction<playdate_graphics_popContext>("popContext");
            gfx.setWrappedFunction<playdate_graphics_clear_lua>("clear");
            gfx.setWrappedFunction<playdate_graphics_imageSizeAtPath_lua>("imageSizeAtPath");
            gfx.setWrappedFunction<playdate_graphics_checkAlphaCollision_lua>("checkAlphaCollision");
            gfx.setWrappedFunction<playdate_graphics_setColor_lua>("setColor");
            gfx.setWrappedFunction<playdate_graphics_getColor_lua>("getColor");
            gfx.setWrappedFunction<playdate_graphics_setBackgroundColor>("setBackgroundColor");
            gfx.setWrappedFunction<playdate_graphics_getBackgroundColor_lua>("getBackgroundColor");
            gfx.setWrappedFunction<playdate_graphics_setPattern_lua>("setPattern");
            gfx.setWrappedFunction<playdate_graphics_setDitherPattern_lua>("setDitherPattern");
            gfx.setWrappedFunction<playdate_graphics_drawLine_lua>("drawLine");
            gfx.setWrappedFunction<playdate_graphics_setLineCapStyle_lua>("setLineCapStyle");
            gfx.setWrappedFunction<playdate_graphics_drawPixel_lua>("drawPixel");
            gfx.setWrappedFunction<playdate_graphics_drawRect_lua>("drawRect");
            gfx.setWrappedFunction<playdate_graphics_fillRect_lua>("fillRect");
            gfx.setWrappedFunction<playdate_graphics_drawRoundRect_lua>("drawRoundRect");
            gfx.setWrappedFunction<playdate_graphics_fillRoundRect_lua>("fillRoundRect");
            gfx.setWrappedFunction<playdate_graphics_drawEllipseInRect_lua>("drawEllipseInRect");
            gfx.setWrappedFunction<playdate_graphics_fillEllipseInRect_lua>("fillEllipseInRect");
            gfx.setWrappedFunction<playdate_graphics_drawPolygon_lua>("drawPolygon");
            gfx.setWrappedFunction<playdate_graphics_fillPolygon_lua>("fillPolygon");
            gfx.setWrappedFunction<playdate_graphics_setPolygonFillRule_lua>("setPolygonFillRule");
            gfx.setWrappedFunction<playdate_graphics_drawTriangle_lua>("drawTriangle");
            gfx.setWrappedFunction<playdate_graphics_fillTriangle_lua>("fillTriangle");
            gfx.setWrappedFunction<playdate_graphics_perlin_lua>("perlin");
            gfx.setWrappedFunction<playdate_graphics_perlinArray_lua>("perlinArray");
            gfx.setWrappedFunction<playdate_graphics_setClipRect_lua>("setClipRect");
            gfx.setWrappedFunction<playdate_graphics_getClipRect_lua>("getClipRect");
            gfx.setWrappedFunction<playdate_graphics_setScreenClipRect_lua>("setScreenClipRect");
            gfx.setWrappedFunction<playdate_graphics_getScreenClipRect_lua>("getScreenClipRect");
            gfx.setWrappedFunction<playdate_graphics_clearClipRect>("clearClipRect");
            gfx.setWrappedFunction<playdate_graphics_setStencilImage_lua>("setStencilImage");
            gfx.setWrappedFunction<playdate_graphics_setStencilPattern_lua>("setStencilPattern");
            gfx.setWrappedFunction<playdate_graphics_clearStencil_lua>("clearStencil");
            gfx.setWrappedFunction<playdate_graphics_clearStencil_lua>("clearStencilImage");
            gfx.setWrappedFunction<playdate_graphics_setDrawMode>("setImageDrawMode");
            gfx.setWrappedFunction<playdate_graphics_getImageDrawMode_lua>("getImageDrawMode");
            gfx.setWrappedFunction<playdate_graphics_setLineWidth_lua>("setLineWidth");
            gfx.setWrappedFunction<playdate_graphics_getLineWidth_lua>("getLineWidth");
            gfx.setWrappedFunction<playdate_graphics_setStrokeLocation_lua>("setStrokeLocation");
            gfx.setWrappedFunction<playdate_graphics_getStrokeLocation_lua>("getStrokeLocation");
            gfx.setWrappedFunction<playdate_graphics_lockFocus_lua>("lockFocus");
            gfx.setWrappedFunction<playdate_graphics_unlockFocus_lua>("unlockFocus");
            gfx.setWrappedFunction<playdate_graphics_setDrawOffset>("setDrawOffset");
            gfx.setWrappedFunction<playdate_graphics_getDrawOffset_lua>("getDrawOffset");
            gfx.setWrappedFunction<playdate_graphics_getDisplayImage_lua>("getDisplayImage");
            gfx.setWrappedFunction<playdate_graphics_getWorkingImage_lua>("getWorkingImage");
            gfx.setWrappedFunction<playdate_graphics_setFont_lua>("setFont");
            gfx.setWrappedFunction<playdate_graphics_getFont_lua>("getFont");
            gfx.setWrappedFunction<playdate_graphics_setFontFamily_lua>("setFontFamily");
            gfx.setWrappedFunction<playdate_graphics_setFontTracking_lua>("setFontTracking");
            gfx.setWrappedFunction<playdate_graphics_getFontTracking_lua>("getFontTracking");
            gfx.setWrappedFunction<playdate_graphics_getSystemFont_lua>("getSystemFont");
            gfx.setWrappedFunction<playdate_graphics_drawText_lua>("drawText");
            gfx.setWrappedFunction<playdate_graphics_drawLocalizedText_lua>("drawLocalizedText");
            gfx.setWrappedFunction<playdate_graphics_getLocalizedText_lua>("getLocalizedText");
            gfx.setWrappedFunction<playdate_graphics_getTextSize_lua>("getTextSize");

            {
                auto image = LuaApiHelper(getContext(), "image");
                image.setSelfIndex();
                image.setString("__name", "playdate.graphics.image");
                image.setInteger("kDitherTypeNone", 0);
                image.setInteger("kDitherTypeDiagonalLine", 1);
                image.setInteger("kDitherTypeVerticalLine", 2);
                image.setInteger("kDitherTypeHorizontalLine", 3);
                image.setInteger("kDitherTypeScreen", 4);
                image.setInteger("kDitherTypeBayer2x2", 5);
                image.setInteger("kDitherTypeBayer4x4", 6);
                image.setInteger("kDitherTypeBayer8x8", 7);
                image.setInteger("kDitherTypeFloydSteinberg", 8);
                image.setInteger("kDitherTypeBurkes", 9);
                image.setInteger("kDitherTypeAtkinson", 10);
                image.setWrappedFunction<playdate_graphics_image_gc_lua>("__gc");
                image.setWrappedFunction<playdate_graphics_image_new_lua>("new");
                image.setWrappedFunction<playdate_graphics_image_load_lua>("load");
                image.setWrappedFunction<playdate_graphics_image_copy_lua>("copy");
                image.setWrappedFunction<playdate_graphics_image_getSize_lua>("getSize");
                image.setWrappedFunction<playdate_graphics_image_draw_lua>("draw");
                image.setWrappedFunction<playdate_graphics_image_drawIgnoringOffset_lua>("drawIgnoringOffset");
                image.setWrappedFunction<playdate_graphics_image_clear_lua>("clear");
                image.setWrappedFunction<playdate_graphics_image_sample_lua>("sample");
                image.setWrappedFunction<playdate_graphics_image_drawRotated_lua>("drawRotated");
                image.setWrappedFunction<playdate_graphics_image_rotatedImage_lua>("rotatedImage");
                image.setWrappedFunction<playdate_graphics_image_drawScaled_lua>("drawScaled");
                image.setWrappedFunction<playdate_graphics_image_scaledImage_lua>("scaledImage");
                image.setWrappedFunction<playdate_graphics_image_drawWithTransform_lua>("drawWithTransform");
                image.setWrappedFunction<playdate_graphics_image_transformedImage_lua>("transformedImage");
                image.setWrappedFunction<playdate_graphics_image_drawSampled_lua>("drawSampled");
                image.setWrappedFunction<playdate_graphics_image_setMaskImage_lua>("setMaskImage");
                image.setWrappedFunction<playdate_graphics_image_getMaskImage_lua>("getMaskImage");
                image.setWrappedFunction<playdate_graphics_image_addMask_lua>("addMask");
                image.setWrappedFunction<playdate_graphics_image_removeMask_lua>("removeMask");
                image.setWrappedFunction<playdate_graphics_image_hasMask_lua>("hasMask");
                image.setWrappedFunction<playdate_graphics_image_clearMask_lua>("clearMask");
                image.setWrappedFunction<playdate_graphics_image_drawTiled_lua>("drawTiled");
                image.setWrappedFunction<playdate_graphics_image_drawBlurred_lua>("drawBlurred");
                image.setWrappedFunction<playdate_graphics_image_drawFaded_lua>("drawFaded");
                image.setWrappedFunction<playdate_graphics_image_setInverted_lua>("setInverted");
                image.setWrappedFunction<playdate_graphics_image_invertedImage_lua>("invertedImage");
                image.setWrappedFunction<playdate_graphics_image_blendWithImage_lua>("blendWithImage");
                image.setWrappedFunction<playdate_graphics_image_blurredImage_lua>("blurredImage");
                image.setWrappedFunction<playdate_graphics_image_fadedImage_lua>("fadedImage");
                image.setWrappedFunction<playdate_graphics_image_vcrPauseFilterImage_lua>("vcrPauseFilterImage");
            }

            {
                auto imageTable = LuaApiHelper(getContext(), "imagetable");
                imageTable.setString("__name", "playdate.graphics.imagetable");
                imageTable.setWrappedFunction<playdate_graphics_imagetable_gc_lua>("__gc");
                imageTable.setWrappedFunction<playdate_graphics_imagetable_getLength_lua>("__len");
                imageTable.setWrappedFunction<playdate_graphics_imagetable_index_lua>("__index");
                imageTable.setWrappedFunction<playdate_graphics_imagetable_newindex_lua>("__newindex");
                imageTable.setWrappedFunction<playdate_graphics_imagetable_new_lua>("new");
                imageTable.setWrappedFunction<playdate_graphics_imagetable_getImage_lua>("getImage");
                imageTable.setWrappedFunction<playdate_graphics_imagetable_setImage_lua>("setImage");
                imageTable.setWrappedFunction<playdate_graphics_imagetable_load_lua>("load");
                imageTable.setWrappedFunction<playdate_graphics_imagetable_getLength_lua>("getLength");
                imageTable.setWrappedFunction<playdate_graphics_imagetable_getSize_lua>("getSize");
                imageTable.setWrappedFunction<playdate_graphics_imagetable_drawImage_lua>("drawImage");
            }

            {
                auto tileMap = LuaApiHelper(getContext(), "tilemap");
                tileMap.setSelfIndex();
                tileMap.setString("__name", "playdate.graphics.tilemap");
                tileMap.setWrappedFunction<playdate_graphics_tilemap_gc_lua>("__gc");
                tileMap.setWrappedFunction<playdate_graphics_tilemap_new_lua>("new");
                tileMap.setWrappedFunction<playdate_graphics_tilemap_setImageTable_lua>("setImageTable");
                tileMap.setWrappedFunction<playdate_graphics_tilemap_setTiles_lua>("setTiles");
                tileMap.setWrappedFunction<playdate_graphics_tilemap_getTiles_lua>("getTiles");
                tileMap.setWrappedFunction<playdate_graphics_tilemap_draw_lua>("draw");
                tileMap.setWrappedFunction<playdate_graphics_tilemap_drawIgnoringOffset_lua>("drawIgnoringOffset");
                tileMap.setWrappedFunction<playdate_graphics_tilemap_setTileAtPosition_lua>("setTileAtPosition");
                tileMap.setWrappedFunction<playdate_graphics_tilemap_getTileAtPosition_lua>("getTileAtPosition");
                tileMap.setWrappedFunction<playdate_graphics_tilemap_setSize_lua>("setSize");
                tileMap.setWrappedFunction<playdate_graphics_tilemap_getSize_lua>("getSize");
                tileMap.setWrappedFunction<playdate_graphics_tilemap_getPixelSize_lua>("getPixelSize");
                tileMap.setWrappedFunction<playdate_graphics_tilemap_getTileSize_lua>("getTileSize");
                tileMap.setWrappedFunction<playdate_graphics_tilemap_getCollisionRects_lua>("getCollisionRects");
            }

            {
                auto font = LuaApiHelper(getContext(), "font");
                font.setSelfIndex();
                font.setString("__name", "playdate.graphics.font");
                font.setInteger("kLanguageEnglish", 0);
                font.setInteger("kLanguageJapanese", 1);
                font.setWrappedFunction<playdate_graphics_font_new_lua>("new");
                font.setWrappedFunction<playdate_graphics_font_newFamily_lua>("newFamily");
                font.setWrappedFunction<playdate_graphics_font_drawText_lua>("drawText");
                font.setWrappedFunction<playdate_graphics_font_getHeight_lua>("getHeight");
                font.setWrappedFunction<playdate_graphics_font_getTextWidth_lua>("getTextWidth");
                font.setWrappedFunction<playdate_graphics_font_setTracking_lua>("setTracking");
                font.setWrappedFunction<playdate_graphics_font_getTracking_lua>("getTracking");
                font.setWrappedFunction<playdate_graphics_font_setLeading_lua>("setLeading");
                font.setWrappedFunction<playdate_graphics_font_getLeading_lua>("getLeading");
                font.setWrappedFunction<playdate_graphics_font_getGlyph_lua>("getGlyph");
            }

            { // Official implementation uses a table with a `_spriteud` userdata field, but using a plain userdata type in `userdata` should work fine
                auto sprite = LuaApiHelper(getContext(), "sprite");
                sprite.setString("__name", "playdate.graphics.sprite");
                sprite.setInteger("kCollisionTypeSlide", 0);
                sprite.setInteger("kCollisionTypeFreeze", 1);
                sprite.setInteger("kCollisionTypeOverlap", 2);
                sprite.setInteger("kCollisionTypeBounce", 3);
                sprite.setWrappedFunction<playdate_graphics_sprite_index_lua>("__index");
                sprite.setWrappedFunction<playdate_graphics_sprite_gc_lua>("__gc");
                sprite.setWrappedFunction<playdate_graphics_sprite_new_lua>("new");
                sprite.setWrappedFunction<playdate_graphics_sprite_update_lua>("update"); // Todo: Different from `:update()`, needs to call on sprite objects if not equal to self
                sprite.setWrappedFunction<playdate_graphics_sprite_setImage_lua>("setImage");
                sprite.setWrappedFunction<playdate_graphics_sprite_getImage_lua>("getImage");
                sprite.setWrappedFunction<playdate_graphics_sprite_add_lua>("add");
                sprite.setWrappedFunction<playdate_graphics_sprite_add_lua>("addSprite");
                sprite.setWrappedFunction<playdate_graphics_sprite_remove_lua>("remove");
                sprite.setWrappedFunction<playdate_graphics_sprite_remove_lua>("removeSprite");
                sprite.setWrappedFunction<playdate_sprite_moveTo>("moveTo");
                sprite.setWrappedFunction<playdate_graphics_sprite_getPosition_lua>("getPosition");
                sprite.setWrappedFunction<playdate_sprite_moveBy>("moveBy");
                sprite.setWrappedFunction<playdate_sprite_setZIndex>("setZIndex");
                sprite.setWrappedFunction<playdate_sprite_getZIndex>("getZIndex");
                sprite.setWrappedFunction<playdate_sprite_setVisible>("setVisible");
                sprite.setWrappedFunction<playdate_sprite_isVisible>("isVisible");
                sprite.setWrappedFunction<playdate_graphics_sprite_setCenter_lua>("setCenter");
                sprite.setWrappedFunction<playdate_graphics_sprite_getCenter_lua>("getCenter");
                sprite.setWrappedFunction<playdate_graphics_sprite_getCenterPoint_lua>("getCenterPoint");
                sprite.setWrappedFunction<playdate_sprite_setSize>("setSize");
                sprite.setWrappedFunction<playdate_graphics_sprite_getSize_lua>("getSize");
                sprite.setWrappedFunction<playdate_graphics_sprite_setScale_lua>("setScale");
                sprite.setWrappedFunction<playdate_graphics_sprite_getScale_lua>("getScale");
                sprite.setWrappedFunction<playdate_graphics_sprite_setRotation_lua>("setRotation");
                sprite.setWrappedFunction<playdate_graphics_sprite_getRotation_lua>("getRotation");
                sprite.setWrappedFunction<playdate_graphics_sprite_copy_lua>("copy");
                sprite.setWrappedFunction<playdate_sprite_setUpdatesEnabled>("setUpdatesEnabled");
                sprite.setWrappedFunction<playdate_sprite_updatesEnabled>("updatesEnabled");
                sprite.setWrappedFunction<playdate_sprite_setTag>("setTag");
                sprite.setWrappedFunction<playdate_sprite_getTag>("getTag");
                sprite.setWrappedFunction<playdate_sprite_setDrawMode>("setImageDrawMode");
                sprite.setWrappedFunction<playdate_graphics_sprite_setImageFlip_lua>("setImageFlip");
                sprite.setWrappedFunction<playdate_sprite_getImageFlip>("getImageFlip");
                sprite.setWrappedFunction<playdate_sprite_setIgnoresDrawOffset>("setIgnoresDrawOffset");
                sprite.setWrappedFunction<playdate_graphics_sprite_setBounds_lua>("setBounds");
                sprite.setWrappedFunction<playdate_graphics_sprite_getBounds_lua>("getBounds");
                sprite.setWrappedFunction<playdate_graphics_sprite_getBoundsRect_lua>("getBoundsRect");
                sprite.setWrappedFunction<playdate_sprite_setOpaque>("setOpaque");
                sprite.setWrappedFunction<playdate_graphics_sprite_isOpaque_lua>("isOpaque");
                sprite.setWrappedFunction<playdate_graphics_sprite_setTilemap_lua>("setTilemap");
                sprite.setWrappedFunction<playdate_graphics_sprite_setClipRect_lua>("setClipRect");
                sprite.setWrappedFunction<playdate_sprite_clearClipRect>("clearClipRect");
                sprite.setWrappedFunction<playdate_graphics_sprite_setClipRectsInRange_lua>("setClipRectsInRange");
                sprite.setWrappedFunction<playdate_sprite_clearClipRectsInRange>("clearClipRectsInRange");
                sprite.setWrappedFunction<playdate_graphics_sprite_setStencilImage_lua>("setStencilImage");
                sprite.setWrappedFunction<playdate_graphics_sprite_setStencilPattern_lua>("setStencilPattern");
                sprite.setWrappedFunction<playdate_sprite_clearStencil>("clearStencil");
                sprite.setWrappedFunction<playdate_sprite_setAlwaysRedraw>("setAlwaysRedraw");
                sprite.setWrappedFunction<playdate_graphics_sprite_getAlwaysRedraw_lua>("getAlwaysRedraw");
                sprite.setWrappedFunction<playdate_sprite_markDirty>("markDirty");
                sprite.setWrappedFunction<playdate_sprite_addDirtyRect_lua>("addDirtyRect");
                sprite.setWrappedFunction<playdate_graphics_sprite_setRedrawsOnImageChange_lua>("setRedrawsOnImageChange");
                sprite.setWrappedFunction<playdate_graphics_sprite_getAllSprites_lua>("getAllSprites");
                sprite.setWrappedFunction<playdate_sprite_getSpriteCount>("spriteCount");
                sprite.setWrappedFunction<playdate_sprite_removeAllSprites>("removeAll");
                sprite.setWrappedFunction<playdate_graphics_sprite_removeSprites_lua>("removeSprites");
                sprite.setWrappedFunction<playdate_graphics_sprite_setCollideRect_lua>("setCollideRect");
                sprite.setWrappedFunction<playdate_graphics_sprite_getCollideRect_lua>("getCollideRect");
                sprite.setWrappedFunction<playdate_graphics_sprite_getCollideBounds_lua>("getCollideBounds");
                sprite.setWrappedFunction<playdate_sprite_clearCollideRect>("clearCollideRect");
                sprite.setWrappedFunction<playdate_graphics_sprite_overlappingSprites_lua>("overlappingSprites");
                sprite.setWrappedFunction<playdate_graphics_sprite_allOverlappingSprites_lua>("allOverlappingSprites");
                sprite.setWrappedFunction<playdate_graphics_sprite_alphaCollision_lua>("alphaCollision");
                sprite.setWrappedFunction<playdate_sprite_setCollisionsEnabled>("setCollisionsEnabled");
                sprite.setWrappedFunction<playdate_sprite_collisionsEnabled>("collisionsEnabled");
                sprite.setWrappedFunction<playdate_graphics_sprite_setGroups_lua>("setGroups");
                sprite.setWrappedFunction<playdate_graphics_sprite_setCollidesWithGroups_lua>("setCollidesWithGroups");
                sprite.setWrappedFunction<playdate_graphics_sprite_setGroupMask_lua>("setGroupMask");
                sprite.setWrappedFunction<playdate_graphics_sprite_getGroupMask_lua>("getGroupMask");
                sprite.setWrappedFunction<playdate_graphics_sprite_setCollidesWithGroupsMask_lua>("setCollidesWithGroupsMask");
                sprite.setWrappedFunction<playdate_graphics_sprite_getCollidesWithGroupsMask_lua>("getCollidesWithGroupsMask");
                sprite.setWrappedFunction<playdate_graphics_sprite_resetGroupMask_lua>("resetGroupMask");
                sprite.setWrappedFunction<playdate_graphics_sprite_resetCollidesWithGroupsMask_lua>("resetCollidesWithGroupsMask");
                sprite.setWrappedFunction<playdate_graphics_sprite_moveWithCollisions_lua>("moveWithCollisions");
                sprite.setWrappedFunction<playdate_graphics_sprite_checkCollisions_lua>("checkCollisions");
                sprite.setWrappedFunction<playdate_graphics_sprite_querySpritesAtPoint_lua>("querySpritesAtPoint");
                sprite.setWrappedFunction<playdate_graphics_sprite_querySpritesInRect_lua>("querySpritesInRect");
                sprite.setWrappedFunction<playdate_graphics_sprite_querySpritesAlongLine_lua>("querySpritesAlongLine");
                sprite.setWrappedFunction<playdate_graphics_sprite_querySpriteInfoAlongLine_lua>("querySpriteInfoAlongLine");
            }

            {
                auto video = LuaApiHelper(getContext(), "video");
                video.setSelfIndex();
                video.setString("__name", "playdate.graphics.video");
                video.setWrappedFunction<playdate_graphics_video_gc_lua>("__gc");
                video.setWrappedFunction<playdate_graphics_video_new_lua>("new");
                video.setWrappedFunction<playdate_graphics_video_getSize_lua>("getSize");
                video.setWrappedFunction<playdate_graphics_video_getFrameCount_lua>("getFrameCount");
                video.setWrappedFunction<playdate_graphics_video_getFrameRate_lua>("getFrameRate");
                video.setWrappedFunction<playdate_graphics_video_setContext_lua>("setContext");
                video.setWrappedFunction<playdate_graphics_video_getContext_lua>("getContext");
                video.setWrappedFunction<playdate_graphics_video_useScreenContext_lua>("useScreenContext");
                video.setWrappedFunction<playdate_graphics_video_renderFrame_lua>("renderFrame");
            }
        }

        { // Todo: Just implement in Lua?
            auto pathfinder = LuaApiHelper(getContext(), "pathfinder");

            {
                auto graph = LuaApiHelper(getContext(), "graph");
                graph.setSelfIndex();
                graph.setString("__name", "playdate.pathfinder.graph");
                graph.setWrappedFunction<playdate_pathfinder_graph_gc_lua>("__gc");
                graph.setWrappedFunction<playdate_pathfinder_graph_new_lua>("new");
                graph.setWrappedFunction<playdate_pathfinder_graph_new2DGrid_lua>("new2DGrid");
                graph.setWrappedFunction<playdate_pathfinder_graph_addNewNode_lua>("addNewNode");
                graph.setWrappedFunction<playdate_pathfinder_graph_addNewNodes_lua>("addNewNodes");
                graph.setWrappedFunction<playdate_pathfinder_graph_addNode_lua>("addNode");
                graph.setWrappedFunction<playdate_pathfinder_graph_addNodes_lua>("addNodes");
                graph.setWrappedFunction<playdate_pathfinder_graph_allNodes_lua>("allNodes");
                graph.setWrappedFunction<playdate_pathfinder_graph_removeNode_lua>("removeNode");
                graph.setWrappedFunction<playdate_pathfinder_graph_removeNodeWithXY_lua>("removeNodeWithXY");
                graph.setWrappedFunction<playdate_pathfinder_graph_removeNodeWithID_lua>("removeNodeWithID");
                graph.setWrappedFunction<playdate_pathfinder_graph_nodeWithID_lua>("nodeWithID");
                graph.setWrappedFunction<playdate_pathfinder_graph_nodeWithXY_lua>("nodeWithXY");
                graph.setWrappedFunction<playdate_pathfinder_graph_addConnections_lua>("addConnections");
                graph.setWrappedFunction<playdate_pathfinder_graph_addConnectionToNodeWithID_lua>("addConnectionToNodeWithID");
                graph.setWrappedFunction<playdate_pathfinder_graph_removeAllConnections_lua>("removeAllConnections");
                graph.setWrappedFunction<playdate_pathfinder_graph_removeAllConnectionsFromNodeWithID_lua>("removeAllConnectionsFromNodeWithID");
                graph.setWrappedFunction<playdate_pathfinder_graph_findPath_lua>("findPath");
                graph.setWrappedFunction<playdate_pathfinder_graph_findPathWithIDs_lua>("findPathWithIDs");
                graph.setWrappedFunction<playdate_pathfinder_graph_setXYForNodeWithID_lua>("setXYForNodeWithID");
            }

            {
                auto node = LuaApiHelper(getContext(), "node");
                node.setSelfIndex();
                node.setString("__name", "playdate.pathfinder.node");
                node.setWrappedFunction<playdate_pathfinder_node_gc_lua>("__gc");
                node.setWrappedFunction<playdate_pathfinder_node_index_lua>("__index");
                node.setWrappedFunction<playdate_pathfinder_node_newindex_lua>("__newindex");
                node.setWrappedFunction<playdate_pathfinder_node_addConnection_lua>("addConnection");
                node.setWrappedFunction<playdate_pathfinder_node_addConnections_lua>("addConnections");
                node.setWrappedFunction<playdate_pathfinder_node_addConnectionToNodeWithXY_lua>("addConnectionToNodeWithXY");
                node.setWrappedFunction<playdate_pathfinder_node_connectedNodes_lua>("connectedNodes");
                node.setWrappedFunction<playdate_pathfinder_node_removeConnection_lua>("removeConnection");
                node.setWrappedFunction<playdate_pathfinder_node_removeAllConnections_lua>("removeAllConnections");
                node.setWrappedFunction<playdate_pathfinder_node_setXY_lua>("setXY");
            }
        }

        {
            auto sound = LuaApiHelper(getContext(), "sound");
            sound.setSelfIndex();
            sound.setInteger("kFilterLowPass", 0);
            sound.setInteger("kFilterHighPass", 1);
            sound.setInteger("kFilterBandPass", 2);
            sound.setInteger("kFilterNotch", 3);
            sound.setInteger("kFilterPEQ", 4);
            sound.setInteger("kFilterLowShelf", 5);
            sound.setInteger("kFilterHighShelf", 6);
            sound.setInteger("kFormat8bitMono", 0);
            sound.setInteger("kFormat8bitStereo", 1);
            sound.setInteger("kFormat16bitMono", 2);
            sound.setInteger("kFormat16bitStereo", 3);
            sound.setInteger("kLFOSquare", 0);
            sound.setInteger("kLFOTriangle", 1);
            sound.setInteger("kLFOSine", 2);
            sound.setInteger("kLFOSampleAndHold", 3);
            sound.setInteger("kLFOSawtoothDown", 5);
            sound.setInteger("kLFOSawtoothUp", 4);
            sound.setInteger("kWaveSquare", 0);
            sound.setInteger("kWaveTriangle", 1);
            sound.setInteger("kWaveSine", 2);
            sound.setInteger("kWaveNoise", 3);
            sound.setInteger("kWaveSawtooth", 4);
            sound.setInteger("kWavePOPhase", 5);
            sound.setInteger("kWavePODigital", 6);
            sound.setInteger("kWavePOVosim", 7);
            sound.setWrappedFunction<playdate_sound_getSampleRate_lua>("getSampleRate");
            sound.setWrappedFunction<playdate_sound_playingSources_lua>("playingSources");
            sound.setWrappedFunction<playdate_sound_addEffect_lua>("addEffect");
            sound.setWrappedFunction<playdate_sound_removeEffect_lua>("removeEffect");
            sound.setWrappedFunction<playdate_sound_getHeadphoneState_lua>("getHeadphoneState");
            sound.setWrappedFunction<playdate_sound_setOutputsActive>("setOutputsActive");
            sound.setWrappedFunction<playdate_sound_getCurrentTime>("getCurrentTime");
            sound.setWrappedFunction<playdate_sound_resetTime_lua>("resetTime");

            {
                auto samplePlayer = LuaApiHelper(getContext(), "sampleplayer");
                samplePlayer.setSelfIndex();
                samplePlayer.setString("__name", "playdate.sound.sampleplayer");
                samplePlayer.setWrappedFunction<playdate_sound_sampleplayer_gc_lua>("__gc");
                samplePlayer.setWrappedFunction<playdate_sound_sampleplayer_new_lua>("new");
                samplePlayer.setWrappedFunction<playdate_sound_sampleplayer_copy_lua>("copy");
                samplePlayer.setWrappedFunction<playdate_sound_sampleplayer_play_lua>("play");
                samplePlayer.setWrappedFunction<playdate_sound_sampleplayer_playAt_lua>("playAt");
                samplePlayer.setWrappedFunction<playdate_sound_sampleplayer_setVolume_lua>("setVolume");
                samplePlayer.setWrappedFunction<playdate_sound_sampleplayer_getVolume_lua>("getVolume");
                samplePlayer.setWrappedFunction<playdate_sound_sampleplayer_setLoopCallback_lua>("setLoopCallback");
                samplePlayer.setWrappedFunction<playdate_sound_sampleplayer_setPlayRange>("setPlayRange");
                samplePlayer.setWrappedFunction<playdate_sound_sampleplayer_setPaused>("setPaused");
                samplePlayer.setWrappedFunction<playdate_sound_sampleplayer_isPlaying>("isPlaying");
                samplePlayer.setWrappedFunction<playdate_sound_sampleplayer_stop>("stop");
                samplePlayer.setWrappedFunction<playdate_sound_sampleplayer_setFinishCallback_lua>("setFinishCallback");
                samplePlayer.setWrappedFunction<playdate_sound_sampleplayer_setSample>("setSample");
                samplePlayer.setWrappedFunction<playdate_sound_sampleplayer_getSample_lua>("getSample");
                samplePlayer.setWrappedFunction<playdate_sound_sampleplayer_getLength>("getLength");
                samplePlayer.setWrappedFunction<playdate_sound_sampleplayer_setRate>("setRate");
                samplePlayer.setWrappedFunction<playdate_sound_sampleplayer_getRate>("getRate");
                samplePlayer.setWrappedFunction<playdate_sound_sampleplayer_setRateMod_lua>("setRateMod");
                samplePlayer.setWrappedFunction<playdate_sound_sampleplayer_setOffset>("setOffset");
                samplePlayer.setWrappedFunction<playdate_sound_sampleplayer_getOffset>("getOffset");
            }

            {
                auto filePlayer = LuaApiHelper(getContext(), "fileplayer");
                filePlayer.setSelfIndex();
                filePlayer.setString("__name", "playdate.sound.fileplayer");
                filePlayer.setWrappedFunction<playdate_sound_fileplayer_gc_lua>("__gc");
                filePlayer.setWrappedFunction<playdate_sound_fileplayer_new_lua>("new");
                filePlayer.setWrappedFunction<playdate_sound_fileplayer_load_lua>("load");
                filePlayer.setWrappedFunction<playdate_sound_fileplayer_play_lua>("play");
                filePlayer.setWrappedFunction<playdate_sound_fileplayer_stop>("stop");
                filePlayer.setWrappedFunction<playdate_sound_fileplayer_pause>("pause");
                filePlayer.setWrappedFunction<playdate_sound_fileplayer_isPlaying>("isPlaying");
                filePlayer.setWrappedFunction<playdate_sound_fileplayer_getLength>("getLength");
                filePlayer.setWrappedFunction<playdate_sound_fileplayer_setFinishCallback_lua>("setFinishCallback");
                filePlayer.setWrappedFunction<playdate_sound_fileplayer_didUnderrun>("didUnderrun");
                filePlayer.setWrappedFunction<playdate_sound_fileplayer_setStopOnUnderrun>("setStopOnUnderrun");
                filePlayer.setWrappedFunction<playdate_sound_fileplayer_setLoopRange_lua>("setLoopRange");
                filePlayer.setWrappedFunction<playdate_sound_fileplayer_setLoopCallback_lua>("setLoopCallback");
                filePlayer.setWrappedFunction<playdate_sound_fileplayer_setBufferSize_lua>("setBufferSize");
                filePlayer.setWrappedFunction<playdate_sound_fileplayer_setRate>("setRate");
                filePlayer.setWrappedFunction<playdate_sound_fileplayer_getRate>("getRate");
                filePlayer.setWrappedFunction<playdate_sound_fileplayer_setRateMod_lua>("setRateMod");
                filePlayer.setWrappedFunction<playdate_sound_fileplayer_setVolume_lua>("setVolume");
                filePlayer.setWrappedFunction<playdate_sound_fileplayer_getVolume_lua>("getVolume");
                filePlayer.setWrappedFunction<playdate_sound_fileplayer_setOffset>("setOffset");
                filePlayer.setWrappedFunction<playdate_sound_fileplayer_getOffset>("getOffset");
            }

            {
                auto sample = LuaApiHelper(getContext(), "sample");
                sample.setSelfIndex();
                sample.setString("__name", "playdate.sound.sample");
                sample.setWrappedFunction<playdate_sound_sample_gc_lua>("__gc");
                sample.setWrappedFunction<playdate_sound_sample_new_lua>("new");
                sample.setWrappedFunction<playdate_sound_sample_getSubsample_lua>("getSubsample");
                sample.setWrappedFunction<playdate_sound_sample_load_lua>("load");
                sample.setWrappedFunction<playdate_sound_sample_getSampleRate_lua>("getSampleRate");
                sample.setWrappedFunction<playdate_sound_sample_getFormat_lua>("getFormat");
                sample.setWrappedFunction<playdate_sound_sample_getLength>("getLength");
                sample.setWrappedFunction<playdate_sound_sample_play_lua>("play");
                sample.setWrappedFunction<playdate_sound_sample_playAt_lua>("playAt");
                sample.setWrappedFunction<playdate_sound_sample_save_lua>("save");
            }

            {
                auto channel = LuaApiHelper(getContext(), "channel");
                channel.setSelfIndex();
                channel.setString("__name", "playdate.sound.channel");
                channel.setWrappedFunction<playdate_sound_channel_gc_lua>("__gc");
                channel.setWrappedFunction<playdate_sound_channel_new_lua>("new");
                channel.setWrappedFunction<playdate_sound_channel_remove_lua>("remove");
                channel.setWrappedFunction<playdate_sound_channel_addEffect>("addEffect");
                channel.setWrappedFunction<playdate_sound_channel_removeEffect>("removeEffect");
                channel.setWrappedFunction<playdate_sound_channel_addSource>("addSource");
                channel.setWrappedFunction<playdate_sound_channel_removeSource>("removeSource");
                channel.setWrappedFunction<playdate_sound_channel_setVolume_lua>("setVolume");
                channel.setWrappedFunction<playdate_sound_channel_getVolume_lua>("getVolume");
                channel.setWrappedFunction<playdate_sound_channel_setPan>("setPan");
                channel.setWrappedFunction<playdate_sound_channel_setPanModulator>("setPanMod");
                channel.setWrappedFunction<playdate_sound_channel_setVolumeModulator>("setVolumeMod");
            }

            {
                auto synth = LuaApiHelper(getContext(), "synth");
                synth.setSelfIndex();
                synth.setString("__name", "playdate.sound.synth");
                synth.setWrappedFunction<playdate_sound_synth_gc_lua>("__gc");
                synth.setWrappedFunction<playdate_sound_synth_new_lua>("new");
                synth.setWrappedFunction<playdate_sound_synth_copy_lua>("copy");
                synth.setWrappedFunction<playdate_sound_synth_playNote_lua>("playNote");
                synth.setWrappedFunction<playdate_sound_synth_playMIDINote_lua>("playMIDINote");
                synth.setWrappedFunction<playdate_sound_synth_noteOff>("noteOff");
                synth.setWrappedFunction<playdate_sound_synth_stop>("stop");
                synth.setWrappedFunction<playdate_sound_synth_isPlaying>("isPlaying");
                synth.setWrappedFunction<playdate_sound_synth_setAmplitudeModulator>("setAmplitudeMod");
                synth.setWrappedFunction<playdate_sound_synth_setADSR_lua>("setADSR");
                synth.setWrappedFunction<playdate_sound_synth_setAttackTime>("setAttack");
                synth.setWrappedFunction<playdate_sound_synth_setDecayTime>("setDecay");
                synth.setWrappedFunction<playdate_sound_synth_setSustainLevel>("setSustain");
                synth.setWrappedFunction<playdate_sound_synth_setReleaseTime>("setRelease");
                synth.setWrappedFunction<playdate_sound_synth_setEnvelopeCurvature_lua>("setEnvelopeCurvature");
                synth.setWrappedFunction<playdate_sound_synth_getEnvelope_lua>("getEnvelope");
                synth.setWrappedFunction<playdate_sound_synth_setFinishCallback_lua>("setFinishCallback");
                synth.setWrappedFunction<playdate_sound_synth_setFrequencyModulator>("setFrequencyMod");
                synth.setWrappedFunction<playdate_sound_synth_setLegato_lua>("setLegato");
                synth.setWrappedFunction<playdate_sound_synth_setParameter>("setParameter");
                synth.setWrappedFunction<playdate_sound_synth_setParameterModulator>("setParameterMod");
                synth.setWrappedFunction<playdate_sound_synth_setVolume_lua>("setVolume");
                synth.setWrappedFunction<playdate_sound_synth_getVolume_lua>("getVolume");
                synth.setWrappedFunction<playdate_sound_synth_setWaveform_lua>("setWaveform");
            }

            {
                auto signal = LuaApiHelper(getContext(), "signal");
                signal.setSelfIndex();
                signal.setString("__name", "playdate.sound.signal");
                signal.setWrappedFunction<playdate_sound_signal_setOffset_lua>("setOffset");
                signal.setWrappedFunction<playdate_sound_signal_setScale_lua>("setScale");
            }

            {
                auto lfo = LuaApiHelper(getContext(), "lfo");
                lfo.setSelfIndex();
                lfo.setString("__name", "playdate.sound.lfo");
                lfo.setWrappedFunction<playdate_sound_lfo_gc_lua>("__gc");
                lfo.setWrappedFunction<playdate_sound_lfo_new_lua>("new");
                lfo.setWrappedFunction<playdate_sound_lfo_setType>("setType");
                lfo.setWrappedFunction<playdate_sound_lfo_setArpeggio_lua>("setArpeggio");
                lfo.setWrappedFunction<playdate_sound_lfo_setCenter>("setCenter");
                lfo.setWrappedFunction<playdate_sound_lfo_setDepth>("setDepth");
                lfo.setWrappedFunction<playdate_sound_lfo_setRate>("setRate");
                lfo.setWrappedFunction<playdate_sound_lfo_setPhase>("setPhase");
                lfo.setWrappedFunction<playdate_sound_lfo_setGlobal>("setGlobal");
                lfo.setWrappedFunction<playdate_sound_lfo_setRetrigger>("setRetrigger");
                lfo.setWrappedFunction<playdate_sound_lfo_setDelay>("setDelay");
            }

            {
                auto envelope = LuaApiHelper(getContext(), "envelope");
                envelope.setSelfIndex();
                envelope.setString("__name", "playdate.sound.envelope");
                envelope.setWrappedFunction<playdate_sound_envelope_gc_lua>("__gc");
                envelope.setWrappedFunction<playdate_sound_envelope_new_lua>("new");
                envelope.setWrappedFunction<playdate_sound_envelope_setAttack>("setAttack");
                envelope.setWrappedFunction<playdate_sound_envelope_setDecay>("setDecay");
                envelope.setWrappedFunction<playdate_sound_envelope_setSustain>("setSustain");
                envelope.setWrappedFunction<playdate_sound_envelope_setRelease>("setRelease");
                envelope.setWrappedFunction<playdate_sound_envelope_setCurvature>("setCurvature");
                envelope.setWrappedFunction<playdate_sound_envelope_setVelocitySensitivity>("setVelocitySensitivity");
                envelope.setWrappedFunction<playdate_sound_envelope_setRateScaling>("setRateScaling");
                envelope.setWrappedFunction<playdate_sound_envelope_setScale_lua>("setScale");
                envelope.setWrappedFunction<playdate_sound_envelope_setOffset_lua>("setOffset");
                envelope.setWrappedFunction<playdate_sound_envelope_setLegato>("setLegato");
                envelope.setWrappedFunction<playdate_sound_envelope_setRetrigger>("setRetrigger");
                envelope.setWrappedFunction<playdate_sound_envelope_trigger_lua>("trigger");
                envelope.setWrappedFunction<playdate_sound_envelope_setGlobal_lua>("setGlobal");
            }

            {
                auto bitCrusher = LuaApiHelper(getContext(), "bitcrusher");
                bitCrusher.setSelfIndex();
                bitCrusher.setString("__name", "playdate.sound.bitcrusher");
                bitCrusher.setWrappedFunction<playdate_sound_effect_bitcrusher_gc_lua>("__gc");
                bitCrusher.setWrappedFunction<playdate_sound_effect_bitcrusher_new_lua>("new");
                bitCrusher.setWrappedFunction<playdate_sound_effect_bitcrusher_setMix_lua>("setMix");
                bitCrusher.setWrappedFunction<playdate_sound_effect_bitcrusher_setMixMod_lua>("setMixMod");
                bitCrusher.setWrappedFunction<playdate_sound_effect_bitcrusher_setAmount>("setAmount");
                bitCrusher.setWrappedFunction<playdate_sound_effect_bitcrusher_setAmountModulator_lua>("setAmountMod");
                bitCrusher.setWrappedFunction<playdate_sound_effect_bitcrusher_setUndersampling>("setUndersampling");
                bitCrusher.setWrappedFunction<playdate_sound_effect_bitcrusher_setUndersampleModulator_lua>("setUndersamplingMod");
            }

            {
                auto ringMod = LuaApiHelper(getContext(), "ringmod");
                ringMod.setSelfIndex();
                ringMod.setString("__name", "playdate.sound.ringmod");
                ringMod.setWrappedFunction<playdate_sound_effect_ringmodulator_gc_lua>("__gc");
                ringMod.setWrappedFunction<playdate_sound_effect_ringmodulator_new_lua>("new");
                ringMod.setWrappedFunction<playdate_sound_effect_ringmodulator_setMix_lua>("setMix");
                ringMod.setWrappedFunction<playdate_sound_effect_ringmodulator_setMixMod_lua>("setMixMod");
                ringMod.setWrappedFunction<playdate_sound_effect_ringmodulator_setFrequency>("setFrequency");
                ringMod.setWrappedFunction<playdate_sound_effect_ringmodulator_setFrequencyModulator_lua>("setFrequencyMod");
            }

            {
                auto onePoleFilter = LuaApiHelper(getContext(), "onepolefilter");
                onePoleFilter.setSelfIndex();
                onePoleFilter.setString("__name", "playdate.sound.onepolefilter");
                onePoleFilter.setWrappedFunction<playdate_sound_effect_onepolefilter_gc_lua>("__gc");
                onePoleFilter.setWrappedFunction<playdate_sound_effect_onepolefilter_new_lua>("new");
                onePoleFilter.setWrappedFunction<playdate_sound_effect_onepolefilter_setMix_lua>("setMix");
                onePoleFilter.setWrappedFunction<playdate_sound_effect_onepolefilter_setMixMod_lua>("setMixMod");
                onePoleFilter.setWrappedFunction<playdate_sound_effect_onepolefilter_setParameter>("setParameter");
                onePoleFilter.setWrappedFunction<playdate_sound_effect_onepolefilter_setParameterModulator_lua>("setParameterMod");
            }

            {
                auto twoPoleFilter = LuaApiHelper(getContext(), "twopolefilter");
                twoPoleFilter.setSelfIndex();
                twoPoleFilter.setString("__name", "playdate.sound.twopolefilter");
                twoPoleFilter.setWrappedFunction<playdate_sound_effect_twopolefilter_gc_lua>("__gc");
                twoPoleFilter.setWrappedFunction<playdate_sound_effect_twopolefilter_new_lua>("new");
                twoPoleFilter.setWrappedFunction<playdate_sound_effect_twopolefilter_setMix_lua>("setMix");
                twoPoleFilter.setWrappedFunction<playdate_sound_effect_twopolefilter_setMixMod_lua>("setMixMod");
                twoPoleFilter.setWrappedFunction<playdate_sound_effect_twopolefilter_setFrequency>("setFrequency");
                twoPoleFilter.setWrappedFunction<playdate_sound_effect_twopolefilter_setFrequencyModulator_lua>("setFrequencyMod");
                twoPoleFilter.setWrappedFunction<playdate_sound_effect_twopolefilter_setResonance>("setResonance");
                twoPoleFilter.setWrappedFunction<playdate_sound_effect_twopolefilter_setResonanceModulator_lua>("setResonanceMod");
                twoPoleFilter.setWrappedFunction<playdate_sound_effect_twopolefilter_setGain>("setGain");
                twoPoleFilter.setWrappedFunction<playdate_sound_effect_twopolefilter_setType>("setType");
            }

            {
                auto overdrive = LuaApiHelper(getContext(), "overdrive");
                overdrive.setSelfIndex();
                overdrive.setString("__name", "playdate.sound.overdrive");
                overdrive.setWrappedFunction<playdate_sound_effect_overdrive_gc_lua>("__gc");
                overdrive.setWrappedFunction<playdate_sound_effect_overdrive_new_lua>("new");
                overdrive.setWrappedFunction<playdate_sound_effect_overdrive_setMix_lua>("setMix");
                overdrive.setWrappedFunction<playdate_sound_effect_overdrive_setMixMod_lua>("setMixMod");
                overdrive.setWrappedFunction<playdate_sound_effect_overdrive_setGain>("setGain");
                overdrive.setWrappedFunction<playdate_sound_effect_overdrive_setLimit>("setLimit");
                overdrive.setWrappedFunction<playdate_sound_effect_overdrive_setLimitModulator_lua>("setLimitMod");
                overdrive.setWrappedFunction<playdate_sound_effect_overdrive_setOffset>("setOffset");
                overdrive.setWrappedFunction<playdate_sound_effect_overdrive_setOffsetModulator_lua>("setOffsetMod");
            }

            {
                auto delayLine = LuaApiHelper(getContext(), "delayline");
                delayLine.setSelfIndex();
                delayLine.setString("__name", "playdate.sound.delayline");
                delayLine.setWrappedFunction<playdate_sound_delayline_gc_lua>("__gc");
                delayLine.setWrappedFunction<playdate_sound_delayline_new_lua>("new");
                delayLine.setWrappedFunction<playdate_sound_delayline_setMix_lua>("setMix");
                delayLine.setWrappedFunction<playdate_sound_delayline_setMixMod_lua>("setMixMod");
                delayLine.setWrappedFunction<playdate_sound_delayline_addTap_lua>("addTap");
                delayLine.setWrappedFunction<playdate_sound_effect_delayline_setFeedback>("setFeedback");
            }

            {
                auto delayLineTap = LuaApiHelper(getContext(), "delaylinetap");
                delayLineTap.setSelfIndex();
                delayLineTap.setString("__name", "playdate.sound.delaylinetap");
                delayLineTap.setWrappedFunction<playdate_sound_delaylinetap_gc_lua>("__gc");
                delayLineTap.setWrappedFunction<playdate_sound_effect_delayline_setTapDelay>("setDelay");
                delayLineTap.setWrappedFunction<playdate_sound_delaylinetap_setDelayMod_lua>("setDelayMod");
                delayLineTap.setWrappedFunction<playdate_sound_delaylinetap_setVolume_lua>("setVolume");
                delayLineTap.setWrappedFunction<playdate_sound_delaylinetap_getVolume_lua>("getVolume");
                delayLineTap.setWrappedFunction<playdate_sound_effect_delayline_setTapChannelsFlipped>("setFlipChannels");
            }

            {
                auto sequence = LuaApiHelper(getContext(), "sequence");
                sequence.setSelfIndex();
                sequence.setString("__name", "playdate.sound.sequence");
                sequence.setWrappedFunction<playdate_sound_sequence_gc_lua>("__gc");
                sequence.setWrappedFunction<playdate_sound_sequence_new_lua>("new");
                sequence.setWrappedFunction<playdate_sound_sequence_play_lua>("play");
                sequence.setWrappedFunction<playdate_sound_sequence_stop>("stop");
                sequence.setWrappedFunction<playdate_sound_sequence_isPlaying>("isPlaying");
                sequence.setWrappedFunction<playdate_sound_sequence_getLength>("getLength");
                sequence.setWrappedFunction<playdate_sound_sequence_goToStep_lua>("goToStep");
                sequence.setWrappedFunction<playdate_sound_sequence_getCurrentStep_lua>("getCurrentStep");
                sequence.setWrappedFunction<playdate_sound_sequence_setTempo>("setTempo");
                sequence.setWrappedFunction<playdate_sound_sequence_setLoops_lua>("setLoops");
                sequence.setWrappedFunction<playdate_sound_sequence_getTrackCount>("getTrackCount");
                sequence.setWrappedFunction<playdate_sound_sequence_addTrack_lua>("addTrack");
                sequence.setWrappedFunction<playdate_sound_sequence_setTrackAtIndex_lua>("setTrackAtIndex");
                sequence.setWrappedFunction<playdate_sound_sequence_getTrackAtIndex_lua>("getTrackAtIndex");
                sequence.setWrappedFunction<playdate_sound_sequence_allNotesOff>("allNotesOff");
            }

            {
                auto track = LuaApiHelper(getContext(), "track");
                track.setSelfIndex();
                track.setString("__name", "playdate.sound.track");
                track.setWrappedFunction<playdate_sound_track_gc_lua>("__gc");
                track.setWrappedFunction<playdate_sound_track_new_lua>("new");
                track.setWrappedFunction<playdate_sound_track_addNote_lua>("addNote");
                track.setWrappedFunction<playdate_sound_track_setNotes_lua>("setNotes");
                track.setWrappedFunction<playdate_sound_track_getNotes_lua>("getNotes");
                track.setWrappedFunction<playdate_sound_track_removeNote_lua>("removeNote");
                track.setWrappedFunction<playdate_sound_track_clearNotes>("clearNotes");
                track.setWrappedFunction<playdate_sound_track_getLength>("getLength");
                track.setWrappedFunction<playdate_sound_track_getNotesActive_lua>("getNotesActive");
                track.setWrappedFunction<playdate_sound_track_getPolyphony>("getPolyphony");
                track.setWrappedFunction<playdate_sound_track_setInstrument_lua>("setInstrument");
                track.setWrappedFunction<playdate_sound_track_getInstrument_lua>("getInstrument");
                track.setWrappedFunction<playdate_sound_track_setMuted>("setMuted");
                track.setWrappedFunction<playdate_sound_track_addControlSignal_lua>("addControlSignal");
                track.setWrappedFunction<playdate_sound_track_getControlSignals_lua>("getControlSignals");
            }

            {
                auto instrument = LuaApiHelper(getContext(), "instrument");
                instrument.setSelfIndex();
                instrument.setString("__name", "playdate.sound.instrument");
                instrument.setWrappedFunction<playdate_sound_instrument_gc_lua>("__gc");
                instrument.setWrappedFunction<playdate_sound_instrument_new_lua>("new");
                instrument.setWrappedFunction<playdate_sound_instrument_addVoice_lua>("addVoice");
                instrument.setWrappedFunction<playdate_sound_instrument_setTranspose>("setTranspose");
                instrument.setWrappedFunction<playdate_sound_instrument_playNote_lua>("playNote");
                instrument.setWrappedFunction<playdate_sound_instrument_playMIDINote_lua>("playMIDINote");
                instrument.setWrappedFunction<playdate_sound_instrument_noteOff_lua>("noteOff");
                instrument.setWrappedFunction<playdate_sound_instrument_allNotesOff>("allNotesOff");
                instrument.setWrappedFunction<playdate_sound_instrument_setVolume_lua>("setVolume");
                instrument.setWrappedFunction<playdate_sound_instrument_getVolume_lua>("getVolume");
            }

            {
                auto controlSignal = LuaApiHelper(getContext(), "controlsignal");
                controlSignal.setSelfIndex();
                controlSignal.setString("__name", "playdate.sound.controlsignal");
                controlSignal.setWrappedFunction<playdate_sound_controlsignal_gc_lua>("__gc");
                controlSignal.setWrappedFunction<playdate_sound_controlsignal_new_lua>("new");
                controlSignal.setWrappedFunction<playdate_sound_controlsignal_addEvent_lua>("addEvent");
                controlSignal.setWrappedFunction<playdate_sound_controlsignal_clearEvents_lua>("clearEvents");
                controlSignal.setWrappedFunction<playdate_sound_controlsignal_setControllerType_lua>("setControllerType");
                controlSignal.setWrappedFunction<playdate_sound_controlsignal_getControllerType_lua>("getControllerType");
            }

            {
                auto micInput = LuaApiHelper(getContext(), "micinput");
                micInput.setSelfIndex();
                micInput.setWrappedFunction<playdate_sound_micinput_recordToSample_lua>("recordToSample");
                micInput.setWrappedFunction<playdate_sound_micinput_stopRecording_lua>("stopRecording");
                micInput.setWrappedFunction<playdate_sound_micinput_startListening_lua>("startListening");
                micInput.setWrappedFunction<playdate_sound_micinput_stopListening_lua>("stopListening");
                micInput.setWrappedFunction<playdate_sound_micinput_getLevel_lua>("getLevel");
                micInput.setWrappedFunction<playdate_sound_micinput_getSource_lua>("getSource");
            }
        }
    }

    // Load Lua sources last so playdate API is mostly populated
    if (luaL_dostring(getContext(), LUA_RUNTIME_SOURCE))
        throw CrankedError("Failed to load Runtime: {}", lua_tostring(getContext(), -1));
}
