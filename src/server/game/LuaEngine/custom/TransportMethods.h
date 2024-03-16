#include <Transport.h>

#ifndef TRANSPORTMETHODS_H
#define TRANSPORTMETHODS_H

namespace LuaTransport
{
    int EnableMovement(Eluna* E, Transport* obj)
    {
        bool state = E->CHECKVAL<bool>(2);
        obj->EnableMovement(state);
        return 0;
    }

    int AddPassenger(Eluna* E, Transport* obj)
    {
        WorldObject* worldObj = E->CHECKOBJ<WorldObject>(2);
        obj->AddPassenger(worldObj);
        return 0;
    }

    int GetPassengers(Eluna* E, Transport* obj)
    {
        auto list = obj->GetPassengers();
        lua_createtable(E->L, list.size(), 0);
        int tbl = lua_gettop(E->L);
        uint32 i = 0;

        for (auto it = list.begin(); it != list.end(); ++it)
        {
            E->Push(*it);
            lua_rawseti(E->L, tbl, ++i);
        }

        lua_settop(E->L, tbl);
        return 1;
    }

    int SummonPassenger(Eluna* E, Transport* obj)
    {
        auto entry = E->CHECKVAL<uint32>(2);
        auto x = E->CHECKVAL<int32>(3);
        auto y = E->CHECKVAL<int32>(4);
        auto z = E->CHECKVAL<int32>(5);
        auto o = E->CHECKVAL<int32>(6);
        auto summonType = E->CHECKVAL<uint32>(7);
        E->Push(obj->SummonPassenger(entry, Position(x, y, z, o), (TempSummonType)summonType));
        return 1;
    }

    ElunaRegister<Transport> TransportMethods[] =
    {
        // Getters
        { "EnableMovement", &LuaTransport::EnableMovement },
        { "AddPassenger", &LuaTransport::AddPassenger },
        { "GetPassengers", &LuaTransport::GetPassengers },
        { "SummonPassenger", &LuaTransport::SummonPassenger },

        { NULL, NULL, METHOD_REG_NONE }
    };
};

#endif
