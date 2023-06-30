
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
// TagPoint.h: interface for the CTagPoint class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TAGPOINT_H__6BCD9697_65CE_496C_8B56_9EAFB4D006AD__INCLUDED_)
#define AFX_TAGPOINT_H__6BCD9697_65CE_496C_8B56_9EAFB4D006AD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <IMarkers.h>
#include <string>

class CTagPoint : public ITagPoint 
{

	Vec3					m_vPosition;
	Vec3					m_vAngles;
	string		m_sName;
	CXGame				*m_pGame;

public:
	CTagPoint(CXGame *pGame){m_pGame = pGame;}
	virtual ~CTagPoint(){};

	virtual void	SetPos(const Vec3 &pos) { m_vPosition = pos;	}
	virtual void	GetPos(Vec3 &pos) { pos = m_vPosition;	}

	virtual void SetAngles(const Vec3 &angles) {m_vAngles = angles;}
	virtual void GetAngles(Vec3 &angles) {angles = m_vAngles;}

	virtual bool SetName(const char *pName) { return m_pGame->RenameTagPoint(m_sName,pName);}
	virtual void OverrideName(const string &name)  { m_sName = name;}
	virtual char *GetName() { return (char *) m_sName.c_str(); }

	void Release() { delete this; }

	unsigned		MemStats();

};

#endif // !defined(AFX_TAGPOINT_H__6BCD9697_65CE_496C_8B56_9EAFB4D006AD__INCLUDED_)
