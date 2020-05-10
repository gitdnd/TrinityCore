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
    }
};

#endif
