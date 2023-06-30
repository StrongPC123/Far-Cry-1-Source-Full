// ScriptObject.cpp: implementation of the CScriptObject class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScriptObject.h"
#include "ScriptSystem.h"
#include "StackGuard.h"

#include <ISystem.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#if defined(_DEBUG) && !defined(LINUX)
#include <crtdbg.h>
#define DEBUG_CLIENTBLOCK new( _NORMAL_BLOCK, __FILE__, __LINE__) 
#define new DEBUG_CLIENTBLOCK
#endif

#ifdef _DEBUG

#define BEGIN_CHECK_STACK \
	int __nStack = lua_stackspace(m_pLS);

#define END_CHECK_STACK \
{			\
	if (__nStack != lua_stackspace(m_pLS)) \
	CryError( "<CryScript> END_CHECK_STATE Failed." );	\
}


#else

#define BEGIN_CHECK_STACK
#define END_CHECK_STACK

#endif


#define GET_FUNCTION lua_rawget
#define SET_FUNCTION lua_rawset

inline int CScriptObject::GetThisRef()
{
	if (m_bDeleted)
	{
		CryError( "Access to deleted script object" );
	}
	return lua_xgetref(m_pLS, m_nRef);
}

#define _GET_THIS() GetThisRef()

CScriptObject::CScriptObject(int nCreationNumber)
{
	m_nRef=nCreationNumber;
	m_pLS = NULL;
	
	m_hDelegationTag= 0;
	m_pSink = NULL;
	m_nIterationCounter=-1;
	m_pSetGetParams=NULL;
	m_bAttached = false;
	m_bDeleted = false;
}

CScriptObject::~CScriptObject()
{
	if(m_pSetGetParams)
		delete m_pSetGetParams;
}

int CScriptObject::GetRef()
{
	return m_nRef;
}


void CScriptObject::Attach()
{
	m_bAttached = true;
	Detach();
	lua_xref(m_pLS,m_nRef);
}


void CScriptObject::Recreate()
{
	m_hDelegationTag= 0;
	m_pSink = NULL;
	m_nIterationCounter=-1;
	m_pSetGetParams=NULL;

	m_bAttached = false;
	m_bDeleted = false;
}

bool CScriptObject::CreateEmpty(CScriptSystem *pScriptSystem)
{
	;
	m_pLS=(lua_State *)pScriptSystem->GetScriptHandle();
	Detach();
	return true;
}

bool CScriptObject::Create(CScriptSystem *pScriptSystem)
{
	m_pLS=(lua_State *)pScriptSystem->GetScriptHandle();
	_GUARD_STACK(m_pLS);
	Detach();

	lua_newtable(m_pLS);
	
	Attach();
	return true;
	
	//return false;
}

bool CScriptObject::CreateGlobal(CScriptSystem *pScriptSystem, const char *sName)
{
	m_pLS=(lua_State *)pScriptSystem->GetScriptHandle();
	_GUARD_STACK(m_pLS);
	Detach();

	lua_newtable(m_pLS);
	Attach();
	_GET_THIS();
	lua_setglobal(m_pLS, sName);	
	
		return true;
}

void CScriptObject::PushBack(int nVal)
{
	_GUARD_STACK(m_pLS);
	if (!_GET_THIS())
			return;
	int nLastPos=lua_getn(m_pLS,-1);
	SetAt(nLastPos+1,nVal);
}

void CScriptObject::PushBack(float fVal)
{
	_GUARD_STACK(m_pLS);
	if (!_GET_THIS())
			return;
	int nLastPos=lua_getn(m_pLS,-1);
	SetAt(nLastPos+1,fVal);
	
}

void CScriptObject::PushBack(const char *sVal)
{
	_GUARD_STACK(m_pLS);
	if (!_GET_THIS())
			return;
	int nLastPos=lua_getn(m_pLS,-1);
	SetAt(nLastPos+1,sVal);
	
}

void CScriptObject::PushBack(bool bVal)
{
	_GUARD_STACK(m_pLS);
	if (!_GET_THIS())
			return;
	int nLastPos=lua_getn(m_pLS,-1);
	SetAt(nLastPos+1,bVal);
	
}

void CScriptObject::PushBack(IScriptObject *pObj)
{
	_GUARD_STACK(m_pLS);
	if (!_GET_THIS())
			return;
	int nLastPos=lua_getn(m_pLS,-1);
	SetAt(nLastPos+1,pObj);
	
}

bool CScriptObject::BeginSetGetChain()
{
	if (!_GET_THIS())
		return false;
	return true;
}

void CScriptObject::EndSetGetChain()
{
	if(lua_istable(m_pLS,-1))
		lua_pop(m_pLS,1);
	else{
		DebugBreak();
	}
}

void CScriptObject::SetValueChain(const char *sKey, int nVal)
{
	lua_pushstring(m_pLS, sKey);
	lua_pushnumber(m_pLS, (lua_Number)nVal);
	SET_FUNCTION(m_pLS, - 3);
}

void CScriptObject::SetValue(const char *sKey, int nVal)
{
	_GUARD_STACK(m_pLS);
	if (!_GET_THIS())
		return;
		
	SetValueChain(sKey,nVal);
	
}

