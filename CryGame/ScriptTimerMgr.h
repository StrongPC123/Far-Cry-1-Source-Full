
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
// ScriptTimerMgr.h: interface for the CScriptTimerMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCRIPTTIMERMGR_H__D0CF3857_AEE5_4606_ADF9_549779C7DEEC__INCLUDED_)
#define AFX_SCRIPTTIMERMGR_H__D0CF3857_AEE5_4606_ADF9_549779C7DEEC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <map>

//////////////////////////////////////////////////////////////////////////
struct ScriptTimer{
	ScriptTimer(IScriptObject *_pTable,int64 _nStartTimer,int64 _nTimer,IScriptObject *_pUserData=NULL,bool _bUpdateDuringPause=true)
	{
		nTimer=_nTimer;
		nStartTime=_nStartTimer;
		pTable=_pTable;
		pUserData=_pUserData;
		bUpdateDuringPause=_bUpdateDuringPause;
	}
	~ScriptTimer()
	{
		if(pTable!=NULL)
			pTable->Release();
		if(pUserData!=NULL)
			pUserData->Release();
	}
	int64 nTimer;
	int64 nStartTime;
	IScriptObject *pTable;
	IScriptObject *pUserData;
	bool		bUpdateDuringPause;
};

typedef std::map<int,ScriptTimer *> ScriptTimerMap;
typedef ScriptTimerMap::iterator  ScriptTimerMapItor;

//////////////////////////////////////////////////////////////////////////
class CScriptTimerMgr  
{
public:
	CScriptTimerMgr(IScriptSystem *pScriptSystem,IEntitySystem *pS,IGame *pGame);
	virtual ~CScriptTimerMgr();
	int		AddTimer(IScriptObject *pTable,int64 nStartTimer,int64 nTimer,IScriptObject *pUserData,bool bUpdateDuringPause);
	void	RemoveTimer(int nTimerID);
	void	Update(int64 nCurrentTime);
	void	Reset();
	void	Pause(bool bPause); 
private:
	ScriptTimerMap m_mapTimers;
	ScriptTimerMap m_mapTempTimers;
	IScriptSystem *m_pScriptSystem;
	IEntitySystem *m_pEntitySystem;	
	IGame					*m_pGame;
	int		m_nLastTimerID;
	bool	m_bPause;
};

#endif // !defined(AFX_SCRIPTTIMERMGR_H__D0CF3857_AEE5_4606_ADF9_549779C7DEEC__INCLUDED_)
