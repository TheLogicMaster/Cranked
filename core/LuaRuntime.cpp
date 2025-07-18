#include "LuaRuntime.hpp"
#include "Perlin.hpp"
#include "gen/LuaRuntimeSource.hpp"

using namespace cranked;

// Todo: Use new functions (setResourceElement/setResourceField, returnValues, rectFromArgs, intRectFromArgs, LuaValRet without calling pushResource/pushRect/pushVec)

static LuaRet import_lua(Cranked *cranked, const char *name) {
    // Todo: Properly normalize paths
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
    if (cranked->rom->findRomFile(name)) // Ignore imports for resource files
        return 0;
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

static void playdate_restart_lua(Cranked *cranked, const char *arg) {
    // Todo
}

static int playdate_getSystemLanguage_lua(Cranked *cranked) {
    return int(cranked->systemLanguage);
}

static LuaRet playdate_getCrankChange_lua(Cranked *cranked) {
    float change = cranked->getCrankChange();
    lua_pushnumber(cranked->getLuaContext(), change);
    lua_pushnumber(cranked->getLuaContext(), change); // Todo: Apply `acceleration` scaling (Maybe scale up a bit after a certain threshold)
    return 2;
}

static int playdate_getCrankTicks(Cranked *cranked, int ticksPerRevolution) {
    float change = cranked->getCrankChange();
    return (int)(change / (float)ticksPerRevolution);
}

static void playdate_setCrankSoundsDisabled_lua(Cranked *cranked, bool disabled) {
    cranked->crankSoundsEnabled = not disabled;
}

static LuaRet playdate_readAccelerometer_lua(Cranked *cranked) {
    pushFloat(cranked, cranked->accelerometerX);
    pushFloat(cranked, cranked->accelerometerY);
    pushFloat(cranked, cranked->accelerometerZ);
    return 3;
}

static bool playdate_buttonIsPressed_lua(Cranked *cranked, LuaVal button) {
    return cranked->currentInputs & (int)button.asButton();
}

static bool playdate_buttonJustPressed_lua(Cranked *cranked, LuaVal button) {
    return cranked->pressedInputs & (int)button.asButton();
}

static bool playdate_buttonJustReleased_lua(Cranked *cranked, LuaVal button) {
    return cranked->releasedInputs & (int)button.asButton();
}

static LuaRet playdate_getButtonState_lua(Cranked *cranked) {
    lua_pushinteger(cranked->getLuaContext(), cranked->currentInputs);
    lua_pushinteger(cranked->getLuaContext(), cranked->pressedInputs);
    lua_pushinteger(cranked->getLuaContext(), cranked->releasedInputs);
    return 3;
}

static void playdate_setButtonQueueSize(Cranked *cranked, int size) {
    // Todo
}

static LuaRet playdate_file_modtime_lua(lua_State *context, const char *path) {
    if (!path)
        throw CrankedError("Nil path");
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
    if (!path)
        throw CrankedError("Nil path");
    FileStat_32 stat{};
    if (context->files.stat(path, stat))
        return -1;
    return (int) stat.size;
}

static LuaRet playdate_file_getType_lua(lua_State *context, const char *path) {
    if (!path)
        throw CrankedError("Nil path");
    auto type = Cranked::fromLuaContext(context)->files.getType(path);
    if (type)
        lua_pushstring(context, type);
    else
        lua_pushnil(context);
    return 1;
}

static bool playdate_file_isdir_lua(Cranked *context, const char *path) {
    if (!path)
        throw CrankedError("Nil path");
    FileStat_32 stat{};
    context->files.stat(path, stat);
    return stat.isdir;
}

static LuaRet playdate_file_listFiles_lua(lua_State *context, const char *path, bool showHidden) {
    if (!path)
        throw CrankedError("Nil path");
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
    if (!path)
        throw CrankedError("Nil path");
    return !context->files.unlink(path, recursive);
}

static bool playdate_file_exists_lua(Cranked *context, const char *path) {
    if (!path)
        throw CrankedError("Nil path");
    return context->files.exists(path);
}

static LuaRet playdate_file_open_lua(lua_State *context, const char *path, int mode) {
    if (!path)
        throw CrankedError("Nil path");
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
            break;
    }
    auto cranked = Cranked::fromLuaContext(context);
    auto file = cranked->files.open(path, openMode);
    if (!file) {
        lua_pushnil(context);
        lua_pushstring(context, cranked->fromVirtualAddress<const char>(cranked->files.lastError));
        return 2;
    }
    pushResource(cranked, file);
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
    auto result = context->files.read(file, buffer.data(), bytes);
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

static void playdate_file_seek_lua(Cranked *context, File file, int offset, SeekWhence whence) {
    context->files.seek(file, offset, whence);
}

static LuaRet json_decode_lua(Cranked *cranked, const char *str) {
    if (!str) {
        pushNil(cranked);
        return 1;
    }
    auto context = cranked->getLuaContext();
    function<void(const nlohmann::json&)> createTable;
    createTable = [&](const nlohmann::json &json){
        int arrayIndex = 1;
        auto table = pushTable(cranked);
        for (auto &entry : json.items()) {
            auto &value = entry.value();
            if (value.is_null())
                pushNil(cranked);
            else if (value.is_boolean())
                pushBool(cranked, value.get<bool>());
            else if (value.is_number_integer())
                pushInt(cranked, value.get<int>());
            else if (value.is_number_float())
                pushFloat(cranked, value.get<float>());
            else if (value.is_string())
                pushString(cranked, value.get<string>().c_str());
            else
                createTable(value);
            if (json.is_array())
                lua_seti(context, table, arrayIndex++);
            else
                lua_setfield(context, table, entry.key().c_str());
        }
    };
    createTable(nlohmann::json::parse(str));
    return 1;
}

static LuaRet json_decodeFile_lua(lua_State *context, File file) {
    auto cranked = Cranked::fromLuaContext(context);
    int size = 0;
    constexpr int BUFFER_SEGMENT = 512;
    vector<char> buffer(BUFFER_SEGMENT);
    while (true) {
        auto returned = cranked->files.read(file, buffer.data() + size, (int) buffer.size() - size);
        if (returned < 0)
            return luaL_error(context, cranked->fromVirtualAddress<char>(cranked->files.lastError));
        if (returned < BUFFER_SEGMENT)
            break;
        size += returned;
        buffer.resize(buffer.size() + BUFFER_SEGMENT);
    }
    return json_decode_lua(cranked, buffer.data());
}

static string jsonEncodeTable(lua_State *context, LuaVal table, bool pretty) {
    // Todo: This should be improved to properly support non-array tables
    function<void(nlohmann::json *)> encodeTable;
    encodeTable = [&](nlohmann::json *jsonValue){
        if (!lua_istable(context, -1))
            throw CrankedError("Invalid table to encode");
        lua_pushnil(context);
        while (lua_next(context, -2)) {
            lua_pushvalue(context, -2);
            nlohmann::json *newValue;
            if (lua_isinteger(context, -1))
                newValue = &(*jsonValue)[max(lua_tointeger(context, -1) - 1, 0)]; // Todo: This only works for array styles tables
            else
                newValue = &(*jsonValue)[lua_tostring(context, -1)];
            LuaVal value{ context, -2 };
            if (value.isInt())
                *newValue = value.asInt();
            else if (value.isBool())
                *newValue = value.asBool();
            else if (value.isNumeric()) {
                auto val = value.asFloat();
                *newValue = rintf(val) == val ? (nlohmann::json)value.asInt() : (nlohmann::json)val;
            } else if (value.isString())
                *newValue = value.asString();
            else if (value.isNil())
                *newValue = nullptr;
            else if (value.isTable()) {
                lua_pushvalue(context, value);
                encodeTable(newValue);
            } else
                throw CrankedError("Invalid value to encode");
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

static void json_encodeToFile_lua(lua_State *context, LuaVal file, LuaVal pretty, LuaVal table) {
    bool isPretty = false;
    if (pretty.isTable())
        table = pretty;
    else
        isPretty = pretty.asBool();
    bool isFile = !file.isString();
    auto cranked = Cranked::fromLuaContext(context);
    File target = isFile ? file.asUserdataObject<File>() : cranked->files.open(file.asString(), FileOptions::Write);
    playdate_file_write_lua(Cranked::fromLuaContext(context), target, jsonEncodeTable(context, table, isPretty).c_str());
    if (!isFile)
        cranked->files.close(target); // Todo: Surely should use RAII here
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
    auto file = cranked->files.open(name, FileOptions::Write);
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
    auto file = context->files.open(name, FileOptions::ReadData);
    json_decodeFile_lua(context->getLuaContext(), file);
    context->files.close(file);
    return 1;
}

static bool playdate_datastore_delete_lua(Cranked *context, LuaVal filename) {
    string name(filename.isString() ? filename.asString() : "data");
    name += ".json";
    return context->files.unlink(name, false) == 0;
}

static void playdate_datastore_writeImage_lua(Cranked *cranked, Bitmap image, const char *path) {
    auto pathStr = string(path);
    if (pathStr.find('/') == string::npos)
        pathStr = "image/" + pathStr;
    auto dataPath = cranked->files.appDataPath / pathStr;
    if (pathStr.ends_with(".gif"))
        writeGIF(pathStr.c_str(), image->toRGB().data(), image->width, image->height);
    else {
        auto data = Rom::writeImage(image);
        ofstream file(dataPath);
        file.write((char *)data.data(), (int)data.size());
    }
}

static LuaValRet playdate_datastore_readImage_lua(Cranked *cranked, const char *path) {
    string stringPath(path);
    if (stringPath.find('/') == string::npos)
        stringPath = "images/" + stringPath;
    try {
        return pushResource(cranked, cranked->graphics.getImage(stringPath));
    } catch (exception &) {
        return pushNil(cranked);
    }
}

static LuaValRet playdate_graphics_image_new_lua(Cranked *cranked, LuaVal width, int height, LuaVal bgcolor) {
    Bitmap image;
    if (width.isString()) {
        auto path = width.asString();
        try {
            image = cranked->graphics.getImage(path);
        } catch (exception &) {
            return pushNil(cranked);
        }
    } else {
        image = cranked->graphics.createBitmap(width.asInt(), height);
        image->clear(bgcolor.isNil() ? LCDSolidColor::Clear : bgcolor.asEnum<LCDSolidColor>());
    }
    return pushResource(cranked, image);
}

static void playdate_graphics_image_gc_lua(Cranked *cranked, Bitmap image) {
    image->dereference();
}

static LuaRet playdate_graphics_image_index_lua(Cranked *cranked, Bitmap image, LuaVal key) {
    if (cranked->luaEngine.indexMetatable())
        return 1;
    if (key.isString()) {
        string str(key.asString());
        if (str == "width")
            pushInt(cranked, image->width);
        else if (str == "height")
            pushInt(cranked, image->height);
        else
            pushNil(cranked);
    } else
        pushNil(cranked);
    return 1;
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
    pushResource(cranked, cranked->heap.construct<LCDBitmap_32>(*image));
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

static tuple<float, float> getArgScaleXY(LuaVal scale, LuaVal yScale) {
    float scaleX = scale.isNumeric() ? scale.asFloat() : 1.0f;
    float scaleY = yScale.isNumeric() ? yScale.asFloat() : scaleX;
    return { scaleX, scaleY };
}

static void playdate_graphics_image_drawRotated_lua(Cranked *cranked, Bitmap image, int x, int y, float angle, LuaVal scale, LuaVal yScale) {
    auto [ scaleX, scaleY ] = getArgScaleXY(scale, yScale);
    cranked->graphics.drawRotatedBitmap(image, x, y, angle, 0.5f, 0.5f, scaleX, scaleY);
}

static LuaValRet playdate_graphics_image_rotatedImage_lua(Cranked *cranked, Bitmap image, float angle, LuaVal scale, LuaVal yScale) {
    auto [ scaleX, scaleY ] = getArgScaleXY(scale, yScale);
    return pushResource(cranked, cranked->graphics.rotateBitmap(image, angle, 0.5f, 0.5f, scaleX, scaleY));
}

static void playdate_graphics_image_drawScaled_lua(Cranked *cranked, Bitmap image, int x, int y, float scale, LuaVal yScale) {
    float scaleY = yScale.isNumeric() ? yScale.asFloat() : scale;
    cranked->graphics.drawScaledBitmap(image, x, y, scale, scaleY);
}

static LuaValRet playdate_graphics_image_scaledImage_lua(Cranked *cranked, Bitmap image, float scale, LuaVal yScale) {
    float scaleY = yScale.isNumeric() ? yScale.asFloat() : scale;
    return pushResource(cranked, cranked->graphics.scaleBitmap(image, scale, scaleY));
}

static void playdate_graphics_image_drawWithTransform_lua(Cranked *cranked, Bitmap image, Transform xform, int x, int y) {
    cranked->graphics.drawTransformedBitmap(image, xform, x, y);
}

static LuaValRet playdate_graphics_image_transformedImage_lua(Cranked *cranked, Bitmap image, Transform xform) {
    return pushResource(cranked, cranked->graphics.transformBitmap(image, xform));
}

static void playdate_graphics_image_drawSampled_lua(Cranked *cranked, Bitmap image, int x, int y, int width, int height, float centerX, float centerY, float dxx, float dyx, float dxy, float dyy, float dx, float dy, float z, float tiltAngle, bool tile) {
    cranked->graphics.drawSampledBitmap(image, x, y, width, height, centerX, centerY, dxx, dyx, dxy, dyy, dx, dy, z, tiltAngle, tile);
}

static void playdate_graphics_image_setMaskImage_lua(Cranked *cranked, Bitmap image, Bitmap maskImage) {
    image->mask = cranked->heap.construct<LCDBitmap_32>(*maskImage);
}

static LuaValRet playdate_graphics_image_getMaskImage_lua(Cranked *cranked, Bitmap image) {
    return pushResource(cranked, image->mask);
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
    IntRect rect;
    GraphicsFlip flip;
    if (arg1.isNumeric()) {
        rect = { arg1.asInt(), (arg1 + 1).asInt(), (arg1 + 2).asInt(), (arg1 + 3).asInt() };
        flip = (arg1 + 4).asFlip();
    } else {
        rect = arg1.asIntRect();
        flip = (arg1 + 1).asFlip();
    }
    cranked->graphics.drawBitmapTiled(image, rect.pos.x, rect.pos.y, rect.size.x, rect.size.y, flip);
}

static void playdate_graphics_image_drawBlurred_lua(Cranked *cranked, Bitmap image, int x, int y, int radius, int numPasses, DitherType ditherType, LuaVal arg1, LuaVal arg2, LuaVal arg3) {
    GraphicsFlip flip{};
    int phaseX{}, phaseY{};
    if (arg2.isNil())
        flip = arg1.asFlip();
    else if (arg3.isNil()) {
        phaseX = arg1.asInt();
        phaseY = arg2.asInt();
    }
    else {
        flip = arg1.asFlip();
        phaseX = arg2.asInt();
        phaseY = arg3.asInt();
    }
    cranked->graphics.drawBlurredBitmap(image, x, y, radius, numPasses, ditherType, flip, phaseX, phaseY);
}

static void playdate_graphics_image_drawFaded_lua(Cranked *cranked, Bitmap image, int x, int y, float alpha, DitherType ditherType) {
    cranked->graphics.drawFadedBitmap(image, x, y, alpha, ditherType);
}

static void playdate_graphics_image_setInverted_lua(Cranked *cranked, Bitmap image, bool flag) {
    image->inverted = flag;
}

static LuaValRet playdate_graphics_image_invertedImage_lua(Cranked *cranked, Bitmap image) {
    // Todo: Does this ignore the inverted flag?
    return pushResource(cranked, cranked->graphics.invertedBitmap(image));
}

static LuaValRet playdate_graphics_image_blendWithImage_lua(Cranked *cranked, Bitmap self, Bitmap image, float alpha, DitherType ditherType) {
    return pushResource(cranked, cranked->graphics.blendedBitmap(self, image, alpha, ditherType));
}

static LuaValRet playdate_graphics_image_blurredImage_lua(Cranked *cranked, Bitmap image, int radius, int numPasses, DitherType ditherType, bool padEdges, int xPhase, int yPhase) {
    return pushResource(cranked, cranked->graphics.blurredBitmap(image, radius, numPasses, ditherType, padEdges, xPhase, yPhase));
}

static LuaValRet playdate_graphics_image_fadedImage_lua(Cranked *cranked, Bitmap image, float alpha, DitherType ditherType) {
    return pushResource(cranked, cranked->graphics.fadedBitmap(image, alpha, ditherType));
}

static LuaValRet playdate_graphics_image_vcrPauseFilterImage_lua(Cranked *cranked, Bitmap image) {
    return pushResource(cranked, cranked->graphics.vcrPauseFilteredBitmap(image));
}

static void playdate_graphics_pushContext_lua(Cranked *cranked, LuaVal imageVal) {
    auto image = imageVal.isUserdataObject() ? imageVal.asUserdataObject<Bitmap>() : nullptr;
    cranked->graphics.pushContext(image);
}

static void playdate_graphics_clear_lua(Cranked *cranked, LuaVal color) {
    cranked->graphics.frameBuffer->clear(color.isNil() ? cranked->graphics.getContext().backgroundColor : (LCDSolidColor) color.asInt());
}

static bool playdate_graphics_checkAlphaCollision_lua(Cranked *cranked, Bitmap image1, int x1, int y1, GraphicsFlip flip1, Bitmap image2, int x2, int y2, GraphicsFlip flip2) {
    return Graphics::checkBitmapMaskCollision(image1, x1, y1, flip1, image2, x2, y2, flip2, { { x1, y1 }, { image1->width, image1->height} });
}

static void playdate_graphics_setColor_lua(Cranked *cranked, LCDSolidColor color) {
    auto &ctx = cranked->graphics.getContext();
    ctx.color = color;
    ctx.drawingColor = color;
}

static LCDSolidColor playdate_graphics_getColor_lua(Cranked *cranked) {
    return cranked->graphics.getContext().drawingColor;
}

static LCDSolidColor playdate_graphics_getBackgroundColor_lua(Cranked *cranked) {
    return cranked->graphics.getContext().backgroundColor;
}

static void playdate_graphics_setPattern_lua(Cranked *cranked, LuaVal pattern, int x, int y) {
    auto &ctx = cranked->graphics.getContext();
    if (pattern.isUserdataObject()) {
        // Todo
    } else {
        auto &color = ctx.color.emplace<PatternColor>();
        for (int i = 0; i < 8; i++)
            color.pattern[i] = pattern.getIntElement(i + 1);
        if (auto firstMask = pattern.getElement(9); !lua_isnil(cranked->getLuaContext(), firstMask.val)) {
            for (int i = 8; i < 16; i++)
                color.pattern[i] = pattern.getIntElement(i + 1);
        } else
            memset(color.pattern.data() + 8, 0xFF, 8);
    }
}

static void playdate_graphics_setDitherPattern_lua(Cranked *cranked, float alpha, DitherType ditherType) {
    auto &ctx = cranked->graphics.getContext();
    ctx.color = DitherColor{ ditherType, alpha };
}

static void playdate_graphics_drawLine_lua(Cranked *cranked, LuaVal x1, int y1, int x2, int y2, int lineWidth) {
    auto &context = cranked->graphics.getContext();
    if (lineWidth == 0) // Todo: Is undocumented line width parameter also used for line segment override?
        lineWidth = context.lineWidth;
    if (x1.isTable())
        cranked->graphics.drawLine(x1.getIntField("x1"), x1.getIntField("y1"), x1.getIntField("x2"), x1.getIntField("y2"), lineWidth, context.color);
    else
        cranked->graphics.drawLine(x1.asInt(), y1, x2, y2, lineWidth, context.color);
}

static void playdate_graphics_setLineCapStyle_lua(Cranked *cranked, LCDLineCapStyle style) {
    cranked->graphics.getContext().lineEndCapStyle = style;
}

static void playdate_graphics_drawPixel_lua(Cranked *cranked, LuaVal x, int y) {
    if (x.isTable())
        cranked->graphics.drawPixel(x.getIntField("x"), x.getIntField("y"), cranked->graphics.getContext().color);
    else
        cranked->graphics.drawPixel(x.asInt(), y, cranked->graphics.getContext().color);
}

static void playdate_graphics_drawRect_lua(Cranked *cranked, LuaVal x, int y, int w, int h) {
    auto &context = cranked->graphics.getContext();
    if (x.isTable())
        cranked->graphics.drawRect(x.getIntField("x"), x.getIntField("y"), x.getIntField("width"), x.getIntField("height"), context.color);
    else
        cranked->graphics.drawRect(x.asInt(), y, w, h, context.color);
}

static void playdate_graphics_fillRect_lua(Cranked *cranked, LuaVal x, int y, int w, int h) {
    auto &context = cranked->graphics.getContext();
    if (x.isTable())
        cranked->graphics.fillRect(x.getIntField("x"), x.getIntField("y"), x.getIntField("width"), x.getIntField("height"), context.color);
    else
        cranked->graphics.fillRect(x.asInt(), y, w, h, context.color);
}

static void playdate_graphics_drawRoundRect_lua(Cranked *cranked, LuaVal x, int y, int w, int h, int radius) {
    auto &context = cranked->graphics.getContext();
    if (x.isTable())
        cranked->graphics.drawRoundRect(x.getIntField("x"), x.getIntField("y"), x.getIntField("width"), x.getIntField("height"), y, context.color);
    else
        cranked->graphics.drawRoundRect(x.asInt(), y, w, h, radius, context.color);
}

static void playdate_graphics_fillRoundRect_lua(Cranked *cranked, LuaVal x, int y, int w, int h, int radius) {
    auto &context = cranked->graphics.getContext();
    if (x.isTable())
        cranked->graphics.fillRoundRect(x.getIntField("x"), x.getIntField("y"), x.getIntField("width"), x.getIntField("height"), y, context.color);
    else
        cranked->graphics.fillRoundRect(x.asInt(), y, w, h, radius, context.color);
}

static void drawEllipse(Cranked *cranked, LuaVal arg1, bool filled) {
    auto &context = cranked->graphics.getContext();
    IntRect rect{};
    LuaVal startAngleArg = arg1 + 4;
    if (arg1.isTable()) {
        rect = arg1.asIntRect();
        startAngleArg = arg1 + 1;
    } else
        rect = {arg1.asInt(), (arg1 + 1).asInt(), (arg1 + 2).asInt() - 1, (arg1 + 3).asInt() - 1};
    float startAngle = startAngleArg.asFloat();
    float endAngle = (startAngleArg + 1).isNil() ? 360 : (startAngleArg + 1).asFloat();
    cranked->graphics.drawEllipse(rect.pos.x, rect.pos.y, rect.size.x, rect.size.y, context.lineWidth, startAngle, endAngle, context.color, filled);
}

static void playdate_graphics_drawEllipseInRect_lua(Cranked *cranked, LuaVal arg1) {
    drawEllipse(cranked, arg1, false);
}

static void playdate_graphics_fillEllipseInRect_lua(Cranked *cranked, LuaVal arg1) {
    drawEllipse(cranked, arg1, true);
}

static vector<int32> getPolygonPoints(Cranked *cranked, LuaVal arg) {
    auto context = cranked->getLuaContext();
    if (!context)
        return {};
    vector<int32> points;
    if (arg.isTable()) {
        int count = arg.getIntField("numberOfVertices");
        for (int i = 0; i < count; i++) {
            auto point = arg.getElement(i + 1);
            points.emplace_back(point.val.getIntField("x"));
            points.emplace_back(point.val.getIntField("y"));
        }
    } else {
        int top = lua_gettop(context);
        for (int i = arg; i <= top; i++)
            points.emplace_back(lua_tointeger(context, i));
    }
    return points;
}

static void playdate_graphics_drawPolygon_lua(Cranked *cranked, LuaVal arg1) {
    auto points = getPolygonPoints(cranked, arg1);
    cranked->graphics.drawPolygon(points.data(), (int)points.size() / 2, cranked->graphics.getContext().color);
}

static void playdate_graphics_fillPolygon_lua(Cranked *cranked, LuaVal arg1) {
    auto points = getPolygonPoints(cranked, arg1);
    auto &ctx = cranked->graphics.getContext();
    cranked->graphics.fillPolygon(points.data(), (int)points.size() / 2, ctx.color, ctx.polygonFillRule);
}

static void playdate_graphics_setPolygonFillRule_lua(Cranked *cranked, LCDPolygonFillRule rule) {
    cranked->graphics.getContext().polygonFillRule = rule;
}

static void playdate_graphics_drawTriangle_lua(Cranked *cranked, int x1, int y1, int x2, int y2, int x3, int y3) {
    cranked->graphics.drawTriangle(x1, y1, x2, y2, x3, y3, cranked->graphics.getContext().color);
}

static void playdate_graphics_fillTriangle_lua(Cranked *cranked, int x1, int y1, int x2, int y2, int x3, int y3) {
    cranked->graphics.fillTriangle(x1, y1, x2, y2, x3, y3, cranked->graphics.getContext().color);
}

static float playdate_graphics_perlin_lua(Cranked *cranked, float x, float y, float z, int repeat, int octaves, float persistence) {
    return Perlin::noise(repeat, x, y, z, octaves, persistence);
}

static LuaValRet playdate_graphics_perlinArray_lua(Cranked *cranked, int count, float x, float dx, float y, float dy, float z, float dz, int repeat, int octaves, float persistence) {
    auto table = pushTable(cranked);
    for (int i = 0; i < count; i++) {
        table.setFloatElement(i + 1, Perlin::noise(repeat, x, y, z, octaves, persistence));
        x += dx;
        y += dy;
        z += dz;
    }
    return table;
}

static void playdate_graphics_setClipRect_lua(Cranked *cranked, LuaVal arg1, int y, int width, int height) {
    if (arg1.isTable())
        cranked->graphics.getContext().clipRect = arg1.asIntRect();
    else
        cranked->graphics.getContext().clipRect = {arg1.asInt(), y, width, height};
}

static LuaRet playdate_graphics_getClipRect_lua(Cranked *cranked) {
    return returnRectValues(cranked, cranked->graphics.getContext().clipRect);
}

static void playdate_graphics_setScreenClipRect_lua(Cranked *cranked, LuaVal arg1, int y, int width, int height) {
    auto &displayContext = cranked->graphics.getContext();
    auto rect = arg1.isTable() ? arg1.asIntRect() : IntRect{arg1.asInt(), y, width, height};
    displayContext.clipRect = rect + displayContext.drawOffset; // Todo: Verify offset direction
}

static LuaRet playdate_graphics_getScreenClipRect_lua(Cranked *cranked) {
    auto &displayContext = cranked->graphics.getContext();
    pushRect(cranked, displayContext.clipRect - displayContext.drawOffset); // Todo: Verify offset direction
    return 1;
}

static void playdate_graphics_setStencilImage_lua(Cranked *cranked, Bitmap image, bool tile) {
    auto &ctx = cranked->graphics.getContext();
    ctx.stencil = image;
    ctx.stencilTiled = tile;
}

static void playdate_graphics_setStencilPattern_lua(Cranked *cranked, LuaVal arg1) {
    // Todo: Can this just be implemented as creating an image, adding the pattern or dither, then setting it as the stencil?
}

static void playdate_graphics_clearStencil_lua(Cranked *cranked) {
    cranked->graphics.getContext().stencil = nullptr;
}

static void playdate_graphics_setDrawMode_lua(Cranked *cranked, BitmapDrawMode drawMode) {
    cranked->graphics.getContext().bitmapDrawMode = drawMode;
}

static LCDBitmapDrawMode playdate_graphics_getImageDrawMode_lua(Cranked *cranked) {
    return cranked->graphics.getContext().bitmapDrawMode;
}

static void playdate_graphics_setLineWidth_lua(Cranked *cranked, int width) {
    cranked->graphics.getContext().lineWidth = width;
}

static int playdate_graphics_getLineWidth_lua(Cranked *cranked) {
    return cranked->graphics.getContext().lineWidth;
}

static void playdate_graphics_setStrokeLocation_lua(Cranked *cranked, StrokeLocation location) {
    cranked->graphics.getContext().strokeLocation = location;
}

static StrokeLocation playdate_graphics_getStrokeLocation_lua(Cranked *cranked) {
    return cranked->graphics.getContext().strokeLocation;
}

static void playdate_graphics_lockFocus_lua(Cranked *cranked, LuaVal imageVal) {
    cranked->graphics.getContext().focusedImage = imageVal.asUserdataObject<Bitmap>();
}

static void playdate_graphics_unlockFocus_lua(Cranked *cranked) {
    cranked->graphics.getContext().focusedImage = nullptr;
}

static LuaRet playdate_graphics_getDrawOffset_lua(Cranked *cranked) {
    auto &offset = cranked->graphics.getContext().drawOffset;
    lua_pushinteger(cranked->getLuaContext(), offset.x);
    lua_pushinteger(cranked->getLuaContext(), offset.y);
    return 2;
}

static LuaRet playdate_graphics_getDisplayImage_lua(Cranked *cranked) {
    pushResource(cranked, cranked->heap.construct<LCDBitmap_32>(*cranked->graphics.frameBuffer));
    return 1;
}

static LuaRet playdate_graphics_getWorkingImage_lua(Cranked *cranked) {
    pushResource(cranked, cranked->heap.construct<LCDBitmap_32>(*cranked->graphics.previousFrameBuffer));
    return 1;
}

static LuaRet playdate_getSecondsSinceEpoch_lua(lua_State *context) {
    lua_pushinteger(context, (int) duration_cast<chrono::seconds>(chrono::system_clock::now().time_since_epoch()).count());
    lua_pushinteger(context, (int) duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count());
    return 2;
}

static LuaRet playdate_getTime_lua(Cranked *cranked) {
    pushTime(cranked, chrono::system_clock::now() + chrono::hours(cranked->timezoneOffset));
    return 1;
}

static LuaRet playdate_getGMTTime_lua(Cranked *cranked) {
    pushTime(cranked, chrono::system_clock::now());
    return 1;
}

static LuaRet playdate_epochFromTime_lua(Cranked *cranked, LuaVal time) {
    auto point = time.asTime() - chrono::hours(cranked->timezoneOffset);
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
    chrono::system_clock::time_point time;
    time += chrono::seconds(seconds);
    time += chrono::milliseconds(milliseconds);
    pushTime(cranked, time + chrono::hours(cranked->timezoneOffset));
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
    return (int)round(1000.0f / cranked->getFrameDelta());
}

static LuaRet playdate_getStats_lua(Cranked *cranked) {
    // Todo (Return Nil when acting as simulator)
    auto stats = pushTable(cranked);
    stats.setFloatField("kernel", 0.f);
    stats.setFloatField("serial", 0.f);
    stats.setFloatField("game", 100.f);
    stats.setFloatField("GC", 0.f);
    stats.setFloatField("wifi", 0.f);
    stats.setFloatField("audio", 0.f);
    stats.setFloatField("trace", 0.f);
    stats.setFloatField("idle", 0.f);
    return 1;
}

static void playdate_setStatsInterval_lua(Cranked *cranked, int seconds) {
    cranked->statsInterval = seconds;
}

static LuaRet playdate_getPowerStatus_lua(Cranked *cranked) {
    // Todo
    auto status = pushTable(cranked);
    status.setBoolField("charging", false);
    status.setBoolField("USB", false);
    status.setBoolField("screws", false);
    return 1;
}

static LuaRet playdate_getSystemMenu_lua(Cranked *cranked) {
    pushTable(cranked); // Just use an empty table
    if (!cranked->luaEngine.getQualifiedLuaGlobal("playdate.menu"))
        throw CrankedError("Playdate menu table missing");
    lua_setmetatable(cranked->getLuaContext(), -2);
    return 1;
}

static void playdate_setMenuImage_lua(Cranked *cranked, LuaVal image, int xOffset) {
    auto bitmap = image.asUserdataObject<Bitmap>();
    cranked->menu.setImage(bitmap, xOffset);
}

static LuaRet playdate_menu_addMenuItem_lua(Cranked *cranked, LuaVal menu, const char *title, LuaVal callback) {
    try {
        pushResource(cranked, cranked->menu.addItem(title, PDMenuItem_32::Type::Button, {}, 0, 0, callback));
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
        pushResource(cranked, cranked->menu.addItem(title, PDMenuItem_32::Type::Checkmark, {}, initialValue, 0, callback));
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
        pushResource(cranked, cranked->menu.addItem(title, PDMenuItem_32::Type::Options, options, initialValue, 0, callback));
    } catch (exception &ex) {
        lua_pushnil(cranked->getLuaContext());
        lua_pushstring(cranked->getLuaContext(), ex.what());
        return 2;
    }
    return 1;
}

static LuaRet playdate_menu_getMenuItems_lua(Cranked *cranked, LuaVal menu) {
    pushTable(cranked);
    int i = 1;
    for (auto &item : cranked->menu.items) {
        if (!item)
            continue;
        pushResource(cranked, item);
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
    if (!cranked->luaEngine.getQualifiedLuaGlobal("cranked.menuCallbacks"))
        throw CrankedError("Menu callbacks table missing");
    lua_pushvalue(cranked->getLuaContext(), callback);
    lua_seti(cranked->getLuaContext(), -2, cranked->menu.findItem(item) + 1);
    lua_pop(cranked->getLuaContext(), 1);
}

static void playdate_menu_item_setTitle_lua(Cranked *cranked, MenuItem item, const char *title) {
    item->title = title;
}

static const char *playdate_menu_item_getTitle_lua(Cranked *cranked, MenuItem item) {
    return item->title.c_str();
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
    if (cranked->luaEngine.indexMetatable())
        return 1;
    if (!keyArg.isString())
        return returnValues(cranked, nullptr);
    string key(keyArg.asString());
    if (key == "value")
        return returnValues(cranked, item->value);
    if (key == "title")
        return returnValues(cranked, item->title.c_str());
    return returnValues(cranked, nullptr);
}

static float playdate_display_getRefreshRate_lua(Cranked *cranked) {
    return cranked->graphics.framerate;
}

static LuaRet playdate_display_getSize_lua(Cranked *cranked) {
    return returnVecValues(cranked, IntVec2{ DISPLAY_WIDTH, DISPLAY_HEIGHT} / cranked->graphics.displayScale );
}

static LuaValRet playdate_display_getRect_lua(Cranked *cranked) {
    return pushRect(cranked, IntRect {0, 0, DISPLAY_WIDTH / cranked->graphics.displayScale, DISPLAY_HEIGHT / cranked->graphics.displayScale}); // Todo: Includes offset?
}

static int playdate_display_getScale_lua(Cranked *cranked) {
    return cranked->graphics.displayScale;
}

static bool playdate_display_getInverted_lua(Cranked *cranked) {
    return cranked->graphics.displayInverted;
}

static LuaRet playdate_display_getMosaic_lua(Cranked *cranked) {
    return returnVecValues(cranked, cranked->graphics.displayMosaic);
}

static LuaRet playdate_display_getOffset_lua(Cranked *cranked) {
    return returnVecValues(cranked, cranked->graphics.displayOffset);
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

static LuaRet playdate_graphics_imagetable_new_lua(Cranked *cranked, LuaVal arg1, LuaVal arg2) {
    if (arg1.isString()) {
        try {
            return returnValues(cranked, cranked->graphics.getBitmapTable(arg1.asString()));
        } catch (exception &ex) {
            return returnValues(cranked, nullptr, ex.what());
        }
    }
    auto table = cranked->heap.construct<LCDBitmapTable_32>(*cranked, arg2.asInt());
    table->bitmaps.resize(arg1.asInt());
    pushResource(cranked, table);
    return 1;
}

static LuaValRet playdate_graphics_imagetable_getImage_lua(Cranked *cranked, BitmapTable table, int arg1, LuaVal arg2) {
    int i = arg2.isNil() ? arg1 - 1 : arg1 * table->cellsPerRow + arg2.asInt() - 1;
    return i < 0 or i >= table->bitmaps.size() ? pushNil(cranked) : pushResource(cranked, table->bitmaps[i]);
}

static void playdate_graphics_imagetable_setImage_lua(Cranked *cranked, BitmapTable table, int n, Bitmap image) {
    if (n > 0 and n <= table->bitmaps.size())
        table->bitmaps[n - 1] = image;
}

static LuaRet playdate_graphics_imagetable_load_lua(Cranked *cranked, BitmapTable table, const char *path) {
    try {
        BitmapTableRef tableRef = cranked->graphics.getBitmapTable(path);
        *table = *tableRef;
        return returnValues(cranked, true);
    } catch (exception &ex) {
        return returnValues(cranked, false, ex.what());
    }
}

static int playdate_graphics_imagetable_getLength_lua(Cranked *cranked, BitmapTable table) {
    return (int)table->bitmaps.size();
}

static LuaRet playdate_graphics_imagetable_getSize_lua(Cranked *cranked, BitmapTable table) {
    return returnVecValues(cranked, IntVec2 { table->cellsPerRow, (int)table->bitmaps.size() / table->cellsPerRow });
}

static void playdate_graphics_imagetable_drawImage_lua(Cranked *cranked, BitmapTable table, int n, int x, int y, GraphicsFlip flip) {
    if (n > 0 and n <= table->bitmaps.size())
        cranked->graphics.drawBitmap(table->bitmaps[n - 1], x, y, flip);
}

static void playdate_graphics_imagetable_gc_lua(Cranked *cranked, BitmapTable table) {
    table->dereference();
}

static LuaRet playdate_graphics_imagetable_index_lua(Cranked *cranked, BitmapTable table, int n) {
    if (cranked->luaEngine.indexMetatable())
        return 1;
    if (n > 0 and n <= table->bitmaps.size())
        pushResource(cranked, table->bitmaps[n - 1]);
    else
        pushNil(cranked);
    return 1;
}

static void playdate_graphics_imagetable_newindex_lua(Cranked *cranked, BitmapTable table, int n, Bitmap image) {
    if (n > 0 and n <= table->bitmaps.size())
        table->bitmaps[n - 1] = image;
}

static LuaValRet playdate_graphics_tilemap_new_lua(Cranked *cranked) {
    return pushResource(cranked, cranked->heap.construct<LCDTileMap_32>(*cranked));
}

static void playdate_graphics_tilemap_gc_lua(Cranked *cranked, TileMap tilemap) {
    tilemap->dereference();
}

static void playdate_graphics_tilemap_setImageTable_lua(Cranked *cranked, TileMap tilemap, BitmapTable table) {
    tilemap->table = table;
}

static void playdate_graphics_tilemap_setTiles_lua(Cranked *cranked, TileMap tilemap, LuaVal data, int width) {
    tilemap->width = width;
    tilemap->tiles.clear();
    for (int i = 1;; i++) {
        auto index = data.getElement(i);
        if (index.val.isNil())
            break;
        tilemap->tiles.push_back(index.val.asInt());
    }
    int height = (int)ceil((float)tilemap->tiles.size() / (float)width);
    tilemap->tiles.resize(width * height); // Round up last row if needed
}

static LuaValRet playdate_graphics_tilemap_getTiles_lua(Cranked *cranked, TileMap tilemap) {
    auto table = pushTable(cranked);
    for (int i = 0; i < tilemap->tiles.size(); i++)
        table.setIntElement(i + 1, tilemap->tiles[i]);
    return table;
}

static void playdate_graphics_tilemap_draw_lua(Cranked *cranked, TileMap tilemap, int x, int y, LuaVal sourceRect) {
    optional<IntRect> rect;
    if (sourceRect.isTable())
        rect = sourceRect.asIntRect();
    else if (sourceRect.isNumeric())
        rect = IntRect{ sourceRect.asInt(), (sourceRect + 1).asInt(), (sourceRect + 2).asInt(), (sourceRect + 3).asInt() };
    cranked->graphics.drawTileMap(tilemap, x, y, false, rect);
}

static void playdate_graphics_tilemap_drawIgnoringOffset_lua(Cranked *cranked, TileMap tilemap, int x, int y, LuaVal sourceRect) {
    optional<IntRect> rect;
    if (sourceRect.isTable())
        rect = sourceRect.asIntRect();
    else if (sourceRect.isNumeric())
        rect = IntRect{ sourceRect.asInt(), (sourceRect + 1).asInt(), (sourceRect + 2).asInt(), (sourceRect + 3).asInt() };
    cranked->graphics.drawTileMap(tilemap, x, y, true, rect);
}

static void playdate_graphics_tilemap_setTileAtPosition_lua(Cranked *cranked, TileMap tilemap, int x, int y, int index) {
    int i = (y - 1) * tilemap->width + x - 1;
    if (i < tilemap->tiles.size())
        tilemap->tiles[i] = index;
}

static LuaValRet playdate_graphics_tilemap_getTileAtPosition_lua(Cranked *cranked, TileMap tilemap, int x, int y) {
    int i = (y - 1) * tilemap->width + x - 1;
    return i < tilemap->tiles.size() ? pushInt(cranked, tilemap->tiles[i]) : pushNil(cranked);
}

static void playdate_graphics_tilemap_setSize_lua(Cranked *cranked, TileMap tilemap, int width, int height) {
    tilemap->tiles.resize(width * height);
    tilemap->width = width;
}

static LuaRet playdate_graphics_tilemap_getSize_lua(Cranked *cranked, TileMap tilemap) {
    pushInt(cranked, tilemap->width);
    pushInt(cranked, tilemap->getHeight());
    return returnVecValues(cranked, IntVec2 { tilemap->width, tilemap->getHeight() });
}

static LuaRet playdate_graphics_tilemap_getPixelSize_lua(Cranked *cranked, TileMap tilemap) {
    auto [width, height] = tilemap->getCellSize();
    return returnVecValues(cranked, IntVec2 { width * tilemap->width, height * tilemap->getHeight() });
}

static LuaRet playdate_graphics_tilemap_getTileSize_lua(Cranked *cranked, TileMap tilemap) {
    return returnVecValues(cranked, tilemap->getCellSize());
}

static LuaValRet playdate_graphics_tilemap_getCollisionRects_lua(Cranked *cranked, TileMap tilemap, LuaVal emptyIDs) {
    unordered_set<int> empty;
    for (int i = 1;; i++) {
        auto index = emptyIDs.getElement(i);
        if (index.val.isNil())
            break;
        empty.emplace(index.val.asInt());
    }
    auto table = pushTable(cranked);
    auto cellSize = tilemap->getCellSize();
    auto [ cellWidth, cellHeight ] = cellSize;
    for (int y = 0; y < tilemap->getHeight(); y++) {
        for (int x = 0; x < tilemap->width; x++) {
            if (empty.contains(tilemap->tiles[y * tilemap->width + x]))
                continue;
            LuaValGuard rect(pushRect(cranked, IntRect{ { cellWidth * x, cellHeight * y }, cellSize }));
            table.setElement(y * tilemap->width * x + 1, rect.val);
        }
    }
    return table;
}

static void playdate_graphics_setFont_lua(Cranked *cranked, Font font, PDFontVariant variant) {
    cranked->graphics.getContext().setFont(variant, font);
}

static LuaValRet playdate_graphics_getFont_lua(Cranked *cranked, PDFontVariant variant) {
    return pushResource(cranked, cranked->graphics.getContext().getFont(variant));
}

static void playdate_graphics_setFontFamily_lua(Cranked *cranked, LuaVal fontFamily) {
    for (int i = 0; i < 3; i++) {
        auto font = fontFamily.getElement(i);
        if (font.val.isNil())
            continue;
        cranked->graphics.getContext().setFont((PDFontVariant)i, font.val.asUserdataObject<Font>());
    }
}

static void playdate_graphics_setFontTracking_lua(Cranked *cranked, int pixels) {
    cranked->graphics.getContext().textTracking = pixels;
}

static int playdate_graphics_getFontTracking_lua(Cranked *cranked) {
    return cranked->graphics.getContext().textTracking;
}

static LuaValRet playdate_graphics_getSystemFont_lua(Cranked *cranked, PDFontVariant variant) {
    return pushResource(cranked, cranked->graphics.getSystemFont(variant));
}

static void playdate_graphics_drawText_lua(Cranked *cranked, const char *text, LuaVal arg2) {
    auto &ctx = cranked->graphics.getContext();
    int offset;
    IntVec2 pos;
    IntVec2 size;
    if (arg2.isTable()) {
        IntRect rect = arg2.asIntRect();
        pos = rect.pos;
        size = rect.size;
        offset = 1;
    } else {
        pos = { arg2.asInt(), (arg2 + 1).asInt() };
        if ((arg2 + 2).isInt()) {
            size = { (arg2 + 2).isInt(), (arg2 + 3).isInt() };
            offset = 4;
        } else {
            size = {};
            offset = 2;
        }
    }
    FontFamily family = (arg2 + offset + 1).isTable() ? (arg2 + offset + 1).asFontFamily() : FontFamily{};
    int leading = (arg2 + offset + 2).isInt() ? (arg2 + offset + 2).asInt() : 0;
    TextWrap wrap = (arg2 + offset + 3).isInt() ? (arg2 + offset + 3).asEnum<TextWrap>() : TextWrap::Clip;
    TextAlign align = (arg2 + offset + 4).isInt() ? (arg2 + offset + 4).asEnum<TextAlign>() : TextAlign::Left;
    cranked->graphics.drawText(pos.x, pos.y, text, ctx.fonts, nullptr, PDStringEncoding::UFT8, size, wrap, align, 0, leading);
}

static void playdate_graphics_drawLocalizedText_lua(Cranked *cranked, const char *key, LuaVal arg2) {
    auto &ctx = cranked->graphics.getContext();
    int offset;
    IntVec2 pos;
    IntVec2 size;
    if (arg2.isTable()) {
        IntRect rect = arg2.asIntRect();
        pos = rect.pos;
        size = rect.size;
        offset = 1;
    } else {
        pos = { arg2.asInt(), (arg2 + 1).asInt() };
        if ((arg2 + 2).isInt()) {
            size = { (arg2 + 2).isInt(), (arg2 + 3).isInt() };
            offset = 4;
        } else {
            size = {};
            offset = 2;
        }
    }
    PDLanguage language = not (arg2 + offset + 1).isNil() ? (arg2 + offset + 1).asLanguage() : cranked->systemLanguage;
    int leading = (arg2 + offset + 2).isInt() ? (arg2 + offset + 2).asInt() : 0;
    TextWrap wrap = (arg2 + offset + 3).isInt() ? (arg2 + offset + 3).asEnum<TextWrap>() : TextWrap::Clip;
    TextAlign align = (arg2 + offset + 4).isInt() ? (arg2 + offset + 4).asEnum<TextAlign>() : TextAlign::Left;
    const char *text = cranked->graphics.getLocalizedText(key, language);
    cranked->graphics.drawText(pos.x, pos.y, text, ctx.fonts, nullptr, PDStringEncoding::UFT8, size, wrap, align, 0, leading);
}

static const char *playdate_graphics_getLocalizedText_lua(Cranked *cranked, const char *key, LuaVal language) {
    return cranked->graphics.getLocalizedText(key, not language.isNil() ? language.asLanguage() : cranked->systemLanguage);
}

static LuaRet playdate_graphics_getTextSize_lua(Cranked *cranked, const char *str, LuaVal fontFamily, int leadingAdjustment) {
    auto family = fontFamily.isTable() ? fontFamily.asFontFamily() : cranked->graphics.getContext().fonts;
    return returnVecValues(cranked, cranked->graphics.measureText(str, family));
}

static LuaValRet playdate_graphics_font_new_lua(Cranked *cranked, const char *path) {
    try {
        return pushResource(cranked, cranked->graphics.getFont(path));
    } catch (exception &) {
        return pushNil(cranked);
    }
}

static LuaValRet playdate_graphics_font_newFamily_lua(Cranked *cranked, LuaVal fontPaths) {
    auto table = pushTable(cranked);
    for (int i = 0; i < 3; i++) {
        auto path = fontPaths.getElement(i);
        if (!path.val.isString())
            continue;
        try {
            LuaValGuard font(pushResource(cranked, cranked->graphics.getFont(path.val.asString())));
            table.setElement(i, font.val);
        } catch (exception &) {}
    }
    return table;
}

static LuaRet playdate_graphics_font_drawText_lua(Cranked *cranked, Font font, const char *text, LuaVal arg2) {
    auto &ctx = cranked->graphics.getContext();
    int offset;
    IntVec2 pos;
    IntVec2 size;
    if (arg2.isTable()) {
        IntRect rect = arg2.asIntRect();
        pos = rect.pos;
        size = rect.size;
        offset = 1;
    } else {
        pos = { arg2.asInt(), (arg2 + 1).asInt() };
        if ((arg2 + 2).isInt()) {
            size = { (arg2 + 2).isInt(), (arg2 + 3).isInt() };
            offset = 4;
        } else {
            size = {};
            offset = 2;
        }
    }
    int leading = (arg2 + offset + 1).isInt() ? (arg2 + offset + 1).asInt() : 0;
    TextWrap wrap = (arg2 + offset + 2).isInt() ? (arg2 + offset + 2).asEnum<TextWrap>() : TextWrap::Clip;
    TextAlign align = (arg2 + offset + 3).isInt() ? (arg2 + offset + 3).asEnum<TextAlign>() : TextAlign::Left;
    cranked->graphics.drawText(pos.x, pos.y, text, ctx.fonts, font, PDStringEncoding::UFT8, size, wrap, align, 0, leading);
    return returnVecValues(cranked, IntVec2{ 0, 0 }); // Todo: Return size
}

static int playdate_graphics_font_getHeight_lua(Cranked *cranked, Font font) {
    return font->glyphHeight;
}

static int playdate_graphics_font_getTextWidth_lua(Cranked *cranked, Font font, const char *text) {
    return cranked->graphics.measureText(text, cranked->graphics.getContext().fonts, font).x;
}

static void playdate_graphics_font_setTracking_lua(Cranked *cranked, Font font, int pixels) {
    font->tracking = pixels;
}

static int playdate_graphics_font_getTracking_lua(Cranked *cranked, Font font) {
    return font->tracking;
}

static void playdate_graphics_font_setLeading_lua(Cranked *cranked, Font font, int pixels) {
    font->leading = pixels;
}

static int playdate_graphics_font_getLeading_lua(Cranked *cranked, Font font) {
    return font->leading;
}

static LuaValRet playdate_graphics_font_getGlyph_lua(Cranked *cranked, Font font, LuaVal character) {
    int c = character.isString() ? character.asString()[0] : character.asInt();
    try {
        auto &page = font->pages.at(c / 256);
        auto &glyph = *page->glyphs.at(c % 256);
        return pushResource(cranked, glyph.bitmap);
    } catch (out_of_range &) {
        return pushNil(cranked);
    }
}

static LuaValRet playdate_graphics_sprite_new_lua(Cranked *context, LuaVal imageOrTilemap) {
    auto sprite = context->graphics.createSprite();
    if (imageOrTilemap.isTable()) { // Todo: Helper checks for correct argument?
        if (LuaValGuard metaTable(imageOrTilemap.getMetaTable()); not metaTable.val.isNil()) {
            if (auto str = metaTable.val.getStringField("__name")) {
                if (string name = str; name == "playdate.graphics.tilemap")
                    sprite->setTileMap(imageOrTilemap.asUserdataObject<TileMap>());
                else if (name == "playdate.graphics.image")
                    sprite->setImage(imageOrTilemap.asUserdataObject<Bitmap>());
            }
        }
    }
    auto spriteVal = pushResource(context, sprite);
    sprite->updateLuaPosition();
    return spriteVal;
}

static void playdate_graphics_sprite_gc_lua(Cranked *cranked, Sprite sprite) {
    sprite->dereference();
}

static void playdate_graphics_sprite_update_lua(Cranked *cranked, Sprite sprite) {
    if (sprite)
        sprite->update();
    else {
        cranked->graphics.updateSprites();
        cranked->graphics.drawSprites();
    }
}

static void playdate_graphics_sprite_setImage_lua(Cranked *cranked, Sprite sprite, Bitmap image, GraphicsFlip flip, float scale, LuaVal yScale) {
    sprite->setImage(image, flip, scale, yScale.isNumeric() ? yScale.asFloat() : scale);
}

static LuaValRet playdate_graphics_sprite_getImage_lua(Cranked *cranked, Sprite sprite) {
    return pushResource(cranked, sprite->image);
}

static void playdate_graphics_sprite_add_lua(Cranked *cranked, LuaVal spriteVal) {
    auto sprite = spriteVal.asUserdataObject<Sprite>();
    cranked->luaEngine.storeResourceValue(sprite, spriteVal);
    cranked->graphics.addSpriteToDrawList(sprite);
    sprite->updateLuaPosition();
    cranked->luaEngine.preserveLuaValue(spriteVal);
}

static void playdate_graphics_sprite_remove_lua(Cranked *cranked, LuaVal spriteVal) {
    auto sprite = spriteVal.asUserdataObject<Sprite>();
    cranked->graphics.removeSpriteFromDrawList(sprite);
    cranked->luaEngine.releaseLuaValue(spriteVal);
}

static LuaRet playdate_graphics_sprite_getPosition_lua(Cranked *cranked, Sprite sprite) {
    return returnVecValues(cranked, sprite->getPosition());
}

static void playdate_graphics_sprite_setCenter_lua(Cranked *cranked, Sprite sprite, float x, float y) {
    sprite->setCenter({ x, y });
}

static LuaRet playdate_graphics_sprite_getCenter_lua(Cranked *cranked, Sprite sprite) {
    return returnVecValues(cranked, sprite->getCenter());
}

static LuaValRet playdate_graphics_sprite_getCenterPoint_lua(Cranked *cranked, Sprite sprite) {
    return pushPoint(cranked, sprite->getCenter());
}

static LuaRet playdate_graphics_sprite_getSize_lua(Cranked *cranked, Sprite sprite) {
    return returnVecValues(cranked, sprite->getSize());
}

static void playdate_graphics_sprite_setScale_lua(Cranked *cranked, Sprite sprite, float scale, LuaVal yScale) {
    sprite->scale = { scale, yScale.isNumeric() ? yScale.asFloat() : scale };
}

static LuaRet playdate_graphics_sprite_getScale_lua(Cranked *cranked, Sprite sprite) {
    return returnVecValues(cranked, sprite->scale);
}

static void playdate_graphics_sprite_setRotation_lua(Cranked *cranked, Sprite sprite, float angle, LuaVal scale, LuaVal yScale) {

    float xScale = scale.isNumeric() ? scale.asFloat() : 1;
    sprite->scale = { xScale, yScale.isNumeric() ? yScale.asFloat() : xScale };
    sprite->rotation = angle;
}

static float playdate_graphics_sprite_getRotation_lua(Cranked *cranked, Sprite sprite) {
    return sprite->rotation;
}

static LuaValRet playdate_graphics_sprite_copy_lua(Cranked *cranked, Sprite sprite) {
    return pushResource(cranked, sprite->copy());
}

static void playdate_graphics_sprite_setImageFlip_lua(Cranked *cranked, Sprite sprite, GraphicsFlip flip, bool flipCollideRect) {
    sprite->flip = flip;
    sprite->collideRectFlip = flipCollideRect ? flip : GraphicsFlip::Unflipped;
}

static void playdate_graphics_sprite_setBounds_lua(Cranked *cranked, Sprite sprite, LuaVal arg1, LuaVal arg2, LuaVal arg3, LuaVal arg4) {
    if (arg1.isTable())
        sprite->setBounds(arg1.asRect());
    else
        sprite->setBounds({ arg1.asFloat(), arg2.asFloat(), arg3.asFloat(), arg4.asFloat() });
}

static LuaRet playdate_graphics_sprite_getBounds_lua(Cranked *cranked, Sprite sprite) {
    return returnRectValues(cranked, sprite->getBounds());
}

static LuaValRet playdate_graphics_sprite_getBoundsRect_lua(Cranked *cranked, Sprite sprite) {
    return pushRect(cranked, sprite->getBounds());
}

static bool playdate_graphics_sprite_isOpaque_lua(Cranked *cranked, Sprite sprite) {
    return sprite->opaque;
}

static void playdate_graphics_sprite_setTilemap_lua(Cranked *cranked, Sprite sprite, TileMap tilemap) {
    sprite->setTileMap(tilemap);
}

static void playdate_graphics_sprite_setClipRect_lua(Cranked *cranked, Sprite sprite, LuaVal arg1, LuaVal arg2, LuaVal arg3, LuaVal arg4) {
    if (arg1.isTable())
        sprite->clipRect = arg1.asIntRect();
    else
        sprite->clipRect = { arg1.asInt(), arg2.asInt(), arg3.asInt(), arg4.asInt() };
}

static void playdate_graphics_sprite_setClipRectsInRange_lua(Cranked *cranked, LuaVal arg1, LuaVal arg2, LuaVal arg3, LuaVal arg4, LuaVal arg5, LuaVal arg6) {
    if (arg4.isNil())
        cranked->graphics.setSpriteClipRectsInRage(arg1.asIntRect(), arg2.asInt(), arg3.asInt());
    else
        cranked->graphics.setSpriteClipRectsInRage(intRectFromArgs(arg1), arg5.asInt(), arg6.asInt());
}

static void playdate_graphics_sprite_setStencilImage_lua(Cranked *cranked, Sprite sprite, Bitmap stencil, bool tile) {
    sprite->stencil = stencil;
    sprite->stencilTiled = tile;
}

static void playdate_graphics_sprite_setStencilPattern_lua(Cranked *cranked, Sprite sprite, LuaVal arg1, LuaVal arg2) {
    // Todo
}

static bool playdate_graphics_sprite_getAlwaysRedraw_lua(Cranked *cranked) {
    return cranked->graphics.alwaysRedrawSprites;
}

void playdate_sprite_addDirtyRect_lua(Cranked *cranked, int x, int y, int width, int height) {
    cranked->graphics.addSpriteDirtyRect({ { x, y }, { width, height } });
}

static void playdate_graphics_sprite_setRedrawsOnImageChange_lua(Cranked *cranked, Sprite sprite, bool flag) {
    sprite->dontRedrawOnImageChange = !flag;
}

static LuaValRet playdate_graphics_sprite_getAllSprites_lua(Cranked *cranked) {
    auto table = pushTable(cranked);
    int i = 1;
    for (auto &sprite : cranked->graphics.spriteDrawList) {
        LuaValGuard ref(cranked->luaEngine.pushResource(sprite));
        table.setElement(i++, ref.val);
    }
    return table;
}

static void playdate_graphics_sprite_removeSprites_lua(Cranked *cranked, LuaVal spriteArray) {
    for (int i = 1;; i++) {
        auto value = spriteArray.getElement(i);
        if (value.val.isNil())
            break;
        cranked->graphics.removeSpriteFromDrawList(value.val.asUserdataObject<Sprite>());
    }
}

static void playdate_graphics_sprite_setCollideRect_lua(Cranked *cranked, Sprite sprite, LuaVal arg1) {
    sprite->setCollisionRect(arg1.isTable() ? arg1.asRect() : rectFromArgs(arg1));
}

static LuaValRet playdate_graphics_sprite_getCollideRect_lua(Cranked *cranked, Sprite sprite) {
    return pushRect(cranked, sprite->collideRect);
}

static LuaRet playdate_graphics_sprite_getCollideBounds_lua(Cranked *cranked, Sprite sprite) {
    return returnRectValues(cranked, sprite->collideRect);
}

static LuaValRet playdate_graphics_sprite_overlappingSprites_lua(Cranked *cranked, Sprite sprite) {
    auto overlapping = cranked->bump.getOverlappingSprites(sprite);
    auto table = pushTable(cranked);
    for (int i = 0; i < overlapping.size(); i++) {
        LuaValGuard ref(pushResource(cranked, overlapping[i]));
        table.setElement(i + 1, ref);
    }
    return table;
}

static LuaValRet playdate_graphics_sprite_allOverlappingSprites_lua(Cranked *cranked) {
    auto overlapping = cranked->bump.getAllOverlappingSprites();
    auto table = pushTable(cranked);
    for (int i = 0; i < (int)overlapping.size() / 2; i++) {
        LuaValGuard pair(pushTable(cranked));
        LuaValGuard first(pushResource(cranked, overlapping[i * 2]));
        pair->setElement(1, first);
        LuaValGuard second(pushResource(cranked, overlapping[i * 2 + 1]));
        pair->setElement(2, second);
        table.setElement(i + 1, pair);
    }
    return table;
}

static bool playdate_graphics_sprite_alphaCollision_lua(Cranked *cranked, Sprite sprite, Sprite anotherSprite) {
    if (not sprite->image or not anotherSprite->image)
        return false;
    auto spritePos = sprite->getBounds().pos.as<int32>();
    auto otherPos = anotherSprite->getBounds().pos.as<int32>();
    return Graphics::checkBitmapMaskCollision(sprite->image, spritePos.x, spritePos.y, GraphicsFlip::Unflipped, anotherSprite->image, otherPos.x, otherPos.y, GraphicsFlip::Unflipped, sprite->getBounds().as<int32>());
}

static void playdate_graphics_sprite_setGroups_lua(Cranked *cranked, Sprite sprite, LuaVal groups) {
    int mask = 0;
    iterateArray(groups, [&](LuaVal group) {
        mask |= 1 << group.asInt() - 1;
        return false;
    });
    sprite->groupMask = mask;
}

static void playdate_graphics_sprite_setCollidesWithGroups_lua(Cranked *cranked, Sprite sprite, LuaVal groups) {
    int mask = 0;
    iterateArray(groups, [&](LuaVal group) {
        mask |= 1 << group.asInt() - 1;
        return false;
    });
    sprite->collidesWithGroupMask = mask;
}

static void playdate_graphics_sprite_setGroupMask_lua(Cranked *cranked, Sprite sprite, uint32 mask) {
    sprite->groupMask = mask;
}

static uint32 playdate_graphics_sprite_getGroupMask_lua(Cranked *cranked, Sprite sprite) {
    return sprite->groupMask;
}

static void playdate_graphics_sprite_setCollidesWithGroupsMask_lua(Cranked *cranked, Sprite sprite, uint32 mask) {
    sprite->collidesWithGroupMask = mask;
}

static uint32 playdate_graphics_sprite_getCollidesWithGroupsMask_lua(Cranked *cranked, Sprite sprite) {
    return sprite->collidesWithGroupMask;
}

static void playdate_graphics_sprite_resetGroupMask_lua(Cranked *cranked, Sprite sprite) {
    sprite->groupMask = 0;
}

static void playdate_graphics_sprite_resetCollidesWithGroupsMask_lua(Cranked *cranked, Sprite sprite) {
    sprite->collidesWithGroupMask = 0;
}

static LuaRet moveOrCheckCollisions(LuaVal spriteVal, LuaVal arg1, LuaVal arg2, bool sim) {
    auto sprite = spriteVal.asUserdataObject<Sprite>();
    auto cranked = &sprite->cranked;
    auto context = cranked->getLuaContext();
    Vec2 goal = arg2.isNil() ? arg1.asPoint() : Vec2{ arg1.asFloat(), arg2.asFloat() };
    auto response = spriteVal.getField("collisionResponse");
    bool hasResponseFunc = response.val.isFunction();
    auto responseType = hasResponseFunc ? Bump::ResponseType{} : response.val.asCollisionResponse();
    auto [actual, collisions] = cranked->bump.move(sprite, goal, sim, [&](Sprite a, Sprite b){
        if (a->collidesWithGroupMask and b->groupMask and not (a->collidesWithGroupMask & b->groupMask))
            return Bump::ResponseType::Ignore;
        if (hasResponseFunc) {
            lua_pushvalue(context, response.val);
            pushResource(cranked, a);
            pushResource(cranked, b);
            lua_call(context, 2, 1); // Todo: Protected?
            LuaValGuard value{LuaVal(context)};
            if (!value.val.isNil())
                return value.val.asCollisionResponse();
        }
        return responseType;
    });
    response.pop();

    pushFloat(cranked, actual.x);
    pushFloat(cranked, actual.y);
    auto collisionsArray = pushTable(cranked);
    for (int i = 0; i < collisions.size(); i++) {
        auto &entry = collisions[i];
        LuaValGuard collision(pushTable(cranked));
        setResourceField(collision.val, "sprite", entry.sprite);
        setResourceField(collision.val, "other", entry.other);
        collision.val.setIntField("type", (int)entry.type);
        collision.val.setBoolField("overlaps", entry.overlaps);
        collision.val.setFloatField("ti", entry.ti);
        setVecField(collision.val, "move", entry.move);
        setVecField(collision.val, "normal", entry.normal);
        setPointField(collision.val, "touch", entry.touch);
        setRectField(collision.val, "spriteRect", entry.entityRect);
        setRectField(collision.val, "otherRect", entry.otherRect);
        if (entry.type == Bump::ResponseType::Bounce)
            setPointField(collision.val, "bounce", entry.bounce);
        if (entry.type == Bump::ResponseType::Slide)
            setPointField(collision.val, "slide", entry.slide);
        collisionsArray.setElement(i + 1, collision.val);
    }
    pushInt(cranked, (int)collisions.size());
    return 4;
}

static LuaRet playdate_graphics_sprite_moveWithCollisions_lua(Cranked *cranked, LuaVal sprite, LuaVal arg1, LuaVal arg2) {
    return moveOrCheckCollisions(sprite, arg1, arg2, false);
}

static LuaRet playdate_graphics_sprite_checkCollisions_lua(Cranked *cranked, LuaVal sprite, LuaVal arg1, LuaVal arg2) {
    return moveOrCheckCollisions(sprite, arg1, arg2, true);
}

static LuaRet returnSpriteList(Cranked *cranked, const vector<Sprite> &sprites) {
    auto table = pushTable(cranked);
    int i = 1;
    for (auto sprite : sprites)
        setResourceElement(table, i++, sprite);
    return 1;
}

static LuaRet playdate_graphics_sprite_querySpritesAtPoint_lua(Cranked *cranked, LuaVal arg1, LuaVal arg2) {
    auto point = arg2.isNil() ? arg1.asVec() : Vec2{ arg1.asFloat(), arg2.asFloat() };
    return returnSpriteList(cranked, cranked->bump.queryPoint(point));
}

static LuaRet playdate_graphics_sprite_querySpritesInRect_lua(Cranked *cranked, LuaVal arg1, LuaVal arg2, LuaVal arg3, LuaVal arg4) {
    auto rect = arg2.isNil() ? arg1.asRect() : Rect{ arg1.asFloat(), arg2.asFloat(), arg3.asFloat(), arg4.asFloat() };
    return returnSpriteList(cranked, cranked->bump.queryRect(rect));
}

static LuaRet playdate_graphics_sprite_querySpritesAlongLine_lua(Cranked *cranked, LuaVal arg1, LuaVal arg2, LuaVal arg3, LuaVal arg4) {
    auto segment = arg2.isNil() ? arg1.asLineSeg() : LineSeg{ arg1.asFloat(), arg2.asFloat(), arg3.asFloat(), arg4.asFloat() };
    return returnSpriteList(cranked, cranked->bump.querySegment(segment));
}

static LuaRet playdate_graphics_sprite_querySpriteInfoAlongLine_lua(Cranked *cranked, LuaVal arg1, LuaVal arg2, LuaVal arg3, LuaVal arg4) {
    auto segment = arg2.isNil() ? arg1.asLineSeg() : LineSeg{ arg1.asFloat(), arg2.asFloat(), arg3.asFloat(), arg4.asFloat() };
    auto segmentInfo = cranked->bump.querySegmentInfo(segment);
    auto table = pushTable(cranked);
    pushInt(cranked, (int)segmentInfo.size());
    int i = 1;
    for (auto &info : segmentInfo) {
        LuaValGuard infoTable(pushTable(cranked));
        table.setElement(i++, infoTable.val);
        setResourceField(infoTable.val, "sprite", info.sprite);
        setPointField(infoTable.val, "entryPoint", info.seg.start);
        setPointField(infoTable.val, "exitPoint", info.seg.end);
        infoTable.val.setFloatField("ti1", info.ti1);
        infoTable.val.setFloatField("ti2", info.ti2);
    }
    return 2;
}

static LuaValRet playdate_graphics_video_new_lua(Cranked *cranked, const char *path) {
    try {
        return pushResource(cranked, cranked->graphics.getVideoPlayer(path));
    } catch (exception &) {
        return pushNil(cranked);
    }
}

static void playdate_graphics_video_gc_lua(Cranked *cranked, VideoPlayer video) {
    video->dereference();
}

static LuaRet playdate_graphics_video_getSize_lua(Cranked *cranked, VideoPlayer video) {
    return returnVecValues(cranked, video->size);
}

static int playdate_graphics_video_getFrameCount_lua(Cranked *cranked, VideoPlayer video) {
    return (int)video->frames.size();
}

static float playdate_graphics_video_getFrameRate_lua(Cranked *cranked, VideoPlayer video) {
    return video->frameRate;
}

static void playdate_graphics_video_setContext_lua(Cranked *cranked, VideoPlayer video, Bitmap image) {
    video->target = image;
}

static NodeGraph playdate_pathfinder_graph_new_lua(Cranked *cranked, int nodeCount, LuaVal coordinates) {
    auto graph = cranked->nativeEngine.createReferencedResource<NodeGraph>();
    for (int i = 1; i <= nodeCount; i++) {
        auto node = graph->addNewNode();
        node->id = i;
        if (!coordinates.isTable())
            continue;
        if (auto coords = coordinates.getElement(i); coords.val.isTable()) {
            node->x = coords.val.getIntElement(1);
            node->x = coords.val.getIntElement(2);
        }
    }
    return graph;
}

static void playdate_pathfinder_graph_gc_lua(Cranked *cranked, NodeGraph graph) {
    graph->dereference();
}

static NodeGraph playdate_pathfinder_graph_new2DGrid_lua(Cranked *cranked, int width, int height, bool allowDiagonals, LuaVal includedNodes) {
    constexpr int cost = 10, diagonalCost = 14;
    const int count = width * height;
    vector connected(count, true);
    vector<GraphNode> nodes(count);
    if (includedNodes.isTable())
        for (int i = 0; i < count; i++)
            connected[i] = includedNodes.getIntElement(i + 1);
    auto graph = cranked->heap.construct<PDNodeGraph_32>(*cranked);
    for (int i = 0; i < count; i++)
        nodes[i] = graph->addNewNode();
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int i = y * width + x;
            auto node = nodes[i];
            node->id = i + 1;
            node->x = x + 1;
            node->y = y + 1;
            const auto handleConnection = [=, &connected, &nodes](int dx, int dy) {
                int cx = x + dx, cy = y + dy;
                bool diagonal = dx != 0 and dy != 0;
                if (cx >= width or cy >= height or (dx == 0 and dy == 0) or (diagonal and not allowDiagonals))
                    return;
                int ci = cy * width + cx;
                if (not connected[ci])
                    return;
                auto other = nodes[ci];
                int weight = diagonal ? diagonalCost : cost;
                node->addConnection(other, weight);
                other->addConnection(node, weight);
            };
            for (int j = 0; j <= 1; j++)
                for (int k = 0; k <= 1; k++)
                    handleConnection(j, k);
        }
    }
    return graph;
}

static GraphNode playdate_pathfinder_graph_addNewNode_lua(Cranked *cranked, NodeGraph graph, int id, int x, int y, LuaVal connectedNodes, LuaVal weights, bool addReciprocalConnections) {
    auto node = graph->addNewNode();
    node->id = id;
    node->x = x;
    node->y = y;
    if (not connectedNodes.isTable() or not weights.isTable())
        return node;
    for (int i = 1;; i++) {
        auto connected = connectedNodes.getElement(i);
        auto weight = weights.getElement(i);
        if (connected.val.isNil() or weight.val.isNil())
            break;
        auto otherNode = connected.val.asUserdataObject<GraphNode>();
        auto weightVal = weight.val.asInt();
        node->addConnection(otherNode, weightVal);
        if (addReciprocalConnections)
            otherNode->addConnection(node, weightVal);
    }
    return node;
}

static LuaValRet playdate_pathfinder_graph_addNewNodes_lua(Cranked *cranked, NodeGraph graph, int count) {
    auto table = pushTable(cranked);
    for (int i = 1; i <= count; i++)
        setResourceElement(table, i, graph->addNewNode());
    return table;
}

static void playdate_pathfinder_graph_addNode_lua(Cranked *cranked, NodeGraph graph, GraphNode node, LuaVal connectedNodes, LuaVal weights, bool addReciprocalConnections) {
    graph->nodes.emplace(node);
    if (not connectedNodes.isTable() or not weights.isTable())
        return;
    for (int i = 1;; i++) {
        auto connected = connectedNodes.getElement(i);
        auto weight = weights.getElement(i);
        if (connected.val.isNil() or weight.val.isNil())
            break;
        auto otherNode = connected.val.asUserdataObject<GraphNode>();
        auto weightVal = weight.val.asInt();
        node->addConnection(otherNode, weightVal);
        if (addReciprocalConnections)
            otherNode->addConnection(node, weightVal);
    }
}

static void playdate_pathfinder_graph_addNodes_lua(Cranked *cranked, NodeGraph graph, LuaVal nodes) {
    for (int i = 1;; i++) {
        auto node = nodes.getElement(i);
        if (node.val.isNil())
            break;
        graph->nodes.emplace(node.val.asUserdataObject<GraphNode>());
    }
}

static LuaValRet playdate_pathfinder_graph_allNodes_lua(Cranked *cranked, NodeGraph graph) {
    auto table = pushTable(cranked);
    int i = 1;
    for (auto &node : graph->nodes)
        setResourceElement(table, i++, node.get());
    return table;
}

static void playdate_pathfinder_graph_removeNode_lua(Cranked *cranked, NodeGraph graph, GraphNode node) {
    node->removeAllConnections();
    eraseByEquivalentKey(graph->nodes, node);
}

static LuaRet playdate_pathfinder_graph_removeNodeWithXY_lua(Cranked *cranked, NodeGraph graph, int x, int y) {
    if (auto node = graph->findByXY(x, y)) {
        node->removeAllConnections();
        pushResource(cranked, node);
        eraseByEquivalentKey(graph->nodes, node);
    } else
        pushNil(cranked);
    return 1;
}

static LuaRet playdate_pathfinder_graph_removeNodeWithID_lua(Cranked *cranked, NodeGraph graph, LuaVal id) {
    if (auto node = graph->findById(id)) {
        node->removeAllConnections();
        pushResource(cranked, node);
        eraseByEquivalentKey(graph->nodes, node);
    } else
        pushNil(cranked);
    return 1;
}

static LuaRet playdate_pathfinder_graph_nodeWithID_lua(Cranked *cranked, NodeGraph graph, LuaVal id) {
    if (auto node = graph->findById(id))
        pushResource(cranked, node);
    else
        pushNil(cranked);
    return 1;
}

static LuaRet playdate_pathfinder_graph_nodeWithXY_lua(Cranked *cranked, NodeGraph graph, int x, int y) {
    if (auto node = graph->findByXY(x, y))
        pushResource(cranked, node);
    else
        pushNil(cranked);
    return 1;
}

static void playdate_pathfinder_graph_addConnections_lua(Cranked *cranked, NodeGraph graph, LuaVal connections) {
    iterateTable(connections, [graph](LuaVal nodeIdVal, LuaVal value) {
        int nodeId = nodeIdVal.asInt();
        auto node = graph->findById(nodeId);
        if (not node or not value.isTable())
            return false;
        for (int i = 1;; i += 2) {
            auto idVal = value.getElement(i);
            auto weightVal = value.getElement(i + 1);
            if (idVal.val.isNil() or weightVal.val.isNil())
                break;
            int id = idVal.val.asInt();
            int weight = weightVal.val.asInt();
            if (auto other = graph->findById(id))
                node->addConnection(other, weight);
        }
        return false;
    });
}

static void playdate_pathfinder_graph_addConnectionToNodeWithID_lua(Cranked *cranked, NodeGraph graph, int fromId, int toId, int weight, bool addReciprocalConnections) {
    auto to = graph->findById(toId);
    auto from = graph->findById(fromId);
    if (not to or not from)
        return;
    from->addConnection(to, weight);
    if (addReciprocalConnections)
        to->addConnection(from, weight);
}

static void playdate_pathfinder_graph_removeAllConnections_lua(Cranked *cranked, NodeGraph graph) {
    for (auto &node : graph->nodes)
        node->removeConnections();
}

static void playdate_pathfinder_graph_removeAllConnectionsFromNodeWithID_lua(Cranked *cranked, NodeGraph graph, int id, bool removeIncoming) {
    if (auto node = graph->findById(id)) {
        node->removeConnections();
        if (removeIncoming)
            node->removeIncomingConnections();
    }
}

static LuaRet findPath(NodeGraph graph, GraphNode startNode, GraphNode goalNode, LuaVal heuristic, bool findPathToGoalAdjacentNodes, bool getIds = false) {
    // Todo: Support Lua heuristic, how to implement findPathToGoalAdjacentNodes? What are rules for adjacent nodes? Closest with same X or Y in each dir?
    auto cranked = &graph->cranked;
    if (!startNode or !goalNode) {
        pushNil(cranked);
        return 1;
    }
    auto path = graph->pathfind(startNode, goalNode, [&](GraphNode a, GraphNode b) {
        return abs(a->x - b->x) + abs(a->y - b->y); // Manhatten distance
    });
    if (not path.empty()) {
        auto table = pushTable(cranked);
        for (int i = 0; i < path.size(); i++) {
            if (getIds)
                table.setIntElement(i + 1, path[i]->id);
            else
                setResourceElement(table, i + 1, path[i]);
        }
    } else
        pushNil(cranked);
    return 1;
}

static LuaRet playdate_pathfinder_graph_findPath_lua(Cranked *cranked, NodeGraph graph, GraphNode startNode, GraphNode goalNode, LuaVal heuristic, bool findPathToGoalAdjacentNodes) {
    return findPath(graph, startNode, goalNode, heuristic, findPathToGoalAdjacentNodes);
}

static LuaRet playdate_pathfinder_graph_findPathWithIDs_lua(Cranked *cranked, NodeGraph graph, int startID, int goalID, LuaVal heuristic, bool findPathToGoalAdjacentNodes) {
    auto startNode = graph->findById(startID);
    auto goalNode = graph->findById(goalID);
    if (not startNode or not goalNode) {
        pushNil(cranked);
        return 1;
    }
    return findPath(graph, startNode, goalNode, heuristic, findPathToGoalAdjacentNodes, true);
}

static void playdate_pathfinder_graph_setXYForNodeWithID_lua(Cranked *cranked, NodeGraph graph, int id, int x, int y) {
    if (auto node = graph->findById(id)) {
        node->x = x;
        node->y = y;
    }
}

static void playdate_pathfinder_node_gc_lua(Cranked *cranked, GraphNode node) {
    node->dereference();
}

static LuaRet playdate_pathfinder_node_index_lua(Cranked *cranked, GraphNode node, LuaVal key) {
    if (cranked->luaEngine.indexMetatable())
        return 1;
    if (key.isString()) {
        string name(key.asString());
        if (name == "id")
            pushInt(cranked, node->id);
        else if (name == "x")
            pushInt(cranked, node->x);
        else if (name == "y")
            pushInt(cranked, node->y);
        else
            pushNil(cranked);
    } else
        pushNil(cranked);
    return 1;
}

static void playdate_pathfinder_node_newindex_lua(Cranked *cranked, GraphNode node, LuaVal key, LuaVal value) {
    if (!key.isString())
        return;
    string name(key.asString());
    int val = value.asInt();
    if (name == "id")
        node->id = val;
    else if (name == "x")
        node->x = val;
    else if (name == "y")
        node->y = val;
}

static void playdate_pathfinder_node_addConnection_lua(Cranked *cranked, GraphNode node, GraphNode other, int weight, bool addReciprocalConnection) {
    node->addConnection(other, weight);
    if (addReciprocalConnection)
        other->addConnection(node, weight);
}

static void playdate_pathfinder_node_addConnections_lua(Cranked *cranked, GraphNode node, LuaVal others, LuaVal weights, bool addReciprocalConnection) {
    for (int i = 1;; i++) {
        auto other = others.getElement(i);
        auto weight = weights.getElement(i);
        if (other.val.isNil() or weight.val.isNil())
            break;
        auto otherNode = other.val.asUserdataObject<GraphNode>();
        auto weightVal = weight.val.asInt();
        node->addConnection(otherNode, weightVal);
        if (addReciprocalConnection)
            otherNode->addConnection(node, weightVal);
    }
}

static void playdate_pathfinder_node_addConnectionToNodeWithXY_lua(Cranked *cranked, GraphNode node, int x, int y, int weight, bool addReciprocalConnection) {
    node->assertValidGraph();
    auto found = node->graph->findByXY(x, y);
    if (not found)
        return;
    node->addConnection(found, weight);
    if (addReciprocalConnection)
        found->addConnection(node, weight);
}

static LuaValRet playdate_pathfinder_node_connectedNodes_lua(Cranked *cranked, GraphNode node) {
    auto table = pushTable(cranked);
    int i = 1;
    for (auto pair : node->getConnections()) {
        LuaValGuard other(pushResource(cranked, pair.first));
        table.setElement(i++, other.val);
    }
    return table;
}

static void playdate_pathfinder_node_removeConnection_lua(Cranked *cranked, GraphNode node, GraphNode other, bool removeReciprocal) {
    node->removeConnection(other);
    if (removeReciprocal)
        other->removeConnection(node);
}

static void playdate_pathfinder_node_removeAllConnections_lua(Cranked *cranked, GraphNode node, bool removeIncoming) {
    node->removeConnections();
    if (removeIncoming)
        node->removeIncomingConnections();
}

static void playdate_pathfinder_node_setXY_lua(Cranked *cranked, GraphNode node, int x, int y) {
    node->x = x;
    node->y = y;
}

static int playdate_sound_getSampleRate_lua(Cranked *cranked) {
    return 44100;
}

static LuaValRet playdate_sound_playingSources_lua(Cranked *cranked) {
    auto table = pushTable(cranked);
    // Todo
    return table;
}

static void playdate_sound_addEffect_lua(Cranked *cranked, SoundEffect effectDerived) {
    cranked->audio.mainChannel->effects.emplace(cranked->audio.effectFromDerived(effectDerived));
}

static void playdate_sound_removeEffect_lua(Cranked *cranked, SoundEffect effectDerived) {
    eraseByEquivalentKey(cranked->audio.mainChannel->effects, cranked->audio.effectFromDerived(effectDerived));
}

static bool playdate_sound_getHeadphoneState_lua(Cranked *cranked, LuaVal changeCallback) {
    lua_pushvalue(cranked->getLuaContext(), changeCallback);
    cranked->luaEngine.setQualifiedLuaGlobal("cranked.headphoneCallback");
    return cranked->audio.headphonesConnected;
}

static void playdate_sound_resetTime_lua(Cranked *cranked) {
    cranked->audio.sampleTime = 0;
}

static SamplePlayer playdate_sound_sampleplayer_new_lua(Cranked *cranked, LuaVal sample) {
    return cranked->audio.allocateSource<SamplePlayer_32>();
}

static void playdate_sound_sampleplayer_gc_lua(Cranked *cranked, SamplePlayer player) {
    player->dereference();
}

static SamplePlayer playdate_sound_sampleplayer_copy_lua(Cranked *cranked, SamplePlayer player) {
    return player->copy();
}

static void playdate_sound_sampleplayer_play_lua(Cranked *cranked, SamplePlayer player, int repeatCount, LuaVal rate) {
    // Todo
}

static bool playdate_sound_sampleplayer_playAt_lua(Cranked *cranked, SamplePlayer player, int when, LuaVal vol, LuaVal rightVol, LuaVal rate) {
    // Todo
    return false;
}

static void playdate_sound_sampleplayer_setVolume_lua(Cranked *cranked, SamplePlayer player, float vol, LuaVal rightVol) {
    // Todo
}

static LuaRet playdate_sound_sampleplayer_getVolume_lua(Cranked *cranked, SamplePlayer player) {
    // Todo
    return 0;
}

static void playdate_sound_sampleplayer_setLoopCallback_lua(Cranked *cranked, SamplePlayer player, LuaVal callback, LuaVal arg) {
    // Todo
}

static void playdate_sound_sampleplayer_setFinishCallback_lua(Cranked *cranked, SamplePlayer player, LuaVal callback, LuaVal arg) {
    // Todo
}

static AudioSample playdate_sound_sampleplayer_getSample_lua(Cranked *cranked, SamplePlayer player) {
    return player->sample;
}

static void playdate_sound_sampleplayer_setRateMod_lua(Cranked *cranked, SamplePlayer player, SynthSignalValue *signalDerived) {
    player->rateModulator = cranked->audio.signalFromDerived(signalDerived);
}

static FilePlayer playdate_sound_fileplayer_new_lua(Cranked *cranked, LuaVal arg1, LuaVal arg2) {
    // Todo
    return cranked->audio.allocateSource<FilePlayer_32>();
}

static void playdate_sound_fileplayer_gc_lua(Cranked *cranked, FilePlayer player) {
    player->dereference();
}

static void playdate_sound_fileplayer_load_lua(Cranked *cranked, FilePlayer player, const char *path) {
    // Todo
}

static void playdate_sound_fileplayer_play_lua(Cranked *cranked, FilePlayer player, int repeatCount) {
    // Todo
}

static void playdate_sound_fileplayer_setFinishCallback_lua(Cranked *cranked, FilePlayer player, LuaVal callback, LuaVal arg) {
    // Todo
}

static void playdate_sound_fileplayer_setLoopRange_lua(Cranked *cranked, FilePlayer player, float start, LuaVal end, LuaVal loopCallback, LuaVal arg) {
    // Todo
}

static void playdate_sound_fileplayer_setLoopCallback_lua(Cranked *cranked, FilePlayer player, LuaVal callback, LuaVal arg) {
    // Todo
}

static bool playdate_sound_fileplayer_setBufferSize_lua(Cranked *cranked, FilePlayer player, float seconds) {
    // Todo
    return false;
}

static void playdate_sound_fileplayer_setRateMod_lua(Cranked *cranked, FilePlayer player, SynthSignalValue *signalDerived) {
    player->rateModulator = cranked->audio.signalFromDerived(signalDerived);
}

static void playdate_sound_fileplayer_setVolume_lua(Cranked *cranked, FilePlayer player, float left, LuaVal right, LuaVal fadeSeconds, LuaVal fadeCallback, LuaVal arg) {
    // Todo
}

static LuaRet playdate_sound_fileplayer_getVolume_lua(Cranked *cranked, FilePlayer player) {
    // Todo
    return 0;
}

static LuaRet playdate_sound_sample_new_lua(Cranked *cranked, LuaVal arg1, LuaVal arg2) {
    // Todo: Support preallocated size for consistency
    AudioSample sample;
    if (arg1.isString()) {
        try {
            sample = cranked->audio.loadSample(arg1.asString());
        } catch (exception &ex) {
            return returnValues(cranked, nullptr, ex.what());
        }
    } else
        sample = cranked->heap.construct<AudioSample_32>(*cranked, 0);
    return returnValues(cranked, sample);
}

static void playdate_sound_sample_gc_lua(Cranked *cranked, AudioSample sample) {
    sample->dereference();
}

static AudioSample playdate_sound_sample_getSubsample_lua(Cranked *cranked, AudioSample sample, int start, int end) {
    // Todo
    return nullptr;
}

static LuaRet playdate_sound_sample_load_lua(Cranked *cranked, AudioSample sample, const char *path) {
    try {
        AudioSampleRef loaded = cranked->audio.loadSample(path);
        sample->data = loaded->data;
        sample->soundFormat = loaded->soundFormat;
        sample->sampleRate = loaded->sampleRate;
        return returnValues(cranked, true); // Todo: Correct return value?
    } catch (exception &ex) {
        return returnValues(cranked, nullptr, ex.what());
    }
}

static uint32 playdate_sound_sample_getSampleRate_lua(Cranked *cranked, AudioSample sample) {
    return sample->sampleRate;
}

static SoundFormat playdate_sound_sample_getFormat_lua(Cranked *cranked, AudioSample sample) {
    return sample->soundFormat;
}

static LuaRet playdate_sound_sample_getLength_lua(Cranked *cranked, AudioSample sample) {
    int length = (int)sample->data.size();
    return returnValues(cranked, length, length);
}

static SamplePlayer playdate_sound_sample_play_lua(Cranked *cranked, AudioSample sample, int repeatCount, LuaVal rate) {
    auto player = cranked->audio.allocateSource<SamplePlayer_32>();
    player->sample = sample;
    playdate_sound_sampleplayer_play_lua(cranked, player, repeatCount, rate);
    return player;
}

static SamplePlayer playdate_sound_sample_playAt_lua(Cranked *cranked, AudioSample sample, int when, LuaVal vol, LuaVal rightVol, LuaVal rate) {
    auto player = cranked->audio.allocateSource<SamplePlayer_32>();
    player->sample = sample;
    playdate_sound_sampleplayer_playAt_lua(cranked, player, when, vol, rightVol, rate);
    return player;
}

static void playdate_sound_sample_save_lua(Cranked *cranked, AudioSample sample, const char *path) {
    // Todo
}

static SoundChannel playdate_sound_channel_new_lua(Cranked *cranked) {
    return cranked->audio.allocateChannel();
}

static void playdate_sound_channel_gc_lua(Cranked *cranked, SoundChannel channel) {
    channel->dereference();
}

static void playdate_sound_channel_remove_lua(Cranked *cranked, SoundChannel channel) {
    eraseByEquivalentKey(cranked->audio.channels, channel);
}

static Synth playdate_sound_synth_new_lua(Cranked *cranked) {
    return cranked->audio.allocateSource<PDSynth_32>();
}

static void playdate_sound_synth_gc_lua(Cranked *cranked, Synth synth) {
    synth->dereference();
}

static Synth playdate_sound_synth_copy_lua(Cranked *cranked, Synth synth) {
    // Todo
    return nullptr;
}

static void playdate_sound_synth_playNote_lua(Cranked *cranked, Synth synth, float pitch, LuaVal volume, LuaVal length, LuaVal when) {
    // Todo
}

static void playdate_sound_synth_playMIDINote_lua(Cranked *cranked, Synth synth, float note, LuaVal volume, LuaVal length, LuaVal when) {
    // Todo
}

static void playdate_sound_synth_setADSR_lua(Cranked *cranked, Synth synth, float attack, float decay, float sustain, float release) {
    // Todo
}

static void playdate_sound_synth_setEnvelopeCurvature_lua(Cranked *cranked, Synth synth, float amount) {
    // Todo
}

static SynthEnvelope playdate_sound_synth_getEnvelope_lua(Cranked *cranked, Synth synth) {
    // Todo
    return nullptr;
}

static void playdate_sound_synth_setFinishCallback_lua(Cranked *cranked, Synth synth, LuaVal callback) {
    // Todo
}

static void playdate_sound_synth_setLegato_lua(Cranked *cranked, Synth synth, bool flag) {
    // Todo
}

static void playdate_sound_synth_setVolume_lua(Cranked *cranked, Synth synth, float left, LuaVal right) {
    // Todo
}

static LuaRet playdate_sound_synth_getVolume_lua(Cranked *cranked, Synth synth) {
    // Todo
    return 0;
}

static void playdate_sound_synth_setWaveform_lua(Cranked *cranked, Synth synth, LuaVal waveform) {
    // Todo
}

static void playdate_sound_synth_setWavetable_lua(Cranked *cranked, Synth synth, AudioSample sample, int sampleSize, int xSize, LuaVal ySize) {
    // Todo
}

static SynthLFO playdate_sound_lfo_new_lua(Cranked *cranked, LFOType type) {
    return cranked->heap.construct<PDSynthLFO_32>(*cranked, type);
}

static void playdate_sound_lfo_gc_lua(Cranked *cranked, SynthLFO lfo) {
    lfo->dereference();
}

static void playdate_sound_lfo_setArpeggio_lua(Cranked *cranked, int note1, LuaVal arg1) {
    // Todo
}

static SynthEnvelope playdate_sound_envelope_new_lua(Cranked *cranked, float attack, float decay, float sustain, float release) {
    return cranked->heap.construct<PDSynthEnvelope_32>(*cranked, attack, decay, sustain, release);
}

static void playdate_sound_envelope_gc_lua(Cranked *cranked, SynthEnvelope envelope) {
    envelope->dereference();
}

static void playdate_sound_envelope_setScale_lua(Cranked *cranked, SynthEnvelope envelope, float scale) {
    envelope->scale = scale;
}

static void playdate_sound_envelope_setOffset_lua(Cranked *cranked, SynthEnvelope envelope, float offset) {
    envelope->offset = offset;
}

static void playdate_sound_envelope_trigger_lua(Cranked *cranked, SynthEnvelope envelope, float velocity, LuaVal length) {
    // Todo
}

static void playdate_sound_envelope_setGlobal_lua(Cranked *cranked, SynthEnvelope envelope, bool flag) {
    // Todo
}

static BitCrusher playdate_sound_effect_bitcrusher_new_lua(Cranked *cranked) {
    return cranked->heap.construct<BitCrusher_32>(*cranked);
}

static void playdate_sound_effect_bitcrusher_gc_lua(Cranked *cranked, BitCrusher bitcrusher) {
    bitcrusher->dereference();
}

static RingModulator playdate_sound_effect_ringmodulator_new_lua(Cranked *cranked) {
    return cranked->heap.construct<RingModulator_32>(*cranked);
}

static void playdate_sound_effect_ringmodulator_gc_lua(Cranked *cranked, RingModulator ringModulator) {
    ringModulator->dereference();
}

static OnePoleFilter playdate_sound_effect_onepolefilter_new_lua(Cranked *cranked) {
    return cranked->heap.construct<OnePoleFilter_32>(*cranked);
}

static void playdate_sound_effect_onepolefilter_gc_lua(Cranked *cranked, OnePoleFilter filter) {
    filter->dereference();
}

static TwoPoleFilter playdate_sound_effect_twopolefilter_new_lua(Cranked *cranked) {
    return cranked->heap.construct<TwoPoleFilter_32>(*cranked);
}

static void playdate_sound_effect_twopolefilter_gc_lua(Cranked *cranked, TwoPoleFilter filter) {
    filter->dereference();
}

static Overdrive playdate_sound_effect_overdrive_new_lua(Cranked *cranked) {
    return cranked->heap.construct<Overdrive_32>(*cranked);
}

static void playdate_sound_effect_overdrive_gc_lua(Cranked *cranked, Overdrive overdrive) {
    overdrive->dereference();
}

static DelayLine playdate_sound_delayline_new_lua(Cranked *cranked, float length) {
    return cranked->heap.construct<DelayLine_32>(*cranked, length, true); // Todo: Stereo flag?
}

static void playdate_sound_delayline_gc_lua(Cranked *cranked, DelayLine delayLine) {
    delayLine->dereference();
}

static DelayLineTap playdate_sound_delayline_addTap_lua(Cranked *cranked, DelayLine delayLine, int delay) {
    return cranked->audio.allocateSource<DelayLineTap_32>(delayLine, delay);
}

static void playdate_sound_delaylinetap_gc_lua(Cranked *cranked, DelayLineTap tap) {
    tap->dereference();
}

static void playdate_sound_delaylinetap_setVolume_lua(Cranked *cranked, DelayLineTap tap) {
    // Todo
}

static void playdate_sound_delaylinetap_getVolume_lua(Cranked *cranked, DelayLineTap tap) {
    // Todo
}

static SoundSequence playdate_sound_sequence_new_lua(Cranked *cranked) {
    return cranked->audio.allocateSource<SoundSequence_32>();
}

static void playdate_sound_sequence_gc_lua(Cranked *cranked, SoundSequence sequence) {
    sequence->dereference();
}

static void playdate_sound_sequence_play_lua(Cranked *cranked, SoundSequence sequence, LuaVal finishCallback) {
    // Todo
}

static void playdate_sound_sequence_goToStep_lua(Cranked *cranked, SoundSequence sequence, int step, bool play) {
    // Todo
    sequence->time = step;
}

static int playdate_sound_sequence_getCurrentStep_lua(Cranked *cranked, SoundSequence sequence) {
    return sequence->time;
}

static void playdate_sound_sequence_setLoops_lua(Cranked *cranked, SoundSequence sequence, int startStep, int endStep, int loopCount) {
    sequence->loopStart = startStep;
    sequence->loopEnd = endStep;
    sequence->loops = loopCount;
}

static LuaRet playdate_sound_sequence_addTrack_lua(Cranked *cranked, SoundSequence sequence, SequenceTrack track) {
    // Todo
    return 0;
}

static void playdate_sound_sequence_setTrackAtIndex_lua(Cranked *cranked, SoundSequence sequence, int n, SequenceTrack track) {
    // Todo
}

static SequenceTrack playdate_sound_sequence_getTrackAtIndex_lua(Cranked *cranked, SoundSequence sequence, int n) {
    // Todo
    return nullptr;
}

static SequenceTrack playdate_sound_track_new_lua(Cranked *cranked) {
    return cranked->audio.allocateSource<SequenceTrack_32>();
}

static void playdate_sound_track_gc_lua(Cranked *cranked, SequenceTrack track) {
    track->dereference();
}

static void playdate_sound_track_addNote_lua(Cranked *cranked, SequenceTrack track, LuaVal arg1, LuaVal note, int length, LuaVal velocity) {
    // Todo
}

static void playdate_sound_track_setNotes_lua(Cranked *cranked, SequenceTrack track, LuaVal list) {
    // Todo
}

static LuaValRet playdate_sound_track_getNotes_lua(Cranked *cranked, SequenceTrack track, int step, LuaVal endStep) {
    auto table = pushTable(cranked);
    // Todo
    return table;
}

static int playdate_sound_track_getNotesActive_lua(Cranked *cranked, SequenceTrack track) {
    // Todo
    return 0;
}

static void playdate_sound_track_setInstrument_lua(Cranked *cranked, SequenceTrack track, LuaVal inst) {
    // Todo
}

static void playdate_sound_track_getInstrument_lua(Cranked *cranked, SequenceTrack track) {
    // Todo
}

static void playdate_sound_track_removeNote_lua(Cranked *cranked, SequenceTrack track, int step, LuaVal note) {
    // Todo
}

static void playdate_sound_track_addControlSignal_lua(Cranked *cranked, SequenceTrack track, ControlSignal signal) {
    // Todo
}

static LuaValRet playdate_sound_track_getControlSignals_lua(Cranked *cranked, SequenceTrack track) {
    auto table = pushTable(cranked);
    // Todo
    return table;
}

static SynthInstrument playdate_sound_instrument_new_lua(Cranked *cranked, Synth synth) {
    // Todo
    return cranked->audio.allocateSource<PDSynthInstrument_32>();
}

static void playdate_sound_instrument_gc_lua(Cranked *cranked, SynthInstrument instrument) {
    instrument->dereference();
}

static void playdate_sound_instrument_addVoice_lua(Cranked *cranked, SynthInstrument instrument, Synth voice, LuaVal note, LuaVal noteEnd, int transpose) {
    // Todo
}

static void playdate_sound_instrument_playNote_lua(Cranked *cranked, SynthInstrument instrument, LuaVal frequency, LuaVal vel, int length, float when) {
    // Todo
}

static void playdate_sound_instrument_playMIDINote_lua(Cranked *cranked, SynthInstrument instrument, LuaVal note, LuaVal vel, int length, float when) {
    // Todo
}

static void playdate_sound_instrument_noteOff_lua(Cranked *cranked, SynthInstrument instrument, LuaVal note, float when) {
    // Todo
}

static void playdate_sound_instrument_setVolume_lua(Cranked *cranked, SynthInstrument instrument, LuaVal left, LuaVal right) {
    // Todo
}

static LuaRet playdate_sound_instrument_getVolume_lua(Cranked *cranked, SynthInstrument instrument) {
    // Todo
    return 0;
}

static ControlSignal playdate_sound_controlsignal_new_lua(Cranked *cranked) {
    return cranked->heap.construct<ControlSignal_32>(*cranked);
}

static void playdate_sound_controlsignal_gc_lua(Cranked *cranked, ControlSignal signal) {
    signal->dereference();
}

static void playdate_sound_controlsignal_addEvent_lua(Cranked *cranked, ControlSignal signal, LuaVal arg1, LuaVal value, bool interpolate) {
    // Todo
}

static void playdate_sound_controlsignal_clearEvents_lua(Cranked *cranked, ControlSignal signal) {
    // Todo
}

static int playdate_sound_controlsignal_setControllerType_lua(Cranked *cranked, ControlSignal signal) {
    // Todo
    return 0;
}

static void playdate_sound_controlsignal_getControllerType_lua(Cranked *cranked, ControlSignal signal) {
    // Todo
}

static float playdate_sound_controlsignal_getValue_lua(Cranked *cranked, ControlSignal signal) {
    // Todo
    return 0;
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
        playdate.setInteger("kButtonLeft", (int)PDButtons::Left);
        playdate.setInteger("kButtonRight", (int)PDButtons::Right);
        playdate.setInteger("kButtonUp", (int)PDButtons::Up);
        playdate.setInteger("kButtonDown", (int)PDButtons::Down);
        playdate.setInteger("kButtonB", (int)PDButtons::B);
        playdate.setInteger("kButtonA", (int)PDButtons::A);
        playdate.setInteger("isSimulator", 0); // Totally legit hardware
        playdate.setWrappedFunction<playdate_apiVersion_lua>("apiVersion");
        playdate.setWrappedFunction<playdate_wait_lua>("wait");
        playdate.setWrappedFunction<playdate_stop_lua>("stop");
        playdate.setWrappedFunction<playdate_start_lua>("start");
        playdate.setWrappedFunction<playdate_restart_lua>("restart");
        playdate.setWrappedFunction<playdate_getSystemLanguage_lua>("getSystemLanguage");
        playdate.setWrappedFunction<playdate_sys_getReduceFlashing>("getReduceFlashing");
        playdate.setWrappedFunction<playdate_sys_getFlipped>("getFlipped");
        playdate.setWrappedFunction<playdate_buttonIsPressed_lua>("buttonIsPressed");
        playdate.setWrappedFunction<playdate_buttonJustPressed_lua>("buttonJustPressed");
        playdate.setWrappedFunction<playdate_buttonJustReleased_lua>("buttonJustReleased");
        playdate.setWrappedFunction<playdate_getButtonState_lua>("getButtonState");
        playdate.setWrappedFunction<playdate_setButtonQueueSize>("setButtonQueueSize");
        playdate.setWrappedFunction<playdate_sys_isCrankDocked>("isCrankDocked");
        playdate.setWrappedFunction<playdate_sys_getCrankAngle>("getCrankPosition");
        playdate.setWrappedFunction<playdate_getCrankChange_lua>("getCrankChange");
        playdate.setWrappedFunction<playdate_getCrankTicks>("getCrankTicks");
        playdate.setWrappedFunction<playdate_setCrankSoundsDisabled_lua>("setCrankSoundsDisabled");
        playdate.setWrappedFunction<playdate_readAccelerometer_lua>("readAccelerometer");
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
            menuApi.setSelfIndex();
            menuApi.setString("__name", "playdate.menu");
            menuApi.setWrappedFunction<playdate_menu_addMenuItem_lua>("addMenuItem");
            menuApi.setWrappedFunction<playdate_menu_addCheckmarkMenuItem_lua>("addCheckmarkMenuItem");
            menuApi.setWrappedFunction<playdate_menu_addOptionsMenuItem_lua>("addOptionsMenuItem");
            menuApi.setWrappedFunction<playdate_menu_getMenuItems_lua>("getMenuItems");
            menuApi.setWrappedFunction<playdate_menu_removeMenuItem_lua>("removeMenuItem");
            menuApi.setWrappedFunction<playdate_menu_removeAllMenuItems_lua>("removeAllMenuItems");

            {
                auto menuItem = LuaApiHelper(getContext(), "item");
                menuItem.setSelfIndex();
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
            geometry.setInteger("kUnflipped", (int)GraphicsFlip::Unflipped);
            geometry.setInteger("kFlippedX", (int)GraphicsFlip::FlipX);
            geometry.setInteger("kFlippedY", (int)GraphicsFlip::FlipY);
            geometry.setInteger("kFlippedXY", (int)GraphicsFlip::FlipXY);

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
            gfx.setInteger("kColorBlack", (int)LCDSolidColor::Black);
            gfx.setInteger("kColorWhite", (int)LCDSolidColor::White);
            gfx.setInteger("kColorClear", (int)LCDSolidColor::Clear);
            gfx.setInteger("kColorXOR", (int)LCDSolidColor::XOR);
            gfx.setInteger("kDrawModeCopy", (int)LCDBitmapDrawMode::Copy);
            gfx.setInteger("kDrawModeWhiteTransparent", (int)LCDBitmapDrawMode::WhiteTransparent);
            gfx.setInteger("kDrawModeBlackTransparent", (int)LCDBitmapDrawMode::BlackTransparent);
            gfx.setInteger("kDrawModeFillWhite", (int)LCDBitmapDrawMode::FillWhite);
            gfx.setInteger("kDrawModeFillBlack", (int)LCDBitmapDrawMode::FillBlack);
            gfx.setInteger("kDrawModeXOR", (int)LCDBitmapDrawMode::XOR);
            gfx.setInteger("kDrawModeNXOR", (int)LCDBitmapDrawMode::NXOR);
            gfx.setInteger("kDrawModeInverted", (int)LCDBitmapDrawMode::Inverted);
            gfx.setInteger("kImageUnflipped", (int)LCDBitmapFlip::Unflipped);
            gfx.setInteger("kImageFlippedX", (int)LCDBitmapFlip::FlipX);
            gfx.setInteger("kImageFlippedY", (int)LCDBitmapFlip::FlipY);
            gfx.setInteger("kImageFlippedXY", (int)LCDBitmapFlip::FlipXY);
            gfx.setInteger("kLineCapStyleButt", (int)LCDLineCapStyle::Butt);
            gfx.setInteger("kLineCapStyleSquare", (int)LCDLineCapStyle::Square);
            gfx.setInteger("kLineCapStyleRound", (int)LCDLineCapStyle::Round);
            gfx.setInteger("kPolygonFillNonZero", (int)LCDPolygonFillRule::NonZero);
            gfx.setInteger("kPolygonFillEvenOdd", (int)LCDPolygonFillRule::EvenOdd);
            gfx.setInteger("kStrokeCentered", (int)StrokeLocation::Centered);
            gfx.setInteger("kStrokeInside", (int)StrokeLocation::Inside);
            gfx.setInteger("kStrokeOutside", (int)StrokeLocation::Outside);
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
            gfx.setWrappedFunction<playdate_graphics_setDrawMode_lua>("setImageDrawMode");
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
                image.setString("__name", "playdate.graphics.image");
                image.setInteger("kDitherTypeNone", (int)DitherType::None);
                image.setInteger("kDitherTypeDiagonalLine", (int)DitherType::DiagonalLine);
                image.setInteger("kDitherTypeVerticalLine", (int)DitherType::VerticalLine);
                image.setInteger("kDitherTypeHorizontalLine", (int)DitherType::HorizontalLine);
                image.setInteger("kDitherTypeScreen", (int)DitherType::Screen);
                image.setInteger("kDitherTypeBayer2x2", (int)DitherType::Bayer2x2);
                image.setInteger("kDitherTypeBayer4x4", (int)DitherType::Bayer4x4);
                image.setInteger("kDitherTypeBayer8x8", (int)DitherType::Bayer8x8);
                image.setInteger("kDitherTypeFloydSteinberg", (int)DitherType::FloydSteinberg);
                image.setInteger("kDitherTypeBurkes", (int)DitherType::Burkes);
                image.setInteger("kDitherTypeAtkinson", (int)DitherType::Atkinson);
                image.setWrappedFunction<playdate_graphics_image_gc_lua>("__gc");
                image.setWrappedFunction<playdate_graphics_image_index_lua>("__index");
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
                font.setInteger("kLanguageEnglish", (int)PDLanguage::English);
                font.setInteger("kLanguageJapanese", (int)PDLanguage::Japanese);
                font.setInteger("kVariantNormal", (int)PDFontVariant::Normal);
                font.setInteger("kVariantBold", (int)PDFontVariant::Bold);
                font.setInteger("kVariantItalic", (int)PDFontVariant::Italic);
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
                auto sprite = LuaApiHelper(getContext(), "sprite"); // Can't have __index, since it is overwritten by sprites.lua // Todo: Could actually work fine with the pre-loading mechanism
                sprite.setString("__name", "playdate.graphics.sprite");
                sprite.setInteger("kCollisionTypeSlide", (int)Bump::ResponseType::Slide);
                sprite.setInteger("kCollisionTypeFreeze", (int)Bump::ResponseType::Freeze);
                sprite.setInteger("kCollisionTypeOverlap", (int)Bump::ResponseType::Overlap);
                sprite.setInteger("kCollisionTypeBounce", (int)Bump::ResponseType::Bounce);
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
                video.setWrappedFunction<playdate_video_getContext>("getContext");
                video.setWrappedFunction<playdate_video_useScreenContext>("useScreenContext");
                video.setWrappedFunction<playdate_video_renderFrame>("renderFrame");
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
            sound.setInteger("kFilterLowPass", (int)TwoPoleFilterType::LowPass);
            sound.setInteger("kFilterHighPass", (int)TwoPoleFilterType::HighPass);
            sound.setInteger("kFilterBandPass", (int)TwoPoleFilterType::BandPass);
            sound.setInteger("kFilterNotch", (int)TwoPoleFilterType::Notch);
            sound.setInteger("kFilterPEQ", (int)TwoPoleFilterType::PEQ);
            sound.setInteger("kFilterLowShelf", (int)TwoPoleFilterType::LowShelf);
            sound.setInteger("kFilterHighShelf", (int)TwoPoleFilterType::HighShelf);
            sound.setInteger("kFormat8bitMono", (int)SoundFormat::Mono8bit);
            sound.setInteger("kFormat8bitStereo", (int)SoundFormat::Stereo8bit);
            sound.setInteger("kFormat16bitMono", (int)SoundFormat::Mono16bit);
            sound.setInteger("kFormat16bitStereo", (int)SoundFormat::Stereo16bit);
            sound.setInteger("kLFOSquare", (int)LFOType::Square);
            sound.setInteger("kLFOTriangle", (int)LFOType::Triangle);
            sound.setInteger("kLFOSine", (int)LFOType::Sine);
            sound.setInteger("kLFOSampleAndHold", (int)LFOType::SampleAndHold);
            sound.setInteger("kLFOSawtoothUp", (int)LFOType::SawtoothUp);
            sound.setInteger("kLFOSawtoothDown", (int)LFOType::SawtoothDown);
            sound.setInteger("kWaveSquare", (int)SoundWaveform::Square);
            sound.setInteger("kWaveTriangle", (int)SoundWaveform::Triangle);
            sound.setInteger("kWaveSine", (int)SoundWaveform::Sine);
            sound.setInteger("kWaveNoise", (int)SoundWaveform::Noise);
            sound.setInteger("kWaveSawtooth", (int)SoundWaveform::Sawtooth);
            sound.setInteger("kWavePOPhase", (int)SoundWaveform::POPhase);
            sound.setInteger("kWavePODigital", (int)SoundWaveform::PODigital);
            sound.setInteger("kWavePOVosim", (int)SoundWaveform::POVosim);
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
                sample.setWrappedFunction<playdate_sound_sample_decompress>("decompress");
                sample.setWrappedFunction<playdate_sound_sample_getSampleRate_lua>("getSampleRate");
                sample.setWrappedFunction<playdate_sound_sample_getFormat_lua>("getFormat");
                sample.setWrappedFunction<playdate_sound_sample_getLength_lua>("getLength");
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
                channel.setWrappedFunction<playdate_sound_channel_setVolume>("setVolume");
                channel.setWrappedFunction<playdate_sound_channel_getVolume>("getVolume");
                channel.setWrappedFunction<playdate_sound_channel_setPan>("setPan");
                channel.setWrappedFunction<playdate_sound_channel_setPanModulator>("setPanMod");
                channel.setWrappedFunction<playdate_sound_channel_setVolumeModulator>("setVolumeMod");
                channel.setWrappedFunction<playdate_sound_channel_getDryLevelSignal>("getDryLevelSignal");
                channel.setWrappedFunction<playdate_sound_channel_getWetLevelSignal>("getWetLevelSignal");
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
                synth.setWrappedFunction<playdate_sound_synth_clearEnvelope>("clearEnvelope");
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
                synth.setWrappedFunction<playdate_sound_synth_setWavetable_lua>("setWavetable");
                synth.setWrappedFunction<playdate_sound_synth_setParameter>("setParameter");
                synth.setWrappedFunction<playdate_sound_synth_setParameterModulator>("setParameterMod");
            }

            {
                auto signal = LuaApiHelper(getContext(), "signal");
                signal.setSelfIndex();
                signal.setString("__name", "playdate.sound.signal");
                signal.setWrappedFunction<playdate_sound_signal_setValueOffset>("setOffset");
                signal.setWrappedFunction<playdate_sound_signal_setValueScale>("setScale");
                signal.setWrappedFunction<playdate_sound_signal_getValue>("getValue");
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
                lfo.setWrappedFunction<playdate_sound_lfo_setStartPhase>("setStartPhase");
                lfo.setWrappedFunction<playdate_sound_lfo_setGlobal>("setGlobal");
                lfo.setWrappedFunction<playdate_sound_lfo_setRetrigger>("setRetrigger");
                lfo.setWrappedFunction<playdate_sound_lfo_setDelay>("setDelay");
                lfo.setWrappedFunction<playdate_sound_lfo_getValue>("getValue");
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
                envelope.setWrappedFunction<playdate_sound_envelope_getValue>("getValue");
            }

            {
                auto bitCrusher = LuaApiHelper(getContext(), "bitcrusher");
                bitCrusher.setSelfIndex();
                bitCrusher.setString("__name", "playdate.sound.bitcrusher");
                bitCrusher.setWrappedFunction<playdate_sound_effect_bitcrusher_gc_lua>("__gc");
                bitCrusher.setWrappedFunction<playdate_sound_effect_bitcrusher_new_lua>("new");
                bitCrusher.setWrappedFunction<playdate_sound_effect_setMix>("setMix");
                bitCrusher.setWrappedFunction<playdate_sound_effect_setMixModulator>("setMixMod");
                bitCrusher.setWrappedFunction<playdate_sound_effect_bitcrusher_setAmount>("setAmount");
                bitCrusher.setWrappedFunction<playdate_sound_effect_bitcrusher_setAmountModulator>("setAmountMod");
                bitCrusher.setWrappedFunction<playdate_sound_effect_bitcrusher_setUndersampling>("setUndersampling");
                bitCrusher.setWrappedFunction<playdate_sound_effect_bitcrusher_setUndersampleModulator>("setUndersamplingMod");
            }

            {
                auto ringMod = LuaApiHelper(getContext(), "ringmod");
                ringMod.setSelfIndex();
                ringMod.setString("__name", "playdate.sound.ringmod");
                ringMod.setWrappedFunction<playdate_sound_effect_ringmodulator_gc_lua>("__gc");
                ringMod.setWrappedFunction<playdate_sound_effect_ringmodulator_new_lua>("new");
                ringMod.setWrappedFunction<playdate_sound_effect_setMix>("setMix");
                ringMod.setWrappedFunction<playdate_sound_effect_setMixModulator>("setMixMod");
                ringMod.setWrappedFunction<playdate_sound_effect_ringmodulator_setFrequency>("setFrequency");
                ringMod.setWrappedFunction<playdate_sound_effect_ringmodulator_setFrequencyModulator>("setFrequencyMod");
            }

            {
                auto onePoleFilter = LuaApiHelper(getContext(), "onepolefilter");
                onePoleFilter.setSelfIndex();
                onePoleFilter.setString("__name", "playdate.sound.onepolefilter");
                onePoleFilter.setWrappedFunction<playdate_sound_effect_onepolefilter_gc_lua>("__gc");
                onePoleFilter.setWrappedFunction<playdate_sound_effect_onepolefilter_new_lua>("new");
                onePoleFilter.setWrappedFunction<playdate_sound_effect_setMix>("setMix");
                onePoleFilter.setWrappedFunction<playdate_sound_effect_setMixModulator>("setMixMod");
                onePoleFilter.setWrappedFunction<playdate_sound_effect_onepolefilter_setParameter>("setParameter");
                onePoleFilter.setWrappedFunction<playdate_sound_effect_onepolefilter_setParameterModulator>("setParameterMod");
            }

            {
                auto twoPoleFilter = LuaApiHelper(getContext(), "twopolefilter");
                twoPoleFilter.setSelfIndex();
                twoPoleFilter.setString("__name", "playdate.sound.twopolefilter");
                twoPoleFilter.setWrappedFunction<playdate_sound_effect_twopolefilter_gc_lua>("__gc");
                twoPoleFilter.setWrappedFunction<playdate_sound_effect_twopolefilter_new_lua>("new");
                twoPoleFilter.setWrappedFunction<playdate_sound_effect_setMix>("setMix");
                twoPoleFilter.setWrappedFunction<playdate_sound_effect_setMixModulator>("setMixMod");
                twoPoleFilter.setWrappedFunction<playdate_sound_effect_twopolefilter_setFrequency>("setFrequency");
                twoPoleFilter.setWrappedFunction<playdate_sound_effect_twopolefilter_setFrequencyModulator>("setFrequencyMod");
                twoPoleFilter.setWrappedFunction<playdate_sound_effect_twopolefilter_setResonance>("setResonance");
                twoPoleFilter.setWrappedFunction<playdate_sound_effect_twopolefilter_setResonanceModulator>("setResonanceMod");
                twoPoleFilter.setWrappedFunction<playdate_sound_effect_twopolefilter_setGain>("setGain");
                twoPoleFilter.setWrappedFunction<playdate_sound_effect_twopolefilter_setType>("setType");
            }

            {
                auto overdrive = LuaApiHelper(getContext(), "overdrive");
                overdrive.setSelfIndex();
                overdrive.setString("__name", "playdate.sound.overdrive");
                overdrive.setWrappedFunction<playdate_sound_effect_overdrive_gc_lua>("__gc");
                overdrive.setWrappedFunction<playdate_sound_effect_overdrive_new_lua>("new");
                overdrive.setWrappedFunction<playdate_sound_effect_setMix>("setMix");
                overdrive.setWrappedFunction<playdate_sound_effect_setMixModulator>("setMixMod");
                overdrive.setWrappedFunction<playdate_sound_effect_overdrive_setGain>("setGain");
                overdrive.setWrappedFunction<playdate_sound_effect_overdrive_setLimit>("setLimit");
                overdrive.setWrappedFunction<playdate_sound_effect_overdrive_setLimitModulator>("setLimitMod");
                overdrive.setWrappedFunction<playdate_sound_effect_overdrive_setOffset>("setOffset");
                overdrive.setWrappedFunction<playdate_sound_effect_overdrive_setOffsetModulator>("setOffsetMod");
            }

            {
                auto delayLine = LuaApiHelper(getContext(), "delayline");
                delayLine.setSelfIndex();
                delayLine.setString("__name", "playdate.sound.delayline");
                delayLine.setWrappedFunction<playdate_sound_delayline_gc_lua>("__gc");
                delayLine.setWrappedFunction<playdate_sound_delayline_new_lua>("new");
                delayLine.setWrappedFunction<playdate_sound_effect_setMix>("setMix");
                delayLine.setWrappedFunction<playdate_sound_effect_setMixModulator>("setMixMod");
                delayLine.setWrappedFunction<playdate_sound_delayline_addTap_lua>("addTap");
                delayLine.setWrappedFunction<playdate_sound_effect_delayline_setFeedback>("setFeedback");
            }

            {
                auto delayLineTap = LuaApiHelper(getContext(), "delaylinetap");
                delayLineTap.setSelfIndex();
                delayLineTap.setString("__name", "playdate.sound.delaylinetap");
                delayLineTap.setWrappedFunction<playdate_sound_delaylinetap_gc_lua>("__gc");
                delayLineTap.setWrappedFunction<playdate_sound_effect_delayline_setTapDelay>("setDelay");
                delayLineTap.setWrappedFunction<playdate_sound_effect_delayline_setTapDelayModulator>("setDelayMod");
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
                instrument.setWrappedFunction<playdate_sound_instrument_setPitchBend>("setPitchBend");
                instrument.setWrappedFunction<playdate_sound_instrument_setPitchBendRange>("setPitchBendRange");
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
                controlSignal.setWrappedFunction<playdate_sound_controlsignal_getValue_lua>("getValue");
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