void CScriptObject::SetValueChain(const char *sKey, float fVal)
{
	lua_pushstring(m_pLS, sKey);
	lua_pushnumber(m_pLS, fVal);
	SET_FUNCTION(m_pLS, - 3);
}

void CScriptObject::SetValue(const char *sKey, float fVal)
{
	_GUARD_STACK(m_pLS);
	if (!_GET_THIS())
		return;
		
	SetValueChain(sKey,fVal);
}

void CScriptObject::SetValueChain(const char *sKey, bool bVal)
{
  lua_pushstring(m_pLS, sKey);
	if (bVal)
		lua_pushnumber(m_pLS, 1);
	else
		lua_pushnil(m_pLS);
	SET_FUNCTION(m_pLS, - 3);
}

void CScriptObject::SetValue(const char *sKey, bool bVal)
{
	_GUARD_STACK(m_pLS);
		// int nVal=bVal?1:0;
	if (!_GET_THIS())
			return;
	SetValueChain(sKey,bVal);
}

void CScriptObject::SetValueChain(const char *sKey, const char *sVal)
{
		lua_pushstring(m_pLS, sKey);
		lua_pushstring(m_pLS, sVal);
		SET_FUNCTION(m_pLS, - 3);
}

void CScriptObject::SetValue(const char *sKey, const char *sVal)
{
	_GUARD_STACK(m_pLS);
	if (!_GET_THIS())
		return;
	SetValueChain(sKey,sVal);
}

void CScriptObject::SetValueChain(const char *sKey, IScriptObject *pObj)
{ 
	lua_pushstring(m_pLS, sKey);
	if (!pObj)	
	{
		CryWarning( VALIDATOR_MODULE_GAME,VALIDATOR_WARNING,"\001 ERROR! Passing NULL IScriptObject to SETVALUE CHAIN!");
#if defined(_DEBUG) && !defined(WIN64)
		DEBUG_BREAK;
#endif
		lua_pushnil(m_pLS);
	}
	else
	  lua_xgetref(m_pLS, pObj->GetRef());
	SET_FUNCTION(m_pLS, - 3);
}

void CScriptObject::SetValue(const char *sKey, IScriptObject *pObj)
{
	_GUARD_STACK(m_pLS);
	if (!_GET_THIS())
		return;

	SetValueChain(sKey,pObj);
}

void CScriptObject::SetValueChain(const char *sKey, USER_DATA ud)
{
	_GUARD_STACK(m_pLS);
	lua_pushstring(m_pLS, sKey);

	if(ud && lua_getref(m_pLS, ud))
	{
		if(lua_isuserdata(m_pLS,-1))
		{
			SET_FUNCTION(m_pLS, - 3);
		}
	}
}

void CScriptObject::SetValue(const char *sKey, USER_DATA ud)
{
	_GUARD_STACK(m_pLS);
	if (!_GET_THIS())
		return;

	SetValueChain(sKey,ud);
}

void CScriptObject::SetToNullChain(const char *sKey)
{
	lua_pushstring(m_pLS, sKey);
	lua_pushnil(m_pLS);
	SET_FUNCTION(m_pLS, - 3);
}

void CScriptObject::SetToNull(const char *sKey)
{
	_GUARD_STACK(m_pLS);
	if (!_GET_THIS())
		return;
		SetToNullChain(sKey);
}

bool CScriptObject::GetValueChain(const char *sKey, int &nVal)
{
	_GUARD_STACK(m_pLS);
	bool res=false;
	lua_pushstring(m_pLS, sKey);
	GET_FUNCTION(m_pLS, - 2);
	if (lua_isnumber(m_pLS, - 1))
	{
		res = true;
		nVal =(int)lua_tonumber(m_pLS, - 1);
	}
	return res;
}

bool CScriptObject::GetValue(const char *sKey, int &nVal)
{
	BEGIN_CHECK_STACK
	bool res = false;
	if (!_GET_THIS())
		return false;
	
	res=GetValueChain(sKey,nVal);
	lua_pop(m_pLS, 1);
	END_CHECK_STACK
	return res;
}

bool CScriptObject::GetValueChain(const char *sKey, HSCRIPTFUNCTION &funcVal)
{
	_GUARD_STACK(m_pLS);
	bool res = false;
	lua_pushstring(m_pLS, sKey);
	GET_FUNCTION(m_pLS, - 2);
	if (lua_isfunction(m_pLS, - 1))
	{
		res = true;
		funcVal =(HSCRIPTFUNCTION)lua_ref(m_pLS,0);
	}
	return res;
}

bool CScriptObject::GetValue(const char *sKey, HSCRIPTFUNCTION &funcVal)
{
	_GUARD_STACK(m_pLS);
		bool res = false;
	if (!_GET_THIS())
		return false;
	res=GetValueChain(sKey,funcVal);
	return res;
}

