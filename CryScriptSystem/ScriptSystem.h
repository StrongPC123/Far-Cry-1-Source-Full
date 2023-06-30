// ScriptSystem.h: interface for the CScriptSystem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCRIPTSYSTEM_H__8FCEA01B_BD85_4E4D_B54F_B09429A7CDFF__INCLUDED_)
#define AFX_SCRIPTSYSTEM_H__8FCEA01B_BD85_4E4D_B54F_B09429A7CDFF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <IScriptSystem.h>
extern "C"{
#include <lua.h>
}
#include <algorithm>
#include <set>
#include <stack>
#include <map>
#include <string>
#include <vector>
#include "FunctionHandler.h"
#if !defined(LINUX)
#include <assert.h>
#endif


struct BreakPoint
{
	BreakPoint()
	{
		nLine=-1;
	}
	BreakPoint(const BreakPoint& b)
	{
		nLine=b.nLine;
		sSourceFile=b.sSourceFile;
	}
	int nLine;
	string sSourceFile;
};

typedef std::set<string> ScriptFileList;
typedef ScriptFileList::iterator ScriptFileListItor;

typedef std::map<int,int> UserDataMap;
typedef UserDataMap::iterator UserDataMapItor;

class CScriptSystem;
class CScriptObject;

struct USER_DATA_CHUNK
{
	int nRef;
	USER_DATA nVal;				//AMD Port
	int nCookie;
};

#define SCRIPT_OBJECT_POOL_SIZE 15000
typedef std::vector<CScriptObject * > ScriptObjectsObjectPool;

/*! IScriptSystem implementation
	@see IScriptSystem 
*/
class CScriptSystem : public IScriptSystem
{
public:
	//! constructor
	CScriptSystem();
	//! destructor
	virtual ~CScriptSystem();
	//!
	bool Init(IScriptSystemSink *pSink,IScriptDebugSink *pDebugSink,bool bStdLibs,int nStackSize);
	//!
	void RegisterErrorHandler(bool bDebugger=false);
	//!
	void FormatAndRaiseError(int nErr);
	//!
	bool _ExecuteFile(const char *sFileName,bool bRaiseError);
	//!
	void ReleaseScriptObject(CScriptObject *p);
	// this is validating call to hunt down possible memory corruptions
	// normally it should be defined as inlined empty function
	static void Validate();
	//!
	IScriptSystemSink* GetSystemSink() { return m_pSink; };
	//!
	void UnrefFunction (HSCRIPTFUNCTION hFunc);

	// interface IScriptSystem -----------------------------------------------------------

