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
     * Indicates to the wrapper function where the return value is on the Lua stack
     */
    struct LuaValRet {
        int position;

        LuaValRet(LuaVal val) : position(val.index) {} // NOLINT(*-explicit-constructor)
    };

    template<class T, int I>
    struct LuaWrapperArgsHelper {};

    struct LuaArgBuffer {
        uint32 data[4];
    };
    static_assert(sizeof(LuaVal) <= sizeof(LuaArgBuffer)); // Ensure LuaVal fits in values array

    template<class T, typename... R, int I>
    struct LuaWrapperArgsHelper<tuple<T, R...>, I> {
        static void get(lua_State *luaContext, ffi_type **types, LuaArgBuffer *values) {
            LuaWrapperArgsHelper<tuple<T>, I>::get(luaContext, types, values);
            LuaWrapperArgsHelper<tuple<R...>, I + 1>::get(luaContext, types, values);
        }
    };

    template<class T, int I>
    struct LuaWrapperArgsHelper<tuple<T>, I> {
        static void get(lua_State *luaContext, ffi_type **types, LuaArgBuffer *values) {
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
            } else if constexpr (is_same_v<T, PDLanguage>) {
                *(PDLanguage *) values[I].data = LuaVal{luaContext, I}.asLanguage();
                types[I] = &ffi_type_sint;
            } else if constexpr (is_same_v<T, PDFontVariant>) {
                *(PDFontVariant *) values[I].data = LuaVal{luaContext, I}.asFontVariant();
                types[I] = &ffi_type_sint;
            } else if constexpr (is_same_v<T, GraphicsFlip>) {
                *(GraphicsFlip *) values[I].data = LuaVal{luaContext, I}.asFlip();
                types[I] = &ffi_type_sint;
            } else if constexpr (is_pointer_v<T> and is_class_v<remove_pointer_t<T>>) { // Treat all struct(class) pointers as unwrapped userdata objects
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
            } else if constexpr (is_same_v<T, int32>) {
                types[I] = &ffi_type_sint32;
                *(int32 *) &values[I] = lua_tointeger(luaContext, I);
            } else if constexpr (is_same_v<T, int16>) {
                types[I] = &ffi_type_sint16;
                *(int16 *) &values[I] = lua_tointeger(luaContext, I);
            } else if constexpr (is_same_v<T, uint8>) {
                types[I] = &ffi_type_uint8;
                *(uint8 *) &values[I] = lua_tointeger(luaContext, I);
            } else if constexpr (is_same_v<T, int> or is_enum_v<T>) {
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
    int luaNativeWrapper(lua_State *luaContext) {
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
        ffi_type *ffiReturnType{};
        if constexpr (is_same_v<ReturnType, LuaRet> or is_same_v<ReturnType, LuaValRet>) {
            static ffi_type luaRetType;
            if (!luaRetType.elements) {
                static ffi_type *luaRetTypeElements[]{&ffi_type_sint, nullptr};
                luaRetType.type = FFI_TYPE_STRUCT;
                luaRetType.elements = luaRetTypeElements;
            }
            ffiReturnType = &luaRetType;
        } else if constexpr (is_same_v<ReturnType, int32>)
            ffiReturnType = &ffi_type_sint32; // NOLINT(*-branch-clone) Repeated branch when `int` is same as `int32`
        else if constexpr (is_same_v<ReturnType, uint32>)
            ffiReturnType = &ffi_type_uint32;
        else if constexpr (is_same_v<ReturnType, int16>)
            ffiReturnType = &ffi_type_sint16;
        else if constexpr (is_same_v<ReturnType, uint8> or is_same_v<ReturnType, bool>)
            ffiReturnType = &ffi_type_uint8;
        else if constexpr (is_same_v<ReturnType, int> or is_enum_v<ReturnType>)
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
        static_assert(sizeof(LuaRet) <= sizeof(returnValue) and sizeof(LuaValRet) <= sizeof(returnValue)); // Ensure LuaRet and LuaValRet fit in returnValue
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
        else if constexpr (is_same_v<ReturnType, int> or is_enum_v<ReturnType>)
            lua_pushinteger(luaContext, *(int *) &returnValue);
        else if constexpr (is_same_v<ReturnType, int32>)
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
        } else if constexpr (is_same_v<ReturnType, LuaValRet>) {
            int pos = ((LuaValRet *) &returnValue)->position;
            if (pos != lua_gettop(luaContext))
                lua_pushvalue(luaContext, pos);
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

        LuaApiHelper(lua_State *luaContext, const char *tableName, bool global = false) : luaContext(luaContext), tableName(tableName), global(global) {
            lua_newtable(luaContext);
        }

        ~LuaApiHelper() {
            if (global)
                lua_setglobal(luaContext, tableName);
            else
                lua_setfield(luaContext, -2, tableName);
        }

        template<auto F>
        void setWrappedFunction(const char *name) const {
            lua_pushcfunction(luaContext, luaNativeWrapper<F>);
            lua_setfield(luaContext, -2, name);
        }

        void setFunction(const char *name, lua_CFunction func) const {
            setLuaFunction(luaContext, name, func);
        }

        void setInteger(const char *name, int value) const {
            lua_pushinteger(luaContext, value);
            lua_setfield(luaContext, -2, name);
        }

        void setString(const char *name, const char *value) const {
            lua_pushstring(luaContext, value);
            lua_setfield(luaContext, -2, name);
        }

        void setSelfIndex() const {
            lua_pushvalue(luaContext, -1);
            lua_setfield(luaContext, -2, "__index");
        }
    };

    inline LCDBitmapFlip bitmapFlipFromString(const string &name) {
        if (name == "flipX")
            return LCDBitmapFlip::FlippedX;
        if (name == "flipY")
            return LCDBitmapFlip::FlippedY;
        if (name == "flipXY")
            return LCDBitmapFlip::FlippedXY;
        return LCDBitmapFlip::Unflipped;
    }

    inline LuaVal pushTable(Cranked *cranked) {
        return cranked->luaEngine.pushTable();
    }

    inline LuaVal pushUserdataObject(Cranked *cranked, void *value, const char *metatable) {
        return cranked->luaEngine.pushUserdataObject(value, metatable);
    }

    inline LuaVal pushUserdataResource(NativeResource *resource, const char *metatable) {
        return resource->cranked.luaEngine.pushUserdataResource(resource, metatable);
    }

    inline LuaVal pushImage(Bitmap image) {
        return image->cranked.luaEngine.pushImage(image);
    }

    inline LuaVal pushSprite(Sprite sprite) {
        return sprite->cranked.luaEngine.pushSprite(sprite);
    }

    inline LuaVal pushFont(Font font) {
        return font->cranked.luaEngine.pushFont(font);
    }

    inline LuaVal pushFile(File file) {
        return file->cranked.luaEngine.pushFile(file);
    }

    inline LuaVal pushMenuItem(MenuItem item) {
        return item->cranked.luaEngine.pushMenuItem(item);
    }

    inline LuaVal pushNil(Cranked *cranked) {
        return cranked->luaEngine.pushNil();
    }

    inline LuaVal pushInt(Cranked *cranked, int value) {
        return cranked->luaEngine.pushInt(value);
    }

    inline LuaVal pushFloat(Cranked *cranked, float value) {
        return cranked->luaEngine.pushFloat(value);
    }

    template<typename T>
    LuaVal pushRect(Cranked *cranked, const Rectangle<T> &rect) {
        return cranked->luaEngine.pushRect(rect);
    }

    inline LuaVal pushTime(Cranked *cranked, const chrono::system_clock::time_point &time) {
        return cranked->luaEngine.pushTime(time);
    }

}