bool CScriptObject::GetValueChain(const char *sKey, bool &bVal)
{
	_GUARD_STACK(m_pLS);
	int nVal;
	bool res;
	lua_pushstring(m_pLS, sKey);
	GET_FUNCTION(m_pLS, - 2);
	if (lua_isnil(m_pLS, - 1))
	{
		res = true;
		bVal = false;
	}
	else if (lua_isnumber(m_pLS, - 1))
	{
		res = true;
		nVal =(int)lua_tonumber(m_pLS, - 1);
		if (nVal)
			bVal = true;
		else
			bVal = false;
	}
	
	return res;
}

bool CScriptObject::GetValue(const char *sKey, bool &bVal)
{
	_GUARD_STACK(m_pLS);
	bool res = false;
	
	if (!_GET_THIS())
		return false;
	
	res=GetValueChain(sKey,bVal);
	
	
		return res;
}

bool CScriptObject::GetValueChain(const char *sKey, float &fVal)
{
	_GUARD_STACK(m_pLS);
	bool res=false;
	lua_pushstring(m_pLS, sKey);
	GET_FUNCTION(m_pLS, - 2);
	if (lua_isnumber(m_pLS, - 1))
	{
		res = true;
		fVal =(float)lua_tonumber(m_pLS, - 1);
	}
	
	return res;
}

bool CScriptObject::GetValue(const char *sKey, float &fVal)
{
	_GUARD_STACK(m_pLS);
		bool res = false;
	if (!_GET_THIS())
		return false;
	res=GetValueChain(sKey,fVal);
	return res;
}

bool CScriptObject::GetValueChain(const char *sKey, const char* &sVal)
{
	_GUARD_STACK(m_pLS);
	bool res=false;
	lua_pushstring(m_pLS, sKey);
	GET_FUNCTION(m_pLS, - 2);
	if (lua_isstring(m_pLS, - 1))
	{
		res = true;
		sVal =(char *)lua_tostring(m_pLS, - 1);
	}
	
	return res;
}

bool CScriptObject::GetValue(const char *sKey, const char* &sVal)
{
	_GUARD_STACK(m_pLS);
	bool res = false;
	if (!_GET_THIS())
		return false;
	
	res=GetValueChain(sKey,sVal);
	return res;
}

bool CScriptObject::GetValueChain(const char *sKey, IScriptObject *pObj)
{
	_GUARD_STACK(m_pLS);
	bool res=false;
	lua_pushstring(m_pLS, sKey);
	GET_FUNCTION(m_pLS, - 2);
	if (lua_istable(m_pLS, - 1))
	{
		res = true;
		lua_pushvalue(m_pLS, - 1);
		pObj->Attach();
	}
	
	return res;
}

bool CScriptObject::GetValue(const char *sKey, IScriptObject *pObj)
{
	_GUARD_STACK(m_pLS);
	bool res = false;
	if (!_GET_THIS())
		return false;
	res=GetValueChain(sKey,pObj);
	return res;
}

bool CScriptObject::GetValueRecursive( const char *szPath, IScriptObject *pObj )
{
	assert(pObj);
	_GUARD_STACK(m_pLS);

	const char *pSrc=szPath;

	IScriptObject *pCurrent=this;

	pObj->Clone(this);

	while(*pSrc)
	{
		char szInterm[256],*pDst=szInterm;

		while(*pSrc)
			*pDst++=*pSrc++;

		*pDst=0;								// zero termination

		if(!pCurrent->GetValue(szInterm,pObj))
			return false;

		pCurrent=pObj;
	}

	return true;
}


bool CScriptObject::GetUDValueChain(const char *sKey, USER_DATA &nValue, int &nCookie)	//AMD Port
{
	_GUARD_STACK(m_pLS);
	lua_pushstring(m_pLS, sKey);
	GET_FUNCTION(m_pLS, - 2);
	if(lua_isuserdata(m_pLS,-1))
	{
		USER_DATA_CHUNK *udc=(USER_DATA_CHUNK *)lua_touserdata(m_pLS,-1);
		if(!udc)
			return false;
		nValue=udc->nVal;
		nCookie=udc->nCookie;
		return true;
	}
	return false;
}

bool CScriptObject::GetUDValue(const char *sKey, USER_DATA &nValue, int &nCookie)		//AMD Port
{
	_GUARD_STACK(m_pLS);
	bool res = false;
	if (!_GET_THIS())
		return false;
	return GetUDValueChain(sKey,nValue,nCookie);
}

void CScriptObject::SetAt(int nIdx, int nVal)
{
	assert(nIdx>=0);		// nIdx=1 -> first element

	_GUARD_STACK(m_pLS);
	if (!_GET_THIS())
		return;
		
	lua_pushnumber(m_pLS, (lua_Number)nVal);
	lua_rawseti(m_pLS, - 2, nIdx);
}

void CScriptObject::SetAt(int nIdx, float fVal)
{
	assert(nIdx>=0);		// nIdx=1 -> first element

	_GUARD_STACK(m_pLS);
	if (!_GET_THIS())
		return;
		
	lua_pushnumber(m_pLS, fVal);
	lua_rawseti(m_pLS, - 2, nIdx);
}

