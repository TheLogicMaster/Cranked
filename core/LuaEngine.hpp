#pragma once

#include "gen/PlaydateAPI.hpp"
#include "Graphics.hpp"
#include "Bump.hpp"

namespace cranked {

    class Cranked;

    // Todo: Ensure all table accesses are raw to avoid meta-method calls (getfield, setfield, geti, seti, getp, setp, gettable, settable)

    // Todo: Disassembler

    class LuaValGuard;

    /**
     * Represents a value on the Lua stack
     * `index` should be positive to use non-relative indices that don't change as values are pushed onto the stack
     */
    struct LuaVal {
        lua_State *context;
        int index;

        LuaVal(lua_State *context, int index) : context(context), index(index) {}

        explicit LuaVal(lua_State *context) : context(context), index(lua_gettop(context)) {}

        operator int() const { // NOLINT(*-explicit-constructor)
            return index;
        }

        LuaVal operator+(int offset) const {
            return {context, index + offset};
        }

        [[nodiscard]] bool isTable() const {
            return lua_istable(context, index);
        }

        [[nodiscard]] bool isNil() const {
            return lua_isnoneornil(context, index);
        }

        [[nodiscard]] bool isString() const {
            return lua_type(context, index) == LUA_TSTRING;
        }

        [[nodiscard]] bool isInt() const {
            return lua_isinteger(context, index);
        }

        [[nodiscard]] bool isFloat() const {
            return !isInt() and lua_type(context, index) == LUA_TNUMBER;
        }

        [[nodiscard]] bool isNumeric() const {
            return lua_type(context, index) == LUA_TNUMBER;
        }

        [[nodiscard]] bool isBool() const {
            return lua_isboolean(context, index);
        }

        [[nodiscard]] bool isLightUserdata() const {
            return lua_islightuserdata(context, index);
        }

        [[nodiscard]] bool isFunction() const {
            return lua_isfunction(context, index); // Doesn't respect `__call`
        }

        [[nodiscard]] bool isUserdataObject() const {
            if (!lua_istable(context, index))
                return false;
            lua_getfield(context, index, "userdata");
            bool isUserdata = lua_isuserdata(context, -1);
            lua_pop(context, 1);
            return isUserdata;
        }

        [[nodiscard]] bool hasName(const string &name) const {
            lua_getfield(context, index, "__name");
            bool match = !lua_isnil(context, -1) and name == lua_tostring(context, -1);
            lua_pop(context, 1);
            return match;
        }

        [[nodiscard]] const char *asString() const {
            return lua_tostring(context, index);
        }

        [[nodiscard]] int asInt() const {
            if (!isInt())
                return (int)lua_tonumber(context, index);
            return lua_tointeger(context, index);
        }

        [[nodiscard]] float asFloat() const {
            return lua_tonumber(context, index);
        }

        [[nodiscard]] bool asBool() const {
            return lua_toboolean(context, index);
        }

        template<typename T> requires is_enum_v<T>
        [[nodiscard]] T asEnum() const {
            return (T)lua_tointeger(context, index);
        }

        [[nodiscard]] void *asLightUserdata() const {
            return lua_touserdata(context, index);
        }

        template<typename T>
        [[nodiscard]] T asUserdataObject() const {
            if (!lua_istable(context, index))
                return nullptr;
            lua_getfield(context, index, "userdata");
            auto value = (T) lua_touserdata(context, -1);
            lua_pop(context, 1);
            return value;
        }

        [[nodiscard]] Rect asRect() const {
            return {getFloatField("x"), getFloatField("y"), getFloatField("width"), getFloatField("height")};
        }

        [[nodiscard]] IntRect asIntRect() const {
            return {getIntField("x"), getIntField("y"), getIntField("width"), getIntField("height")};
        }

        [[nodiscard]] LineSeg asLineSeg() const {
            return {getFloatField("x1"), getFloatField("y1"), getFloatField("x2"), getFloatField("y2")};
        }

        [[nodiscard]] IntLineSeg asIntLineSeg() const {
            return {getIntField("x1"), getIntField("y1"), getIntField("x2"), getIntField("y2")};
        }

        [[nodiscard]] Vec2 asVec() const {
            return {getFloatField("dx"), getFloatField("dy")};
        }

