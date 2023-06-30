#ifndef _SCRIPTOBJECTSYNCHED2DTABLE_H_
#define _SCRIPTOBJECTSYNCHED2DTABLE_H_

#include <IScriptSystem.h>
#include <_ScriptableEx.h>

class CSynched2DTable;

/*! In this class are all Synched2DTable-related script-functions implemented.

	IMPLEMENTATIONS NOTES:
	These function will never be called from C-Code. They're script-exclusive.
*/
class CScriptObjectSynched2DTable : public _ScriptableEx<CScriptObjectSynched2DTable>, public IScriptObjectSink
{
public:
	//! constructor
	CScriptObjectSynched2DTable() {}
	//! destructor
	virtual ~CScriptObjectSynched2DTable() {}

	//!
	bool Create(IScriptSystem *pScriptSystem, CSynched2DTable *pSynched2DTable);

	//!
	void OnRelease()
	{
		m_pScriptThis=NULL;
		delete this;
	}

	// is called from CXGame::Reset() to add the scripting 
	static void InitializeTemplate(IScriptSystem *pSS);

	// script functions -----------------------------------------------

	int SetEntryXY( IFunctionHandler *pH );
	//! returns "" if the entry is not present
	int GetEntryXY( IFunctionHandler *pH );
	//! like GetEntryXY but returns 0.0 in case the value is not present or a string
	int GetEntryFloatXY( IFunctionHandler *pH );
	int SetEntriesY( IFunctionHandler *pH );
	int GetColumnCount( IFunctionHandler *pH );
	int GetLineCount( IFunctionHandler *pH );

private: // ---------------------------------------------------------

	CSynched2DTable *				m_pSynched2DTable;				//!<
};

#endif //_SCRIPTOBJECTSYNCHED2DTABLE_H_