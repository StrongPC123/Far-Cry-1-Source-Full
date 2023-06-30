
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
// XDemoMgr.h: interface for the CXDemoMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XDEMOMGR_H__C81155DD_B67B_46E3_A1AF_A2B88C7DC9B0__INCLUDED_)
#define AFX_XDEMOMGR_H__C81155DD_B67B_46E3_A1AF_A2B88C7DC9B0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CXGame;

class CXDemoMgr  
{
public:
	CXDemoMgr();
	virtual ~CXDemoMgr();
	//! create a new demo
	bool StartRecord(const char *sFileName, CStream &stm);
	//! finish recording/playing
	void Stop();
	//! open an existing demo file
	bool StartDemoPlay(const char *sFileName, CXGame *pGame);
	//! add a chunk to the opened demo
	bool AddChunk(float fTimestamp, CStream &stm, IEntity *e);
	//! read a chunk from the opened demo
	//! return true if is time to play this
	//! chunk false if not
	bool PlayChunk(float fCurrentTime, CXClient *pClient);
	//! End of demo
	bool EOD();
	
private:
	bool ReadChunk(CStream &stm, float &fTimestamp, Vec3 &angles);
	
	FILE *m_pFile;
	
	unsigned long m_nFileSize;
	float m_fCurrTime;
	float m_fGameTime;
	
	struct DemoChunk
	{
		CStream stm;
		float fTimestamp;
		bool bPlayed;
		Vec3 angles;
	} m_ChunkToPlay;
	
	bool bStreamStarted;
	CXGame *m_pGame;
}; 

#endif // !defined(AFX_XDEMOMGR_H__C81155DD_B67B_46E3_A1AF_A2B88C7DC9B0__INCLUDED_)
