////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   brushindoor.cpp
//  Version:     v1.00
//  Created:     4/12/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BrushIndoor.h"
#include "Brush.h"
#include "Objects\BrushObject.h"

#include <I3DEngine.h>

//////////////////////////////////////////////////////////////////////////
CBrushIndoor::CBrushIndoor()
{
	m_buildingId = -1;
}

//////////////////////////////////////////////////////////////////////////
CBrushIndoor::~CBrushIndoor()
{

}

//////////////////////////////////////////////////////////////////////////
void CBrushIndoor::MakeIndoor()
{
	ReleaseIndoor();
	IndoorBaseInterface bi;

	bi.m_pLog = GetIEditor()->GetSystem()->GetILog();
	bi.m_pRenderer = GetIEditor()->GetSystem()->GetIRenderer();
	bi.m_p3dEngine = GetIEditor()->GetSystem()->GetI3DEngine();
	bi.m_pConsole = GetIEditor()->GetSystem()->GetIConsole();
	bi.m_pSystem = GetIEditor()->GetSystem();
	m_buildingId = GetBuildMgr()->CreateBuilding(bi);
	GetBuildMgr()->SetBuildingPos( Vec3(0,0,0),m_buildingId );
}

//////////////////////////////////////////////////////////////////////////
void CBrushIndoor::ReleaseIndoor()
{
	if (m_buildingId)
	{
		GetBuildMgr()->DeleteBuilding(m_buildingId);
		m_buildingId = -1;
	}
}

//////////////////////////////////////////////////////////////////////////
IIndoorBase* CBrushIndoor::GetBuildMgr()
{
	return GetIEditor()->Get3DEngine()->GetBuildingManager();
}

//////////////////////////////////////////////////////////////////////////
void CBrushIndoor::AddObject( IStatObj *object )
{
	if (m_buildingId >= 0)
	{
		GetBuildMgr()->SetOutsideStatObj( m_buildingId,object,false );
		GetBuildMgr()->SetBuildingBBox( object->GetBoxMin(),object->GetBoxMax(),m_buildingId );
	}
}

//////////////////////////////////////////////////////////////////////////
void CBrushIndoor::RemoveObject( IStatObj *object )
{
	if (m_buildingId >= 0)
	{
		GetBuildMgr()->SetOutsideStatObj( m_buildingId,object,true );
	}
}

//////////////////////////////////////////////////////////////////////////
void CBrushIndoor::UpdateObject( IStatObj *object )
{
	if (m_buildingId >= 0)
	{
		//GetBuildMgr()->SetOutsideStatObj( m_buildingId,object,true );
	}
}

//////////////////////////////////////////////////////////////////////////
void CBrushIndoor::SetBounds( const BBox &bbox )
{
	if (m_buildingId >= 0)
	{
		GetBuildMgr()->SetBuildingBBox( bbox.min,bbox.max,m_buildingId );
	}
}

//////////////////////////////////////////////////////////////////////////
void CBrushIndoor::AddBrush( CBrushObject *brushObj )
{
	m_brushes.insert( brushObj );
}

//////////////////////////////////////////////////////////////////////////
void CBrushIndoor::RemoveBrush( CBrushObject *brushObj )
{
	m_brushes.erase( brushObj );
}

//////////////////////////////////////////////////////////////////////////
void CBrushIndoor::RecalcBounds()
{
	BBox bounds,box;
	bounds.Reset();
	for (Brushes::iterator it = m_brushes.begin(); it != m_brushes.end(); ++it)
	{
		CBrushObject *brushObj = *it;
		brushObj->GetBoundBox( box );
		bounds.Add( box.min );
		bounds.Add( box.max );
	}
	if (m_buildingId >= 0)
	{
		GetBuildMgr()->SetBuildingBBox( bounds.min,bounds.max,m_buildingId );
	}
}

//////////////////////////////////////////////////////////////////////////
void CBrushIndoor::GetObjects( std::vector<CBrushObject*> &objects )
{
	objects.clear();
	objects.insert( objects.end(),m_brushes.begin(),m_brushes.end() );
}