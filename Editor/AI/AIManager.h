////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   aimanager.h
//  Version:     v1.00
//  Created:     11/9/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __aimanager_h__
#define __aimanager_h__

#if _MSC_VER > 1000
#pragma once
#endif

// forward declarations.
class CAIGoalLibrary;
class CAIBehaviorLibrary;

//////////////////////////////////////////////////////////////////////////
class CAIManager
{
public:
	CAIManager();
	~CAIManager();
	void Init( ISystem *system );

	IAISystem*	GetAISystem();

	CAIGoalLibrary*	GetGoalLibrary() { return m_goalLibrary; };
	CAIBehaviorLibrary*	GetBehaviorLibrary() { return m_behaviorLibrary; };

	//////////////////////////////////////////////////////////////////////////
	//! AI Anchor Actions enumeration.
	void GetAnchorActions( std::vector<CString> &actions ) const;
	int AnchorActionToId( const char *sAction ) const;

	// Enumerate all AI characters.

	//////////////////////////////////////////////////////////////////////////
	void ReloadScripts();

private:
	void EnumAnchorActions();

	CAIGoalLibrary* m_goalLibrary;
	CAIBehaviorLibrary* m_behaviorLibrary;
	IAISystem*	m_aiSystem;

	//! AI Anchor Actions.
	friend struct CAIAnchorDump;
	typedef std::map<CString,int> AnchorActions;
	AnchorActions m_anchorActions;
};

#endif // __aimanager_h__