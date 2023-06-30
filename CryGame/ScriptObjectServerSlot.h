
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
//////////////////////////////////////////////////////////////////////

#ifndef _SCRIPTOBJECTSERVERSLOT_H_
#define _SCRIPTOBJECTSERVERSLOT_H_

#include <IScriptSystem.h>
#include <_ScriptableEx.h>

class CXServerSlot;

/*! This class implements script-functions for exposing the serverslots functionalities

	REMARKS:
	this object doesn't have a global mapping(is not present as global variable into the script state)
	This object isn't instantiated when the client is connected to a remote server
	
	IMPLEMENTATIONS NOTES:
	These function will never be called from C-Code. They're script-exclusive.
*/
class CScriptObjectServerSlot :
public _ScriptableEx<CScriptObjectServerSlot>
{
public:
	//! constructor
	CScriptObjectServerSlot();
	//! destructor
	virtual ~CScriptObjectServerSlot();
	void Create(IScriptSystem *pScriptSystem);
	void SetServerSlot(CXServerSlot *pSS){m_pSS=pSS;}
	static void InitializeTemplate(IScriptSystem *pSS);
private:
	int BanByID(IFunctionHandler *pH);
	int BanByIP(IFunctionHandler *pH);
	int SetPlayerId(IFunctionHandler *pH);
	int GetPlayerId(IFunctionHandler *pH);
	int GetName(IFunctionHandler *pH);
	int GetModel(IFunctionHandler *pH);
	int GetColor(IFunctionHandler *pH);
	int SetGameState(IFunctionHandler *pH);
	int SendText(IFunctionHandler *pH);
	int Ready(IFunctionHandler *pH);
	int IsReady(IFunctionHandler *pH);
	int IsContextReady(IFunctionHandler *pH);
	int ResetPlayTime(IFunctionHandler *pH);
	int GetPlayTime(IFunctionHandler *pH);
	int SendCommand(IFunctionHandler *pH);
	int Disconnect(IFunctionHandler *pH);
	int GetId(IFunctionHandler *pH);
	int GetPing(IFunctionHandler *pH);

	CXServerSlot *				m_pSS;				//!<
};

#endif