        [[nodiscard]] Vec2 asPoint() const {
            return {getFloatField("x"), getFloatField("y")};
        }

        [[nodiscard]] IntVec2 asIntPoint() const {
            return {getIntField("x"), getIntField("y")};
        }

        [[nodiscard]] Transform asTransform() const {
            return {
                getFloatField("m11"), getFloatField("m12"), getFloatField("tx"),
                getFloatField("m21"), getFloatField("m22"), getFloatField("ty")
            };
        }

        [[nodiscard]] FontFamily asFontFamily() {
            return {
                getUserdataObjectElement<Font>((int)PDFontVariant::Normal),
                getUserdataObjectElement<Font>((int)PDFontVariant::Bold),
                getUserdataObjectElement<Font>((int)PDFontVariant::Italic)
            };
        }

        [[nodiscard]] chrono::system_clock::time_point asTime() const {
            chrono::system_clock::time_point time{};
            time += chrono::years(getIntField("year"));
            time += chrono::months(getIntField("month"));
            time += chrono::days(getIntField("day"));
            time += chrono::hours(getIntField("hour"));
            time += chrono::minutes(getIntField("minute"));
            time += chrono::seconds(getIntField("second"));
            time += chrono::milliseconds(getIntField("millisecond"));
            return time;
        }

        template<typename T>
        [[nodiscard]] T asNamedEnum(T defaultVal = {}) const {
            if (isInt())
                return (T)asInt();
            if (isString())
                return magic_enum::enum_cast<T>(asString(), magic_enum::case_insensitive).value_or(defaultVal);
            return defaultVal;
        }

        [[nodiscard]] GraphicsFlip asFlip() const {
            return asNamedEnum<GraphicsFlip>();
        }

        [[nodiscard]] BitmapDrawMode asBitmapDrawMode() const {
            return asNamedEnum<BitmapDrawMode>();
        }

        [[nodiscard]] FontVariant asFontVariant() const {
            return asNamedEnum<FontVariant>();
        }

        [[nodiscard]] PDLanguage asLanguage() const {
            if (isString()) {
                auto str = stringToLower(asString());
                if (str == "en")
                    return PDLanguage::English;
                if (str == "jp")
                    return PDLanguage::Japanese;
            }
            return asNamedEnum<PDLanguage>();
        }

        [[nodiscard]] PDButtons asButton() const {
            return asNamedEnum<PDButtons>();
        }

        [[nodiscard]] Bump::ResponseType asCollisionResponse() const {
            return asNamedEnum<Bump::ResponseType>();
        }

        void setStringElement(int n, const char *value) const {
            lua_pushstring(context, value);
            lua_seti(context, index, n);
        }

        void setIntElement(int n, int value) const {
            lua_pushinteger(context, value);
            lua_seti(context, index, n);
        }

        void setFloatElement(int n, float value) const {
            lua_pushnumber(context, value);
            lua_seti(context, index, n);
        }

        void setBoolElement(int n, bool value) const {
            lua_pushboolean(context, value);
            lua_seti(context, index, n);
        }

        void setNilElement(int n) const {
            lua_pushnil(context);
            lua_seti(context, index, n);
        }

        void setLightUserdataElement(int n, void *value) const {
            lua_pushlightuserdata(context, value);
            lua_seti(context, index, n);
        }

        void setElement(int n, LuaVal value) const {
            lua_pushvalue(context, value);
            lua_seti(context, index, n);
        }

        [[nodiscard]] const char *getStringElement(int n) const {
            lua_geti(context, index, n);
            auto value = lua_tostring(context, -1);
            lua_pop(context, 1);
            return value;
        }

        [[nodiscard]] int getIntElement(int n) const {
            lua_geti(context, index, n);
            auto value = lua_tointeger(context, -1);
            lua_pop(context, 1);
            return value;
        }

        [[nodiscard]] float getFloatElement(int n) const {
            lua_geti(context, index, n);
            auto value = lua_tonumber(context, -1);
            lua_pop(context, 1);
            return value;
        }

        [[nodiscard]] bool getBoolElement(int n) const {
            lua_geti(context, index, n);
            auto value = lua_toboolean(context, -1);
            lua_pop(context, 1);
            return value;
        }