void CScriptObject::SetAt(int nIdx, bool bVal)
{
	assert(nIdx>=0);		// nIdx=1 -> first element

	_GUARD_STACK(m_pLS);
	if (!_GET_THIS())
		return;
	
	if (bVal)
		lua_pushnumber(m_pLS, 1);
	else
		lua_pushnil(m_pLS);
	lua_rawseti(m_pLS, - 2, nIdx);
}

void CScriptObject::SetAt(int nIdx, const char* sVal)
{
	assert(nIdx>=0);		// nIdx=1 -> first element

	_GUARD_STACK(m_pLS);
	if (!_GET_THIS())
		return;
		
	lua_pushstring(m_pLS, sVal);
	lua_rawseti(m_pLS, - 2, nIdx);
}

void CScriptObject::SetAt(int nIdx, IScriptObject *pObj)
{
	_GUARD_STACK(m_pLS);
	if (!_GET_THIS())
		return;
		
	lua_xgetref(m_pLS, pObj->GetRef());
	lua_rawseti(m_pLS, - 2, nIdx);
}

void CScriptObject::SetAtUD(int nIdx, USER_DATA nVal)
{
	_GUARD_STACK(m_pLS);
	if (!_GET_THIS())
		return;

	if(nVal && lua_getref(m_pLS, nVal))
	{
		if(lua_isuserdata(m_pLS, -1))
		{
			lua_rawseti(m_pLS, -2, nIdx);
		}
	}
}

void CScriptObject::SetNullAt(int nIdx)
{
	_GUARD_STACK(m_pLS);
	if (!_GET_THIS())
		return;
		
	lua_pushnil(m_pLS);
	lua_rawseti(m_pLS, - 2, nIdx);
}

int CScriptObject::Count()
{
	_GUARD_STACK(m_pLS);
	int top = lua_gettop(m_pLS);
	
	int nCount=0;
	if (!_GET_THIS())
		return -1;
		
	int trgTable = top + 1;
		
	lua_pushnil(m_pLS);  // first key
	while (lua_next(m_pLS, trgTable) != 0) 
	{
		// `key' is at index -2 and `value' at index -1
		nCount++;
		lua_pop(m_pLS, 1); // pop value, leave index.
	}
	//lua_settop(m_pLS, top); // Restore stack.
	return nCount;
}

bool CScriptObject::GetAt(int nIdx, int &nVal)
{
	_GUARD_STACK(m_pLS);
		bool res = false;
	if (!_GET_THIS())
		return false;
	
	if (lua_getn(m_pLS, - 1) < nIdx)
	{
		return false;
	}
		
	lua_rawgeti(m_pLS, - 1, nIdx);
	if (lua_isnumber(m_pLS, - 1))
	{
		res = true;
		nVal =(int)lua_tonumber(m_pLS, - 1);
	}
	return res;
}

bool CScriptObject::GetAt(int nIdx, float &fVal)
{
	_GUARD_STACK(m_pLS);
		bool res = false;
	if (!_GET_THIS())
		return false;
	
	if (lua_getn(m_pLS, - 1) < nIdx)
	{
		return false;
	}
	
	lua_rawgeti(m_pLS, - 1, nIdx);
	if (lua_isnumber(m_pLS, - 1))
	{
		res = true;
		fVal = lua_tonumber(m_pLS, - 1);
	}
	return res;
}

bool CScriptObject::GetAt(int nIdx, bool &bVal)
{
	_GUARD_STACK(m_pLS);
		bool res = false;
	int nVal;
	if (!_GET_THIS())
		return false;
	
	if (lua_getn(m_pLS, - 1) < nIdx)
	{
		return false;
	}	
	
	lua_rawgeti(m_pLS, - 1, nIdx);
	if (lua_isnumber(m_pLS, - 1))
	{
		res = true;
		nVal =(int)lua_tonumber(m_pLS, - 1);
		if (nVal)
			bVal = true;
		else
			bVal = false;
	}
	return res;
}

bool CScriptObject::GetAt(int nIdx, const char* &sVal)
{
	_GUARD_STACK(m_pLS);
	bool res = false;
	if (!_GET_THIS())
		return false;
	
	if (lua_getn(m_pLS, - 1) < nIdx)
	{
		return false;
	}
	
	lua_rawgeti(m_pLS, - 1, nIdx);
	if (lua_isstring(m_pLS, - 1))
	{
		res = true;
		sVal =(char *)lua_tostring(m_pLS, - 1);
	}
	return res;
}

bool CScriptObject::GetAt(int nIdx, IScriptObject *pObj)
{
	_GUARD_STACK(m_pLS);
	bool res = false;
	if (!_GET_THIS())
		return false;
	
	if (lua_getn(m_pLS, - 1) < nIdx)
	{
		return false;
	}	
	
	lua_rawgeti(m_pLS, - 1, nIdx);
	if (lua_istable(m_pLS, - 1))
	{
		res = true;
		lua_pushvalue(m_pLS, - 1);
		pObj->Attach();
		
	}
	return res;
}

