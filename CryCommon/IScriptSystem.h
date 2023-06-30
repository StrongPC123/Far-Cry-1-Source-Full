#ifndef _ISCRIPTSYSTEM_H_
#define _ISCRIPTSYSTEM_H_

#if !defined(_XBOX) && !defined(LINUX)
	#ifdef CRYSCRIPTSYSTEM_EXPORTS
		#define CRYSCRIPTSYSTEM_API __declspec(dllexport)
	#else
		#define CRYSCRIPTSYSTEM_API __declspec(dllimport)
	#endif
#else
	#define CRYSCRIPTSYSTEM_API
#endif

class ICrySizer;
struct IWeakScriptObject;
struct IScriptObject;
struct ISystem;

#define HSCRIPT void*
#define HSCRIPTFUNCTION unsigned int
enum {
	//## [Sergiy] this is the same as LUA_NOREF; I think this is better to use since 
	//## in LUA documentation only LUA_NOREF(-2) and LUA_REFNIL (-1) are described as
	//## special values, 0 isn't. Please put 0 here if you know for sure and it is documented
	//## that 0 is invalid reference value for LUA
	INVALID_SCRIPT_FUNCTION = -2 
};
#define THIS_PTR void*
#define HTAG int
#define USER_DATA ULONG_PTR			//## AMD Port

typedef int(*SCRIPT_FUNCTION)(HSCRIPT hScript);
typedef int HBREAKPOINT;

enum BreakState{
	bsStepNext,
	bsStepInto,
	bsContinue,
	bsNoBreak
};

struct IFunctionHandler;
struct IScriptDebugSink;

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
/*! IScriptSystem callback interface
	this interface must be implemented by host applicatio the use the scripting system
	to receive error messages and notification
*/

// Summary:
//   Callback interface.
// Description:
//   This interface must be implemented by the host application in order to 
//   receive error messages and various notifications from the Scripting 
//   System.
struct IScriptSystemSink
{
	/*! called by the scripting system when an error occur
		@param sSourceFile the script file where the error occured
		@param sFuncName the script function where the error occured
		@param nLineNum the line number of the the script file where the error occured (-1 if there is no line no info)
		@param sErrorDesc descriptio of the error
	*/
	virtual void OnScriptError(const char *sSourceFile,const char *sFuncName,int nLineNum,const char *sErrorDesc) = 0;

	/*! called by the scripting system to ask for permission to change a global tagged value
		@return true if permission is granted, false if not
	*/
	virtual bool CanSetGlobal (const char* sVarName) = 0;

	/*! called by the scripting system when a global tagged value is set by inside a script
		@param sVarName the name of the variable that has been changed
	*/
	virtual void OnSetGlobal(const char *sVarName) = 0;
	/*! called by the scripting system for each load script when DumpLoadedScripts is called
		@param sScriptPath path of a script
	*/
	virtual void OnLoadedScriptDump(const char *sScriptPath) = 0;
	virtual void OnCollectUserData(INT_PTR nValue,int nCookie) = 0;		//## AMD Port
};

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
/*! main interface of the scripting engine
		this interface is mapped 1:1 on a script state
		all script loaded from the same interface instance are visible together
	*/
struct IScriptSystem
{
	//DOC-IGNORE-BEGIN
	//! internal use
	virtual IFunctionHandler *GetFunctionHandler()=0;
	//! internal use
	virtual HSCRIPT GetScriptHandle() = 0;
	//DOC-IGNORE-END

