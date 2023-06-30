// ScriptSystem.cpp: implementation of the CScriptSystem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <string.h>
#include <stdio.h>
#include "ScriptSystem.h"
#include "ScriptObject.h"
#include "RecycleAllocator.h"
#include "StackGuard.h"

#include <ISystem.h> // For warning and errors.
// needed for crypak
#include <ICryPak.h>
#include <IConsole.h>
#include <ILog.h>
#include <IDataProbe.h>

extern "C"
{
#define LUA_PRIVATE
#include "lua.h"
#include "lualib.h"
#include "luadebug.h"
}

#if defined(_DEBUG) && !defined(LINUX)
#include <crtdbg.h>
#define DEBUG_CLIENTBLOCK new( _NORMAL_BLOCK, __FILE__, __LINE__) 
#define new DEBUG_CLIENTBLOCK
#endif

#include "LuaCryPakIo.h"

//#ifndef WIN64 // experimental
#define USE_RAW_CALL
//#endif

#ifdef _DEBUG

#define BEGIN_CHECK_STACK \
	int __nStack = lua_stackspace(m_pLS);

/*
#define END_CHECK_STACK \
{			\
	char sTemp[120];	\
if (__nStack != lua_stackspace(m_pLS)) \
{DEBUG_BREAK;}	\
sprintf(sTemp, "STACK=%d\n", __nStack); \
::OutputDebugString(sTemp);	\
}
*/

#define END_CHECK_STACK \
{			\
	if (__nStack != lua_stackspace(m_pLS)) \
	{CryError( "<ScriptSystem> END_CHECK_STACK Failed." );}	\
}



#else

#define BEGIN_CHECK_STACK
#define END_CHECK_STACK

#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
static int I = 0;
extern "C" int g_dumpStackOnAlloc = 0; // used in .c file.

//static int64 g_numScriptSystemValidations = 0;

inline void CScriptSystem::Validate()
{
//#if defined(WIN64) && defined(_DEBUG)
	//++g_numScriptSystemValidations;
	//assert ((g_numScriptSystemValidations <135000) || IsHeapValid());
//#endif
}


/*LUA_API void alberto_pushfunc(lua_State *L, HSCRIPTFUNCTION func) 
{
L->top->value.cl =(struct Closure *) func;
ttype(L->top) = LUA_TFUNCTION;
incr_top;//(L);
}*/
IScriptObject *CScriptSystem::GetLocalVariables(int nLevel)
{
	Validate();
	IScriptObject *pObj;
	lua_Debug ar;
	nLevel=0;
	const char *name;
	lua_newtable(m_pLS);
	int nTable=lua_ref(m_pLS,1);
	lua_getref(m_pLS,nTable);
	pObj=CreateEmptyObject();
	pObj->Attach();
	while(lua_getstack(m_pLS, nLevel, &ar) != 0)
	{
		//return 0; /* failure: no such level in the stack */

		//create a table and fill it with the local variables (name,value)

		int i = 1;
		while ((name = lua_getlocal(m_pLS, &ar, i)) != NULL) 
		{
			lua_getref(m_pLS,nTable);
			lua_pushstring(m_pLS,name);
			//::OutputDebugString(name);
			//::OutputDebugString("\n");
			lua_pushvalue(m_pLS,-3);
			lua_rawset(m_pLS,-3);
			//pop table and value
			lua_pop(m_pLS,2);
			i++;
		}
		nLevel++;
	}
	
	Validate();
	return pObj;
}

#define LEVELS1	12	/* size of the first part of the stack */
#define LEVELS2	10	/* size of the second part of the stack */

IScriptObject *CScriptSystem::GetCallsStack()
{
	Validate();
	IScriptObject *pStackTrace=CreateObject();
	int level = 0; 
	int firstpart = 1;  /* still before eventual `...' */
	lua_Debug ar;
	while (lua_getstack(m_pLS, level++, &ar)) {

		l_char buff[120];  /* enough to fit following `sprintf's */
		if (level == 2)
		{
			//luaL_addstring(&b, l_s("stack traceback:\n"));
		}
		else if (level > LEVELS1 && firstpart)
		{
			/* no more than `LEVELS2' more levels? */
			if (!lua_getstack(m_pLS, level+LEVELS2, &ar))
				level--;  /* keep going */
			else {
				//		luaL_addstring(&b, l_s("       ...\n"));  /* too many levels */
				while (lua_getstack(m_pLS, level+LEVELS2, &ar))  /* find last levels */
					level++;
			}
			firstpart = 0;
			continue;
		}

		IScriptObject *pEntry=CreateObject();
		sprintf(buff, l_s("%4d:  "), level-1);

		lua_getinfo(m_pLS, l_s("Snl"), &ar);
		switch (*ar.namewhat)
		{
		case 'g':  case 'l':  /* global, local */
			sprintf(buff, "function `%.50s'", ar.name);
			break;
		case 'f':  /* field */
			sprintf(buff, "method `%.50s'", ar.name);
			break;
		case 't':  /* tag method */
			sprintf(buff, "`%.50s' tag method", ar.name);
			break;
		default: {
			if (*ar.what == 'm')  /* main? */
				sprintf(buff, "main of %.70s", ar.short_src);
			else if (*ar.what == 'C')  /* C function? */
				sprintf(buff, "%.70s", ar.short_src);
			else
				sprintf(buff, "function <%d:%.70s>", ar.linedefined, ar.short_src);
						 }
		}

		pEntry->SetValue("description",buff);
		pEntry->SetValue("line",ar.currentline);
		if (ar.source) {
			pEntry->SetValue("sourcefile",ar.source);
		}
		pStackTrace->PushBack(pEntry);
		pEntry->Release();

	}
	Validate();
	return pStackTrace;
}