bool CScriptObject::GetAtUD(int nIdx, USER_DATA &nVal, int &nCookie)
{
	_GUARD_STACK(m_pLS);
	bool res = false;
	if (!_GET_THIS())
		return false;

	if (lua_getn(m_pLS, -1) < nIdx)
	{
		return false;
	}	

	lua_rawgeti(m_pLS, -1, nIdx);
	if (lua_isuserdata(m_pLS, - 1))
	{
		USER_DATA_CHUNK *udc=(USER_DATA_CHUNK *)lua_touserdata(m_pLS,-1);
		if(!udc)
			return false;
		nVal=udc->nVal;
		nCookie=udc->nCookie;
		return true;
	}

	return res;
}

/*void CScriptObject::SetThis(THIS_PTR pThis)
{
	//m_pThis = pThis;
}

THIS_PTR CScriptObject::GetThis()
{
	THIS_PTR res = NULL;
	return m_pThis;
}*/

bool CScriptObject::AddFunction(const char *sName, SCRIPT_FUNCTION pThunk, int nFuncID)
{
	_GUARD_STACK(m_pLS);
	if (!_GET_THIS())
		return false;
		
	lua_pushstring(m_pLS, sName);
	lua_pushnumber(m_pLS, (lua_Number)nFuncID);
	//lua_newuserdatabox(m_pLS, m_pThis);
	lua_pushcclosure(m_pLS, (lua_CFunction)pThunk, 1);
	SET_FUNCTION(m_pLS, -3);
	return true;
}

bool CScriptObject::AddSetGetHandlers(SCRIPT_FUNCTION pSetThunk,SCRIPT_FUNCTION pGetThunk)
{

	if((!m_pSetGetParams) && pSetThunk && pGetThunk && m_hDelegationTag)
	{
		m_pSetGetParams=new CScriptObject::SetGetParams;
		//int nTag = lua_newtag(m_pLS);
		lua_newuserdatabox(m_pLS, this);
		//lua_newuserdatabox(m_pLS, m_pThis);
		lua_pushcclosure(m_pLS, CScriptObject::SetTableTagHandler, 1);
		lua_settagmethod(m_pLS, m_hDelegationTag, "settable");
/*		lua_newuserdatabox(m_pLS, this);
		//lua_newuserdatabox(m_pLS, m_pThis);
		lua_pushcclosure(m_pLS, CScriptObject::GetTableTagHandler, 1);
		lua_settagmethod(m_pLS, nTag, "gettable");*/

		if (!_GET_THIS())
		{
			lua_pushnil(m_pLS);
			lua_settagmethod(m_pLS, m_hDelegationTag, "settable");
			/*lua_pushnil(m_pLS);
			lua_settagmethod(m_pLS, nTag, "gettable");;*/
			return false;	
		}

		lua_settag(m_pLS, m_hDelegationTag);
		lua_pop(m_pLS,1);

		m_pSetGetParams->m_pSetThunk=pSetThunk;
		m_pSetGetParams->m_pGetThunk=pGetThunk;
		m_pSetGetParams->m_hSetGetTag=m_hDelegationTag;
	}
	else
	{
		if(m_pSetGetParams)
		{
			if(m_pSetGetParams->m_hSetGetTag)
			{
				lua_pushnil(m_pLS);
				lua_settagmethod(m_pLS, m_pSetGetParams->m_hSetGetTag, "settable");
				/*lua_pushnil(m_pLS);
				lua_settagmethod(m_pLS, m_pSetGetParams->m_hSetGetTag, "gettable");;*/
				m_pSetGetParams->m_hSetGetTag=NULL;
			}
			delete m_pSetGetParams;
			m_pSetGetParams=NULL;
		}
	}
	return true;
}
#ifdef PS2
#define FIXME_ASSERT(cond) { if(!(cond)) { FORCE_EXIT() } }
#else

#ifdef _DEBUG
#if defined(WIN64) || defined(LINUX64)
#define FIXME_ASSERT(cond) assert (cond)
#else
#define FIXME_ASSERT(cond) { if(!(cond)) { DEBUG_BREAK; } }
#endif
#else
//Timur[2/20/2003]
//@FIXME Should be removed in gold Release.
// Debugging mod
#define FIXME_ASSERT(cond)  { if(!(cond)) { CryError( "<ScriptSystem> ASSERT %s",#cond ); }}
#endif

#endif
int CScriptObject::GetTableTagHandler(lua_State *L)
{
	CScriptObject *pThis = (CScriptObject *)lua_touserdata(L, - 1);
	int nRet;
	FIXME_ASSERT(pThis->m_pSetGetParams->m_pGetThunk!=NULL);
	/*{
		//lua_pop(L,2);
		FIXME_ASSERT(lua_istable(L,1));
		lua_pop(L,1);
		lua_rawget(L,1);
		
		return 1;
	}*/
	
	if(((nRet=pThis->m_pSetGetParams->m_pGetThunk((HSCRIPT)L))==-1))
	{
		//lua_pop(L,2);
		FIXME_ASSERT(lua_istable(L,1));
		lua_pop(L,1);
		lua_rawget(L,1);
		return 1;
	}
	else return nRet;
}

