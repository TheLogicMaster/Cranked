#pragma once

#include "Cranked.hpp"

namespace cranked {

    struct Point { // Dedicated type just to differentiate Lua vector and point args
        template<numeric_type T>
        Point(Vector2<T> vec) : vec(vec) {} // NOLINT(*-explicit-constructor)

        Vec2 vec;
    };

    inline LuaVal pushTable(Cranked *cranked) {
        return cranked->luaEngine.pushTable();
    }

    inline LuaVal pushNil(Cranked *cranked) {
        return cranked->luaEngine.pushNil();
    }

    inline LuaVal pushBool(Cranked *cranked, bool value) {
        return cranked->luaEngine.pushBool(value);
    }

    inline LuaVal pushString(Cranked *cranked, const char *str) {
        return cranked->luaEngine.pushString(str);
    }

    inline LuaVal pushInt(Cranked *cranked, int value) {
        return cranked->luaEngine.pushInt(value);
    }

    inline LuaVal pushFloat(Cranked *cranked, float value) {
        return cranked->luaEngine.pushFloat(value);
    }

    inline LuaVal pushUserdataObject(Cranked *cranked, void *value, const char *metatable) {
        return cranked->luaEngine.pushUserdataObject(value, metatable);
    }

    template<numeric_type T>
    LuaVal pushRect(Cranked *cranked, Rectangle<T> rect) {
        return cranked->luaEngine.pushRect(rect);
    }

    template<numeric_type T>
    LuaVal pushVec(Cranked *cranked, Vector2<T> vec) {
        return cranked->luaEngine.pushVec(vec);
    }

    template<numeric_type T>
    LuaVal pushPoint(Cranked *cranked, Vector2<T> point) {
        return cranked->luaEngine.pushPoint(point);
    }

    inline LuaVal pushTransform(Cranked *cranked, const Transform &transform) {
        return cranked->luaEngine.pushTransform(transform);
    }

    inline LuaVal pushResource(Cranked *cranked, NativeResource *resource) {
        return cranked->luaEngine.pushResource(resource);
    }

    inline LuaVal pushTime(Cranked *cranked, const chrono::system_clock::time_point &time) {
        return cranked->luaEngine.pushTime(time);
    }

    template<is_resource_ptr R>
    void setResourceElement(LuaVal val, int n, R resource) {
        return Cranked::fromLuaContext(val.context)->luaEngine.setResourceElement(val, n, resource);
    }

    template<is_resource_ptr R>
    void setResourceField(LuaVal val, const char *name, R resource) {
        return Cranked::fromLuaContext(val.context)->luaEngine.setResourceField(val, name, resource);
    }

    template<numeric_type T>
    void setVecElement(LuaVal val, int n, Vector2<T> vec) {
        return Cranked::fromLuaContext(val.context)->luaEngine.setVecElement(val, n, vec);
    }

    template<numeric_type T>
    void setVecField(LuaVal val, const char *name, Vector2<T> vec) {
        return Cranked::fromLuaContext(val.context)->luaEngine.setVecField(val, name, vec);
    }

    template<numeric_type T>
    void setPointElement(LuaVal val, int n, Vector2<T> point) {
        return Cranked::fromLuaContext(val.context)->luaEngine.setPointElement(val, n, point);
    }

    template<numeric_type T>
    void setPointField(LuaVal val, const char *name, Vector2<T> point) {
        return Cranked::fromLuaContext(val.context)->luaEngine.setPointField(val, name, point);
    }

    template<numeric_type T>
    void setRectElement(LuaVal val, int n, Rectangle<T> rect) {
        return Cranked::fromLuaContext(val.context)->luaEngine.setRectElement(val, n, rect);
    }

    template<numeric_type T>
    void setRectField(LuaVal val, const char *name, Rectangle<T> rect) {
        return Cranked::fromLuaContext(val.context)->luaEngine.setRectField(val, name, rect);
    }

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

        template<numeric_type T>
        LuaValRet(Rectangle<T> rect) : LuaValRet(pushRect(rect)) {} // NOLINT(*-explicit-constructor)

