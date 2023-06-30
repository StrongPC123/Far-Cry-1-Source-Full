/*
** $Id: lualib.h,v 1.21 2001/03/26 14:31:49 roberto Exp $
** Lua standard libraries
** See Copyright Notice in lua.h
*/


#ifndef lualib_h
#define lualib_h

#include "lua.h"


#ifdef PS2
#define LUALIB_API    extern "C"
#else
#ifndef LUALIB_API
#define LUALIB_API	extern
#endif
#endif

#define LUA_ALERT               "_ALERT"

LUALIB_API int lua_baselibopen (lua_State *L);
LUALIB_API int lua_iolibopen (lua_State *L);
LUALIB_API int lua_strlibopen (lua_State *L);
LUALIB_API int lua_mathlibopen (lua_State *L);
LUALIB_API int lua_dblibopen (lua_State *L);
LUALIB_API void lua_bitlibopen (lua_State *L);


#endif
