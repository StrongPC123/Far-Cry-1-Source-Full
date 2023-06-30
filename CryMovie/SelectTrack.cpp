////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   selecttrack.cpp
//  Version:     v1.00
//  Created:     23/7/2002 by Lennert Schneider.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "SelectTrack.h"

//////////////////////////////////////////////////////////////////////////
void CSelectTrack::SerializeKey( ISelectKey &key,XmlNodeRef &keyNode,bool bLoading )
{
	if (bLoading)
	{
		const char *szSelection;
		szSelection= keyNode->getAttr( "node" );

		strncpy( key.szSelection,szSelection,sizeof(key.szSelection) );
		key.szSelection[sizeof(key.szSelection)-1] = 0;
	}
	else
	{
		keyNode->setAttr( "node",key.szSelection);
	}
}

//////////////////////////////////////////////////////////////////////////
void CSelectTrack::GetKeyInfo( int key,const char* &description,float &duration )
{
	assert( key >= 0 && key < (int)m_keys.size() );
	CheckValid();
	description = 0;
	duration = m_keys[key].fDuration;
	if (strlen(m_keys[key].szSelection) > 0)
		description = m_keys[key].szSelection;
}