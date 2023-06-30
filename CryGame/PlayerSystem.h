
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
// PlayerSystem.h: interface for the CPlayerSystem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLAYERSYSTEM_H__9A74BB7C_D0B5_4B01_BCF7_ED9E03F7A25C__INCLUDED_)
#define AFX_PLAYERSYSTEM_H__9A74BB7C_D0B5_4B01_BCF7_ED9E03F7A25C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


typedef std::vector<EntityClassId> PlayerVector;

//!store all player entity class ids
class CPlayerSystem  
{
	PlayerVector	m_vPlayerClasses;

public:
	CPlayerSystem(){}
	virtual ~CPlayerSystem(){}

	void AddPlayerClass(const EntityClassId classid) { m_vPlayerClasses.push_back(classid);}
	bool IsPlayerClass(const EntityClassId classid) { return ( m_vPlayerClasses.end() != std::find(m_vPlayerClasses.begin(),m_vPlayerClasses.end(), classid) );}
};

#endif // !defined(AFX_PLAYERSYSTEM_H__9A74BB7C_D0B5_4B01_BCF7_ED9E03F7A25C__INCLUDED_)
