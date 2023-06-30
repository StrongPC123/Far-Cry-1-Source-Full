// ScriptObject.h: interface for the CScriptObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCRIPTOBJECT_H__6EA3E6D6_4FF9_4709_BD62_D5A97C40DB68__INCLUDED_)
#define AFX_SCRIPTOBJECT_H__6EA3E6D6_4FF9_4709_BD62_D5A97C40DB68__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IScriptSystem.h"
extern "C"{
#include <lua.h>
}
class CScriptSystem;
/*! IScriptObject implementation
	@see IScriptObject
*/
class CScriptObject : public IScriptObject  
{
public:
	//! constructor
	CScriptObject(int nCreationNumber);
	//! destructor
	virtual ~CScriptObject();

	bool CreateEmpty(CScriptSystem *pScriptSystem);
	bool Create(CScriptSystem *pScriptSystem);
	bool CreateGlobal(CScriptSystem *pScriptSystem,const char *sName);
	
	// interface IScriptObject ----------------------------------------------------------------

	virtual int GetRef();
	virtual void Attach();
	virtual void Attach(IScriptObject *so);
	virtual void Delegate(IScriptObject *pObj);
	virtual void PushBack(int nVal);
	virtual void PushBack(float fVal);
	virtual void PushBack(const char *sVal);
	virtual void PushBack(bool bVal);
	virtual void PushBack(IScriptObject *pObj);
	virtual void SetValue(const char *sKey,int nVal);
	virtual void SetValue(const char *sKey,float fVal);
	virtual void SetValue(const char *sKey,const char *sVal);
	virtual void SetValue(const char *sKey,bool bVal);
	virtual void SetValue(const char *sKey,IScriptObject *pObj);
	virtual void SetValue(const char *sKey, USER_DATA ud);
	virtual void SetToNull(const char *sKey);
	virtual bool GetValue(const char *sKey,int &nVal);
	virtual bool GetValue(const char *sKey,float &fVal);
	virtual bool GetValue(const char *sKey,bool &bVal);
	virtual bool GetValue(const char *sKey,const char* &sVal);
	virtual bool GetValue(const char *sKey,IScriptObject *pObj);
	virtual bool GetValue(const char *sKey,HSCRIPTFUNCTION &funcVal);
	virtual bool GetUDValue(const char *sKey, USER_DATA &nValue, int &nCookie);	//AMD Port
	virtual bool GetFuncData(const char *sKey, unsigned int * &pCode, int &iSize);
	virtual bool BeginSetGetChain(); 
	virtual bool GetValueChain(const char *sKey, int &nVal);
	virtual bool GetValueChain(const char *sKey, float &fVal);
	virtual bool GetValueChain(const char *sKey, bool &bVal);
	virtual bool GetValueChain(const char *sKey, const char* &sVal);
	virtual bool GetValueChain(const char *sKey, IScriptObject *pObj);
	virtual bool GetValueChain(const char *sKey, HSCRIPTFUNCTION &funcVal);
	virtual bool GetUDValueChain(const char *sKey, USER_DATA &nValue, int &nCookie);		//AMD Port
	virtual void SetValueChain(const char *sKey, int nVal);
	virtual void SetValueChain(const char *sKey, float fVal);
	virtual void SetValueChain(const char *sKey, const char *sVal);
	virtual void SetValueChain(const char *sKey, bool bVal);
	virtual void SetValueChain(const char *sKey, IScriptObject *pObj);
	virtual void SetValueChain(const char *sKey, USER_DATA ud);
	virtual void SetToNullChain(const char *sKey);
	virtual void EndSetGetChain(); 
	virtual ScriptVarType GetValueType(const char *sKey);
	virtual ScriptVarType GetAtType(int nIdx);
	virtual void SetAt(int nIdx,int nVal);
	virtual void SetAt(int nIdx,float fVal);
	virtual void SetAt(int nIdx,bool bVal);
	virtual void SetAt(int nIdx,const char* sVal);
	virtual void SetAt(int nIdx,IScriptObject *pObj);
	virtual void SetAtUD(int nIdx,USER_DATA nVal);
	virtual void SetNullAt(int nIdx);
	virtual bool GetAt(int nIdx,int &nVal);
	virtual bool GetAt(int nIdx,float &fVal);
	virtual bool GetAt(int nIdx,bool &bVal);
	virtual bool GetAt(int nIdx,const char* &sVal);
	virtual bool GetAt(int nIdx,IScriptObject *pObj);
	virtual bool GetAtUD(int nIdx,USER_DATA &nVal, int &nCookie);
	virtual bool BeginIteration();
	virtual bool MoveNext();
	virtual bool GetCurrent(int &nVal);
	virtual bool GetCurrent(float &fVal);
	virtual bool GetCurrent(bool &bVal);
	virtual bool GetCurrent(const char* &sVal);
	virtual bool GetCurrent(IScriptObject *pObj);
	virtual bool GetCurrentPtr(const void * &pObj);
	virtual bool GetCurrentFuncData(unsigned int * &pCode, int &iSize);
	virtual bool GetCurrentKey(int &nKey);
	virtual bool GetCurrentKey(const char* &sKey);
	virtual ScriptVarType GetCurrentType();
	virtual void EndIteration();
	virtual void Clear();
	virtual int Count();
	virtual bool Clone(IScriptObject *pObj);
	virtual void Dump(IScriptObjectDumpSink *p);
	virtual void SetNativeData(void *);
	virtual void *GetNativeData();
	virtual bool AddFunction(const char *sName,SCRIPT_FUNCTION pThunk,int nFuncID);
	virtual bool AddSetGetHandlers(SCRIPT_FUNCTION pSetThunk,SCRIPT_FUNCTION pGetThunk);
	virtual void RegisterParent(IScriptObjectSink *pSink);
	virtual void Detach();
	virtual void Release();
	virtual bool GetValueRecursive( const char *szPath, IScriptObject *pObj );

	// --------------------------------------------------------------------------

	// Create object from pool.
	void Recreate();

private: // -------------------------------------------------------------------
	
	//!
	bool CloneTable(int nSource,int nDest);
	//!
	static int SetTableTagHandler(lua_State *L);
	//!
	static int GetTableTagHandler(lua_State *L);
	//!
	static int IndexTagHandler(lua_State *L);
	//!
	int GetThisRef();

	struct SetGetParams
	{
		SCRIPT_FUNCTION m_pSetThunk;
		SCRIPT_FUNCTION m_pGetThunk;
		HTAG m_hSetGetTag;
	} *m_pSetGetParams;

	lua_State *						m_pLS;
	int										m_nRef;
	int										m_nIterationCounter;
	HTAG									m_hDelegationTag;
	IScriptObjectSink *		m_pSink;

	//////////////////////////////////////////////////////////////////////////
	// Flags.
	unsigned int					m_bDeleted;			//!<
	unsigned int					m_bAttached;		//!<

#ifdef _DEBUG
public:
	int m_nCreationNumber;
#endif

};

#endif // !defined(AFX_SCRIPTOBJECT_H__6EA3E6D6_4FF9_4709_BD62_D5A97C40DB68__INCLUDED_)
