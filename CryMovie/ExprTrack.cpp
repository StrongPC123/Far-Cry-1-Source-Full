////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   selecttrack.cpp
//  Version:     v1.00
//  Created:     20/8/2002 by Lennert Schneider.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ExprTrack.h"

//////////////////////////////////////////////////////////////////////////
void CExprTrack::SerializeKey( IExprKey &key,XmlNodeRef &keyNode,bool bLoading )
{
	if (bLoading)
	{
		const char *desc;
		desc=keyNode->getAttr( "name");
		strncpy(key.pszName, desc, sizeof(key.pszName));
		key.pszName[sizeof(key.pszName)-1]=0;
		keyNode->getAttr("amp", key.fAmp);
		keyNode->getAttr("blendin", key.fBlendIn);
		keyNode->getAttr("hold", key.fHold);
		keyNode->getAttr("blendout", key.fBlendOut);
	}
	else
	{
		keyNode->setAttr("name", key.pszName);
		keyNode->setAttr("amp", key.fAmp);
		keyNode->setAttr("blendin", key.fBlendIn);
		keyNode->setAttr("hold", key.fHold);
		keyNode->setAttr("blendout", key.fBlendOut);
	}
}

//////////////////////////////////////////////////////////////////////////
void CExprTrack::GetKeyInfo( int key,const char* &description, float &duration )
{
	assert( key >= 0 && key < (int)m_keys.size() );
	CheckValid();
	duration=m_keys[key].fBlendIn+m_keys[key].fHold+m_keys[key].fBlendOut;
	if (strlen(m_keys[key].pszName) > 0)
		description=m_keys[key].pszName;
	else
		description=0;
}