        [[nodiscard]] void *getLightUserdataElement(int n) const {
            lua_geti(context, index, n);
            auto value = lua_touserdata(context, -1);
            lua_pop(context, 1);
            return value;
        }

        template<typename T>
        T getUserdataObjectElement(int n) {
            lua_geti(context, index, n);
            auto value = LuaVal(context, -1).asUserdataObject<T>();
            lua_pop(context, 1);
            return value;
        }

        void setStringField(const char *name, const char *value) const {
            lua_pushstring(context, value);
            lua_setfield(context, index, name);
        }

        void setIntField(const char *name, int value) const {
            lua_pushinteger(context, value);
            lua_setfield(context, index, name);
        }

        void setFloatField(const char *name, float value) const {
            lua_pushnumber(context, value);
            lua_setfield(context, index, name);
        }

        void setBoolField(const char *name, bool value) const {
            lua_pushboolean(context, value);
            lua_setfield(context, index, name);
        }

        void setLightUserdataField(const char *name, void *value) const {
            lua_pushlightuserdata(context, value);
            lua_setfield(context, index, name);
        }

        void setNilField(const char *name) const {
            lua_pushnil(context);
            lua_setfield(context, index, name);
        }

        [[nodiscard]] const char *getStringField(const char *name) const {
            lua_getfield(context, index, name);
            auto value = lua_tostring(context, -1);
            lua_pop(context, 1);
            return value;
        }

        [[nodiscard]] int getIntField(const char *name) const {
            lua_getfield(context, index, name);
            auto value = lua_tointeger(context, -1);
            lua_pop(context, 1);
            return value;
        }

        [[nodiscard]] float getFloatField(const char *name) const {
            lua_getfield(context, index, name);
            auto value = lua_tonumber(context, -1);
            lua_pop(context, 1);
            return value;
        }

        [[nodiscard]] bool getBoolField(const char *name) const {
            lua_getfield(context, index, name);
            auto value = lua_toboolean(context, -1);
            lua_pop(context, 1);
            return value;
        }

        [[nodiscard]] void *getLightUserdataField(const char *name) const {
            lua_getfield(context, index, name);
            auto value = lua_touserdata(context, -1);
            lua_pop(context, 1);
            return value;
        }

        template<typename T>
        [[nodiscard]] T getUserdataObjectField(const char *name) const {
            lua_getfield(context, index, name);
            auto value = LuaVal(context, -1).asUserdataObject<T>();
            lua_pop(context, 1);
            return value;
        }

        /**
         * This pushes a value onto the stack, so it must be popped in reverse order with respect to other retrieved fields
         */
        [[nodiscard]] LuaValGuard getField(const char *name) const;

        [[nodiscard]] LuaVal getFieldRaw(const char *name) const {
            lua_getfield(context, index, name);
            return LuaVal(context);
        }

        /**
         * This pushes a value onto the stack, so it must be popped in reverse order with respect to other retrieved fields
         */
        [[nodiscard]] LuaValGuard getElement(int n) const;

        [[nodiscard]] LuaVal getElementRaw(int n) const {
            lua_geti(context, index, n);
            return LuaVal(context);
        }

        [[nodiscard]] LuaVal getMetaTable() const {
            lua_getmetatable(context, index);
            return LuaVal(context);
        }
    };

    /**
     * Manages a LuaVal lifecycle. Usually manually popping isn't needed, since `LuaValGuard` destructors should be called in reverse construction order
     */
    class LuaValGuard {
    public:
        explicit LuaValGuard(LuaVal val) : val(val), popped(false) {}

        LuaValGuard() = delete;

        LuaValGuard(const LuaValGuard &other) = delete;

        LuaValGuard(LuaValGuard &&other) noexcept: val(other.val), popped(other.popped) {
            other.popped = true;
        }

        ~LuaValGuard() noexcept(false) {
            if (!popped)
                pop();
        }

        LuaVal *operator->() {
            return &val;
        }

        LuaVal &operator*() {
            return val;
        }

        operator LuaVal() const { // NOLINT(*-explicit-constructor)
            return val;
        }