	/*! load a script file and run it
			@param sFileName path of the script file
			@param bRaiseError if true the script engine will call IScirptSystemSink::OnError() if
				an error will occur
			@return false if the execution fail if not true

			REMARKS: all global variables and function declared into the executed script
			will persist for all the script system lifetime
	*/
	virtual bool ExecuteFile(const char *sFileName,bool bRaiseError = true, bool bForceReload=false) = 0; 
	/*! execute an ASCII buffer
			@param sBuffer an 8bit ASCII buffer containing the script that must be executed
			@param bRaiseError if true the script engine will call IScirptSystemSink::OnError() if
				an error will occur
			@return false if the execution fail if not true

			REMARKS: all global variables and function declared into the executed buffer
			will persist for all the script system lifetime
	*/
	virtual bool ExecuteBuffer(const char *sBuffer, size_t nSize) = 0;
	/*! unload a script 
			@param sFileName apath of the script file

			NOTES: the script engine never load twice the same file(same path) because internally
			stores a list of loaded files.This function simple remove the specified path from
			this list
	*/
	virtual void UnloadScript(const char *sFileName) = 0;
	/*! unload all scripts
			
			NOTES: the script engine never load twice the same file(same path) because internally
			stores a list of loaded files.This function simple remove all entries from
			this list
	*/
	virtual void UnloadScripts() = 0;
	/*! realod a script
			@param sFileName apath of the script file
			@param bRaiseError if true the script engine will call IScirptSystemSink::OnError() if
				an error will occur
			@return false if the execution fail if not true
	*/
	virtual bool ReloadScript(const char *sFileName,bool bRaiseError = true) = 0;
	//! reload all scripts previously loaded
	virtual bool ReloadScripts() = 0;
	//! generate a OnLoadedScriptDump() for every loaded script
	virtual void DumpLoadedScripts() = 0 ;
	//! return a object that store all global variables as members 
	virtual IScriptObject *GetGlobalObject() = 0;

	/*! create a new IScriptObject that is not mapped to a object in the script
		this is used to store an object that lua pass to a C++ function
		@return a pointer to the created object
	*/
	virtual IScriptObject* CreateEmptyObject() = 0;
	/*!	create a new IScriptObject 
		@return a pointer to the created object
	*/
	virtual IScriptObject* CreateObject() = 0;
	/*!	create a global IScriptObject 
		@param sName the name of the object into the script scope
		@param pThis pointer to the C++ class that will receive a call from the script
			[this parameter can be NULL]
		@return a pointer to the created object
	*/
	virtual IScriptObject* CreateGlobalObject(const char *sName) = 0;

	
	/*! start a call to script function
		@param sTableName name of the script table that contai the function
		@param sFuncName function name


		CALLING A SCRIPT FUNCTION:
			to call a function in the script object you must
			call BeginCall
			push all parameters whit PushParam
			call EndCall

	
		EXAMPLE:

			m_ScriptSystem->BeginCall("Player","OnInit");

			m_ScriptSystem->PushParam(pObj);

			m_ScriptSystem->PushParam(nTime);

			m_ScriptSystem->EndCall();
			
	*/
	//##@{
	virtual int BeginCall(HSCRIPTFUNCTION hFunc) = 0;						 // Márcio: changed the return type 
	virtual int BeginCall(const char *sFuncName) = 0;						 // from void to int for error checking
	virtual int BeginCall(const char *sTableName, const char *sFuncName) = 0;//
	//##@}

	/*! end a call to script function
		@param ret reference to the variable that will store an eventual return value
	*/
	//##@{
	virtual void EndCall()=0;
	virtual void EndCall(int &nRet)=0;
	virtual void EndCall(float &fRet)=0;
	virtual void EndCall(const char *&sRet)=0;
#if defined(WIN64) || defined(LINUX)
	inline void EndCall(char *&sRet) {EndCall ((const char*&)sRet);}
#endif
	virtual void EndCall(bool &bRet)=0;
	virtual void EndCall(IScriptObject *pScriptObject)=0;
	//##@}

	/*! function under development ingnore it*/
	//##@{
	virtual HSCRIPTFUNCTION GetFunctionPtr(const char *sFuncName) = 0;
	virtual	HSCRIPTFUNCTION GetFunctionPtr(const char *sTableName, const char *sFuncName) = 0;
	//##@}
	virtual void ReleaseFunc(HSCRIPTFUNCTION f) = 0;
	/*! push a parameter during a function call
		@param val value that must be pushed
	*/
	//##@{
	virtual void PushFuncParam(int nVal) = 0;
	virtual void PushFuncParam(float fVal) = 0;
	virtual void PushFuncParam(const char *sVal) = 0;
	virtual void PushFuncParam(bool bVal) = 0;
	virtual void PushFuncParam(IScriptObject *pVal) = 0;
	//##@}

	/*! set a global script variable
		@param sKey variable name
		@param val variable value
	*/
	//##@{
	virtual void SetGlobalValue(const char *sKey, int nVal) = 0;
	virtual void SetGlobalValue(const char *sKey, float fVal) = 0;
	virtual void SetGlobalValue(const char *sKey, const char *sVal) = 0;
	virtual void SetGlobalValue(const char *sKey, IScriptObject *pObj) = 0;
	//##@}

