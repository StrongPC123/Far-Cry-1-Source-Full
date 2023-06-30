/*
** $Id: lgc.h,v 1.12 2001/06/21 16:41:34 roberto Exp $
** Garbage Collector
** See Copyright Notice in lua.h
*/

#ifndef lgc_h
#define lgc_h


#include "lobject.h"


#define luaC_checkGC(L) if (G(L)->nblocks >= G(L)->GCthreshold) \
			  luaC_collectgarbage(L)


void luaC_callallgcTM (lua_State *L);
void luaC_collect (lua_State *L, int all);
void luaC_collectgarbage (lua_State *L);


#endif