        void pop() {
            if (popped)
                throw CrankedError("Double LuaVal pop");
            if (lua_gettop(val.context) != val.index)
                throw CrankedError("Unbalanced LuaVal pop");
            lua_pop(val.context, 1);
            popped = true;
        }

        LuaVal val;
        bool popped;
    };

    inline LuaValGuard LuaVal::getField(const char *name) const {
        return LuaValGuard(getFieldRaw(name));
    }

    inline LuaValGuard LuaVal::getElement(int n) const {
        return LuaValGuard(getElementRaw(n));
    }

    class LuaEngine {
    public:
        explicit LuaEngine(Cranked &cranked);
        ~LuaEngine();

        void init();
        void reset();

        void runUpdateLoop();

        lua_State *getContext() const {
            if (!luaInterpreter)
                throw CrankedError("Lua context not available");
            return inLuaUpdate ? luaUpdateThread : luaInterpreter;
        }

        [[nodiscard]] bool isLoaded() const {
            return luaInterpreter != nullptr;
        }

        [[nodiscard]] string getLuaError(int result = LUA_ERRRUN) const {
            switch (result) {
                case -1:
                    return "Native exception";
                case LUA_OK:
                    return "Ok";
                case LUA_YIELD:
                    return "Yield";
                case LUA_ERRRUN: {
                    auto message = lua_tostring(getContext(), -1);
                    return message ? message : "Invalid error message";
                }
                case LUA_ERRSYNTAX:
                    return "Syntax error";
                case LUA_ERRMEM:
                    return "Memory allocation error";
                case LUA_ERRERR:
                    return "Error running message handler";
                case LUA_ERRFILE:
                    return "File error";
                default:
                    return "Unknown";
            }
        }

        static const char *getLuaClass(ResourceType type) {
            switch (type) {
                case ResourceType::Sprite: return "playdate.graphics.sprite";
                case ResourceType::Bitmap: return "playdate.graphics.image";
                case ResourceType::BitmapTable: return "playdate.graphics.imagetable";
                case ResourceType::Font: return "playdate.graphics.font";
                case ResourceType::TileMap: return "playdate.graphics.tilemap";
                case ResourceType::VideoPlayer: return "playdate.graphics.video";
                case ResourceType::Channel: return "playdate.sound.channel";
                case ResourceType::FilePlayer: return "playdate.sound.fileplayer";
                case ResourceType::SamplePlayer: return "playdate.sound.sampleplayer";
                case ResourceType::AudioSample: return "playdate.sound.sample";
                case ResourceType::Synth: return "playdate.sound.synth";
                case ResourceType::Instrument: return "playdate.sound.instrument";
                case ResourceType::Track: return "playdate.sound.track";
                case ResourceType::Sequence: return "playdate.sound.sequence";
                case ResourceType::ControlSignal: return "playdate.sound.controlsignal";
                case ResourceType::LFO: return "playdate.sound.lfo";
                case ResourceType::Envelope: return "playdate.sound.envelope";
                case ResourceType::TwoPoleFilter: return "playdate.sound.twopolefilter";
                case ResourceType::OnePoleFilter: return "playdate.sound.onepolefilter";
                case ResourceType::BitCrusher: return "playdate.sound.bitcrusher";
                case ResourceType::RingModulator: return "playdate.sound.ringmod";
                case ResourceType::Overdrive: return "playdate.sound.overdrive";
                case ResourceType::DelayLine: return "playdate.sound.delayline";
                case ResourceType::DelayLineTap: return "playdate.sound.delaylinetap";
                case ResourceType::File: return "playdate.file.file";
                case ResourceType::MenuItem: return "playdate.menu.item";
                case ResourceType::NodeGraph: return "playdate.pathfinder.graph";
                case ResourceType::GraphNode: return "playdate.pathfinder.node";
                default: return nullptr;
            }
        }

        LuaVal pushTable() const {
            lua_newtable(getContext());
            return LuaVal{getContext()};
        }

        LuaVal pushUserdataObject(void *value, const char *metatable) const {
            auto object = pushTable();
            object.setLightUserdataField("userdata", value);
            if (!getQualifiedLuaGlobal(metatable))
                throw CrankedError("Missing metatable for: " + string(metatable));
            lua_setmetatable(getContext(), -2);
            return object;
        }

