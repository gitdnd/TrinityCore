/*
* Copyright (C) 2010 - 2016 Eluna Lua Engine <http://emudevs.com/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef _ELUNA_TEMPLATE_H
#define _ELUNA_TEMPLATE_H

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
};
#include "LuaEngine.h"
#include "ElunaUtility.h"
#include "SharedDefines.h"
#include "ElunaCompat.h"


class ElunaGlobal
{
public:
    struct ElunaRegister
    {
        const char* name;
        int(*mfunc)(Eluna*);
    };

    static int thunk(lua_State* L)
    {
        ElunaRegister* l = static_cast<ElunaRegister*>(lua_touserdata(L, lua_upvalueindex(1)));
        Eluna* E = static_cast<Eluna*>(lua_touserdata(L, lua_upvalueindex(2)));
        int args = lua_gettop(L);
        int expected = l->mfunc(E);
        args = lua_gettop(L) - args;
        if (args < 0 || args > expected) // Assert instead?
        {
            ELUNA_LOG_ERROR("[Eluna]: %s returned unexpected amount of arguments %i out of %i. Report to devs", l->name, args, expected);
        }
        for (; args < expected; ++args)
            lua_pushnil(L);
        return expected;
    }

    static void SetMethods(Eluna* E, ElunaRegister* methodTable)
    {
        if (!methodTable)
            return;

        lua_pushglobaltable(E->L);

        for (; methodTable && methodTable->name && methodTable->mfunc; ++methodTable)
        {
            lua_pushstring(E->L, methodTable->name);
            lua_pushlightuserdata(E->L, (void*)methodTable);
            lua_pushlightuserdata(E->L, (void*)E);
            lua_pushcclosure(E->L, thunk, 2);
            lua_settable(E->L, -3);
        }

        lua_remove(E->L, -1);
    }
};

class ElunaObject
{
public:
    template<typename T>
    ElunaObject(T * obj, bool manageMemory, Eluna* _e);

    ~ElunaObject()
    {
    }

    // Get wrapped object pointer
    void* GetObj() const { return object; }
    // Returns whether the object is valid or not
    bool IsValid() const { return !callstackid || callstackid == e->GetCallstackId(); }
    // Returns whether the object can be invalidated or not
    bool CanInvalidate() const { return _invalidate; }
    // Returns pointer to the wrapped object's type name
    const char* GetTypeName() const { return type_name; }

    // Sets the object pointer that is wrapped
    void SetObj(void* obj)
    {
        ASSERT(obj);
        object = obj;
        SetValid(true);
    }
    // Sets the object pointer to valid or invalid
    void SetValid(bool valid)
    {
        ASSERT(!valid || (valid && object));
        if (valid)
            if (CanInvalidate())
                callstackid = e->GetCallstackId();
            else
                callstackid = 0;
        else
            callstackid = 1;
    }
    // Sets whether the pointer will be invalidated at end of calls
    void SetValidation(bool invalidate)
    {
        _invalidate = invalidate;
    }
    // Invalidates the pointer if it should be invalidated
    void Invalidate()
    {
        if (CanInvalidate())
            callstackid = 1;
    }

private:
    uint64 callstackid;
    bool _invalidate;
    void* object;
    const char* type_name;
    Eluna* e;
};

template<typename T>
struct ElunaRegister
{
    const char* name;
    int(*mfunc)(Eluna*, T*);
};

template<typename T>
class ElunaTemplate
{
public:
    static const char* tname;
    static bool manageMemory;

    // name will be used as type name
    // If gc is true, lua will handle the memory management for object pushed
    // gc should be used if pushing for example WorldPacket,
    // that will only be needed on lua side and will not be managed by TC/mangos/<core>
    static void Register(Eluna* E, const char* name, bool gc = false)
    {
        ASSERT(!tname || name);

        tname = name;
        manageMemory = gc;

        lua_newtable(E->L);
        int methods = lua_gettop(E->L);

        // store method table in globals so that
        // scripts can add functions in Lua
        lua_pushvalue(E->L, methods);
        lua_setglobal(E->L, tname);

        luaL_newmetatable(E->L, tname);
        int metatable = lua_gettop(E->L);

        // tostring
        lua_pushcfunction(E->L, ToString);
        lua_setfield(E->L, metatable, "__tostring");

        // garbage collecting
        if (manageMemory)
        {
            lua_pushcfunction(E->L, CollectGarbage);
            lua_setfield(E->L, metatable, "__gc");
        }

        // make methods accessible through metatable
        lua_pushvalue(E->L, methods);
        lua_setfield(E->L, metatable, "__index");

        // make new indexes saved to methods
        lua_pushvalue(E->L, methods);
        lua_setfield(E->L, metatable, "__newindex");

        // make new indexes saved to methods
        /*lua_pushcfunction(E->L, Add);
        lua_setfield(E->L, metatable, "__add");

        // make new indexes saved to methods
        lua_pushcfunction(E->L, Substract);
        lua_setfield(E->L, metatable, "__sub");

        // make new indexes saved to methods
        lua_pushcfunction(E->L, Multiply);
        lua_setfield(E->L, metatable, "__mul");

        // make new indexes saved to methods
        lua_pushcfunction(E->L, Divide);
        lua_setfield(E->L, metatable, "__div");

        // make new indexes saved to methods
        lua_pushcfunction(E->L, Mod);
        lua_setfield(E->L, metatable, "__mod");

        // make new indexes saved to methods
        lua_pushcfunction(E->L, Pow);
        lua_setfield(E->L, metatable, "__pow");

        // make new indexes saved to methods
        lua_pushcfunction(E->L, UnaryMinus);
        lua_setfield(E->L, metatable, "__unm");

        // make new indexes saved to methods
        lua_pushcfunction(E->L, Concat);
        lua_setfield(E->L, metatable, "__concat");

        // make new indexes saved to methods
        lua_pushcfunction(E->L, Length);
        lua_setfield(E->L, metatable, "__len");

        // make new indexes saved to methods
        lua_pushcfunction(E->L, Equal);
        lua_setfield(E->L, metatable, "__eq");

        // make new indexes saved to methods
        lua_pushcfunction(E->L, Less);
        lua_setfield(E->L, metatable, "__lt");

        // make new indexes saved to methods
        lua_pushcfunction(E->L, LessOrEqual);
        lua_setfield(E->L, metatable, "__le");

        // make new indexes saved to methods
        lua_pushcfunction(E->L, Call);
        lua_setfield(E->L, metatable, "__call");*/

        // special method to get the object type
        lua_pushcfunction(E->L, GetType);
        lua_setfield(E->L, methods, "GetObjectType");

        // pop methods and metatable
        lua_pop(E->L, 2);
    }

    template<typename C>
    static void SetMethods(Eluna* E, ElunaRegister<C>* methodTable)
    {
        if (!methodTable)
            return;

        luaL_getmetatable(E->L, tname);
        if (!lua_istable(E->L, -1))
        {
            lua_remove(E->L, -1);
            ELUNA_LOG_ERROR("%s missing metatable", tname);
            return;
        }

        lua_getfield(E->L, -1, "__index");
        lua_remove(E->L, -2);
        if (!lua_istable(E->L, -1))
        {
            lua_remove(E->L, -1);
            ELUNA_LOG_ERROR("%s missing method table from metatable", tname);
            return;
        }

        for (; methodTable && methodTable->name && methodTable->mfunc; ++methodTable)
        {
            lua_pushstring(E->L, methodTable->name);
            lua_pushlightuserdata(E->L, (void*)methodTable);
            lua_pushlightuserdata(E->L, (void*)E);
            lua_pushcclosure(E->L, thunk, 2);
            lua_settable(E->L, -3);
        }

        lua_remove(E->L, -1);
    }

    static int Push(Eluna* E, T const* obj)
    {
        if (!obj)
        {
            lua_pushnil(E->L);
            return 1;
        }

        // Create new userdata
        ElunaObject** ptrHold = static_cast<ElunaObject**>(lua_newuserdata(E->L, sizeof(ElunaObject*)));
        if (!ptrHold)
        {
            ELUNA_LOG_ERROR("%s could not create new userdata", tname);
            lua_pushnil(E->L);
            return 1;
        }
        *ptrHold = new ElunaObject(const_cast<T*>(obj), manageMemory, E);

        // Set metatable for it
        lua_pushstring(E->L, tname);
        lua_rawget(E->L, LUA_REGISTRYINDEX);
        if (!lua_istable(E->L, -1))
        {
            ELUNA_LOG_ERROR("%s missing metatable", tname);
            lua_pop(E->L, 2);
            lua_pushnil(E->L);
            return 1;
        }
        lua_setmetatable(E->L, -2);
        return 1;
    }

    static T* Check(lua_State* L, int narg, bool error = true)
    {
        ElunaObject* elunaObj = Eluna::CHECKTYPE(L, narg, tname, error);
        if (!elunaObj)
            return NULL;

        if (!elunaObj->IsValid())
        {
            char buff[256];
            snprintf(buff, 256, "%s expected, got pointer to nonexisting (invalidated) object (%s). Check your code.", tname, luaL_typename(L, narg));
            if (error)
            {
                luaL_argerror(L, narg, buff);
            }
            else
            {
                ELUNA_LOG_ERROR("%s", buff);
            }
            return NULL;
        }
        return static_cast<T*>(elunaObj->GetObj());
    }

    static int GetType(lua_State* L)
    {
        lua_pushstring(L, tname);
        return 1;
    }

    static int SetInvalidation(lua_State* L)
    {
        ElunaObject* elunaObj = Eluna::CHECKOBJ<ElunaObject>(L, 1);
        bool invalidate = Eluna::CHECKVAL<bool>(L, 2);

        elunaObj->SetValidation(invalidate);
        return 0;
    }

    /*static int CallMethod(Eluna* e)
    {
        T* obj = Eluna::CHECKOBJ<T>(e->L, 1); // get self
        if (!obj)
            return 0;
        ElunaRegister<T>* l = static_cast<ElunaRegister<T>*>(lua_touserdata(e->L, lua_upvalueindex(1)));
        int top = lua_gettop(e->L);
        int expected = l->mfunc(e, obj);
        int args = lua_gettop(e->L) - top;
        if (args < 0 || args > expected)
        {
            ELUNA_LOG_ERROR("[Eluna]: %s returned unexpected amount of arguments %i out of %i. Report to devs", l->name, args, expected);
            ASSERT(false);
        }
        lua_settop(e->L, top + expected);
        return expected;
    }*/

    // Metamethods ("virtual")

    // Remember special cases like ElunaTemplate<Vehicle>::CollectGarbage
    static int CollectGarbage(lua_State* L)
    {
        // Get object pointer (and check type, no error)
        ElunaObject* obj = Eluna::CHECKOBJ<ElunaObject>(L, 1, false);
        if (obj && manageMemory)
            delete static_cast<T*>(obj->GetObj());
        delete obj;
        return 0;
    }

    static int ToString(lua_State* L)
    {
        T* obj = Eluna::CHECKOBJ<T>(L, 1, true); // get self
        lua_pushfstring(L, "%s: %p", tname, obj);
        return 1;
    }

    static int thunk(lua_State* L)
    {
        T* obj = Eluna::CHECKOBJ<T>(L, 1); // get self
        if (!obj)
            return 0;
        ElunaRegister<T>* l = static_cast<ElunaRegister<T>*>(lua_touserdata(L, lua_upvalueindex(1)));
        Eluna* E = static_cast<Eluna*>(lua_touserdata(L, lua_upvalueindex(2)));
        int args = lua_gettop(L);
        int expected = l->mfunc(E, obj);
        args = lua_gettop(L) - args;
        if (args < 0 || args > expected) // Assert instead?
        {
            ELUNA_LOG_ERROR("[Eluna]: %s returned unexpected amount of arguments %i out of %i. Report to devs", l->name, args, expected);
        }
        for (; args < expected; ++args)
            lua_pushnil(L);
        return expected;
    }

    static int ArithmeticError(Eluna* E) { return luaL_error(E->L, "attempt to perform arithmetic on a %s value", tname); }
    static int CompareError(Eluna* E) { return luaL_error(E->L, "attempt to compare %s", tname); }
    static int Add(Eluna* E) { return ArithmeticError(E); }
    static int Substract(Eluna* E) { return ArithmeticError(E); }
    static int Multiply(Eluna* E) { return ArithmeticError(E); }
    static int Divide(Eluna* E) { return ArithmeticError(E); }
    static int Mod(Eluna* E) { return ArithmeticError(E); }
    static int Pow(Eluna* E) { return ArithmeticError(E); }
    static int UnaryMinus(Eluna* E) { return ArithmeticError(E); }
    static int Concat(Eluna* E) { return luaL_error(E->L, "attempt to concatenate a %s value", tname); }
    static int Length(Eluna* E) { return luaL_error(E->L, "attempt to get length of a %s value", tname); }
    static int Equal(Eluna* E) { Eluna::Push(E->L, Eluna::CHECKOBJ<T>(E->L, 1) == Eluna::CHECKOBJ<T>(E->L, 2)); return 1; }
    static int Less(Eluna* E) { return CompareError(E); }
    static int LessOrEqual(Eluna* E) { return CompareError(E); }
    static int Call(Eluna* E) { return luaL_error(E->L, "attempt to call a %s value", tname); }
};

template<typename T>
ElunaObject::ElunaObject(T * obj, bool manageMemory, Eluna * _e) : callstackid(1), _invalidate(!manageMemory), object(obj), e(_e), type_name(ElunaTemplate<T>::tname)
{
    SetValid(true);
}

template<typename T> const char* ElunaTemplate<T>::tname = NULL;
template<typename T> bool ElunaTemplate<T>::manageMemory = false;

#endif
