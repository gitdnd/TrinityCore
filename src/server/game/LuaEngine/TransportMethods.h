#include <Transport.h>

#ifndef TRANSPORTMETHODS_H
#define TRANSPORTMETHODS_H

namespace LuaTransport
{
    int EnableMovement(lua_State* L, Transport* obj)
    {
        bool state = Eluna::CHECKVAL<bool>(L, 2);
        obj->EnableMovement(state);
        return 0;
    }

    int AddPassenger(lua_State* L, Transport* obj)
    {
        WorldObject* worldObj = Eluna::CHECKOBJ<WorldObject>(L, 2);
        obj->AddPassenger(worldObj);
        return 0;
    }

    int GetPassengers(lua_State* L, Transport* obj)
    {
        auto list = obj->GetPassengers();
        lua_createtable(L, list.size(), 0);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (auto it = list.begin(); it != list.end(); ++it)
        {
            Eluna::Push(L, *it);
            lua_rawseti(L, tbl, ++i);
        }

        lua_settop(L, tbl);
        return 1;
    }
};

#endif
