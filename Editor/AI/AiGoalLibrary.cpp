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

#include "StdAfx.h"
#include "AiGoalLibrary.h"
#include "AiGoal.h"
#include "AiBehaviorLibrary.h"

#include "IAISystem.h"
#include "AIManager.h"

//////////////////////////////////////////////////////////////////////////
// CAIGoalLibrary implementation.
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
CAIGoalLibrary::CAIGoalLibrary()
{
}

//////////////////////////////////////////////////////////////////////////
void CAIGoalLibrary::InitAtomicGoals()
{
	// Create Atomic goals.
	IAISystem *ai = GetIEditor()->GetAI()->GetAISystem();
	char xmlBuf[32768];
	strcpy( xmlBuf,"" );
	int num = 0;//ai->EnumAtomicGoals( xmlBuf,sizeof(xmlBuf) );
	
	XmlParser parser;
	XmlNodeRef node = parser.parseBuffer( xmlBuf );
	
	//FILE *file = fopen( "c:\\test.xml","wt" );
	//fprintf( file,"%s",xmlBuf );
	//fclose(file);

	if (!node)
		return;
	
	for (int i = 0; i < node->getChildCount(); i++)
	{
		// Create new atomic goal.
		XmlNodeRef goalNode = node->getChild(i);
		CString goalName = goalNode->getTag();
		CString goalDesc;
		goalNode->getAttr( "Description",goalDesc );
		
		CAIGoalPtr goal = new CAIGoal;
		goal->SetName( goalName );
		goal->SetDescription( goalDesc );
		goal->SetAtomic(true);
		XmlNodeRef params = new CXmlNode("Params");
		for (int j = 0; j < goalNode->getChildCount(); j++)
		{
			params->addChild(goalNode->getChild(j));
		}
		goal->GetParamsTemplate() = params;
		AddGoal( goal );
	}

	CAIGoalPtr goal = new CAIGoal;
	goal->SetName( "Attack" );
	goal->SetDescription( "Attacks enemy" );

	CAIGoalStage stage;
	stage.name = "locate";
	stage.blocking = true;
	goal->AddStage( stage );
	stage.name = "approach";
	stage.blocking = true;
	goal->AddStage( stage );
	stage.name = "pathfind";
	stage.blocking = true;
	goal->AddStage( stage );


	//goal->AddStage( 
	AddGoal( goal );
}

//////////////////////////////////////////////////////////////////////////
void CAIGoalLibrary::AddGoal( CAIGoal* goal )
{
	CAIGoalPtr g;
	if (m_goals.Find(goal->GetName(),g))
	{
		// Goal with this name already exist in the library.
	}
	m_goals[goal->GetName()] = goal;
}
	
//////////////////////////////////////////////////////////////////////////
void CAIGoalLibrary::RemoveGoal( CAIGoal* goal )
{
	m_goals.Erase( goal->GetName() );
}

//////////////////////////////////////////////////////////////////////////
CAIGoal* CAIGoalLibrary::FindGoal( const CString &name ) const
{
	CAIGoalPtr goal=0;
	m_goals.Find( name,goal );
	return goal;
}

//////////////////////////////////////////////////////////////////////////
void CAIGoalLibrary::ClearGoals()
{
	m_goals.Clear();
}

//////////////////////////////////////////////////////////////////////////
void CAIGoalLibrary::LoadGoals( const CString &path )
{
	// Scan all goal files.
}

//////////////////////////////////////////////////////////////////////////
void CAIGoalLibrary::GetGoals( std::vector<CAIGoalPtr> &goals ) const
{
	m_goals.GetAsVector(goals);
}