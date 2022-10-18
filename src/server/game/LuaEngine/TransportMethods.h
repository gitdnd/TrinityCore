#include <Transport.h>

#ifndef TRANSPORTMETHODS_H
#define TRANSPORTMETHODS_H

namespace LuaTransport
{
    int EnableMovement(Eluna* E, Transport* obj)
    {
        bool state = Eluna::CHECKVAL<bool>(E->L, 2);
        obj->EnableMovement(state);
        return 0;
    }

    int AddPassenger(Eluna* E, Transport* obj)
    {
        WorldObject* worldObj = Eluna::CHECKOBJ<WorldObject>(E->L, 2);
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
            Eluna::Push(E->L, *it);
            lua_rawseti(E->L, tbl, ++i);
        }

        lua_settop(E->L, tbl);
        return 1;
    }

    int SummonPassenger(Eluna* E, Transport* obj)
    {
        auto entry = Eluna::CHECKVAL<uint32>(E->L, 2);
        auto x = Eluna::CHECKVAL<int32>(E->L, 3);
        auto y = Eluna::CHECKVAL<int32>(E->L, 4);
        auto z = Eluna::CHECKVAL<int32>(E->L, 5);
        auto o = Eluna::CHECKVAL<int32>(E->L, 6);
        auto summonType = Eluna::CHECKVAL<uint32>(E->L, 7);
        Eluna::Push(E->L, obj->SummonPassenger(entry, Position(x, y, z, o), (TempSummonType)summonType));
        return 1;
    }
};

#endif
