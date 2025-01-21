/*
* Copyright (C) 2010 - 2024 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

// Eluna
#include "LuaEngine.h"
#include "ElunaIncludes.h"
#include "ElunaTemplate.h"
#include "ElunaUtility.h"

#if defined TRACKABLE_PTR_NAMESPACE
ElunaConstrainedObjectRef<Aura> GetWeakPtrFor(Aura const* obj)
{
#if defined ELUNA_TRINITY
    Map* map = obj->GetOwner()->GetMap();
#elif defined ELUNA_CMANGOS
    Map* map = obj->GetTarget()->GetMap();
#endif
    return { obj->GetWeakPtr(), map };
}
ElunaConstrainedObjectRef<BattleGround> GetWeakPtrFor(BattleGround const* obj) { return { obj->GetWeakPtr(), obj->GetBgMap() }; }
ElunaConstrainedObjectRef<Group> GetWeakPtrFor(Group const* obj) { return { obj->GetWeakPtr(), nullptr }; }
ElunaConstrainedObjectRef<Guild> GetWeakPtrFor(Guild const* obj) { return { obj->GetWeakPtr(), nullptr }; }
ElunaConstrainedObjectRef<Map> GetWeakPtrFor(Map const* obj) { return { obj->GetWeakPtr(), obj }; }
ElunaConstrainedObjectRef<Object> GetWeakPtrForObjectImpl(Object const* obj)
{
    if (obj->isType(TYPEMASK_WORLDOBJECT))
        return { obj->GetWeakPtr(), static_cast<WorldObject const*>(obj)->GetMap() };

    if (obj->GetTypeId() == TYPEID_ITEM)
        if (Player const* player = static_cast<Item const*>(obj)->GetOwner())
            return { obj->GetWeakPtr(), player->GetMap() };

    // possibly dangerous item
    return { obj->GetWeakPtr(), nullptr };
}
ElunaConstrainedObjectRef<Quest> GetWeakPtrFor(Quest const* obj) { return { obj->GetWeakPtr(), nullptr }; }
ElunaConstrainedObjectRef<Spell> GetWeakPtrFor(Spell const* obj) { return { obj->GetWeakPtr(), obj->GetCaster()->GetMap() }; }
#if ELUNA_EXPANSION >= EXP_WOTLK
ElunaConstrainedObjectRef<Vehicle> GetWeakPtrFor(Vehicle const* obj)
{
#if defined ELUNA_TRINITY
    Map* map = obj->GetBase()->GetMap();
#elif defined ELUNA_CMANGOS
    Map* map = obj->GetOwner()->GetMap();
#endif
    return { obj->GetWeakPtr(), map };
}
#endif
#endif

// Template by Mud from http://stackoverflow.com/questions/4484437/lua-integer-type/4485511#4485511
template<> int ElunaTemplate<unsigned long long>::Add(lua_State* L) { Eluna* E = Eluna::GetEluna(L); E->Push(E->CHECKVAL<unsigned long long>(1) + E->CHECKVAL<unsigned long long>(2)); return 1; }
template<> int ElunaTemplate<unsigned long long>::Subtract(lua_State* L) { Eluna* E = Eluna::GetEluna(L); E->Push(E->CHECKVAL<unsigned long long>(1) - E->CHECKVAL<unsigned long long>(2)); return 1; }
template<> int ElunaTemplate<unsigned long long>::Multiply(lua_State* L) { Eluna* E = Eluna::GetEluna(L); E->Push(E->CHECKVAL<unsigned long long>(1) * E->CHECKVAL<unsigned long long>(2)); return 1; }
template<> int ElunaTemplate<unsigned long long>::Divide(lua_State* L) { Eluna* E = Eluna::GetEluna(L); E->Push(E->CHECKVAL<unsigned long long>(1) / E->CHECKVAL<unsigned long long>(2)); return 1; }
template<> int ElunaTemplate<unsigned long long>::Mod(lua_State* L) { Eluna* E = Eluna::GetEluna(L); E->Push(E->CHECKVAL<unsigned long long>(1) % E->CHECKVAL<unsigned long long>(2)); return 1; }
// template<> int ElunaTemplate<unsigned long long>::UnaryMinus(lua_State* L) { Eluna::GetEluna(L)->Push(-E->CHECKVAL<unsigned long long>(L, 1)); return 1; }
template<> int ElunaTemplate<unsigned long long>::Equal(lua_State* L) { Eluna* E = Eluna::GetEluna(L); E->Push(E->CHECKVAL<unsigned long long>(1) == E->CHECKVAL<unsigned long long>(2)); return 1; }
template<> int ElunaTemplate<unsigned long long>::Less(lua_State* L) { Eluna* E = Eluna::GetEluna(L); E->Push(E->CHECKVAL<unsigned long long>(1) < E->CHECKVAL<unsigned long long>(2)); return 1; }
template<> int ElunaTemplate<unsigned long long>::LessOrEqual(lua_State* L) { Eluna* E = Eluna::GetEluna(L); E->Push(E->CHECKVAL<unsigned long long>(1) <= E->CHECKVAL<unsigned long long>(2)); return 1; }
template<> int ElunaTemplate<unsigned long long>::Pow(lua_State* L)
{
    Eluna* E = Eluna::GetEluna(L);
    E->Push(static_cast<unsigned long long>(powl(static_cast<long double>(E->CHECKVAL<unsigned long long>(1)), static_cast<long double>(E->CHECKVAL<unsigned long long>(2)))));
    return 1;
}
template<> int ElunaTemplate<unsigned long long>::ToString(lua_State* L)
{
    Eluna* E = Eluna::GetEluna(L);

    unsigned long long l = E->CHECKVAL<unsigned long long>(1);
    std::ostringstream ss;
    ss << l;
    E->Push(ss.str());
    return 1;
}

template<> int ElunaTemplate<long long>::Add(lua_State* L) { Eluna* E = Eluna::GetEluna(L); E->Push(E->CHECKVAL<long long>(1) + E->CHECKVAL<long long>(2)); return 1; }
template<> int ElunaTemplate<long long>::Subtract(lua_State* L) { Eluna* E = Eluna::GetEluna(L); E->Push(E->CHECKVAL<long long>(1) - E->CHECKVAL<long long>(2)); return 1; }
template<> int ElunaTemplate<long long>::Multiply(lua_State* L) { Eluna* E = Eluna::GetEluna(L); E->Push(E->CHECKVAL<long long>(1) * E->CHECKVAL<long long>(2)); return 1; }
template<> int ElunaTemplate<long long>::Divide(lua_State* L) { Eluna* E = Eluna::GetEluna(L); E->Push(E->CHECKVAL<long long>(1) / E->CHECKVAL<long long>(2)); return 1; }
template<> int ElunaTemplate<long long>::Mod(lua_State* L) { Eluna* E = Eluna::GetEluna(L); E->Push(E->CHECKVAL<long long>(1) % E->CHECKVAL<long long>(2)); return 1; }
template<> int ElunaTemplate<long long>::UnaryMinus(lua_State* L) { Eluna* E = Eluna::GetEluna(L); E->Push(-E->CHECKVAL<long long>(1)); return 1; }
template<> int ElunaTemplate<long long>::Equal(lua_State* L) { Eluna* E = Eluna::GetEluna(L); E->Push(E->CHECKVAL<long long>(1) == E->CHECKVAL<long long>(2)); return 1; }
template<> int ElunaTemplate<long long>::Less(lua_State* L) { Eluna* E = Eluna::GetEluna(L); E->Push(E->CHECKVAL<long long>(1) < E->CHECKVAL<long long>(2)); return 1; }
template<> int ElunaTemplate<long long>::LessOrEqual(lua_State* L) { Eluna* E = Eluna::GetEluna(L); E->Push(E->CHECKVAL<long long>(1) <= E->CHECKVAL<long long>(2)); return 1; }
template<> int ElunaTemplate<long long>::Pow(lua_State* L)
{
    Eluna* E = Eluna::GetEluna(L);
    E->Push(static_cast<long long>(powl(static_cast<long double>(E->CHECKVAL<long long>(1)), static_cast<long double>(E->CHECKVAL<long long>(2)))));
    return 1;
}
template<> int ElunaTemplate<long long>::ToString(lua_State* L)
{
    Eluna* E = Eluna::GetEluna(L);

    long long l = E->CHECKVAL<long long>(1);
    std::ostringstream ss;
    ss << l;
    E->Push(ss.str());
    return 1;
}

template<> int ElunaTemplate<ObjectGuid>::Equal(lua_State* L) { Eluna* E = Eluna::GetEluna(L); E->Push(E->CHECKVAL<ObjectGuid>(1) == E->CHECKVAL<ObjectGuid>(2)); return 1; }
template<> int ElunaTemplate<ObjectGuid>::ToString(lua_State* L)
{
    Eluna* E = Eluna::GetEluna(L);
#if defined ELUNA_TRINITY
    E->Push(E->CHECKVAL<ObjectGuid>(1).ToString());
#else
    E->Push(E->CHECKVAL<ObjectGuid>(1).GetString());
#endif
    return 1;
}