int CScriptObject::SetTableTagHandler(lua_State *L)
{
	CScriptObject *pThis = (CScriptObject *)lua_touserdata(L, - 1);
	int nRet;
	FIXME_ASSERT(pThis->m_pSetGetParams->m_pGetThunk!=NULL);
	/*if(!pThis->m_pSetThunk)
	{
		FIXME_ASSERT(lua_istable(L,1));
		lua_pop(L,1);
		lua_rawset(L,1);
		return 0;
	}*/
	if( /*!lua_isfunction(L,-2) && !lua_iscfunction(L,-2) && !lua_isuserdata(L,-2) &&*/ (!((nRet=pThis->m_pSetGetParams->m_pSetThunk((HSCRIPT)L))==-1)))
	{
		FIXME_ASSERT(lua_istable(L,1));
		lua_pop(L,1);
	}
	else{
		FIXME_ASSERT(lua_istable(L,1));
		lua_pop(L,1);
		lua_rawset(L,1);
		return 0;
	}
	return 0;
}

void CScriptObject::RegisterParent(IScriptObjectSink *pSink)
{
	m_pSink = pSink;
}

bool CScriptObject::Clone(IScriptObject *pObj)
{	
	//BEGIN_CHECK_STACK
	_GUARD_STACK(m_pLS);
	int top = lua_gettop(m_pLS);
	
	if (!lua_xgetref(m_pLS, pObj->GetRef()))
		return false;
	if (!_GET_THIS())
		return false;
	
	int srcTable = top + 1;
	int trgTable = top + 2;
	
	lua_pushnil(m_pLS);  // first key
	while (lua_next(m_pLS, srcTable) != 0) 
	{
		// `key' is at index -2 and `value' at index -1
		lua_pushvalue(m_pLS, - 2); // Push again index.
		lua_pushvalue(m_pLS, - 2); // Push value.
		SET_FUNCTION(m_pLS, trgTable);
		lua_pop(m_pLS, 1); // pop value, leave index.
	}
	//lua_settop(m_pLS, top); // Restore stack.
	//END_CHECK_STACK
	
		return true;
}



void CScriptObject::Dump(IScriptObjectDumpSink *p)
{
	if(!p)return;
	_GUARD_STACK(m_pLS);
	int top = lua_gettop(m_pLS);
	
	if (!_GET_THIS())
		return;
	
	
	int trgTable = top + 1;
	
	
	lua_pushnil(m_pLS);  // first key
	int reftop=lua_gettop(m_pLS);
	while (lua_next(m_pLS, trgTable) != 0) 
	{
		// `key' is at index -2 and `value' at index -1
		if(lua_isnumber(m_pLS, - 2))
		{
			int nIdx = (int)lua_tonumber(m_pLS, - 2); // again index
			if (lua_isnil(m_pLS, - 1))
			{
				p->OnElementFound(nIdx, svtNull);
			}
			else if (lua_isfunction(m_pLS, - 1))
			{
				p->OnElementFound(nIdx, svtFunction);
			}
			else if (lua_isnumber(m_pLS, - 1))
			{
				p->OnElementFound(nIdx, svtNumber);
			}
			else if (lua_isstring(m_pLS, - 1))
			{
				p->OnElementFound(nIdx, svtString);
			}
			else if (lua_istable(m_pLS, - 1))
			{
				p->OnElementFound(nIdx, svtObject);
			}
			else if (lua_isuserdata(m_pLS, - 1))
			{
				p->OnElementFound(nIdx, svtUserData);
			}
		}
		else
		{
			const char *sName = lua_tostring(m_pLS, - 2); // again index
			if (lua_isnil(m_pLS, - 1))
			{
				p->OnElementFound(sName, svtNull);
			}
			else if (lua_isfunction(m_pLS, - 1))
			{
				p->OnElementFound(sName, svtFunction);
			}
			else if (lua_isnumber(m_pLS, - 1))
			{
				p->OnElementFound(sName, svtNumber);
			}
			else if (lua_isstring(m_pLS, - 1))
			{
				p->OnElementFound(sName, svtString);
			}
			else if (lua_istable(m_pLS, - 1))
			{
				p->OnElementFound(sName, svtObject);
			}
			else if (lua_isuserdata(m_pLS, - 1))
			{
				p->OnElementFound(sName, svtUserData);
			}
		}
		lua_settop(m_pLS, reftop); // pop value, leave index.
	}
	//lua_settop(m_pLS, top); // Restore stack.
	//END_CHECK_STACK
}

ScriptVarType CScriptObject::GetValueType(const char *sKey)
{
	_GUARD_STACK(m_pLS);
	ScriptVarType svtRetVal=svtNull;
	if (!_GET_THIS())
		return svtNull;
	
	lua_pushstring(m_pLS, sKey);
	GET_FUNCTION(m_pLS, - 2);

	if (lua_isnil(m_pLS, - 1))
	{
		svtRetVal=svtNull;
	}
	else if (lua_isfunction(m_pLS, - 1))
	{
		svtRetVal=svtFunction;
	}
	else if (lua_isnumber(m_pLS, - 1))
	{
		svtRetVal=svtNumber;
	}
	else if (lua_isstring(m_pLS, - 1))
	{
		svtRetVal=svtString;
	}
	else if (lua_istable(m_pLS, - 1))
	{
		svtRetVal=svtObject;
	}
	else if (lua_isuserdata(m_pLS, - 1))
	{
		svtRetVal=svtUserData;
	}

	//lua_pop(m_pLS, 2);
	//END_CHECK_STACK
	return svtRetVal;
}