        LuaVal pushResource(NativeResource *resource) {
            if (!resource)
                return pushNil();
            if (auto value = getResourceValue(resource); not value.isNil())
                return value;
            lua_pop(getContext(), 1);
            auto value = pushUserdataObject(resource->address, getLuaClass(resource->type));
            resource->reference();
            storeResourceValue(resource, value);
            return value;
        }

        LuaVal pushNil () {
            lua_pushnil(getContext());
            return LuaVal{getContext()};
        }

        LuaVal pushBool(bool value) const {
            lua_pushboolean(getContext(), value);
            return LuaVal{getContext()};
        }

        LuaVal pushString(const char *str) const {
            lua_pushstring(getContext(), str);
            return LuaVal{getContext()};
        }

        LuaVal pushInt(int value) const {
            lua_pushinteger(getContext(), value);
            return LuaVal{getContext()};
        }

        LuaVal pushFloat(float value) {
            lua_pushnumber(getContext(), value);
            return LuaVal{getContext()};
        }

        template<typename T>
        LuaVal pushRect(Rectangle<T> rect) {
            LuaVal val = pushTable();
            if (!getQualifiedLuaGlobal("playdate.geometry.rect"))
                throw CrankedError("Rect table missing");
            lua_setmetatable(getContext(), -2);
            auto floatRect = rect.template as<float>();
            val.setFloatField("x", floatRect.pos.x);
            val.setFloatField("y", floatRect.pos.y);
            val.setFloatField("width", floatRect.size.x);
            val.setFloatField("height", floatRect.size.y);
            return val;
        }

        template<typename T>
        LuaVal pushVec(Vector2<T> point) {
            LuaVal val = pushTable();
            if (!getQualifiedLuaGlobal("playdate.geometry.vector2D"))
                throw CrankedError("Vector table missing");
            lua_setmetatable(getContext(), -2);
            auto floatPoint = point.template as<float>();
            val.setFloatField("dx", floatPoint.x);
            val.setFloatField("dy", floatPoint.y);
            return val;
        }

        template<typename T>
        LuaVal pushPoint(Vector2<T> point) {
            LuaVal val = pushTable();
            if (!getQualifiedLuaGlobal("playdate.geometry.point"))
                throw CrankedError("Point table missing");
            lua_setmetatable(getContext(), -2);
            auto floatPoint = point.template as<float>();
            val.setFloatField("x", floatPoint.x);
            val.setFloatField("y", floatPoint.y);
            return val;
        }

        LuaVal pushTransform(const Transform &transform) {
            LuaVal val = pushTable();
            if (!getQualifiedLuaGlobal("playdate.geometry.transform"))
                throw CrankedError("Transform table missing");
            lua_setmetatable(getContext(), -2);
            val.setFloatField("m11", transform.m11);
            val.setFloatField("m12", transform.m12);
            val.setFloatField("m21", transform.m21);
            val.setFloatField("m22", transform.m22);
            val.setFloatField("tx", transform.tx);
            val.setFloatField("ty", transform.ty);
            return val;
        }

        LuaVal pushTime(const chrono::system_clock::time_point &time) const {
            LuaVal table = pushTable();
            auto ymd = chrono::year_month_day{chrono::floor<chrono::days>(time)};
            auto timeOffset = time - chrono::floor<chrono::days>(time);
            chrono::hh_mm_ss hms{timeOffset};
            table.setIntField("year", (int) ymd.year());
            table.setIntField("month", (int) (uint) ymd.month());
            table.setIntField("day", (int) (uint) ymd.day());
            table.setIntField("hour", (int) hms.hours().count());
            table.setIntField("minute", (int) hms.minutes().count());
            table.setIntField("second", (int) hms.seconds().count());
            table.setIntField("millisecond", (int) duration_cast<chrono::milliseconds>(timeOffset).count());
            return table;
        }

        template<is_resource_ptr R>
        void setResourceElement(LuaVal val, int n, R resource) {
            pushResource(resource);
            lua_seti(val.context, val.index, n);
        }

        template<is_resource_ptr R>
        void setResourceField(LuaVal val, const char *name, R resource) {
            pushResource(resource);
            lua_setfield(val.context, val.index, name);
        }

