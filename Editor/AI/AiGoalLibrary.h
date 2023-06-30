////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   aigoallibrary.h
//  Version:     v1.00
//  Created:     21/3/2002 by Timur.
//  Compilers:   Visual C++ 7.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __aigoallibrary_h__
#define __aigoallibrary_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "AiGoal.h"
class CAIBehaviorLibrary;

/*!
 * CAIGoalLibrary is collection of global AI goals.
 */
class CAIGoalLibrary
{
public:
	CAIGoalLibrary();
	~CAIGoalLibrary() {};

	//! Add new goal to the library.
	void AddGoal( CAIGoal* goal );
	//! Remove goal from the library.
	void RemoveGoal( CAIGoal* goal );

	CAIGoal* FindGoal( const CString &name ) const;

	//! Clear all goals from library.
	void ClearGoals();

	//! Get all stored goals as a vector.
	void GetGoals( std::vector<CAIGoalPtr> &goals ) const;

	//! Load all goals from givven path and add them to library.
	void LoadGoals( const CString &path );

	//! Initialize atomic goals from AI system.
	void InitAtomicGoals();

private:
	StdMap<CString,CAIGoalPtr> m_goals;
};

#endif // __aigoallibrary_h__