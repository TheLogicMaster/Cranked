#pragma once

#include "Cranked.hpp"

namespace cranked {

/**
 * Indicates to the wrapper function that the native function is responsible for the return values
 */
    struct LuaRet {
        int count;

        inline LuaRet(int count) : count(count) {} // NOLINT: Implicit conversion is desired here
    };

/**
 * Represents a value on the Lua stack
 * `index` should be positive to use non-relative indices that don't change as values are pushed onto the stack
 */
    struct LuaVal {
        lua_State *context;
        int index;

        LuaVal(lua_State *context, int index) : context(context), index(index) {}

        inline operator int() const { // NOLINT: Implicit conversion is desired here
            return index;
        }

        inline LuaVal operator+(int offset) const {
            return {context, index + offset};
        }

        inline bool isTable() {
            return lua_istable(context, index);
        }

        inline bool isNil() {
            return lua_isnil(context, index);
        }

        inline bool isString() {
            return lua_isstring(context, index);
        }

        inline bool isInt() {
            return lua_isinteger(context, index);
        }

        inline bool isFloat() {
            return lua_isnumber(context, index);
        }

        inline bool isBool() {
            return lua_isboolean(context, index);
        }

        inline bool isLightUserdata() {
            return lua_islightuserdata(context, index);
        }

        inline bool isUserdataObject() {
            if (!lua_istable(context, index))
                return false;
            lua_getfield(context, index, "userdata");
            bool isUserdata = lua_isuserdata(context, -1);
            lua_pop(context, 1);
            return isUserdata;
        }

        inline bool hasName(const string &name) {
            lua_getfield(context, index, "__name");
            bool match = !lua_isnil(context, -1) and name == lua_tostring(context, -1);
            lua_pop(context, 1);
            return match;
        }

        inline const char *asString() {
            return lua_tostring(context, index);
        }

        inline int asInt() {
            return lua_tointeger(context, index);
        }

        inline float asFloat() {
            return lua_tonumber(context, index);
        }

        inline bool asBool() {
            return lua_toboolean(context, index);
        }

        inline void *asLightUserdata() {
            return lua_touserdata(context, index);
        }

        template<typename T>
        inline T *asUserdataObject() {
            if (!lua_istable(context, index))
                return nullptr;
            lua_getfield(context, index, "userdata");
            auto value = (T *) lua_touserdata(context, -1);
            lua_pop(context, 1);
            return value;
        }

        inline Rect asRect() {
            return {getFloatField("x"), getFloatField("y"), getFloatField("width"), getFloatField("height")};
        }

        inline IntRect asIntRect() {
            return {getIntField("x"), getIntField("y"), getIntField("width"), getIntField("height")};
        }