        template<numeric_type T>
        void setVecElement(LuaVal val, int n, Vector2<T> vec) {
            pushVec(vec);
            lua_seti(val.context, val.index, n);
        }

        template<numeric_type T>
        void setVecField(LuaVal val, const char *name, Vector2<T> vec) {
            pushVec(vec);
            lua_setfield(val.context, val.index, name);
        }

        template<numeric_type T>
        void setPointElement(LuaVal val, int n, Vector2<T> point) {
            pushPoint(point);
            lua_seti(val.context, val.index, n);
        }

        template<numeric_type T>
        void setPointField(LuaVal val, const char *name, Vector2<T> point) {
            pushPoint(point);
            lua_setfield(val.context, val.index, name);
        }

        template<numeric_type T>
        void setRectElement(LuaVal val, int n, Rectangle<T> rect) {
            pushRect(rect);
            lua_seti(val.context, val.index, n);
        }

        template<numeric_type T>
        void setRectField(LuaVal val, const char *name, Rectangle<T> rect) {
            pushRect(rect);
            lua_setfield(val.context, val.index, name);
        }

        [[nodiscard]] bool indexMetatable() const {
            if (!isLoaded())
                return false;
            auto context = getContext();
            if (!lua_getmetatable(context, 1))
                return false;
            lua_pushvalue(context, 2);
            if (!lua_rawget(context, -2)) {
                lua_pop(context, 2);
                return false;
            }
            lua_copy(context, -1, -2);
            lua_pop(context, 1);
            return true;
        }

        void setQualifiedLuaGlobal(const char *name) const {
            auto nameStr = string(name);
            size_t pos;
            auto luaContext = getContext();
            lua_getglobal(luaContext, "_G");
            while ((pos = nameStr.find('.')) != string::npos) {
                auto currentName = nameStr.substr(0, pos);
                lua_getfield(luaContext, -1, currentName.c_str());
                if (lua_isnil(luaContext, -1)) {
                    lua_pop(luaContext, 1);
                    lua_createtable(luaContext, 0, 0);
                    lua_pushvalue(luaContext, -1);
                    lua_setfield(luaContext, -3, currentName.c_str());
                }
                lua_copy(luaContext, -1, -2);
                lua_pop(luaContext, 1);
                nameStr.erase(0, pos + 1);
            }
            lua_pushvalue(luaContext, -2);
            lua_setfield(luaContext, -2, nameStr.c_str());
            lua_pop(luaContext, 2);
        }

        bool getQualifiedLuaGlobal(const char *name, bool createMissing = false) const { // Todo: Return LuaVal?
            auto nameStr = string(name);
            size_t pos;
            auto luaContext = getContext();
            lua_getglobal(luaContext, "_G");
            string currentName;
            auto getField = [&](bool last) {
                lua_getfield(luaContext, -1, currentName.c_str());
                auto missing = lua_isnil(luaContext, -1) and !last;
                if (missing and createMissing) {
                    lua_pop(luaContext, 1);
                    lua_createtable(luaContext, 0, 0);
                    lua_pushvalue(luaContext, -1);
                    lua_setfield(luaContext, -3, currentName.c_str());
                }
                lua_copy(luaContext, -1, -2);
                lua_pop(luaContext, 1);
                return !missing or createMissing;
            };
            while ((pos = nameStr.find('.')) != string::npos) {
                currentName = nameStr.substr(0, pos);
                if (!getField(false))
                    return false;
                nameStr.erase(0, pos + 1);
            }
            currentName = nameStr;
            return getField(true);
        }

        /**
         * Iterates over the key-value pairs in a given table. The stack must remain balanced after func calls.
         * Return `true` to break out of iteration.
         */
        void iterateTable(LuaVal table, const function<bool(LuaVal, LuaVal)> &func) const {
            if (!table.isTable())
                return;
            auto context = getContext();
            lua_pushvalue(context, table.index);
            while (lua_next(context, -2)) {
                lua_pushvalue(context, -2);
                if (func(LuaVal{context, -1}, LuaVal{context, -2}))
                    break;
                lua_pop(context, 2);
            }
            lua_pop(context, 1);
        }

