////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   SurfaceType.cpp
//  Version:     v1.00
//  Created:     19/11/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Surface type class implementation.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "SurfaceType.h"
#include "GameEngine.h"

//////////////////////////////////////////////////////////////////////////
CSurfaceType::CSurfaceType()
{
	m_detailScale[0] = 1;
	m_detailScale[1] = 1;
	m_projAxis = ESFT_Z;
}

//////////////////////////////////////////////////////////////////////////
CSurfaceType::~CSurfaceType()
{
	m_detailScale[0] = 1;
	m_detailScale[1] = 1;
}

//////////////////////////////////////////////////////////////////////////
void CSurfaceType::Serialize( CXmlArchive &xmlAr )
{
	if (xmlAr.bLoading)
	{
		XmlNodeRef sfType = xmlAr.root;

		// Name
		sfType->getAttr( "Name",m_name );
		sfType->getAttr( "DetailTexture",m_detailTexture );
		sfType->getAttr( "DetailScaleX",m_detailScale[0] );
		sfType->getAttr( "DetailScaleY",m_detailScale[1] );
		sfType->getAttr( "Material",m_material );
		sfType->getAttr( "ProjectAxis",m_projAxis );
		sfType->getAttr( "Bumpmap",m_bumpmap );

		XmlNodeRef sfDetObjs = sfType->findChild( "DetailObjects" );
		
		m_detailObjects.clear();
		/*for (int i = 0; i < sfDetObjs->getChildCount(); i++)
		{
			CString s = "";
			sfDetObjs->getChild(i)->getAttr( "Name",s );
			m_detailObjects.push_back( s );
		}*/
	}
	else
	{
		////////////////////////////////////////////////////////////////////////
		// Storing
		////////////////////////////////////////////////////////////////////////
		XmlNodeRef sfType = xmlAr.root;

		// Name
		sfType->setAttr( "Name",m_name );
		sfType->setAttr( "DetailTexture",m_detailTexture );
		sfType->setAttr( "DetailScaleX",m_detailScale[0] );
		sfType->setAttr( "DetailScaleY",m_detailScale[1] );
		sfType->setAttr( "Material",m_material );
		sfType->setAttr( "ProjectAxis",m_projAxis );
		sfType->setAttr( "Bumpmap",m_bumpmap );

		switch (m_projAxis)
		{
		case 0:
			sfType->setAttr( "ProjAxis","X" );
			break;
		case 1:
			sfType->setAttr( "ProjAxis","Y" );
			break;
		case 2:
		default:
			sfType->setAttr( "ProjAxis","Z" );
		};

		XmlNodeRef sfDetObjs = sfType->newChild( "DetailObjects" );
		
		for (int i = 0; i < m_detailObjects.size(); i++)
		{
			XmlNodeRef sfDetObj = sfDetObjs->newChild( "Object" );
			sfDetObj->setAttr( "Name",m_detailObjects[i] );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CSurfaceType::SetMaterial( const CString &mtl )
{
	m_material = mtl;
	GetIEditor()->GetGameEngine()->PutSurfaceTypesToGame();
};