        inline chrono::system_clock::time_point asTime() {
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

        inline void setStringElement(int n, const char *value) {
            lua_pushstring(context, value);
            lua_seti(context, index, n);
        }

        inline void setIntElement(int n, int value) {
            lua_pushinteger(context, value);
            lua_seti(context, index, n);
        }

        inline void setFloatElement(int n, float value) {
            lua_pushnumber(context, value);
            lua_seti(context, index, n);
        }

        inline void setBoolElement(int n, bool value) {
            lua_pushboolean(context, value);
            lua_seti(context, index, n);
        }

        inline void setLightUserdataElement(int n, void *value) {
            lua_pushlightuserdata(context, value);
            lua_seti(context, index, n);
        }

        inline const char *getStringElement(int n) {
            lua_geti(context, index, n);
            auto value = lua_tostring(context, -1);
            lua_pop(context, 1);
            return value;
        }

        inline int getIntElement(int n) {
            lua_geti(context, index, n);
            auto value = lua_tointeger(context, -1);
            lua_pop(context, 1);
            return value;
        }

        inline float getFloatElement(int n) {
            lua_geti(context, index, n);
            auto value = lua_tonumber(context, -1);
            lua_pop(context, 1);
            return value;
        }

        inline bool getBoolElement(int n) {
            lua_geti(context, index, n);
            auto value = lua_toboolean(context, -1);
            lua_pop(context, 1);
            return value;
        }

        inline void *getLightUserdataElement(int n) {
            lua_geti(context, index, n);
            auto value = lua_touserdata(context, -1);
            lua_pop(context, 1);
            return value;
        }

        template<typename T>
        inline T *getUserdataObjectElement(int n) {
            lua_geti(context, index, n);
            auto value = LuaVal(context, -1).asUserdataObject<T>();
            lua_pop(context, 1);
            return value;
        }

        inline void setStringField(const char *name, const char *value) {
            lua_pushstring(context, value);
            lua_setfield(context, index, name);
        }

        inline void setIntField(const char *name, int value) {
            lua_pushinteger(context, value);
            lua_setfield(context, index, name);
        }

        inline void setFloatField(const char *name, float value) {
            lua_pushnumber(context, value);
            lua_setfield(context, index, name);
        }

        inline void setBoolField(const char *name, bool value) {
            lua_pushboolean(context, value);
            lua_setfield(context, index, name);
        }

        inline void setLightUserdataField(const char *name, void *value) {
            lua_pushlightuserdata(context, value);
            lua_setfield(context, index, name);
        }

        inline const char *getStringField(const char *name) {
            lua_getfield(context, index, name);
            auto value = lua_tostring(context, -1);
            lua_pop(context, 1);
            return value;
        }

        inline int getIntField(const char *name) {
            lua_getfield(context, index, name);
            auto value = lua_tointeger(context, -1);
            lua_pop(context, 1);
            return value;
        }

        inline float getFloatField(const char *name) {
            lua_getfield(context, index, name);
            auto value = lua_tonumber(context, -1);
            lua_pop(context, 1);
            return value;
        }

        inline bool getBoolField(const char *name) {
            lua_getfield(context, index, name);
            auto value = lua_toboolean(context, -1);
            lua_pop(context, 1);
            return value;
        }

        inline void *getLightUserdataField(const char *name) {
            lua_getfield(context, index, name);
            auto value = lua_touserdata(context, -1);
            lua_pop(context, 1);
            return value;
        }

        template<typename T>
        inline T *getUserdataObjectField(const char *name) {
            lua_getfield(context, index, name);
            auto value = LuaVal(context, -1).asUserdataObject<T>();
            lua_pop(context, 1);
            return value;
        }

        /**
         * This pushes a value onto the stack, so it must be popped in reverse order with respect to other retrieved fields
         */
        class LuaValGuard getField(const char *name);

        inline LuaVal getFieldRaw(const char *name) {
            lua_getfield(context, index, name);
            return {context, lua_gettop(context)};
        }
    };

/**
 * Manages a LuaVal lifecycle. Usually manually popping isn't needed, since `LuaValGuard` destructors should be called in reverse construction order
 */
    class LuaValGuard {
    public:
        explicit inline LuaValGuard(LuaVal val) : val(val), popped(false) {}

        inline LuaValGuard() = delete;

        inline LuaValGuard(const LuaValGuard &other) = delete;

        inline LuaValGuard(LuaValGuard &&other) noexcept: val(other.val), popped(other.popped) {
            other.popped = true;
        }

        inline ~LuaValGuard() noexcept(false) {
            if (!popped)
                pop();
        }

        void pop() {
            if (popped)
                throw CrankedError("Double LuaVal pop");
            if (lua_gettop(val.context) != val.index)
                throw CrankedError("Unbalanced LuaVal pop");
            lua_pop(val.context, 1);
        }

        LuaVal val;
        bool popped;
    };

    inline LuaValGuard LuaVal::getField(const char *name) {
        return LuaValGuard(getFieldRaw(name));
    }

    template<typename T>
    struct LuaResource {

        ResourceRef<T> resource;
        // Todo: Lua component by preserving Lua table object
    };

    template<class T, int I>
    struct LuaWrapperArgsHelper {
    };

    struct LuaArgBuffer {
        uint32 data[4];
    };
    static_assert(sizeof(LuaVal) <= sizeof(LuaArgBuffer)); // Ensure LuaVal fits in values array

    template<class T, typename... R, int I>
    struct LuaWrapperArgsHelper<tuple<T, R...>, I> {
        inline static void get(lua_State *luaContext, ffi_type **types, LuaArgBuffer *values) {
            LuaWrapperArgsHelper<tuple<T>, I>::get(luaContext, types, values);
            LuaWrapperArgsHelper<tuple<R...>, I + 1>::get(luaContext, types, values);
        }
    };

