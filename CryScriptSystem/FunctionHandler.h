// FunctionHandler.h: interface for the CFunctionHandler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FUNCTIONHANDLER_H__CB02D9A1_DFAA_4DA3_8DF7_E2E8769F4ECE__INCLUDED_)
#define AFX_FUNCTIONHANDLER_H__CB02D9A1_DFAA_4DA3_8DF7_E2E8769F4ECE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IScriptSystem.h"
struct lua_State;
/*! IFunctionHandler implementation
	@see IFunctionHandler
*/
class CFunctionHandler : public IFunctionHandler  
{
public:
	CFunctionHandler();
	virtual ~CFunctionHandler();
	
public:
	//IFunctionHandler
	void __Attach(HSCRIPT hScript);
	THIS_PTR GetThis();
	//THIS_PTR GetThis2();
	int GetFunctionID();
	int GetParamCount();

	bool GetParam(int nIdx,int &n);					//AMD Port				
#if defined(WIN64) || defined(LINUX64)
	bool GetParam(int nIdx,INT_PTR &n);					//AMD Port				
#endif
	bool GetParam(int nIdx,float &f);
	bool GetParam(int nIdx,const char * &s);
	bool GetParam(int nIdx,bool &b);
	bool GetParam(int nIdx,IScriptObject *pObj);
	bool GetParam(int nIdx,HSCRIPTFUNCTION &hFunc, int nReference);
	bool GetParam(int nIdx,USER_DATA &ud);
	bool GetParamUDVal(int nIdx,USER_DATA&val,int &cookie);	//AMD Port
	ScriptVarType GetParamType(int nIdx);
	int EndFunction(int nRetVal);
	int EndFunction(float fRetVal);
	int EndFunction(const char* fRetVal);
	int EndFunction(bool bRetVal);
	int EndFunction(IScriptObject *pObj);
	int EndFunction(HSCRIPTFUNCTION hFunc);
	int EndFunction(USER_DATA ud);
	int EndFunction();
	int EndFunctionNull();

	// 2 return params versions.
	virtual int EndFunction(int nRetVal1,int nRetVal2);
	virtual int EndFunction(float fRetVal1,float fRetVal2);

	void Unref (HSCRIPTFUNCTION hFunc);	
private:
	lua_State *m_pLS;
};

#endif // !defined(AFX_FUNCTIONHANDLER_H__CB02D9A1_DFAA_4DA3_8DF7_E2E8769F4ECE__INCLUDED_)