	//! get the value of a global variable
	//##@{
	virtual bool GetGlobalValue(const char *sKey, int &nVal) = 0;
	virtual bool GetGlobalValue(const char *sKey, float &fVal) = 0;
	virtual bool GetGlobalValue(const char *sKey, const char * &sVal) = 0;
	virtual bool GetGlobalValue(const char *sKey, IScriptObject *pObj) = 0;
	virtual void SetGlobalToNull(const char *sKey) = 0;
	//##@}

	/*! create a global tagged value
		a tagged value is a varible tha is visible in lua as a normal
		script variable but his value is stored into a c++ defined memory block
		@param sKey name of the value
		@param pVal pointer to the C++ variable tha will store the value
		@return if succeded an handle to the tagged value else NULL
	*/
	//##@{
	virtual HTAG CreateTaggedValue(const char *sKey, int *pVal) = 0;
	virtual HTAG CreateTaggedValue(const char *sKey, float *pVal) = 0;
	virtual HTAG CreateTaggedValue(const char *sKey, char *pVal) = 0;
	//##@}

	virtual USER_DATA CreateUserData(INT_PTR nVal,int nCookie)=0;
	/*! remove a tagged value
		@param tag handle to a tagged value
	*/
	virtual void RemoveTaggedValue(HTAG tag) = 0;

	/*! generate a script error
		@param sErr "printf like" string tha define the error
	*/
	virtual void RaiseError(const char *sErr,...) = 0;

	/*! force a Garbage collection cycle
		in the current status of the engine the automatic GC is disabled
		so this function must be called explicitly
	*/
	virtual void ForceGarbageCollection() = 0;

	/*! number of "garbaged" object
	*/
	virtual int GetCGCount() = 0;

	/*! legacy function*/
	virtual void SetGCThreshhold(int nKb) = 0;

	/*! un bind all gc controlled userdata variables (from this point is the application that must explicitly delete those objs)*/
	virtual void UnbindUserdata() = 0;

	/*! release and destroy the script system*/
	virtual void Release() = 0;

	//!debug functions
	//##@{
	virtual void EnableDebugger(IScriptDebugSink *pDebugSink) = 0;
	virtual IScriptObject *GetBreakPoints() = 0;
	virtual HBREAKPOINT AddBreakPoint(const char *sFile,int nLineNumber)=0;
	virtual IScriptObject *GetLocalVariables(int nLevel = 0) = 0;
	/*!return a table containing 1 entry per stack level(aka per call)
		an entry will look like this table
		
		[1]={
			description="function bau()",
			line=234,
			sourcefile="/scripts/bla/bla/bla.lua"
		}
	 
	 */
	virtual IScriptObject *GetCallsStack() = 0;
	virtual void DebugContinue() = 0;
	virtual void DebugStepNext() = 0;
	virtual void DebugStepInto() = 0;
	virtual void DebugDisable() = 0;
	virtual BreakState GetBreakState() = 0;
	//##@}
	virtual void GetMemoryStatistics(ICrySizer *pSizer) = 0;
	//! Is not recusive but combines the hash values of the whole table when the specifies variable is a table
	//! otherwise is has to be a lua function
	//!	@param sPath zero terminated path to the variable (e.g. _localplayer.cnt), max 255 characters
	//!	@param szKey zero terminated name of the variable (e.g. luaFunc), max 255 characters
	//! @param dwHash is used as input and output
	virtual void GetScriptHash( const char *sPath, const char *szKey, unsigned int &dwHash )=0;

	//////////////////////////////////////////////////////////////////////////
	// Called one time after initialization of system to register script system console vars.
	//////////////////////////////////////////////////////////////////////////
	virtual void PostInit() = 0;
};

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
struct IScriptObjectSink
{
	virtual void OnRelease()=0;
};

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
enum ScriptVarType
{
	svtNull = 0,
	svtString,
	svtNumber,
	svtFunction,
	svtObject,
	svtUserData,
};