int listvars(lua_State *L, int level) 
{
	CScriptSystem::Validate();
	char sTemp[1000];
	lua_Debug ar;
	int i = 1;
	const char *name;
	if (lua_getstack(L, level, &ar) == 0)
		return 0; /* failure: no such level in the stack */
	while ((name = lua_getlocal(L, &ar, i)) != NULL) 
	{
		sprintf(sTemp, "%s =", name);
		OutputDebugString(sTemp);
		
		
		if (lua_isnumber(L, i))
		{
			int n = (int)lua_tonumber(L, i);
			itoa(n, sTemp, 10);
			OutputDebugString(sTemp);
		}
		else if (lua_isstring(L, i))
		{
			OutputDebugString(lua_tostring(L, i));
		}else
			if (lua_isnil(L, i))
			{
				OutputDebugString("nil");
			}
			else
			{
				OutputDebugString("<<unknown>>");
			}
			OutputDebugString("\n");
			i++;
			lua_pop(L, 1); /* remove variable value */		
	}
	/*lua_getglobals(L);
	i = 1;
	while ((name = lua_getlocal(L, &ar, i)) != NULL) 
	{
		sprintf(sTemp, "%s =", name);
		OutputDebugString(sTemp);
		
		
		if (lua_isnumber(L, i))
		{
			int n = (int)lua_tonumber(L, i);
			itoa(n, sTemp, 10);
			OutputDebugString(sTemp);
		}
		else if (lua_isstring(L, i))
		{
			OutputDebugString(lua_tostring(L, i));
		}else
			if (lua_isnil(L, i))
			{
				OutputDebugString("nil");
			}
			else
			{
				OutputDebugString("<<unknown>>");
			}
			OutputDebugString("\n");
			i++;
			lua_pop(L, 1); 
	}*/
	
	CScriptSystem::Validate();
	return 1;
}

static void callhook(lua_State *L, lua_Debug *ar)
{
	CScriptSystem *pSS=(CScriptSystem *)lua_getuserptr(L);
	if(pSS->m_bsBreakState!=bsStepInto)return;
	lua_getinfo(L, "Sl", ar);
	ScriptDebugInfo sdi;
	pSS->m_sLastBreakSource = sdi.sSourceName = ar->source;
	pSS->m_nLastBreakLine =	sdi.nCurrentLine = ar->currentline;
	pSS->m_pDebugSink->OnExecuteLine(sdi);
}

static void linehook(lua_State *L, lua_Debug *ar)
{
	CScriptSystem *pSS=(CScriptSystem *)lua_getuserptr(L);

	if(pSS->m_bsBreakState!=bsNoBreak)
	{
		switch(pSS->m_bsBreakState)
		{
		case bsContinue:
			if(pSS->m_BreakPoint.nLine==ar->currentline)
			{
					lua_getinfo(L, "Sl", ar);
					if(ar->source)
					{
						if(stricmp(ar->source,pSS->m_BreakPoint.sSourceFile.c_str())==0)
							break;
					}
			}
			return;
		case bsStepNext:
		case bsStepInto:
			if(pSS->m_BreakPoint.nLine!=ar->currentline)
			{
				lua_getinfo(L, "Sl", ar);
				if((stricmp(pSS->m_sLastBreakSource.c_str(),ar->source)==0)){
					break;
				}
			}
			return;
		
			/*lua_getinfo(L, "S", ar);
			if(ar->source)
			{
				if(pSS->m_BreakPoint.nLine!=ar->currentline && (stricmp(pSS->m_sLastBreakSource.c_str(),ar->source)!=0))
				break;
			}
			return;*/
		default:
			return;
		};
		ScriptDebugInfo sdi;
		pSS->m_sLastBreakSource = sdi.sSourceName = ar->source;
		pSS->m_nLastBreakLine =	sdi.nCurrentLine = ar->currentline;
		pSS->m_pDebugSink->OnExecuteLine(sdi);
	}
 		
}