ScriptVarType CScriptObject::GetAtType(int nIdx)
{
	_GUARD_STACK(m_pLS);
	ScriptVarType svtRetVal=svtNull;
	if (!_GET_THIS())
		return svtNull;
	
	if (lua_getn(m_pLS, - 1) < nIdx)
	{
		//lua_pop(m_pLS, 1);
		return svtNull;
	}
	
	lua_rawgeti(m_pLS, - 1, nIdx);

	if (lua_isnil(m_pLS, - 1))
	{
		svtRetVal=svtNull;
	}
	else if (lua_isfunction(m_pLS, - 1))
	{
		svtRetVal=svtFunction;
	}
	else if (lua_isnumber(m_pLS, - 1))
	{
		svtRetVal=svtNumber;
	}
	else if (lua_isstring(m_pLS, - 1))
	{
		svtRetVal=svtString;
	}
	else if (lua_istable(m_pLS, - 1))
	{
		svtRetVal=svtObject;
	}

	//lua_pop(m_pLS, 2);
	//END_CHECK_STACK
	return svtRetVal;
}

void CScriptObject::Detach()
{
	if (m_bAttached)
	{
		lua_xunref(m_pLS, m_nRef);
		m_bAttached = false;
	}
}

void CScriptObject::Release()
{
	
	AddSetGetHandlers(NULL,NULL);
	if (m_pSink)
	{
		m_pSink->OnRelease();
		if(m_nRef)
		{
			if (_GET_THIS())
			{
				lua_setnativedata(m_pLS,-1,NULL);
				lua_pop(m_pLS,1);
			}
		}
	}
	m_pSink = NULL;
	Detach();
	//m_hSetGetTag=NULL;
	CScriptSystem *pScriptSystem=(CScriptSystem *)lua_getuserptr(m_pLS);
	pScriptSystem->ReleaseScriptObject(this);
	m_bDeleted = true;
	//delete this;
}

bool CScriptObject::BeginIteration()
{
	if(m_nIterationCounter!=-1)
		return false;

	m_nIterationCounter=lua_gettop(m_pLS);
	if (!_GET_THIS())
		return false;
	lua_pushnil(m_pLS);
	return true;
}

bool CScriptObject::MoveNext()
{
	if(m_nIterationCounter==-1)
		return false;
  //leave only the index into the stack
	while((lua_gettop(m_pLS)-(m_nIterationCounter+1))>1)
	{
		lua_pop(m_pLS,1);
	}
	return (lua_next(m_pLS, m_nIterationCounter+1) != 0);
}

bool CScriptObject::GetCurrent(int &nVal)
{
	if(m_nIterationCounter==-1)
		return false;
	if (lua_isnumber(m_pLS, -1))
	{
		nVal =(int)lua_tonumber(m_pLS, -1);
		return true;
	}
	return false;
}

bool CScriptObject::GetCurrent(float &fVal)
{
	if(m_nIterationCounter==-1)
		return false;
	if (lua_isnumber(m_pLS, -1))
	{
		fVal =lua_tonumber(m_pLS, -1);
		return true;
	}
	return false;
}

bool CScriptObject::GetCurrent(bool &bVal)
{
	if(m_nIterationCounter==-1)
		return false;
	bool res=false;
	if (lua_isnil(m_pLS, - 1))
	{
		res = true;
		bVal = false;
	}
	else if (lua_isnumber(m_pLS, - 1))
	{
		res = true;
		int nVal =(int)lua_tonumber(m_pLS, - 1);
		if (nVal)
			bVal = true;
		else
			bVal = false;
	}
	return res;
}

bool CScriptObject::GetCurrent(const char* &sVal)
{
	if(m_nIterationCounter==-1)
		return false;
	if (lua_isstring(m_pLS, - 1))
	{
		sVal =(char *)lua_tostring(m_pLS, - 1);
		return true;
	}
	return false;
}


bool CScriptObject::GetCurrentPtr(const void * &pObj)
{
	if(m_nIterationCounter==-1)
		return false;

	pObj=lua_topointer(m_pLS, - 1);

	return pObj!=0;
}


bool CScriptObject::GetCurrent(IScriptObject *pObj)
{
	if(m_nIterationCounter==-1)
		return false;
	bool res=false;
	if (lua_istable(m_pLS, -1))
	{
		res = true;
		lua_pushvalue(m_pLS, -1);
		pObj->Attach();
	}
	return res;
}


// MartinM, used to create a hash value out of a lua function (for cheat protection)
bool CScriptObject::GetCurrentFuncData(unsigned int * &pCode, int &iSize)
{
	if(m_nIterationCounter==-1)
		return false;
	if (lua_isfunction(m_pLS, -1))
	{
		lua_getluafuncdata(m_pLS,-1,&pCode,&iSize);
		return true;
	}
	return false;
}