    template<class T, int I>
    struct LuaWrapperArgsHelper<tuple<T>, I> {
        inline static void get(lua_State *luaContext, ffi_type **types, LuaArgBuffer *values) {
            if constexpr (is_same_v<T, Cranked *> or is_same_v<T, lua_State *>) // Forward declaration does not work here
                return; // Just ignore Cranked or lua_State context arg in signature, since index is offset by 1 accordingly
            else if constexpr (is_same_v<T, LuaVal>) {
                static ffi_type luaArgType;
                if (!luaArgType.elements) {
                    static ffi_type *luaArgTypeElements[]{&ffi_type_pointer, &ffi_type_sint, nullptr};
                    luaArgType.type = FFI_TYPE_STRUCT;
                    luaArgType.elements = luaArgTypeElements;
                }
                types[I] = &luaArgType;
                *(LuaVal *) values[I].data = {luaContext, I};
            } else if constexpr (is_pointer_v<T> and
                                 is_class_v<remove_pointer_t<T>>) { // Treat all struct(class) pointers as unwrapped userdata objects
                types[I] = &ffi_type_pointer;
                lua_getfield(luaContext, I, "userdata");
                *(void **) &values[I] = lua_touserdata(luaContext, -1);
                lua_pop(luaContext, 1);
            } else if constexpr (is_same_v<T, uint8 *> or is_same_v<T, const char *>) {
                types[I] = &ffi_type_pointer;
                auto string = lua_tostring(luaContext, I);
                *(const char **) &values[I] = string;
            } else if constexpr (is_same_v<T, bool>) {
                types[I] = &ffi_type_uint8;
                *(uint8 *) &values[I] = lua_toboolean(luaContext, I);
            } else if constexpr (is_same_v<T, float>) {
                types[I] = &ffi_type_float;
                *(float *) &values[I] = lua_tonumber(luaContext, I);
            } else if constexpr (is_same_v<T, uint32>) {
                types[I] = &ffi_type_uint32;
                *(uint32 *) &values[I] = lua_tointeger(luaContext, I);
            } else if constexpr (is_same_v<T, int32> or is_enum_v<T>) {
                types[I] = &ffi_type_sint32;
                *(int32 *) &values[I] = lua_tointeger(luaContext, I);
            } else if constexpr (is_same_v<T, int16>) {
                types[I] = &ffi_type_sint16;
                *(int16 *) &values[I] = lua_tointeger(luaContext, I);
            } else if constexpr (is_same_v<T, uint8>) {
                types[I] = &ffi_type_uint8;
                *(uint8 *) &values[I] = lua_tointeger(luaContext, I);
            } else if constexpr (is_same_v<T, int>) {
                types[I] = &ffi_type_sint;
                *(int *) &values[I] = lua_tointeger(luaContext, I);
            } else
                static_assert(dependent_false<T>::value, "Invalid Lua wrapper function arg");
        }
    };

/**
 * This wraps a native function and marshals the arguments from the Lua stack and back (Only supports single return values for conversion)
 * Use `LuaArg` for parameters like tables that aren't convertable or where more control is needed
 * Use `LuaRet` for return types where the function is in charge of pushing return values onto the Lua stack
 * Returned std::string* values are deleted but cstrings are left alone
 * Just don't think too hard about the massive amount of resulting specializations
 */
    template<auto F>
    inline int luaNativeWrapper(lua_State *luaContext) {
        using Helper = FunctionTypeHelper<decltype(F)>;
        using ReturnType = typename Helper::RetType;

        constexpr int argCount = Helper::ArgCount;
        constexpr bool takesContext = argCount != 0 and
                (is_same_v<typename Helper::template NthArg<0>, Cranked *> or is_same_v<typename Helper::template NthArg<0>, lua_State *>);
        constexpr int luaArgCount = takesContext ? Helper::ArgCount - 1 : argCount;

        vector<ffi_type *> ffiArgTypes(argCount);
        vector<void *> ffiArgs(argCount);

        vector<LuaArgBuffer> argValues(argCount);

        for (int i = 0; i < argCount; i++)
            ffiArgs[i] = argValues[i].data;

        // Set first arg as Cranked or lua_State if needed
        auto cranked = Cranked::fromLuaContext(luaContext);
        if constexpr (takesContext) {
            ffiArgs[0] = is_same_v<typename Helper::template NthArg<0>, Cranked *> ? (void *) &cranked : (void *) &luaContext;
            ffiArgTypes[0] = &ffi_type_pointer;
        }

        // Read args off of Lua stack
        if constexpr (luaArgCount > 0)
            LuaWrapperArgsHelper<typename Helper::ArgTypes, takesContext ? 0 : 1>::get(luaContext, &ffiArgTypes[0], &argValues[0]);

        // Determine FFI return type
        ffi_type *ffiReturnType;
        if constexpr (is_same_v<ReturnType, LuaRet>) {
            static ffi_type luaRetType;
            if (!luaRetType.elements) {
                static ffi_type *luaRetTypeElements[]{&ffi_type_sint, nullptr};
                luaRetType.type = FFI_TYPE_STRUCT;
                luaRetType.elements = luaRetTypeElements;
            }
            ffiReturnType = &luaRetType;
        } else if constexpr (is_same_v<ReturnType, int32> or is_enum_v<ReturnType>)
            ffiReturnType = &ffi_type_sint32; // NOLINT: Repeated branch when `int` is same as `int32`
        else if constexpr (is_same_v<ReturnType, uint32>)
            ffiReturnType = &ffi_type_uint32;
        else if constexpr (is_same_v<ReturnType, int16>)
            ffiReturnType = &ffi_type_sint16;
        else if constexpr (is_same_v<ReturnType, uint8> or is_same_v<ReturnType, bool>)
            ffiReturnType = &ffi_type_uint8;
        else if constexpr (is_same_v<ReturnType, int>)
            ffiReturnType = &ffi_type_sint;
        else if constexpr (is_same_v<ReturnType, float>)
            ffiReturnType = &ffi_type_float;
        else if constexpr (is_same_v<ReturnType, const char *> or is_same_v<ReturnType, string *>)
            ffiReturnType = &ffi_type_pointer;
        else if constexpr (is_same_v<ReturnType, void>)
            ffiReturnType = &ffi_type_void;
        else
            static_assert(dependent_false<ReturnType>{}, "Invalid Lua wrapper function return type");

        // Call function
        uint64 returnValue{};
        ffi_cif cif;
        static_assert(sizeof(LuaRet) <= sizeof(returnValue)); // Ensure LuaRet fits in returnValue
        if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, argCount, ffiReturnType, ffiArgTypes.data()) != FFI_OK)
            throw CrankedError("Failed to prep FFI CIF in Lua wrapper");
        try {
            ffi_call(&cif, (void (*)()) F, &returnValue, ffiArgs.data());
        } catch (exception &ex) {
            return luaL_error(luaContext, ex.what());
        }