string& FormatPath( const string &sPath )
{
	static string strTemp;
	static char sLowerName[300];
	strcpy(sLowerName, sPath.c_str());
	int i = 0;
	while (sLowerName[i] != 0)
	{
		if (sLowerName[i] == '\\')
			sLowerName[i] = '/';
		i++;
	}
	strTemp = _strlwr(sLowerName);
	return strTemp;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
CScriptSystem::CScriptSystem()
{
	m_nObjCreationNumber=1;
	m_pLS = NULL;
	m_bDebug = false;
	m_pSink = NULL;
	m_pDebugSink = NULL;
	m_nGCTag=0;
	m_bsBreakState=bsNoBreak;
	m_BreakPoint.nLine=0;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
CScriptSystem::~CScriptSystem()
{	
/*	while (!m_stkPooledWeakObjects.empty())
	{
		delete m_stkPooledWeakObjects.top();
		m_stkPooledWeakObjects.pop();
	}
	
	while (!m_stkPooledWeakObjectsEmpty.empty())
	{
		delete m_stkPooledWeakObjectsEmpty.top();
		m_stkPooledWeakObjectsEmpty.pop();
	}
	*/
	if (m_pLS)
	{
		lua_close(m_pLS); 
		
		m_pLS = NULL;
	}
	
	#ifdef _DEBUG
	recycle_cleanup();
	#endif

	for (int i = 0; i <m_stkScriptObjectsPool.size(); i++)
	{
		delete m_stkScriptObjectsPool[i];
	}
	m_stkScriptObjectsPool.clear();
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
extern "C" {
	int vl_initvectorlib(lua_State *L);
}

bool CScriptSystem::Init(IScriptSystemSink *pSink, IScriptDebugSink *pDebugSink, bool bStdLibs, int nStackSize)
{
	m_pSink = pSink;
	m_pLS = lua_open(0);
	if (pDebugSink)
	{
		m_bDebug = true;
		m_pDebugSink = pDebugSink;
		}
	else
	{
		m_pDebugSink = NULL;
		m_bDebug = false;
	}
	if (m_pLS && m_bDebug)
	{
		lua_dblibopen(m_pLS);
		lua_setlinehook(m_pLS, linehook);
		lua_setcallhook(m_pLS, callhook);
	}
	if (bStdLibs)
	{
		lua_baselibopen(m_pLS);
		lua_strlibopen(m_pLS);
		lua_mathlibopen(m_pLS);
		lua_iolibopen(m_pLS);
		lua_bitlibopen(m_pLS);
	}
	vl_initvectorlib(m_pLS);
	lua_setuserptr(m_pLS,this);
	lua_iolibopen(m_pLS);
	RegisterErrorHandler(m_bDebug);
	RegisterTagHandlers();

	//initvectortag(m_pLS);
	return m_pLS?true:false;
}

void CScriptSystem::PostInit()
{
	//////////////////////////////////////////////////////////////////////////
	// Register console vars.
	//////////////////////////////////////////////////////////////////////////
	if (GetISystem()->GetIConsole())
	{
		GetISystem()->GetIConsole()->Register( "lua_stackonmalloc",&g_dumpStackOnAlloc,0 );
	}
}

//////////////////////////////////////////////////////////////////////////
void CScriptSystem::EnableDebugger(IScriptDebugSink *pDebugSink)
{
	m_pDebugSink = pDebugSink;
	if(pDebugSink)
	{
		m_bDebug=true;
		lua_dblibopen(m_pLS);
		lua_setlinehook(m_pLS, linehook);
		lua_setcallhook(m_pLS, callhook);
		lua_setuserptr(m_pLS,this);
		RegisterErrorHandler(true);
	}
	else
	{
		m_bDebug=false;
		lua_setlinehook(m_pLS, NULL);
		lua_setcallhook(m_pLS, NULL);
		RegisterErrorHandler(false);
	}
}

//////////////////////////////////////////////////////////////////////////
extern "C" void DumpCallStack( lua_State *L )
{
	lua_Debug ar;
	int nRes;
	int iCurrentLine=-1;									// no line number info

	memset(&ar,0,sizeof(lua_Debug));

	static int counter = 0;
	counter++;
	int level = 0;
	GetISystem()->GetILog()->Log( "\001--- Lua Call Stack Trace N%d\n",counter );
	while (lua_getstack(L, level++, &ar))
	{
		nRes = lua_getinfo(L, "lnS", &ar);
		iCurrentLine=ar.currentline;
		GetISystem()->GetILog()->Log( "\001%s, %s (%d)\n",ar.name,ar.source,ar.currentline );
	}
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
int CScriptSystem::ErrorHandler(lua_State *L)
{
	const char *sFuncName = NULL;
	const char *sSourceFile = NULL;
	lua_Debug ar;
	int nRes;
	int iCurrentLine=-1;									// no line number info
	CScriptSystem *pThis = (CScriptSystem *)lua_touserdata(L, - 1);

	memset(&ar,0,sizeof(lua_Debug));
	
	lua_pop(L, 1);
	if (lua_isstring(L, 1))
	{
		const char *sErr = lua_tostring(L, 1);
		nRes = lua_getstack(L, 1, &ar);
		
		if (nRes != 0)
		{
			nRes = lua_getinfo(L, "lnS", &ar);
			iCurrentLine=ar.currentline;
			sFuncName = ar.name;
			sSourceFile = ar.source;
		}
		if (!sFuncName) 
			sFuncName = "undefined";
		if (!sSourceFile)
			sSourceFile= "undefined";
		
		pThis->m_pSink->OnScriptError(sSourceFile, sFuncName, iCurrentLine, sErr);
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
int CScriptSystem::SetGlobalTagHandlerString(lua_State *L)
{
	CScriptSystem *pThis = (CScriptSystem *)lua_touserdata(L, - 1);
	const char *sName = lua_tostring(L, 1);
	
	if (!pThis->CanSetGlobal(sName))
		return 0;

  char *var = (char *) lua_touserdata(L, 2);
  const char *val = (const char *) lua_tostring(L, 3);
	if (var && val)
		strcpy(var, val);
	pThis->NotifySetGlobal(sName);
	return 0;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
int CScriptSystem::SetGlobalTagHandlerInt(lua_State *L)
{
	CScriptSystem *pThis = (CScriptSystem *)lua_touserdata(L, - 1);
	const char *sName = lua_tostring(L, 1);
	if (!pThis->CanSetGlobal(sName))
		return 0;
  int* var = (int*) lua_touserdata(L, 2);
  int  val = (int)  lua_tonumber(L, 3);
	*var = val;
	pThis->NotifySetGlobal(sName);
	return 0;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
int CScriptSystem::SetGlobalTagHandlerFloat(lua_State *L)
{
	CScriptSystem *pThis = (CScriptSystem *)lua_touserdata(L, - 1);
	const char *sName = lua_tostring(L, 1);
	if (!pThis->CanSetGlobal(sName))
		return 0;
  float* var = (float*) lua_touserdata(L, 2);
  float  val = (float)  lua_tonumber(L, 3);
	*var = val;
	pThis->NotifySetGlobal(sName);
	return 0;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
int CScriptSystem::GetGlobalTagHandlerString(lua_State *L)
{
	CScriptSystem *pThis = (CScriptSystem *)lua_touserdata(L, - 1);
	const char *sName = lua_tostring(L, 1);
  const char *var = (const char *) lua_touserdata(L, 2);
  
	lua_pushstring(L, var);
	return 1;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
int CScriptSystem::GetGlobalTagHandlerFloat(lua_State *L)
{
	CScriptSystem *pThis = (CScriptSystem *)lua_touserdata(L, - 1);
	const char *sName = lua_tostring(L, 1);
  float* var = (float*) lua_touserdata(L, 2);
  
	lua_pushnumber(L, *var);
	
	return 1;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
int CScriptSystem::GetGlobalTagHandlerInt(lua_State *L)
{
	CScriptSystem *pThis = (CScriptSystem *)lua_touserdata(L, - 1);
	const char *sName = lua_tostring(L, 1);
  int* var = (int*) lua_touserdata(L, 2);
  
	lua_pushnumber(L, (lua_Number)*var);
	
	return 1;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void CScriptSystem::NotifySetGlobal(const char *sVarName)
{
	if (m_pSink)
		m_pSink->OnSetGlobal(sVarName);
}

bool CScriptSystem::CanSetGlobal (const char *sVarName)
{
	return !m_pSink || m_pSink->CanSetGlobal(sVarName);
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
extern "C" int errorfb (lua_State *L);

void CScriptSystem::RegisterErrorHandler(bool bDebugger)
{
	/*lua_newuserdatabox(m_pLS, this);
	lua_pushcclosure(m_pLS, CScriptSystem::ErrorHandler, 1);
	lua_setglobal(m_pLS, LUA_ERRORMESSAGE);*/
	if(bDebugger)
	{
		lua_newuserdatabox(m_pLS, this);
		lua_pushcclosure(m_pLS, CScriptSystem::ErrorHandler, 1);
		lua_setglobal(m_pLS, LUA_ERRORMESSAGE);
	}
	else
	{
		lua_newuserdatabox(m_pLS, this);
		lua_pushcclosure(m_pLS, CScriptSystem::ErrorHandler, 1);
		lua_setglobal(m_pLS, LUA_ALERT);
		lua_pushcclosure(m_pLS, errorfb, 0);
		lua_setglobal(m_pLS, LUA_ERRORMESSAGE);
	}
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void CScriptSystem::RegisterTagHandlers()
{
	static int prova = 50;
	static char sprova[256];
	m_nFloatTag = lua_newtag(m_pLS);
	m_nIntTag = lua_newtag(m_pLS);
	m_nStringTag = lua_newtag(m_pLS);

	m_nGCTag = lua_newtag(m_pLS);
	
	lua_newuserdatabox(m_pLS, this);
	lua_pushcclosure(m_pLS, CScriptSystem::GCTagHandler, 1);
	lua_settagmethod(m_pLS, m_nGCTag, "gc");
	
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
HTAG CScriptSystem::CreateTaggedValue(const char *sKey, int *pVal)
{
	int nTag = lua_newtag(m_pLS);
	
	lua_newuserdatabox(m_pLS, this);
	lua_pushcclosure(m_pLS, CScriptSystem::SetGlobalTagHandlerInt, 1);
	lua_settagmethod(m_pLS, nTag, "setglobal");
	lua_newuserdatabox(m_pLS, this);
	lua_pushcclosure(m_pLS, CScriptSystem::GetGlobalTagHandlerInt, 1);
	lua_settagmethod(m_pLS, nTag, "getglobal");
	
	lua_newuserdatabox(m_pLS, pVal);
	lua_settag(m_pLS, nTag);
	lua_setglobal(m_pLS, sKey);
	
	return nTag;
}
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
HTAG CScriptSystem::CreateTaggedValue(const char *sKey, float *pVal)
{
	int nTag = lua_newtag(m_pLS);
	
	lua_newuserdatabox(m_pLS, this);
	lua_pushcclosure(m_pLS, CScriptSystem::SetGlobalTagHandlerFloat, 1);
	lua_settagmethod(m_pLS, nTag, "setglobal");
	lua_newuserdatabox(m_pLS, this);
	lua_pushcclosure(m_pLS, CScriptSystem::GetGlobalTagHandlerFloat, 1);
	lua_settagmethod(m_pLS, nTag, "getglobal");
	
	lua_newuserdatabox(m_pLS, pVal);
	lua_settag(m_pLS, nTag);
	lua_setglobal(m_pLS, sKey);
	
	return nTag;
}
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
HTAG CScriptSystem::CreateTaggedValue(const char *sKey, char *pVal)
{
	int nTag = lua_newtag(m_pLS);
	
	lua_newuserdatabox(m_pLS, this);
	lua_pushcclosure(m_pLS, CScriptSystem::SetGlobalTagHandlerString, 1);
	lua_settagmethod(m_pLS, nTag, "setglobal");
	lua_newuserdatabox(m_pLS, this);
	lua_pushcclosure(m_pLS, CScriptSystem::GetGlobalTagHandlerString, 1);
	lua_settagmethod(m_pLS, nTag, "getglobal");
	
	lua_newuserdatabox(m_pLS, pVal);
	lua_settag(m_pLS, nTag);
	lua_setglobal(m_pLS, sKey);
	
	return nTag;
}

void CScriptSystem::RemoveTaggedValue(HTAG htag)
{
	lua_pushnil(m_pLS);
	lua_settagmethod(m_pLS, htag, "setglobal");
	lua_pushnil(m_pLS);
	lua_settagmethod(m_pLS, htag, "getglobal");
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void CScriptSystem::FormatAndRaiseError(int nErr)
{
	const char *sFuncName = NULL;
	switch (nErr)	
	{
			case 0:				/*GetLog()->Log("ScriptSystem:Success!!!\n"); */
				break;
			case LUA_ERRRUN:
				sFuncName = "undefined";
				m_pSink->OnScriptError("", sFuncName, -1, "ScriptSystem:error while running the chunk");
				break;
			case LUA_ERRSYNTAX:	
				m_pSink->OnScriptError("", "", -1, "ScriptSystem:precompiling the file");
				break;
			case LUA_ERRMEM:	
				m_pSink->OnScriptError("", "", -1, "ScriptSystem:memory allocation error in");
				break;
			case LUA_ERRERR:
				m_pSink->OnScriptError("", "", -1, "error while running _ERRORMESSAGE");
				break;
			case LUA_ERRFILE:	
				m_pSink->OnScriptError(m_strCurrentFile.c_str(), "", -1, "Error opening/parsing file ");
				break; 	
			default:
				break;	
	};
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
IFunctionHandler *CScriptSystem::GetFunctionHandler()
{
	return &m_feFuntionHandler;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
IScriptObject *CScriptSystem::CreateEmptyObject()
{
	BEGIN_CHECK_STACK
		CScriptObject *pObj = CreateScriptObject();
	if (!pObj->CreateEmpty(this))
	{
		pObj->Release();
		END_CHECK_STACK
			return NULL;
	};
	END_CHECK_STACK
		return pObj;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
IScriptObject *CScriptSystem::CreateObject()
{
	BEGIN_CHECK_STACK
		CScriptObject *pObj = CreateScriptObject();
	if (!pObj->Create(this))
	{
		pObj->Release();
		END_CHECK_STACK
			return NULL;
	};
	//pObj->SetThis(pThis);
	END_CHECK_STACK
		return pObj;
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
IScriptObject* CScriptSystem::CreateGlobalObject(const char *sName)
{
	BEGIN_CHECK_STACK
		CScriptObject *pObj = CreateScriptObject();
	if (!pObj->CreateGlobal(this, sName))
	{
		pObj->Release();
		END_CHECK_STACK
			return NULL;
	};
//	pObj->SetThis(pThis);
	END_CHECK_STACK
		return pObj;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
bool CScriptSystem::_ExecuteFile(const char *sFileName, bool bRaiseError)
{
	FILE *pFile = NULL;
	
	m_strCurrentFile = sFileName;
	
//#ifdef USE_CRYPAK
	ICryPak *pPak=GetISystem()->GetIPak();
	pFile = pPak->FOpen(sFileName, "rb");
//#else
//	pFile = fxopen(sFileName, "rb"); 
//#endif

	if (!pFile)
	{
		if (bRaiseError)
			RaiseError("Unable to open %s", sFileName);
		
		return false;
	}

	int nSize=0;

//#ifdef USE_CRYPAK
	pPak->FSeek(pFile, 0, SEEK_END); 
	nSize = pPak->FTell(pFile); 
	pPak->FSeek(pFile, 0, SEEK_SET); 
	if (nSize==0)
	{
		pPak->FClose(pFile); 
		return (false);
	}	
/*
#else
	fseek(pFile, 0, SEEK_END);		
	nSize = ftell(pFile);		
	fseek(pFile, 0, SEEK_SET);
	if (nSize == 0)
	{
		fclose(pFile);
		return false;
	}		
#endif
*/

	char *pBuffer;	
	pBuffer = new char[nSize];

//#ifdef USE_CRYPAK
	if (pPak->FRead(pBuffer, nSize, 1, pFile) == 0) 
	{		
		delete [] pBuffer;
		pPak->FClose(pFile); 
		return false;
	}
	pPak->FClose(pFile); 
/*
#else
	if (fread(pBuffer, nSize, 1, pFile) == 0)
	{		
		fclose(pFile);
		return false;
	}

	fclose(pFile);
#endif		
*/
	char script_decr_key[32] = "3CE2698701289029F3926DF0191189A";
	//////////////////////////////////////////////////////////////////////////
	// Check if it is encrypted script.
	//////////////////////////////////////////////////////////////////////////
	string CRY_ENCRYPT_FILE_HEADER = string("")+"c"+"r"+"y"+"e"+"h"+"d"+"r";
	if (nSize > CRY_ENCRYPT_FILE_HEADER.size())
	{
		if (memcmp(pBuffer,CRY_ENCRYPT_FILE_HEADER.c_str(),CRY_ENCRYPT_FILE_HEADER.size()) == 0)
		{
			// Decrypt this buffer.
			GetISystem()->GetIDataProbe()->AESDecryptBuffer( pBuffer,nSize,pBuffer,nSize,script_decr_key );
		}
	}
	//////////////////////////////////////////////////////////////////////////
	if (m_bDebug && m_pDebugSink)
	{
		m_pDebugSink->OnLoadSource(sFileName,(unsigned char*) pBuffer, nSize);		
	}
			
	//int nRes = lua_dofile(m_pLS, sFileName);

	char szFileName[_MAX_PATH + 1];
	szFileName[0] = '@';
	strcpy(&szFileName[1], sFileName);

	int nRes=lua_dobuffer(m_pLS,pBuffer,nSize,/*sFileName*/ szFileName);

	delete [] pBuffer;

	if (nRes)
	{
		if (bRaiseError)
		{
			FormatAndRaiseError(nRes);
		}
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
bool CScriptSystem::ExecuteFile(const char *sFileName, bool bRaiseError,bool bForceReload)
{
	if (strlen(sFileName) <= 0)
		return false;
	string sTemp;
	sTemp = FormatPath(sFileName);
	//ScriptFileListItor itor = std::find(m_dqLoadedFiles.begin(), m_dqLoadedFiles.end(), sTemp.c_str());
	ScriptFileListItor itor = m_dqLoadedFiles.find(sTemp);
	if (itor == m_dqLoadedFiles.end() || bForceReload)
	{
		if (!_ExecuteFile(sTemp.c_str(), bRaiseError))
		{
#if defined(_DEBUG) && (defined(WIN64) || defined(LINUX64))
			char szBuf[0x800];
			_snprintf (szBuf, sizeof(szBuf), "Can't execute script %s : %s\n", sFileName, sTemp.c_str());
			OutputDebugString (szBuf);
#endif
			return false;
		}
		if(itor == m_dqLoadedFiles.end())
			m_dqLoadedFiles.insert(sTemp);
	}
#if defined(_DEBUG) && (defined(WIN64) || defined(LINUX64))
	char szBuf[0x800];
	_snprintf (szBuf, sizeof(szBuf), "Script %s executed\n", sFileName);
	OutputDebugString (szBuf);
#endif
	return true;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void CScriptSystem::UnloadScript(const char *sFileName)
{
	if (strlen(sFileName) <= 0)
		return;
	string sTemp;
	sTemp = FormatPath(sFileName);
	//ScriptFileListItor itor = std::find(m_dqLoadedFiles.begin(), m_dqLoadedFiles.end(), sTemp.c_str());
	ScriptFileListItor itor = m_dqLoadedFiles.find(sTemp);
	if (itor != m_dqLoadedFiles.end())
	{
#if !defined(LINUX)
		::OutputDebugString("ERASE : ");
		::OutputDebugString(sTemp.c_str());
		::OutputDebugString("\n");
#endif
		m_dqLoadedFiles.erase(itor);
	}
}
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void CScriptSystem::UnloadScripts()
{
	m_dqLoadedFiles.clear();
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
bool CScriptSystem::ReloadScripts()
{
	ScriptFileListItor itor;
	itor = m_dqLoadedFiles.begin();
	while (itor != m_dqLoadedFiles.end())
	{
		ReloadScript(itor->c_str(), true);
		++itor;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
bool CScriptSystem::ReloadScript(const char *sFileName, bool bRaiseError)
{
	string strTemp = FormatPath(sFileName);
	ScriptFileListItor itor = m_dqLoadedFiles.find(strTemp);//std::find(m_dqLoadedFiles.begin(), m_dqLoadedFiles.end(), strTemp);
	if (itor == m_dqLoadedFiles.end())
	{
		RaiseError("Error reloading \"%s\" the file was not loaded", sFileName);
		return false; 
	}
	return _ExecuteFile(strTemp.c_str(), bRaiseError);
}

void CScriptSystem::DumpLoadedScripts()
{
	ScriptFileListItor itor;
	itor = m_dqLoadedFiles.begin();
	while (itor != m_dqLoadedFiles.end())
	{
		m_pSink->OnLoadedScriptDump(itor->c_str());
		++itor;
	}
}

/*
void CScriptSystem::GetScriptHashFunction( IScriptObject &Current, unsigned int &dwHash)
{
	unsigned int *pCode=0;
	int iSize=0;

	if(pCurrent.GetCurrentFuncData(pCode,iSize))						// function ?
	{
		if(pCode) 																						// lua function ?
			GetScriptHashFunction(pCode,iSize,dwHash);
	}
}
*/


void CScriptSystem::GetScriptHash( const char *sPath, const char *szKey, unsigned int &dwHash )
{
//	IScriptObject *pCurrent;



//	GetGlobalValue(szKey,pCurrent);

//	if(!pCurrent)		// is not a table
//	{
//	}
/*
	else if(lua_isfunction(m_pLS, -1))
	{
		GetScriptHashFunction(*pCurrent,dwHash);

		return;
	}
	else
	{
		lua_pop(m_pLS, 1);
		return;
	}
*/
	/*
	pCurrent->BeginIteration();

	while(pCurrent->MoveNext())
	{
		char *szKeyName;

		if(!pCurrent->GetCurrentKey(szKeyName))
			szKeyName="NO";

		ScriptVarType type=pCurrent->GetCurrentType();

		if(type==svtObject)		// svtNull,svtFunction,svtString,svtNumber,svtUserData,svtObject
		{
			void *pVis;
			
			pCurrent->GetCurrentPtr(pVis);

			GetISystem()->GetILog()->Log("  table '%s/%s'",sPath.c_str(),szKeyName);				

			if(setVisited.count(pVis)!=0)
			{
				GetISystem()->GetILog()->Log("    .. already processed ..");				
				continue;
			}

			setVisited.insert(pVis);

			{
				IScriptObject *pNewObject = m_pScriptSystem->CreateEmptyObject();

				pCurrent->GetCurrent(pNewObject);

				Debug_Full_recursive(pNewObject,sPath+string("/")+szKeyName,setVisited);

				pNewObject->Release();
			}
		}
		else if(type==svtFunction)		// svtNull,svtFunction,svtString,svtNumber,svtUserData,svtObject
		{
			GetScriptHashFunction(*pCurrent,dwHash);
		}
	}
	
	pCurrent->EndIteration();
	*/
}



//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
bool CScriptSystem::ExecuteBuffer(const char *sBuffer, size_t nSize)
{
	int nRes = lua_dobuffer(m_pLS, sBuffer, nSize, "__script_buffer__");
	if (nRes)
	{
		FormatAndRaiseError(nRes);
		return false;
	}
	return (true);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void CScriptSystem::Release()
{
#ifdef _DEBUG
	char sTemp[100];
	int nCreationNumber;
#endif
	while(!m_stkScriptObjectsPool.empty())
	{
		CScriptObject *pObj=m_stkScriptObjectsPool.back();
#ifdef _DEBUG
		nCreationNumber=pObj->m_nCreationNumber;
#endif
		delete pObj;
		m_stkScriptObjectsPool.pop_back();
#ifdef _DEBUG
		sprintf(sTemp,"delete[%d] obj Pool size %d new\n",nCreationNumber,m_stkScriptObjectsPool.size());
		::OutputDebugString(sTemp);
#endif
	}
	delete this;
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
int CScriptSystem::BeginCall(HSCRIPTFUNCTION hFunc)
{
	// alberto_pushfunc(m_pLS,hFunc);
	m_nTempTop = lua_gettop(m_pLS);
	if(hFunc==0)
	{
		RaiseError("(BeginCall) failed NULL parameter");
		m_nTempArg = -1;
		return 0;
	}

	if(!lua_getref(m_pLS, (int)hFunc))
	{
		m_nTempArg = -1;
		return 0;
	}
	if(!lua_isfunction(m_pLS,-1))
	{
		RaiseError("Function Ptr:%d not found",hFunc);
		m_nTempArg = -1;
		return 0;
	}
	m_nTempArg = 0;
	

	return 1;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
int CScriptSystem::BeginCall(const char *sTableName, const char *sFuncName)
{
	lua_getglobal(m_pLS, sTableName);

	if(!lua_istable(m_pLS,-1))
	{
		RaiseError("(BeginCall)Tried to call %s:%s(), Table %s not found (check for syntax errors or if the file wasn't loaded)",sTableName,sFuncName,sTableName);
		m_nTempArg = -1;
		lua_pop(m_pLS,1);
		return 0;
	}

	lua_pushstring(m_pLS, sFuncName);
	lua_rawget(m_pLS, - 2);
	lua_remove(m_pLS, - 2); // Remove table global.
	m_nTempArg = 0;

	if(!lua_isfunction(m_pLS,-1))
	{
		RaiseError("Function %s:%s not found(check for syntax errors or if the file wasn't loaded)",sTableName,sFuncName);
		m_nTempArg = -1;

		return 0;
	}

	return 1;
}

int CScriptSystem::BeginCall(const char *sFuncName)
{
	lua_getglobal(m_pLS, sFuncName);
	m_nTempArg = 0;
#ifdef _DEBUG
	if(!lua_isfunction(m_pLS,-1))
	{
		RaiseError("Function %s not found(check for syntax errors or if the file wasn't loaded)",sFuncName);
		m_nTempArg = -1;

		return 0;
	}
#endif	

	return 1;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
HSCRIPTFUNCTION CScriptSystem::GetFunctionPtr(const char *sFuncName)
{
	_GUARD_STACK(m_pLS);
	HSCRIPTFUNCTION func;
	lua_getglobal(m_pLS, sFuncName);
	if (lua_isnil(m_pLS, -1) ||(!lua_isfunction(m_pLS, -1)))
	{
		lua_pop(m_pLS, 1);
		return NULL;
	}
	func = (HSCRIPTFUNCTION)lua_ref(m_pLS, 0);
	
	return func;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
HSCRIPTFUNCTION CScriptSystem::GetFunctionPtr(const char *sTableName, const char *sFuncName)
{
	_GUARD_STACK(m_pLS);
	HSCRIPTFUNCTION func;
	lua_getglobal(m_pLS, sTableName);
	if(!lua_istable(m_pLS,-1))
	{
//		RaiseError("(GetFunctionPtr)Table %s: not found(check for syntax errors or if the file wasn't loaded)", sTableName);
		lua_pop(m_pLS,1);
		return 0;
	}
	lua_pushstring(m_pLS, sFuncName);
	lua_gettable(m_pLS, - 2);
	lua_remove(m_pLS, - 2); // Remove table global.
	if (lua_isnil(m_pLS, -1) ||(!lua_isfunction(m_pLS, -1)))
	{
		lua_pop(m_pLS, 1);
		return FALSE;
	}
	func = (HSCRIPTFUNCTION)lua_ref(m_pLS, 0);
	return func;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void CScriptSystem::EndCall()
{
	Validate();

	if(m_nTempArg==-1)
		return;
#ifdef USE_RAW_CALL
	int nRes = lua_call(m_pLS, m_nTempArg, 0);
	if (nRes)
		FormatAndRaiseError(nRes);
#else
	lua_rawcall(m_pLS, m_nTempArg, 0);
#endif

	Validate();
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void CScriptSystem::EndCall(int &nRet)
{
	Validate();

	if(m_nTempArg==-1)
		return;
#ifdef USE_RAW_CALL
	int nRes = lua_call(m_pLS, m_nTempArg, 1);
	if (nRes)
		FormatAndRaiseError(nRes);
#else
	lua_rawcall(m_pLS, m_nTempArg, 1);
#endif

	Validate();

	if(lua_gettop(m_pLS))
	{
		if (lua_isnumber(m_pLS, -1))
		{
			nRet = (int)lua_tonumber(m_pLS, -1);
		}
		lua_pop(m_pLS, 1);
	}
	
	Validate();

}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void CScriptSystem::EndCall(float &fRet)
{
	Validate();
	if(m_nTempArg==-1)
		return;
#ifdef USE_RAW_CALL
	int nRes = lua_call(m_pLS, m_nTempArg, 1);
	if (nRes)
		FormatAndRaiseError(nRes);
#else
	lua_rawcall(m_pLS, m_nTempArg, 1);
#endif
	Validate();
	if(lua_gettop(m_pLS))
	{
		if (lua_isnumber(m_pLS, -1))
		{
			fRet = lua_tonumber(m_pLS, -1);
		}
		lua_pop(m_pLS, 1);
	}
	Validate();

}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void CScriptSystem::EndCall(const char *&sRet)
{
	Validate();
	if(m_nTempArg==-1)
		return;
#ifdef USE_RAW_CALL
	int nRes = lua_call(m_pLS, m_nTempArg, 1);
	if (nRes)
		FormatAndRaiseError(nRes);
#else
	lua_rawcall(m_pLS, m_nTempArg, 1);
#endif
	Validate();
	if(lua_gettop(m_pLS))
	{
		if (lua_isstring(m_pLS, -1))
		{
			sRet = lua_tostring(m_pLS, -1);
		}
		lua_pop(m_pLS, 1);
	}
	Validate();

}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void CScriptSystem::EndCall(bool &bRet)
{
	Validate();
	if(m_nTempArg==-1)
		return;
#ifdef USE_RAW_CALL
	int nRes = lua_call(m_pLS, m_nTempArg, 1);
	if (nRes)
		FormatAndRaiseError(nRes);
	
#else
	lua_rawcall(m_pLS, m_nTempArg, 1);
#endif
	Validate();
	if (lua_gettop(m_pLS))
	{
		if (lua_isnil(m_pLS, -1))
			bRet = false;
		else
			bRet = true;
		lua_pop(m_pLS, 1);
	}
	Validate();
}
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void CScriptSystem::EndCall(IScriptObject *pObj)
{
	Validate();
	if(m_nTempArg==-1)
		return;
#ifdef USE_RAW_CALL
	int nRes = lua_call(m_pLS, m_nTempArg, 1);
	if (nRes)
		FormatAndRaiseError(nRes);
#else
	lua_rawcall(m_pLS, m_nTempArg, 1);
#endif
	Validate();
	if (lua_gettop(m_pLS))
	{
		if (lua_istable(m_pLS, -1))
		{
			//int nRef = lua_ref(m_pLS, 1);
			pObj->Attach();
		}
		else
		{
			lua_pop(m_pLS, 1);
		}
	}
	Validate();
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void CScriptSystem::PushFuncParam(int nVal)
{
	Validate();
	if(m_nTempArg==-1)
		return;
	lua_pushnumber(m_pLS, (lua_Number)nVal);
	m_nTempArg++;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void CScriptSystem::PushFuncParam(float fVal)
{
	Validate();
	if(m_nTempArg==-1)
		return;
	lua_pushnumber(m_pLS, fVal);
	m_nTempArg++;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void CScriptSystem::PushFuncParam(const char *sVal)
{
	Validate();
	if(m_nTempArg==-1)
		return;
	lua_pushstring(m_pLS, sVal);
	m_nTempArg++;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void CScriptSystem::PushFuncParam(bool bVal)
{
	Validate();
	if(m_nTempArg==-1)
		return;
	if (bVal)
	{
		lua_pushnumber(m_pLS, 1);
	}
	else
	{
		lua_pushnil(m_pLS);
	}
	m_nTempArg++;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void CScriptSystem::PushFuncParam(IScriptObject *pVal)
{
	Validate();
	if(m_nTempArg==-1)
		return;
	if(!lua_xgetref(m_pLS, pVal->GetRef()))
	{
		lua_pushnil(m_pLS);
	}
	m_nTempArg++;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void CScriptSystem::SetGlobalToNull(const char *sKey)
{
	Validate();
	lua_pushnil(m_pLS);
	lua_setglobal(m_pLS, sKey);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void CScriptSystem::SetGlobalValue(const char *sKey, int nVal)
{
	Validate();
	lua_pushnumber(m_pLS, (lua_Number)nVal);
	lua_setglobal(m_pLS, sKey);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void CScriptSystem::SetGlobalValue(const char *sKey, float fVal)
{
	Validate();
	lua_pushnumber(m_pLS, fVal);
	lua_setglobal(m_pLS, sKey);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void CScriptSystem::SetGlobalValue(const char *sKey, const char *sVal)
{
	Validate();
	lua_pushstring(m_pLS, sVal);
	lua_setglobal(m_pLS, sKey);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void CScriptSystem::SetGlobalValue(const char *sKey, IScriptObject *pObj)
{
	Validate();
	lua_xgetref(m_pLS, pObj->GetRef());
	lua_setglobal(m_pLS, sKey);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
IScriptObject *CScriptSystem::GetGlobalObject()
{
	Validate();
	CScriptObject *pObj = CreateScriptObject();
	lua_getglobals(m_pLS);
	pObj->CreateEmpty(this);
	pObj->Attach();
	return pObj;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
bool CScriptSystem::GetGlobalValue(const char *sKey, int &nVal)
{
	Validate();
	lua_getglobal(m_pLS, sKey);
	if (lua_isnumber(m_pLS, -1))
	{
		nVal =(int)(lua_tonumber(m_pLS, 1));
	} else 
	{
		lua_pop(m_pLS, 1);
		return false;
	}
	lua_pop(m_pLS, 1);
	return true;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
bool CScriptSystem::GetGlobalValue(const char *sKey, float &fVal)
{
	Validate();
	lua_getglobal(m_pLS, sKey);
	if (lua_isnumber(m_pLS, -1))
	{
		fVal =(float)(lua_tonumber(m_pLS, 1));
	} 
	else 
	{
		lua_pop(m_pLS, 1);
		return false;
	}
	lua_pop(m_pLS, 1);
	return true;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
bool CScriptSystem::GetGlobalValue(const char *sKey, const char * &sVal)
{
	Validate();
	lua_getglobal(m_pLS, sKey);
	if (lua_isstring(m_pLS, -1))
	{
		sVal =(lua_tostring(m_pLS, -1));
		if(sVal==NULL)
		{
			//this cannot happen if it does call alberto
      lua_pop(m_pLS, 1);
			CryError( "<ScriptSystem> CScriptSystem::GetGlobalValue: Key %s cannot be converted to string.",sKey );
			return false;
		}
	} 
	else 
	{
		lua_pop(m_pLS, 1);
		return false;
	}
	lua_pop(m_pLS, 1);
	return true;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
bool CScriptSystem::GetGlobalValue(const char *sKey, IScriptObject *pObj)
{
	Validate();
	lua_getglobal(m_pLS, sKey); 
	if (lua_istable(m_pLS, -1))
	{
		pObj->Attach();
	}
	else
	{
		lua_pop(m_pLS, 1);
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void CScriptSystem::ForceGarbageCollection()
{
	/*char sTemp[200];
	lua_StateStats lss;
	lua_getstatestats(m_pLS,&lss);
	sprintf(sTemp,"protos=%d closures=%d tables=%d udata=%d strings=%d\n",lss.nProto,lss.nClosure,lss.nHash,lss.nUdata,lss.nString);
	OutputDebugString("BEFORE GC STATS :");
	OutputDebugString(sTemp);*/

	Validate();
	lua_setgcthreshold(m_pLS, 0);
	Validate();
	/*lua_getstatestats(m_pLS,&lss);
	sprintf(sTemp,"protos=%d closures=%d tables=%d udata=%d strings=%d\n",lss.nProto,lss.nClosure,lss.nHash,lss.nUdata,lss.nString);
	OutputDebugString("AFTER GC STATS :");
	OutputDebugString(sTemp);*/

}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
int CScriptSystem::GetCGCount()
{
	Validate();
	return lua_getgccount(m_pLS);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void CScriptSystem::SetGCThreshhold(int nKb)
{
	Validate();
	lua_setgcthreshold(m_pLS, nKb);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void CScriptSystem::RaiseError(const char *sErr,...)
{
	Validate();
	va_list		arglist;
	char		sBuf[2048];
	lua_Debug ar;
	int nRes;
	int iCurrentLine=-1;									// no line number info
	const char *sFuncName = NULL;
	const char *sSourceFile = NULL;
  
	va_start(arglist, sErr);
	vsprintf(sBuf, sErr, arglist);
	va_end(arglist);	
	
	nRes = lua_getstack(m_pLS, 1, &ar);	
	if (nRes != 0)
	{
		nRes = lua_getinfo(m_pLS, "lnS", &ar);
		iCurrentLine=ar.currentline;
		sFuncName = ar.name;
		sSourceFile = ar.source;
	}
	if (!sFuncName) 
		sFuncName = "undefined";
	if (!sSourceFile)
		sSourceFile= "undefined";

	if (sBuf) 
		m_pSink->OnScriptError(sSourceFile, sFuncName, iCurrentLine, sBuf);
}

int CScriptSystem::GCTagHandler(lua_State *L)
{
	Validate();
	CScriptSystem *pThis = (CScriptSystem *)lua_touserdata(L, - 1);
	USER_DATA_CHUNK *udc=(USER_DATA_CHUNK *)lua_touserdata(L,1);
	if(pThis && pThis->m_pSink && udc)
	{
		//pThis->m_mapUserData.erase(udc->nVal);
		pThis->m_pSink->OnCollectUserData(udc->nVal,udc->nCookie);
		lua_unref(L,(int)udc->nRef);
		udc->nVal=udc->nCookie=0xDEADBEEF;
	}
	return 0;
}

USER_DATA CScriptSystem::CreateUserData(INT_PTR nVal,int nCookie)	//AMD Port
{
	Validate();
//	UserDataMapItor itor=m_mapUserData.find(nVal);
	int nRef=0;
//	if(itor==m_mapUserData.end())
//	{
		USER_DATA_CHUNK *pUDC=(USER_DATA_CHUNK *)lua_newuserdata(m_pLS,sizeof(USER_DATA_CHUNK));
		pUDC->nVal=nVal;
		pUDC->nCookie=nCookie;
		
		lua_settag(m_pLS, m_nGCTag);
		pUDC->nRef=lua_ref(m_pLS,0);
		//m_mapUserData.insert(UserDataMapItor::value_type(nVal,nRef));
//	}
//	else
//	{
		//::OutputDebugString("Reusing pointer\n");
		//nRef=itor->second;
//	}
		Validate();
	return pUDC->nRef;
}
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Pointer to Global ISystem.
static ISystem* gISystem = 0;
ISystem* GetISystem()
{
	return gISystem;
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
IScriptSystem *CreateScriptSystem(ISystem *pSystem,IScriptSystemSink *pSink, IScriptDebugSink *pDebugSink, bool bStdLibs)
{
	gISystem = pSystem;
	CScriptSystem *pScriptSystem = new CScriptSystem;
	pScriptSystem->Validate();
	if (!pScriptSystem->Init(pSink, pDebugSink, bStdLibs, 1024))
	{
		pScriptSystem->Release();
	}
	return pScriptSystem;
}

CScriptObject *CScriptSystem::CreateScriptObject()
{
	Validate();
	//char sTemp[100];
	if(m_stkScriptObjectsPool.empty())
	{
		//sprintf(sTemp,"Pool size %d new\n",m_stkScriptObjectsPool.size());
		//::OutputDebugString(sTemp);
	//	if(m_nObjCreationNumber==42)
	//		DEBUG_BREAK;
		return new CScriptObject(m_nObjCreationNumber++);
		
	}
	else
	{
		CScriptObject *pObj;
//		sprintf(sTemp,"Pool size %d cached\n",m_stkScriptObjectsPool.size());
		//::OutputDebugString(sTemp);

		pObj = m_stkScriptObjectsPool.back();
		pObj->Recreate();
		m_stkScriptObjectsPool.pop_back();
		return pObj;
	}
}

void CScriptSystem::ReleaseScriptObject(CScriptObject *p)
{
	Validate();
	if(m_stkScriptObjectsPool.size()<SCRIPT_OBJECT_POOL_SIZE){
		m_stkScriptObjectsPool.push_back(p);
	}
	else
	{
		char sTemp[100];
		sprintf(sTemp,"chached>> Pool size %d\n",m_stkScriptObjectsPool.size());
		::OutputDebugString(sTemp);
		delete p;
		Validate();
	}
	
}

void CScriptSystem::UnbindUserdata()
{
	Validate();
	lua_pushnil(m_pLS);
	lua_settagmethod(m_pLS,m_nGCTag,"gc");
}

void CScriptSystem::UnrefFunction (HSCRIPTFUNCTION hFunc)
{
	lua_unref (m_pLS, hFunc);
}

HBREAKPOINT CScriptSystem::AddBreakPoint(const char *sFile,int nLineNumber)
{
	m_BreakPoint.sSourceFile=sFile;
	m_BreakPoint.nLine=nLineNumber;
	DebugContinue();
	return 0;
}

IScriptObject *CScriptSystem::GetBreakPoints()
{
	Validate();
	IScriptObject *pBreakPoints=CreateObject();
	if(m_BreakPoint.sSourceFile.length())
	{
		IScriptObject *pBP=CreateObject();
		pBP->SetValue("line",m_BreakPoint.nLine);
		pBP->SetValue("sourcefile",m_BreakPoint.sSourceFile.c_str());
		pBreakPoints->PushBack(pBP);
		pBP->Release();
		Validate();
	}
	return pBreakPoints;
}

void CScriptSystem::ReleaseFunc(HSCRIPTFUNCTION f)
{
	if(f)
	{
		lua_unref(m_pLS,f);
		Validate();
	}
}

