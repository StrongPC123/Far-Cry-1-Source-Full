#ifndef __LUA_DBG_INTERFACE_H__
#define __LUA_DBG_INTERFACE_H__

#pragma once

class CLUADbg;

bool InvokeDebugger(CLUADbg *pDebugger, const char *pszSourceFile = NULL, int iLine = 0, const char *pszReason = NULL);

#endif