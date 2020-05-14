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

    int SummonPassenger(lua_State* L, Transport* obj)
    {
        auto entry = Eluna::CHECKVAL<uint32>(L, 2);
        auto x = Eluna::CHECKVAL<int32>(L, 3);
        auto y = Eluna::CHECKVAL<int32>(L, 4);
        auto z = Eluna::CHECKVAL<int32>(L, 5);
        auto o = Eluna::CHECKVAL<int32>(L, 6);
        auto summonType = Eluna::CHECKVAL<uint32>(L, 7);
        Eluna::Push(L, obj->SummonPassenger(entry, Position(x, y, z, o), (TempSummonType)summonType));
        return 1;
    }

    int SetVisible(lua_State* L, Transport* obj)
    {
        bool visible = Eluna::CHECKVAL<bool>(L, 2);
        if (visible)
        {
            obj->m_serverSideVisibility.SetValue(SERVERSIDE_VISIBILITY_GM, SEC_PLAYER);
        }
        else
        {
            obj->m_serverSideVisibility.SetValue(SERVERSIDE_VISIBILITY_GM, SEC_GAMEMASTER);
        }
        obj->UpdateObjectVisibility(true);
        return 0;
    }
};

#endif