// Returns literal representation of the type value
inline const char* ScriptVarTypeAsCStr(ScriptVarType t)
{
	switch (t)
	{
		case svtNull: return "Null";
		case svtString: return "String";
		case svtNumber: return "Number";
		case svtFunction: return "Function";
		case svtObject: return "Object";
		case svtUserData: return "UserData";
		default: return "#Unknown";
	}
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

struct IScriptObjectDumpSink
{
	virtual void OnElementFound(const char *sName,ScriptVarType type) = 0;
	virtual void OnElementFound(int nIdx,ScriptVarType type) = 0;
};

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
struct IScriptObject
{
	//DOC-IGNORE-BEGIN
	//! internal use
	virtual int GetRef() = 0;
	//! internal use
	//DOC-IGNORE-END
	virtual void Attach() = 0;
	virtual void Attach(IScriptObject *so) = 0;

	virtual void Delegate(IScriptObject *pObj) = 0;
	virtual void PushBack(int nVal) = 0;
	virtual void PushBack(float fVal) = 0;
	virtual void PushBack(const char *sVal) = 0;
	virtual void PushBack(bool bVal) = 0;
	virtual void PushBack(IScriptObject *pObj) = 0;
		
	/*! Set the value of a memeber varible
		@param sKey variable name
		@param val variable value
	*/
	//##@{
	virtual void SetValue(const char *sKey, int nVal) = 0;
	virtual void SetValue(const char *sKey, float fVal) = 0;
	virtual void SetValue(const char *sKey, const char *sVal) = 0;
	virtual void SetValue(const char *sKey, bool bVal) = 0;
	virtual void SetValue(const char *sKey, IScriptObject *pObj) = 0;
	virtual void SetValue(const char *sKey, USER_DATA ud) = 0;
	//##@}
	/*! Set the value of a member variable to nil 
		@param sKey variable name
	*/
	virtual void SetToNull(const char *sKey) = 0;

	/*! Get the value of a memeber varible
		@param sKey variable name
		@param val reference to the C++ variable that will store the value
		@return false if failed true if succeded
	*/
	//##@{
	virtual bool GetValue(const char *sKey, int &nVal) = 0;
	virtual bool GetValue(const char *sKey, float &fVal) = 0;
	virtual bool GetValue(const char *sKey, bool &bVal) = 0;
	virtual bool GetValue(const char *sKey, const char* &sVal) = 0;
//#if defined(WIN64)
//	inline bool GetValue(const char *sKey, char* &sVal) {return GetValue (sKey, (const char*&)sVal);}
//#endif
	virtual bool GetValue(const char *sKey, IScriptObject *pObj) = 0;
	virtual bool GetValue(const char *sKey, HSCRIPTFUNCTION &funcVal) = 0;
	virtual bool GetUDValue(const char *sKey, USER_DATA &nValue, int &nCookie) = 0;		//## AMD Port
//#ifdef WIN64
	inline bool GetUDValue(const char *sKey, INT_PTR &nValue, int &nCookie){return GetUDValue(sKey, (USER_DATA&)nValue, nCookie);}		//## AMD Port
//#endif
	//! used to create a hash value out of a lua function (for cheat protection)
	virtual bool GetFuncData(const char *sKey, unsigned int * &pCode, int &iSize) = 0;
	//##@}

	/*! Get the value of a memeber varible
		@param sKey variable name
		@param val reference to the C++ variable that will store the value
		@return false if failed true if succeded
	*/
	//##@{
	virtual bool BeginSetGetChain()=0; 
	virtual bool GetValueChain(const char *sKey, int &nVal) = 0;
	//virtual bool GetValueChain(const char *sKey, unsigned int &nVal) = 0;
	virtual bool GetValueChain(const char *sKey, float &fVal) = 0;
	virtual bool GetValueChain(const char *sKey, bool &bVal) = 0;
	virtual bool GetValueChain(const char *sKey, const char* &sVal) = 0;
	virtual bool GetValueChain(const char *sKey, IScriptObject *pObj) = 0;
	virtual bool GetValueChain(const char *sKey, HSCRIPTFUNCTION &funcVal) = 0;
	virtual bool GetUDValueChain(const char *sKey, USER_DATA &nValue, int &nCookie) = 0;		//## AMD Port
//#ifdef WIN64
	inline  bool GetUDValueChain(const char *sKey, INT_PTR &nValue, int &nCookie){return GetUDValueChain(sKey, (USER_DATA&)nValue, nCookie);}		//#ingore AMD Port
//#endif
	virtual void SetValueChain(const char *sKey, int nVal) = 0;
	virtual void SetValueChain(const char *sKey, float fVal) = 0;
	virtual void SetValueChain(const char *sKey, const char *sVal) = 0;
	virtual void SetValueChain(const char *sKey, bool bVal) = 0;
	virtual void SetValueChain(const char *sKey, IScriptObject *pObj) = 0;
	virtual void SetValueChain(const char *sKey, USER_DATA ud) = 0;
	virtual void SetToNullChain(const char *sKey)=0;
	virtual void EndSetGetChain()=0; 
	//##@}

	/*!Get the vaue type of a table member 
		@param sKey variable name
		@return the value type (svtNull if doesn't exist)
	*/
	virtual ScriptVarType GetValueType(const char *sKey) = 0;
	virtual ScriptVarType GetAtType(int nIdx) = 0;
	
	/*! Set the value of a memeber varible at the specified index
		this mean that you will use the object as vector into the script
		@param nIdx index of the variable
		@param val variable value
	*/
	//##@{
	virtual void SetAt(int nIdx,int nVal)=0;
	virtual void SetAt(int nIdx,float fVal)=0;
	virtual void SetAt(int nIdx,bool bVal)=0;
	virtual void SetAt(int nIdx,const char* sVal)=0;
	virtual void SetAt(int nIdx,IScriptObject *pObj)=0;
	virtual void SetAtUD(int nIdx,USER_DATA nValue)=0;
	//##@}

	/*! Set the value of a member variable to nil at the specified index
		@param nIdx index of the variable
	*/
	virtual void SetNullAt(int nIdx)=0;

	/*! Get the value of a memeber varible at the specified index
		@param nIdx index of the variable
		@param val reference to the C++ variable that will store the value
		@return false if failed true if succeded
	*/
	//##@{
	virtual bool GetAt(int nIdx,int &nVal)=0;
	virtual bool GetAt(int nIdx,float &fVal)=0;
	virtual bool GetAt(int nIdx,bool &bVal)=0;
	virtual bool GetAt(int nIdx,const char* &sVal)=0;
	virtual bool GetAt(int nIdx,IScriptObject *pObj)=0;
	virtual bool GetAtUD(int nIdx, USER_DATA &nValue, int &nCookie)=0;
	//##@}

	virtual bool BeginIteration() = 0;
	virtual bool MoveNext() = 0;
	virtual bool GetCurrent(int &nVal) = 0;
	virtual bool GetCurrent(float &fVal) = 0;
	virtual bool GetCurrent(bool &bVal) = 0;
	virtual bool GetCurrent(const char* &sVal) = 0;
	virtual bool GetCurrent(IScriptObject *pObj) = 0;
	//! used to get a unique identifier for the table (to iterate without problems with cycles)
	virtual bool GetCurrentPtr(const void * &pObj)=0;
	//! used to create a hash value out of a lua function (for cheat protection)
	virtual bool GetCurrentFuncData(unsigned int * &pCode, int &iSize) = 0;
	virtual bool GetCurrentKey(const char* &sVal) = 0;

#if defined(WIN64) || defined(LINUX)
	inline bool GetCurrentKey(char* &sVal) {return GetCurrentKey((const char*&)sVal);}
	inline bool GetCurrent(char* &sVal) {return GetCurrent ((const char*&)sVal);}
	inline bool GetAt(int nIdx,char* &sVal) {return GetAt(nIdx, (const char*&)sVal);}
#endif

	virtual bool GetCurrentKey(int &nKey) = 0;
	virtual ScriptVarType GetCurrentType() = 0;
	virtual void EndIteration() = 0;

	virtual void SetNativeData(void *) =0;
	virtual void *GetNativeData() = 0;

	virtual void Clear() = 0;
	//! get the count of elements into the object
	virtual int Count()=0 ;

  	/* 
	*/
	virtual bool Clone(IScriptObject *pObj)=0;
	//DOC-IGNORE-BEGIN
	/*under development*/
	virtual void Dump(IScriptObjectDumpSink *p) = 0;
	//DOC-IGNORE-END
	/*!	add a function to the object
		@param name of the function
		@param C++ function pointer(declared as SCRIPT_FUNCTION)
		@param nFuncID id of the function that will be passed beck by the engine
		@return false if failed true if succeded
	*/
	virtual bool AddFunction(const char *sName, SCRIPT_FUNCTION pThunk, int nFuncID) = 0;
	//!
	virtual bool AddSetGetHandlers(SCRIPT_FUNCTION pSetThunk,SCRIPT_FUNCTION pGetThunk) = 0;
	/*!	register the host object as parent
		@param pSink pointer to an object that implement IScriptObjectSink

		NOTE: if the parent is registered the script object will notify when is deleted
	*/
	virtual void RegisterParent(IScriptObjectSink *pSink) = 0;
	//! detach the IScriptObject from the "real" script object
	//! used to detach from the C++ code quick objects like vectors or temporary structures
	virtual void Detach() = 0;
	//! delete the IScriptObject/
	virtual void Release() = 0;
	//! @param szPath e.g. "cnt.table1.table2", "", "mytable", max 255 characters
	//! @return true=path was valid, false otherwise
	virtual bool GetValueRecursive( const char *szPath, IScriptObject *pObj ) = 0;
};



////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//DOC-IGNORE-BEGIN
/*! internal use*/
struct IWeakScriptObject
{
	virtual IScriptObject *GetScriptObject() = 0;
	virtual void Release() = 0 ;
};
//DOC-IGNORE-END
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// Description:
//   This interface is used by the C++ function mapped to the script
//   to retrieve the function parameters passed by the script and
//   to return an optiona result value to the script.
struct IFunctionHandler
{
	//DOC-IGNORE-BEGIN
	/*! internal use */
	virtual void __Attach(HSCRIPT hScript) = 0;
	/*! internal use */
	virtual THIS_PTR GetThis() = 0;
	//virtual THIS_PTR GetThis2() = 0;
	/*! internal use */
	virtual int GetFunctionID() = 0;
	//DOC-IGNORE-END

	//!	Get the number of parameter passed by lua
	virtual int GetParamCount() = 0;

	/*! get the nIdx param passed by the script
		@param nIdx 1-based index of the parameter
		@param val reference to the C++ variable that will store the value
		@param nReference should the function create strong reference to the object? By default 0, means weak reference is created
	*/
	//##@{
	virtual bool GetParam(int nIdx, int &n) = 0;
	virtual bool GetParam(int nIdx, float &f) = 0;
	virtual bool GetParam(int nIdx, const char * &s) = 0;
#if defined(WIN64) || defined(LINUX)
	inline bool GetParam(int nIdx, char * &s) {return GetParam(nIdx, (const char*&)s);}
#endif
#if defined(WIN64) || defined(LINUX64)
	virtual bool GetParam(int nIdx, INT_PTR &n) = 0;	//## AMD Port
#endif
	virtual bool GetParam(int nIdx,bool &b) = 0;
	virtual bool GetParam(int nIdx, IScriptObject *pObj) = 0;
	virtual bool GetParam(int nIdx, HSCRIPTFUNCTION &hFunc, int nReference = 0) = 0;
	virtual bool GetParam(int nIdx,USER_DATA &ud)=0;
	virtual bool GetParamUDVal(int nIdx,USER_DATA &val,int &cookie)=0;	//## AMD Port
//#ifdef WIN64
	inline bool GetParamUDVal(int nIdx,INT_PTR&val,int &cookie){return GetParamUDVal(nIdx, (USER_DATA&)val, cookie);}	//## AMD Port
//#endif
	//##@}
	virtual ScriptVarType GetParamType(int nIdx) = 0;

	/*! get the return value that you must return from your "SCRIPT_FUNCTION"
		@param the value that xou want to return to the script
	*/
	//##@{
	virtual int EndFunctionNull() = 0;
	virtual int EndFunction(int nRetVal) = 0;
	virtual int EndFunction(float fRetVal) = 0;
	virtual int EndFunction(const char* fRetVal) = 0;
	virtual int EndFunction(bool bRetVal) = 0;
	virtual int EndFunction(IScriptObject *pObj) = 0;
	virtual int EndFunction(HSCRIPTFUNCTION hFunc) = 0;
	virtual int EndFunction(USER_DATA ud) = 0;
	virtual int EndFunction() = 0;

	// 2 return params versions.
	virtual int EndFunction(int nRetVal1,int nRetVal2) = 0;
	virtual int EndFunction(float fRetVal1,float fRetVal2) = 0;
	//##@}

	virtual void Unref(HSCRIPTFUNCTION hFunc) = 0;
};

//DOC-IGNORE-BEGIN
//! under development
struct ScriptDebugInfo{
	const char *sSourceName;
	int nCurrentLine;
};

//! under development
struct IScriptDebugSink{
	virtual void OnLoadSource(const char *sSourceName,unsigned char *sSource,long nSourceSize)=0;
	virtual void OnExecuteLine(ScriptDebugInfo &sdiDebugInfo)=0;
};
//DOC-IGNORE-END

/////////////////////////////////////////////////////////////////////////////
//Utility classes
/////////////////////////////////////////////////////////////////////////////

//#define USE_WEAK_OBJS
class _SmartScriptObject
{
	_SmartScriptObject(const _SmartScriptObject&)
	{
	}
	_SmartScriptObject& operator =(const _SmartScriptObject &)
	{
		return *this;
	}
	_SmartScriptObject& operator =(IScriptObject *)
	{
		return *this;
	}
public:
	_SmartScriptObject()
	{
			m_pSO=NULL;
	}
	explicit _SmartScriptObject(IScriptSystem *pSS,IScriptObject *p)
	{
    	m_pSO=pSS->CreateEmptyObject();
			m_pSO->Attach(p);
	}
	explicit _SmartScriptObject(IScriptSystem *pSS,bool bCreateEmpty=false)
	{
		if(!bCreateEmpty)
		{
			m_pSO=pSS->CreateObject();
		}
		else{
			m_pSO=pSS->CreateEmptyObject();
		}
	}
	~_SmartScriptObject()
	{
		if(m_pSO)
			m_pSO->Release();

	}
	IScriptObject *operator ->(){
		return m_pSO;
	}
	IScriptObject *operator *(){
		return m_pSO;
	}
	operator const IScriptObject*() const
	{
		return m_pSO;
	}
	operator IScriptObject*()
	{
		return m_pSO;
	}
	bool Create(IScriptSystem *pSS)
	{
		m_pSO=pSS->CreateObject();
		return m_pSO?true:false;
	}

	//////////////////////////////////////////////////////////////////////////
	// Boolean comparasions.
	//////////////////////////////////////////////////////////////////////////
	bool operator !() const 
	{
		return m_pSO == NULL;
	};
	bool  operator ==(const IScriptObject* p2) const 
	{
		return m_pSO == p2;
	};
	bool  operator ==(IScriptObject* p2) const 
	{
		return m_pSO == p2;
	};
	bool  operator !=(const IScriptObject* p2) const 
	{
		return m_pSO != p2;
	};
	bool  operator !=(IScriptObject* p2) const 
	{
		return m_pSO != p2;
	};
	bool  operator <(const IScriptObject* p2) const 
	{
		return m_pSO < p2;
	};
	bool  operator >(const IScriptObject* p2) const 
	{
		return m_pSO > p2;
	};

protected:
	IScriptObject *m_pSO;
};

class _HScriptFunction
{
public:
	_HScriptFunction(){m_pScriptSystem=0;m_hFunc=0;};
	_HScriptFunction(IScriptSystem *pSS){m_pScriptSystem=pSS;m_hFunc=0;}
	_HScriptFunction(IScriptSystem *pSS,HSCRIPTFUNCTION hFunc){m_pScriptSystem=pSS;m_hFunc=0;}
	~_HScriptFunction(){ if(m_hFunc)m_pScriptSystem->ReleaseFunc(m_hFunc);m_hFunc=0; }
	void Init(IScriptSystem *pSS,HSCRIPTFUNCTION hFunc){if(m_hFunc)m_pScriptSystem->ReleaseFunc(m_hFunc);m_hFunc=hFunc;m_pScriptSystem=pSS;}
	operator HSCRIPTFUNCTION() const
	{
		return m_hFunc;
	}
	_HScriptFunction& operator =(HSCRIPTFUNCTION f){
		if(m_hFunc)m_pScriptSystem->ReleaseFunc(m_hFunc);
		m_hFunc=f;
		return *this;
	}
private:
	HSCRIPTFUNCTION m_hFunc;
	IScriptSystem *m_pScriptSystem;
};

extern "C"{
	CRYSCRIPTSYSTEM_API IScriptSystem *CreateScriptSystem( ISystem *pSystem,IScriptSystemSink *pSink,IScriptDebugSink *pDebugSink,bool bStdLibs);
}


typedef IScriptSystem *(*CREATESCRIPTSYSTEM_FNCPTR)(ISystem *pSystem,IScriptSystemSink *pSink,IScriptDebugSink *pDebugSink,bool bStdLibs);

#endif //_ISCRIPTSYSTEM_H_