        void iterateArray(LuaVal array, const function<bool(LuaVal)> &func) const {
            if (!array.isTable())
                return;
            for (int i = 1;; i++) {
                auto val = array.getElement(i);
                if (val.val.isNil() or func(val.val))
                    break;
            }
        }

        void invokeLuaCallback(const string &name) const {
            if (!isLoaded())
                return;
            auto start = lua_gettop(getContext());
            lua_getglobal(getContext(), "playdate");
            lua_getfield(getContext(), -1, name.c_str());
            if (!lua_isnil(getContext(), -1))
                lua_call(getContext(), 0, 0); // Todo: Protected?
            lua_settop(getContext(), start);
        }

        bool invokeLuaInputCallback(const string &name, const vector<float> &args = {}) {
            if (!isLoaded())
                return false;
            auto start = lua_gettop(getContext());
            lua_getglobal(getContext(), "cranked");
            lua_getfield(getContext(), -1, "dispatchInputEvent");
            lua_pushstring(getContext(), name.c_str());
            for (float arg: args)
                lua_pushnumber(getContext(), arg);
            lua_call(getContext(), args.size() + 1, 1); // Todo: Protected?
            bool handled = lua_toboolean(getContext(), -1);
            lua_settop(getContext(), start);
            return handled;
        }

        /**
         * Store a Lua value weakly by light userdata pointer
         */
        void storeResourceValue(void *ptr, LuaVal val) const {
            if (!getQualifiedLuaGlobal("cranked.resources"))
                throw CrankedError("Preserved table missing");
            auto context = getContext();
            lua_pushlightuserdata(context, ptr);
            lua_pushvalue(context, val);
            lua_rawset(context, -3);
            lua_pop(context, 1);
        }

        LuaVal getResourceValue(void *ptr) const {
            if (!getQualifiedLuaGlobal("cranked.resources"))
                throw CrankedError("Preserved table missing");
            auto context = getContext();
            lua_pushlightuserdata(context, ptr);
            lua_rawget(context, -2);
            lua_copy(context, -1, -2);
            lua_pop(context, 1);
            return LuaVal(context);
        }

        void clearResourceValue(void *ptr) const {
            if (!isLoaded())
                return;
            if (!getQualifiedLuaGlobal("cranked.resources"))
                throw CrankedError("Preserved table missing");
            auto context = getContext();
            lua_pushlightuserdata(context, ptr);
            lua_pushnil(context);
            lua_rawset(context, -3);
            lua_pop(context, 1);
        }

        /**
         * Preserves a given Lua value in a reference counted global table
         */
        void preserveLuaValue(int index) const {
            if (!isLoaded())
                return;
            if (!getQualifiedLuaGlobal("cranked.preserved"))
                throw CrankedError("Preserved table missing");
            lua_pushvalue(getContext(), index);
            lua_rawget(getContext(), -2);
            int count = lua_tointeger(getContext(), -1);
            lua_pop(getContext(), 1);
            lua_pushvalue(getContext(), index);
            lua_pushinteger(getContext(), count + 1);
            lua_rawset(getContext(), -3);
            lua_pop(getContext(), 1);
        }

        void releaseLuaValue(int index) const {
            if (!isLoaded())
                return;
            if (!getQualifiedLuaGlobal("cranked.preserved"))
                throw CrankedError("Preserved table missing");
            lua_pushvalue(getContext(), index);
            lua_rawget(getContext(), -2);
            int count = lua_tointeger(getContext(), -1);
            lua_pop(getContext(), 1);
            lua_pushvalue(getContext(), index);
            if (count <= 1)
                lua_pushnil(getContext());
            else
                lua_pushinteger(getContext(), count - 1);
            lua_rawset(getContext(), -3);
            lua_pop(getContext(), 1);
        }

        unordered_set<string> loadedLuaFiles; // Todo: Should be made private

    private:
        static void *luaAllocator(void *ud, void *ptr, size_t osize, size_t nsize);

        static void luaHook(lua_State *luaState, lua_Debug *luaDebug);

        void registerLuaGlobals();

        Cranked &cranked;
        lua_State *luaInterpreter{};
        lua_State *luaUpdateThread{};
        bool inLuaUpdate{};
    };

}