        template<numeric_type T>
        LuaValRet(Vector2<T> vector) : LuaValRet(pushPoint(vector)) {} // NOLINT(*-explicit-constructor)
    };

    template<typename T, int I>
    auto getLuaNativeWrapperArg(lua_State *context) {
        if constexpr (is_same_v<T, LuaVal>)
            return LuaVal{context, I};
        else if constexpr (is_type_listed<T, GraphicsFlip, BitmapDrawMode, FontVariant, PDLanguage, PDButtons, Bump::ResponseType>)
            return LuaVal{context, I}.asNamedEnum<T>();
        else if constexpr (is_type_listed<T, int, uint8, int8, uint16, int16, uint32, int32> or is_enum_v<T>) {
            if (lua_isboolean(context, I))
                return (T)lua_toboolean(context, I);
            return (T)lua_tonumber(context, I);
        } else if constexpr (is_same_v<T, float>)
            return lua_tonumber(context, I);
        else if constexpr (is_same_v<T, bool>)
            return (bool)lua_toboolean(context, I);
        else if constexpr (is_same_v<T, Vec2>)
            return LuaVal{context, I}.asVec();
        else if constexpr (is_same_v<T, Rect>)
            return LuaVal{context, I}.asRect();
        else if constexpr (is_same_v<T, IntRect>)
            return LuaVal{context, I}.asIntRect();
        else if constexpr (is_same_v<T, Point>)
            return Point(LuaVal{context, I}.asPoint());
        else if constexpr (is_same_v<T, Transform>)
            return Transform(LuaVal{context, I}.asTransform());
        else if constexpr (is_same_v<T, uint8 *> or is_same_v<T, const char *>)
            return (T)lua_tostring(context, I);
        else if constexpr (is_pointer_v<T>) {
            if (lua_isnoneornil(context, I))
                return (T) nullptr;
            lua_getfield(context, I, "userdata");
            auto value = lua_touserdata(context, -1);
            lua_pop(context, 1);
            return (T) value;
        } else
            static_assert(dependent_false_v<T>, "Invalid Lua wrapper function arg");
    }

    template<typename T>
    int luaNativeWrapperReturn(lua_State *context, T val) {
        if constexpr (is_same_v<T, LuaRet>)
            return val.count;
        else if constexpr (is_type_listed<T, int, uint8, int8, uint16, int16, uint32, int32> or is_enum_v<T>)
            lua_pushinteger(context, (int)val);
        else if constexpr (is_same_v<T, bool>)
            lua_pushboolean(context, val);
        else if constexpr (is_same_v<T, float>)
            lua_pushnumber(context, val);
        else if constexpr (is_type_listed<T, const char *, uint8 *>)
            lua_pushstring(context, val);
        else if constexpr (is_resource_ptr<T>)
            pushResource(Cranked::fromLuaContext(context), val);
        else if constexpr (is_same_v<T, Vec2>)
            pushVec(Cranked::fromLuaContext(context), val);
        else if constexpr (is_type_listed<T, IntRect, Rect>)
            pushRect(Cranked::fromLuaContext(context), val);
        else if constexpr (is_same_v<T, Point>)
            pushPoint(Cranked::fromLuaContext(context), val.vec);
        else if constexpr (is_same_v<T, Transform>)
            pushTransform(Cranked::fromLuaContext(context), val);
        else if constexpr (is_same_v<T, string *>) {
            lua_pushstring(context, val->c_str());
            delete val;
        } else if constexpr (is_same_v<T, LuaValRet>) {
            if (val.position != lua_gettop(context))
                lua_pushvalue(context, val.position);
        } else
            static_assert(dependent_false_v<T>, "Invalid Lua wrapper return type");
        return 1;
    }

    template<auto F, typename T, typename I>
    struct LuaNativeWrapperHelper {};

    template<auto F, typename...T, int ...I>
    struct LuaNativeWrapperHelper<F, tuple<T...>, integer_sequence<int, I...>> {
        static auto call(lua_State *luaContext) {
            return F(getLuaNativeWrapperArg<T, I + 1>(luaContext) ...);
        }
    };

    template<auto F, typename...T, int ...I>
    struct LuaNativeWrapperHelper<F, tuple<Cranked *, T...>, integer_sequence<int, I...>> {
        static auto call(lua_State *luaContext) {
            auto context = Cranked::fromLuaContext(luaContext);
            return F(context, getLuaNativeWrapperArg<T, I + 1>(luaContext) ...);
        }
    };

    template<auto F, typename...T, int ...I>
    struct LuaNativeWrapperHelper<F, tuple<lua_State *, T...>, integer_sequence<int, I...>> {
        static auto call(lua_State *luaContext) {
            return F(luaContext, getLuaNativeWrapperArg<T, I + 1>(luaContext) ...);
        }
    };

    /**
     * This wraps a native function and marshals the arguments from the Lua stack and back.
     * Use `LuaVal` for parameters like tables that aren't convertable or where more control is needed
     * Use `LuaRet` for return types where the function is in charge of pushing return values onto the Lua stack
     * Use `LuaValRet` for return types where a LuaVal value should be returned
     * Returned c++ `string*` values are deleted but c-strings are left alone
     */
    template<auto Func>
    int luaNativeWrapper(lua_State *luaContext) {
        // Todo: Could this instead work by having a Lua function which immediately yields and goes back to the Lua entry point?
        using F = FunctionTypeHelper<decltype(Func)>;
        constexpr int ArgCount = F::ArgCount != 0 and is_type_listed<typename F::template NthArg<0>, Cranked *, lua_State *> ? F::ArgCount - 1 : F::ArgCount;
        using Wrapper = LuaNativeWrapperHelper<Func, typename F::ArgTypes, make_integer_sequence<int, ArgCount>>;
        if constexpr (is_same_v<typename F::RetType, void>) {
            Wrapper::call(luaContext);
            return 0;
        } else
            return luaNativeWrapperReturn(luaContext, Wrapper::call(luaContext));
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

    template<typename ...Args>
    LuaRet returnValues(Cranked *cranked, Args ...args) {
        int count = 0;
        ([&] {
            const auto arg = args;
            using A = remove_const_t<decltype(arg)>;
            count++;
            if constexpr (is_same_v<A, float>)
                cranked->luaEngine.pushFloat(arg);
            else if constexpr (is_same_v<A, int> or is_same_v<A, int32> or is_same_v<A, uint32> or is_same_v<A, int16> or is_same_v<A, uint16> or is_same_v<A, int8> or is_same_v<A, uint8>)
                cranked->luaEngine.pushInt(arg);
            else if constexpr (is_same_v<A, bool>)
                cranked->luaEngine.pushBool(arg);
            else if constexpr (is_same_v<A, const char *>)
                cranked->luaEngine.pushString(arg);
            else if constexpr (is_same_v<A, string>)
                cranked->luaEngine.pushString(arg.c_str());
            else if constexpr (is_same_v<A, nullptr_t>)
                (void)cranked->luaEngine.pushNil();
            else if constexpr (is_resource_ptr<A>)
                cranked->luaEngine.pushResource(arg);
            else if constexpr (is_same_v<A, IntRect> or is_same_v<A, Rect>)
                cranked->luaEngine.pushRect(arg);
            else if constexpr (is_same_v<A, IntVec2> or is_same_v<A, Vec2>)
                cranked->luaEngine.pushVec(arg);
            else if constexpr (is_same_v<A, Transform>)
                cranked->luaEngine.pushTransform(arg);
            else
                static_assert(dependent_false_v<A>, "Invalid return value");
        }()
#ifndef __CLION_IDE__ // Todo: Remove yet another CLion workaround when it is fixed
        , ...
#endif
        );
        return count;
    }

    inline LCDBitmapFlip bitmapFlipFromString(const string &name) {
        if (name == "flipX")
            return LCDBitmapFlip::FlipX;
        if (name == "flipY")
            return LCDBitmapFlip::FlipY;
        if (name == "flipXY")
            return LCDBitmapFlip::FlipXY;
        return LCDBitmapFlip::Unflipped;
    }

    template<numeric_type T>
    LuaRet returnRectValues(Cranked *cranked, Rectangle<T> rect) {
        cranked->luaEngine.pushFloat(rect.pos.template as<float>().x);
        cranked->luaEngine.pushFloat(rect.pos.template as<float>().y);
        cranked->luaEngine.pushFloat(rect.size.template as<float>().x);
        cranked->luaEngine.pushFloat(rect.size.template as<float>().y);
        return 4;
    }

    template<numeric_type T>
    LuaRet returnVecValues(Cranked *cranked, Vector2<T> point) {
        cranked->luaEngine.pushFloat(point.template as<float>().x);
        cranked->luaEngine.pushFloat(point.template as<float>().y);
        return 2;
    }

    inline IntRect intRectFromArgs(LuaVal firstArg) {
        return { firstArg.asInt(), (firstArg + 1).asInt(), (firstArg + 2).asInt(), (firstArg + 3).asInt() };
    }

    inline Rect rectFromArgs(LuaVal firstArg) {
        return { firstArg.asFloat(), (firstArg + 1).asFloat(), (firstArg + 2).asFloat(), (firstArg + 3).asFloat() };
    }

    inline void iterateTable(LuaVal table, const function<bool(LuaVal, LuaVal)> &func) {
        Cranked::fromLuaContext(table.context)->luaEngine.iterateTable(table, func);
    }

    inline void iterateArray(LuaVal array, const function<bool(LuaVal)> &func) {
        Cranked::fromLuaContext(array.context)->luaEngine.iterateArray(array, func);
    }

}