	virtual IFunctionHandler *GetFunctionHandler();
	virtual HSCRIPT GetScriptHandle() { return (HSCRIPT)m_pLS; }
	virtual bool ExecuteFile(const char *sFileName,bool bRaiseError,bool bForceReload);
	virtual bool ExecuteBuffer(const char *sBuffer, size_t nSize);
	virtual void UnloadScript(const char *sFileName);
	virtual void UnloadScripts();
	virtual bool ReloadScript(const char *sFileName,bool bRaiseError);
	virtual bool ReloadScripts();
	virtual void DumpLoadedScripts();
	virtual IScriptObject *GetGlobalObject();
	virtual IScriptObject *CreateEmptyObject();
	virtual IScriptObject *CreateObject();
	virtual IScriptObject *CreateGlobalObject(const char *sName);
	virtual int BeginCall(HSCRIPTFUNCTION hFunc);
	virtual int BeginCall(const char *sFuncName);
	virtual int BeginCall(const char *sTableName,const char *sFuncName);
	virtual void EndCall();
	virtual void EndCall(int &nRet);
	virtual void EndCall(float &fRet);
	virtual void EndCall(const char *&sRet);
	virtual void EndCall(bool &bRet);
	virtual void EndCall(IScriptObject *pScriptObject);
	virtual HSCRIPTFUNCTION GetFunctionPtr(const char *sFuncName);
	virtual HSCRIPTFUNCTION GetFunctionPtr(const char *sTableName, const char *sFuncName);
	virtual void ReleaseFunc(HSCRIPTFUNCTION f);
	virtual void PushFuncParam(int nVal);
	virtual void PushFuncParam(float fVal);
	virtual void PushFuncParam(const char *sVal);
	virtual void PushFuncParam(bool bVal);
	virtual void PushFuncParam(IScriptObject *pVal);
	virtual void SetGlobalValue(const char *sKey, int nVal);
	virtual void SetGlobalValue(const char *sKey, float fVal);
	virtual void SetGlobalValue(const char *sKey, const char *sVal);
	virtual void SetGlobalValue(const char *sKey, IScriptObject *pObj);
	virtual void SetGlobalToNull(const char *sKey);
	virtual bool GetGlobalValue(const char *sKey, int &nVal);
	virtual bool GetGlobalValue(const char *sKey, float &fVal);
	virtual bool GetGlobalValue(const char *sKey, const char * &sVal);
	virtual bool GetGlobalValue(const char *sKey, IScriptObject *pObj);
	virtual HTAG CreateTaggedValue(const char *sKey, int *pVal);
	virtual HTAG CreateTaggedValue(const char *sKey, float *pVal);
	virtual HTAG CreateTaggedValue(const char *sKey, char *pVal);
	virtual void RemoveTaggedValue(HTAG htag);
	virtual USER_DATA CreateUserData(INT_PTR nVal,int nCookie);
	virtual void RaiseError(const char *sErr,...);
	virtual void ForceGarbageCollection();
	virtual int GetCGCount();
	virtual void SetGCThreshhold(int nKb);
	virtual void UnbindUserdata();
	virtual void Release();
	virtual void EnableDebugger(IScriptDebugSink *pDebugSink);
	virtual IScriptObject *GetBreakPoints();
	virtual HBREAKPOINT AddBreakPoint(const char *sFile,int nLineNumber);
	virtual IScriptObject *GetLocalVariables(int nLevel = 0);
	virtual IScriptObject *GetCallsStack();
	virtual void DebugContinue(){m_bsBreakState=bsContinue;}
	virtual void DebugStepNext(){m_bsBreakState=bsStepNext;}
	virtual void DebugStepInto(){m_bsBreakState=bsStepInto;}
	virtual void DebugDisable(){m_bsBreakState=bsNoBreak;}
	virtual BreakState GetBreakState(){return m_bsBreakState;}
	virtual void GetMemoryStatistics(ICrySizer *pSizer);
	virtual void GetScriptHash( const char *sPath, const char *szKey, unsigned int &dwHash );
	virtual void PostInit();

private: // ---------------------------------------------------------------------

	//!
	static int ErrorHandler(lua_State *L);
	//!
	static int SetGlobalTagHandlerFloat(lua_State *L);
	//!
	static int GetGlobalTagHandlerFloat(lua_State *L);
	//!
	static int SetGlobalTagHandlerInt(lua_State *L);
	//!
	static int GetGlobalTagHandlerInt(lua_State *L);
	//!
	static int SetGlobalTagHandlerString(lua_State *L);
	//!
	static int GetGlobalTagHandlerString(lua_State *L);
	//!
	void NotifySetGlobal(const char *sVarName);
	//!
	bool CanSetGlobal(const char *sVarName);
	//!
	CScriptObject *CreateScriptObject();
	//!
	void RegisterTagHandlers();
	//!
	static int GCTagHandler(lua_State *L);

//	void GetScriptHashFunction( IScriptObject &Current, unsigned int &dwHash);
	
	// ----------------------------------------------------------------------------

	lua_State *								m_pLS;
	bool 											m_bDebug;											//!< temp value for function calls
	int 											m_nTempArg;
	int 											m_nTempTop;

	int 											m_nFloatTag;
	int 											m_nIntTag;
	int												m_nStringTag;

	int												m_nGCTag;
	
	string										m_strCurrentFile;

	CFunctionHandler					m_feFuntionHandler;
	ScriptFileList						m_dqLoadedFiles;

	IScriptSystemSink *				m_pSink;
	ScriptObjectsObjectPool		m_stkScriptObjectsPool;

	UserDataMap								m_mapUserData;

public: // -----------------------------------------------------------------------

	BreakPoint								m_BreakPoint;									//!
	string										m_sLastBreakSource;						//!
	int												m_nLastBreakLine;							//!
  BreakState								m_bsBreakState;								//!
	IScriptDebugSink *				m_pDebugSink;									//!
	int												m_nObjCreationNumber;					//!< debug variable
};


#endif // !defined(AFX_SCRIPTSYSTEM_H__8FCEA01B_BD85_4E4D_B54F_B09429A7CDFF__INCLUDED_)
