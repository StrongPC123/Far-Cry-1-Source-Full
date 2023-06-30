// FunctionHandler.cpp: implementation of the CFunctionHandler class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FunctionHandler.h"
#include "ScriptSystem.h"
extern "C"{
	#include <lua.h>
}
#if defined(_DEBUG) && !defined(LINUX)
#include <crtdbg.h>
#define DEBUG_CLIENTBLOCK new( _NORMAL_BLOCK, __FILE__, __LINE__) 
#define new DEBUG_CLIENTBLOCK
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFunctionHandler::CFunctionHandler()
{

}

CFunctionHandler::~CFunctionHandler()
{

}

void CFunctionHandler::__Attach(HSCRIPT hScript)
{
	m_pLS=(lua_State *)hScript;
	//m_nFunctionID = static_cast<int>(lua_tonumber(m_pLS,-2));
	//m_pThisPtr=lua_touserdata(m_pLS,-1);
	//lua_pop(m_pLS, 2);
}

/*THIS_PTR CFunctionHandler::GetThis()
{
	return lua_touserdata(m_pLS,-1);
}*/

THIS_PTR CFunctionHandler::GetThis()
{
	if(!lua_istable(m_pLS,1))return NULL;
	return lua_getnativedata(m_pLS,1);
}

int CFunctionHandler::GetFunctionID()
{
	return static_cast<int>(lua_tonumber(m_pLS,-1));
}

int CFunctionHandler::GetParamCount()
{
	return lua_gettop(m_pLS)-2;// -2 are "this" and "func ID"
}

#if defined(WIN64) || defined(LINUX64)
bool CFunctionHandler::GetParam(int nIdx,INT_PTR &n)	//AMD Port
{
	int nRealIdx=nIdx+1;
	if(!lua_isnumber(m_pLS,nRealIdx))return false;
	n=(int)lua_tonumber(m_pLS,nRealIdx);
	return true;
}
#endif

bool CFunctionHandler::GetParam(int nIdx,int &n)
{
	int nRealIdx=nIdx+1;
	if(!lua_isnumber(m_pLS,nRealIdx))return false;
	n=(int)lua_tonumber(m_pLS,nRealIdx);
	return true;
}

bool CFunctionHandler::GetParam(int nIdx,float &f)
{
	int nRealIdx=nIdx+1;
	if(!lua_isnumber(m_pLS,nRealIdx))return false;
	f=(float)lua_tonumber(m_pLS,nRealIdx);
	return true;
}

bool CFunctionHandler::GetParam(int nIdx,const char * &s)
{
	//if(!lua_isstring(m_pLS,nIdx))return false;
	s=(char *)lua_tostring(m_pLS,nIdx+1);
	if(s)
		return true;
	else 
		return false;
}

bool CFunctionHandler::GetParam(int nIdx,bool &b)
{
	int nRealIdx=nIdx+1;
	if(lua_isnil(m_pLS,nRealIdx))
	{
		b=false;
	}
	else if (lua_isnumber(m_pLS,nRealIdx))
	{
		int nVal=0;
		nVal = (int)lua_tonumber(m_pLS,nRealIdx);
		if(nVal)
			b=true;
		else
			b=false;
	}
	return true;
}

bool CFunctionHandler::GetParam(int nIdx,IScriptObject *pObj)
{
	int nRealIdx=nIdx+1;
	if(!lua_istable(m_pLS,nRealIdx))return false;
	lua_pushvalue(m_pLS,nRealIdx );
	pObj->Attach();
	return true;
}

bool CFunctionHandler::GetParam(int nIdx,HSCRIPTFUNCTION &hFunc, int nReference)
{
	int nRealIdx=nIdx+1;
	if(!lua_isfunction(m_pLS,nRealIdx))
	{
		return false;
	}
	
	lua_pushvalue(m_pLS,nRealIdx);
	int nRef = lua_ref(m_pLS,nReference);
	switch(nRef)
	{
	case LUA_NOREF:
	case LUA_REFNIL:
		return false;
	default:
		hFunc=(HSCRIPTFUNCTION)nRef;
		return true;
	}
}

