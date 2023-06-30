// XConsoleVariable.h: interface for the CXConsoleVariable class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XCONSOLEVARIABLE_H__AB510BA3_4D53_4C45_A2A0_EA15BABE0C34__INCLUDED_)
#define AFX_XCONSOLEVARIABLE_H__AB510BA3_4D53_4C45_A2A0_EA15BABE0C34__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VAR_STRING_SIZE 256

#include <IScriptSystem.h>

struct IScriptSystem;
class CXConsole;


class CXConsoleVariable :
public ICVar
{                     
public:
	//! constructor
	//! \param pConsole must not be 0
	CXConsoleVariable(CXConsole *pConsole,IScriptSystem *pSS,const char *sName,int nFlags,int nType, const char *help);
	//! constructor
	//! \param pConsole must not be 0
	CXConsoleVariable(CXConsole *pConsole,IScriptSystem *pSS,const char *sName,void *pVar,int nFlags,int nType, const char *help);
	//! destructor
	virtual ~CXConsoleVariable();

	void GetMemoryUsage (class ICrySizer* pSizer);

	// ------------------------------------------------------------------------------------------

//! @see ICVar
	virtual int GetIVal();
//! @see ICVar
	virtual float GetFVal();
//! @see ICVar
	virtual char *GetString();
//! @see ICVar
	virtual void Set(const char* s);
//! @see ICVar
	virtual void ForceSet(const char* s);
//! @see ICVar
	virtual void Set(float f);
//! @see ICVar
	virtual void Set(int i);
//! @see ICVar
	virtual void ClearFlags(int flags);
//! @see ICVar
	virtual int GetFlags();
//! @see ICVar
	virtual int SetFlags( int flags );
//! @see ICVar
	virtual int GetType();
//! @see ICVar
	virtual const char* GetName();
//! @see ICVar
	virtual const char* GetHelp();
//! @see ICVar
	virtual void Release();
//! @see ICVar
	virtual void Refresh();


	//! Changes the variable storage pointer
	void SetSrc(void* pSrc);


//hack
	bool							m_bLoadedFromScript;					//!<

	bool CanGetValueFromScript();

private: 	// ------------------------------------------------------------------------------------------

	char							m_sName[128];									//!<
	const char *			m_psHelp;											//!< pointer to the help string, might be 0
	int								m_nFlags;											//!< e.g. VF_SERVER_ONCE, VF_CHEAT, ...
	int								m_nType;											//!< can be only CVAR_STRING or CVAR_INT or CVAR_FLOAT
	
	char *						m_sValue;											//!<
	int *							m_nValue;											//!<
	float *						m_fValue;											//!<

	int								m_localInt;										//!<
	float							m_localFloat;									//!<
	char							localString[VAR_STRING_SIZE];	//!<

	// indicates whether the principal value (m_sValue for string-typed var; m_nValue for int-typed var; m_fValue for float-typed var)
	// needs to be deleted along with the duplicated values. If this is false, the principal value doesn't belong to this object and doesn't
	// need to be deallocated. The other two need to be deallocated ANYWAY
	bool							m_bAutoDelete;								//!<
	
	CXConsole *				m_pConsole;										//!<
	IScriptSystem *		m_pScriptSystem;							//!<
	HTAG							m_hScriptTag;									//!<
};

#endif // !defined(AFX_XCONSOLEVARIABLE_H__AB510BA3_4D53_4C45_A2A0_EA15BABE0C34__INCLUDED_)
