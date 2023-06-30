////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   aigoal.h
//  Version:     v1.00
//  Created:     21/3/2002 by Timur.
//  Compilers:   Visual C++ 7.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __aigoal_h__
#define __aigoal_h__

#if _MSC_VER > 1000
#pragma once
#endif

/*
 *	CAIGoalStage is single stage of AI goal.
 */
class CAIGoalStage
{
public:
	//! Name of goal used by this stage.
	CString name;
	//! True if this stage will block goal pipeline execution.
	bool blocking;
	//! Goal parameters.
	XmlNodeRef params;
};

/*!
 *	CAIGoal contain definition of AI goal pipe.
 */
class CAIGoal : public CRefCountBase
{
public:
	CAIGoal();
	~CAIGoal();

	const CString& GetName() { return m_name; }
	void SetName( const CString& name ) { m_name = name; }

	//! Get human readable description of this goal.
	const CString& GetDescription() { return m_description; }
	//! Set human readable description of this goal.
	void SetDescription( const CString& desc ) { m_description = desc; }

	//////////////////////////////////////////////////////////////////////////
	//! Return true if this goal is Atomic goal, (atomic goals defined by system)
	bool IsAtomic() const { return m_atomic; };
	//! Set this goal as atomic.
	void SetAtomic( bool atomic ) { m_atomic = atomic; };
	
	//! Return true if goal was modified by user and should be stored in goal database.
	bool IsModified() const { return m_modified; };
	// Mark this goal as modified.
	void SetModified( bool modified ) { m_modified = modified; }

	//! Get number of stages in goal.
	int GetStageCount() const { return m_stages.size(); };
	//! Get goal stage at specified index.
	CAIGoalStage&	GetStage( int index ) { return m_stages[index]; }
	const CAIGoalStage&	GetStage( int index ) const { return m_stages[index]; }
	void AddStage( const CAIGoalStage &stage ) { m_stages.push_back(stage); }

	//! Template for parameters used in goal.
	XmlNodeRef&	GetParamsTemplate() { return m_paramsTemplate; };

	//! Serialize Goal to/from xml.
	void Serialize( XmlNodeRef &node,bool bLoading );

private:
	CString m_name;
	CString m_description;

	std::vector<CAIGoalStage> m_stages;
	XmlNodeRef m_attributes;
	//! True if its atomic goal.
	bool m_atomic;
	bool m_modified;

	//! Parameters template for this goal.
	XmlNodeRef m_paramsTemplate;
};

// Define smart pointer to AIGoal
typedef TSmartPtr<CAIGoal> CAIGoalPtr;

#endif // __aigoal_h__
