#
# Copyright (C) 2010 - 2014 Eluna Lua Engine <http://emudevs.com/>
# This program is free software licensed under GPL version 3
# Please see the included DOCS/LICENSE.md for more information
#

file(GLOB sources *.c)
list(REMOVE_ITEM sources ${CMAKE_CURRENT_SOURCE_DIR}/lua.c)
list(REMOVE_ITEM sources ${CMAKE_CURRENT_SOURCE_DIR}/luac.c)

set(lua_STAT_SRCS
  ${sources}
)

#LuaSocket
file(GLOB LuaSocket LuaSocket/*.C)
    LIST(APPEND lua_STAT_SRCS ${LuaSocket})
    source_group("LuaSocket" FILES ${LuaSocket})

#LuaSec
file(GLOB LuaSec LuaSec/*.C)
    LIST(APPEND lua_STAT_SRCS ${LuaSec})
    source_group("LuaSec" FILES ${LuaSec})

add_library(lualib STATIC ${lua_STAT_SRCS})


target_include_directories(lualib
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/LuaSocket
    ${CMAKE_CURRENT_SOURCE_DIR}/LuaSec)

target_link_libraries(lualib
  PRIVATE
    trinity-dependency-interface
  PUBLIC
    openssl)

add_definitions(-DLUASOCKET_DEBUG)
add_definitions(-DLUA_COMPAT_MODULE)