bool CFunctionHandler::GetParam(int nIdx,USER_DATA &ud)
{
	int nRealIdx=nIdx+1;
	if(!lua_isuserdata(m_pLS,nRealIdx))
	{
		return false;
	}
//	lua_pushvalue(m_pLS,nRealIdx);
	USER_DATA_CHUNK *udc=(USER_DATA_CHUNK *)lua_touserdata(m_pLS,nRealIdx);
	//ud = (int)lua_ref(m_pLS,0 );
	ud=udc->nRef;
	if(ud)
	{
		return true;
	}
	return false;
}

bool CFunctionHandler::GetParamUDVal(int nIdx,USER_DATA &val,int &cookie)		//AMD Port
{
	USER_DATA_CHUNK *udc=(USER_DATA_CHUNK *)lua_touserdata(m_pLS,nIdx+1);
	if(!udc)
		return false;
	val=udc->nVal;
  cookie=udc->nCookie;
	return true;
}

ScriptVarType CFunctionHandler::GetParamType(int nIdx)
{
	int nRealIdx=nIdx+1;
		if (lua_isnil(m_pLS,nRealIdx))
		{
			return svtNull;
		}
		else if (lua_iscfunction(m_pLS,nRealIdx) || lua_isfunction(m_pLS,nRealIdx))
		{
			return svtFunction;
		}
		else if (lua_isnumber(m_pLS,nRealIdx))
		{
			return svtNumber;
		}
		else if (lua_isstring(m_pLS,nRealIdx))
		{
			return svtString;
		}
		else if (lua_istable(m_pLS,nRealIdx))
		{
			return svtObject;
		}
		else if (lua_isuserdata(m_pLS, nRealIdx))	// Added by Márcio
		{											// was missing the userdata type
			return svtUserData;						//
		}											//
		return svtNull;
}

int CFunctionHandler::EndFunction(int nRetVal)
{
	lua_pushnumber(m_pLS, (lua_Number)nRetVal);
	return 1;
}

int CFunctionHandler::EndFunction(float fRetVal)
{
	lua_pushnumber(m_pLS,fRetVal);
	return 1;
}

int CFunctionHandler::EndFunction(int nRetVal1,int nRetVal2)
{
	lua_pushnumber(m_pLS, (lua_Number)nRetVal1);
	lua_pushnumber(m_pLS, (lua_Number)nRetVal2);
	return 2;
}

int CFunctionHandler::EndFunction(float fRetVal1,float fRetVal2)
{
	lua_pushnumber(m_pLS,fRetVal1);
	lua_pushnumber(m_pLS,fRetVal2);
	return 2;
}

int CFunctionHandler::EndFunction(const char* sRetVal)
{
	lua_pushstring(m_pLS,sRetVal);
	return 1;
}

int CFunctionHandler::EndFunction(bool bRetVal)
{
	if(bRetVal)
		lua_pushnumber(m_pLS,1);
	else
		lua_pushnil(m_pLS);
	return 1;
}

int CFunctionHandler::EndFunction(IScriptObject *pObj)
{
	lua_xgetref(m_pLS,pObj->GetRef());
	return 1;
}

int CFunctionHandler::EndFunction(HSCRIPTFUNCTION hFunc)
{
	lua_getref(m_pLS,(int)hFunc);
	return 1;
}

int CFunctionHandler::EndFunction(USER_DATA ud)
{
	lua_getref(m_pLS,(int)ud);
	return 1;
}

int CFunctionHandler::EndFunctionNull()
{
	lua_pushnil(m_pLS);
	return 1;
}

int CFunctionHandler::EndFunction()
{
	// return 0
	return (EndFunctionNull());
}


void CFunctionHandler::Unref (HSCRIPTFUNCTION hFunc)
{
	lua_unref (m_pLS, hFunc);
}