        // Push result
        if constexpr (is_same_v<ReturnType, LuaRet>)
            return ((LuaRet *) &returnValue)->count;
        else if constexpr (is_same_v<ReturnType, int>)
            lua_pushinteger(luaContext, *(int *) &returnValue);
        else if constexpr (is_same_v<ReturnType, int32> or is_enum_v<ReturnType>)
            lua_pushinteger(luaContext, (int) *(int32 *) &returnValue);
        else if constexpr (is_same_v<ReturnType, uint32>)
            lua_pushinteger(luaContext, (int) *(uint32 *) &returnValue);
        else if constexpr (is_same_v<ReturnType, int16>)
            lua_pushinteger(luaContext, (int) *(int16 *) &returnValue);
        else if constexpr (is_same_v<ReturnType, uint8>)
            lua_pushinteger(luaContext, (int) *(uint8 *) &returnValue);
        else if constexpr (is_same_v<ReturnType, bool>)
            lua_pushboolean(luaContext, *(bool *) &returnValue);
        else if constexpr (is_same_v<ReturnType, const char *>)
            lua_pushstring(luaContext, *(const char **) &returnValue);
        else if constexpr (is_same_v<ReturnType, float>)
            lua_pushnumber(luaContext, *(float *) &returnValue);
        else if constexpr (is_same_v<ReturnType, string *>) {
            auto str = *(string **) &returnValue;
            lua_pushstring(luaContext, str->c_str());
            delete str;
        }

