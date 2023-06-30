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
#include "AiGoal.h"

//////////////////////////////////////////////////////////////////////////
// CAIgoal implementation.
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
CAIGoal::CAIGoal()
{
	m_atomic = false;
	m_modified = false;
}

//////////////////////////////////////////////////////////////////////////
CAIGoal::~CAIGoal()
{
	m_atomic = false;
}

//////////////////////////////////////////////////////////////////////////
void CAIGoal::Serialize( XmlNodeRef &node,bool bLoading )
{
	if (bLoading)
	{
		m_stages.clear();

		// Loading.
		node->getAttr( "Name",m_name );

		m_stages.resize( node->getChildCount() );
		for (int i = 0; i < node->getChildCount(); i++)
		{
			// Write goals stages to xml.
			CAIGoalStage &stage = m_stages[i];
			XmlNodeRef stageNode = node->getChild(i);
			stageNode->getAttr( "Blocking",stage.blocking );
			stage.params->copyAttributes( stageNode );
			stage.params->delAttr( "Blocking" );
		}
	}
	else
	{
		// Saving.
		node->setAttr( "Name",m_name );

		for (int i = 0; i < m_stages.size(); i++)
		{
			// Write goals stages to xml.
			CAIGoalStage &stage = m_stages[i];
			XmlNodeRef stageNode = node->newChild( stage.name );
			stageNode->copyAttributes( stage.params );
			stageNode->setAttr( "Blocking",stage.blocking );
		}
	}
}