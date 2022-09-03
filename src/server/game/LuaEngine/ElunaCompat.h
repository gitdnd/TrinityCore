#ifndef ELUNACOMPAT_H
#define ELUNACOMPAT_H

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
};

#if LUA_VERSION_NUM == 501
    int luaL_getsubtable(lua_State* L, int i, const char* name);
    const char* luaL_tolstring(lua_State* L, int idx, size_t* len);
    int lua_absindex(lua_State* L, int i);
    #define lua_pushglobaltable(L) \
        lua_pushvalue((L), LUA_GLOBALSINDEX);
    #define lua_rawlen(L, idx) \
        lua_objlen(L, idx);
    #define lua_pushunsigned(L, u) \
        lua_pushinteger(L, u);
    #define lua_load(L, buf_read, dec_buf, str, NULL) \
        lua_load(L, buf_read, dec_buf, str);

#endif

#endif