        return is_void_v<ReturnType> ? 0 : 1;
    }

/**
 * Sets a named field on the table at the top of the stack to the specified native function
 */
    inline void setLuaFunction(lua_State *luaContext, const char *name, lua_CFunction func) {
        lua_pushcfunction(luaContext, func);
        lua_setfield(luaContext, -2, name);
    }

    class LuaApiHelper {
    public:
        lua_State *luaContext;
        const char *tableName;
        bool global;

        inline LuaApiHelper(lua_State *luaContext, const char *tableName, bool global = false) : luaContext(luaContext), tableName(tableName), global(global) {
            lua_newtable(luaContext);
        }

        inline ~LuaApiHelper() {
            if (global)
                lua_setglobal(luaContext, tableName);
            else
                lua_setfield(luaContext, -2, tableName);
        }

        template<auto F>
        inline void setWrappedFunction(const char *name) const {
            lua_pushcfunction(luaContext, luaNativeWrapper<F>);
            lua_setfield(luaContext, -2, name);
        }

        inline void setFunction(const char *name, lua_CFunction func) const {
            setLuaFunction(luaContext, name, func);
        }

        inline void setInteger(const char *name, int value) {
            lua_pushinteger(luaContext, value);
            lua_setfield(luaContext, -2, name);
        }

        inline void setString(const char *name, const char *value) {
            lua_pushstring(luaContext, value);
            lua_setfield(luaContext, -2, name);
        }

        inline void setSelfIndex() {
            lua_pushvalue(luaContext, -1);
            lua_setfield(luaContext, -2, "__index");
        }
    };

    inline LCDBitmapFlip bitmapFlipFromString(const string &name) {
        if (name == "flipX")
            return LCDBitmapFlip::FlippedX;
        else if (name == "flipY")
            return LCDBitmapFlip::FlippedY;
        else if (name == "flipXY")
            return LCDBitmapFlip::FlippedXY;
        else
            return LCDBitmapFlip::Unflipped;
    }

    inline LuaVal pushTable(Cranked *cranked) {
        lua_newtable(cranked->getLuaContext());
        return {cranked->getLuaContext(), lua_gettop(cranked->getLuaContext())};
    }

    inline LuaVal pushUserdataObject(Cranked *cranked, void *value, const char *metatable) {
        auto object = pushTable(cranked);
        object.setLightUserdataField("userdata", value);
        cranked->luaEngine.getQualifiedLuaGlobal(metatable);
        lua_setmetatable(cranked->getLuaContext(), -2);
        return object;
    }

    inline LuaVal pushUserdataResource(NativeResource *resource, const char *metatable) {
        auto object = pushUserdataObject(&resource->cranked, resource, metatable);
        resource->reference();
        return object;
    }

    inline void releaseUserdataResource(LuaVal value) {
        auto resource = (NativeResource *)value.getLightUserdataField("userdata");
        resource->dereference();
    }

    inline LuaVal pushImage(Bitmap image) {
        return pushUserdataResource(image, "playdate.graphics.image");
    }

    inline LuaVal pushSprite(Sprite sprite) {
        return pushUserdataResource(sprite, "playdate.graphics.sprite");
    }

    inline LuaVal pushFile(File file) {
        return pushUserdataResource(file, "playdate.file.file");
    }

    inline LuaVal pushMenuItem(MenuItem item) {
        return pushUserdataResource(item, "playdate.menu.item");
    }

    template<typename T>
    inline LuaVal pushRect(Cranked *cranked, const Rectangle<T> &rect) {
        LuaVal val = pushTable(cranked);
        cranked->luaEngine.getQualifiedLuaGlobal("playdate.geometry.rect");
        lua_setmetatable(cranked->getLuaContext(), -2);
        auto floatRect = rect.template as<float>();
        val.setFloatField("x", floatRect.pos.x);
        val.setFloatField("y", floatRect.pos.y);
        val.setFloatField("width", floatRect.size.x);
        val.setFloatField("height", floatRect.size.y);
        return val;
    }

    inline LuaVal pushTime(Cranked *cranked, const chrono::system_clock::time_point &time) {
        LuaVal table = pushTable(cranked);
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

}