// MartinM, used to create a hash value out of a lua function (for cheat protection)
bool CScriptObject::GetFuncData(const char *sKey, unsigned int * &pCode, int &iSize)
{
	_GUARD_STACK(m_pLS);
	bool res = false;
	lua_pushstring(m_pLS, sKey);
	GET_FUNCTION(m_pLS, - 2);

	if (lua_isfunction(m_pLS, -1))
	{
		lua_getluafuncdata(m_pLS,-1,&pCode,&iSize);
		return true;
	}
	return false;
}


bool CScriptObject::GetCurrentKey(const char* &sKey)
{
	if(m_nIterationCounter==-1)
		return false;

//	if (lua_isstring(m_pLS, - 2))
  if(lua_rawtag(m_pLS, -2)==LUA_TSTRING)			// get the internal type without converting it
	{
		sKey=(char *)lua_tostring(m_pLS, - 2);
		return true;
	}
	return false;
}

bool CScriptObject::GetCurrentKey(int &nKey)
{
	if(m_nIterationCounter==-1)
		return false;

//	if(lua_isnumber(m_pLS,-2))
  if(lua_rawtag(m_pLS, -2)==LUA_TNUMBER)			// get the internal type without converting it
	{
		nKey =(int)lua_tonumber(m_pLS, -2);
		return true;
	}
	return false;
}


void CScriptObject::Attach(IScriptObject *so)
{
	if(!lua_xgetref(m_pLS,so->GetRef()))
	{
		
		return;
	}
	Attach();
}

void CScriptObject::EndIteration()
{
	if(m_nIterationCounter==-1)
	{
		//Timur[2/20/2003]
		// This is invalid call, fire warning.
		FIXME_ASSERT(0);
		return;
	}

	lua_settop(m_pLS,m_nIterationCounter);
	m_nIterationCounter=-1;
}

ScriptVarType CScriptObject::GetCurrentType()
{
	ScriptVarType svtRetVal=svtNull;

	if (lua_isnil(m_pLS, - 1))
	{
		svtRetVal=svtNull;
	}
	else if (lua_isfunction(m_pLS, - 1))
	{
		svtRetVal=svtFunction;
	}
	else if (lua_isnumber(m_pLS, - 1))
	{
		svtRetVal=svtNumber;
	}
	else if (lua_isstring(m_pLS, - 1))
	{
		svtRetVal=svtString;
	}
	else if (lua_isuserdata(m_pLS, - 1))
	{
		svtRetVal=svtUserData;
	}
	else if (lua_istable(m_pLS, - 1))
	{
		svtRetVal=svtObject;
	}
	return svtRetVal;	
}

void CScriptObject::Clear()
{
	_GUARD_STACK(m_pLS);
	int top = lua_gettop(m_pLS);
	
	if (!_GET_THIS())
		return;

	int trgTable = top + 1;
		
	lua_pushnil(m_pLS);  // first key
	while (lua_next(m_pLS, trgTable) != 0) 
	{
			lua_pop(m_pLS, 1); // pop value, leave index.
			lua_pushvalue(m_pLS, -1); // Push again index.
			lua_pushnil(m_pLS);
			lua_rawset(m_pLS,trgTable);
	}
//	lua_settop(m_pLS, top); // Restore stack.
//	END_CHECK_STACK
}

void CScriptObject::SetNativeData(void *nd)
{
	_GUARD_STACK(m_pLS);
	if (!_GET_THIS())
		return;
	lua_setnativedata(m_pLS,-1,nd);
}

void *CScriptObject::GetNativeData()
{
	_GUARD_STACK(m_pLS);
	if (!_GET_THIS())
		return NULL;
	return lua_getnativedata(m_pLS,-1);
}

int CScriptObject::IndexTagHandler(lua_State *L)
{
	CScriptObject *p=(CScriptObject *)lua_touserdata(L,-2);
	int nRet=0;
	if(p && p->m_pSetGetParams)
	{
		if (p->m_bDeleted)
		{
			CryError( "<CScriptObject::IndexTagHandler> Access to deleted script object" );
		}
		if((nRet=p->m_pSetGetParams->m_pGetThunk((HSCRIPT)L))!=-1)
		{
			return nRet;
		}
		//lua_pop(L,1);
	}

	if(lua_istable(L,4))
	{
		if(!lua_getnativedata(L,1))return 0;
		lua_pushvalue(L,2);
		lua_rawget(L,-2);
		lua_remove(L,-2);
		return 1;
	}
	return 0;
}

void CScriptObject::Delegate(IScriptObject *pObj)
{
	if(!pObj)return;
	m_hDelegationTag = lua_newtag(m_pLS);
	lua_newuserdatabox(m_pLS, this);
	if(!lua_xgetref(m_pLS,pObj->GetRef()))return;
	lua_pushcclosure(m_pLS, CScriptObject::IndexTagHandler, 2);
	lua_settagmethod(m_pLS, m_hDelegationTag, "index");
	if (!_GET_THIS())
	{
		lua_pushnil(m_pLS);
		lua_settagmethod(m_pLS, m_hDelegationTag, "index");
		return ;
	}

	lua_settag(m_pLS, m_hDelegationTag);
	lua_pop(m_pLS,1);
}