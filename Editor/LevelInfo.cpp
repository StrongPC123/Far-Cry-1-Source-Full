////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   levelinfo.cpp
//  Version:     v1.00
//  Created:     4/6/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "LevelInfo.h"
#include "Objects\ObjectManager.h"
#include "Material\MaterialManager.h"

#include "Heightmap.h"
#include "VegetationMap.h"

//////////////////////////////////////////////////////////////////////////
CLevelInfo::CLevelInfo()
{
	m_pReport = GetIEditor()->GetErrorReport();
}

//////////////////////////////////////////////////////////////////////////
void CLevelInfo::SaveLevelResources( const CString &toPath )
{
}

//////////////////////////////////////////////////////////////////////////
void CLevelInfo::Validate()
{
	m_pReport->Clear();
	m_pReport->SetImmidiateMode(false);
	// Validate level.
	GetIEditor()->GetHeightmap()->GetVegetationMap()->Validate( *m_pReport );

	ValidateObjects();
	ValidateMaterials();

	if (m_pReport->GetErrorCount() == 0)
	{
		AfxMessageBox( _T("No Errors Found") );
	}
	else
		m_pReport->Display();
}

//////////////////////////////////////////////////////////////////////////
void CLevelInfo::ValidateObjects()
{
	CWaitCursor cursor;

	// Validate all objects
	CBaseObjectsArray objects;
	GetIEditor()->GetObjectManager()->GetObjects( objects );

	int i;

	CLogFile::WriteLine( "Validating Objects..." );
	for (i = 0; i < objects.size(); i++)
	{
		CBaseObject *pObject = objects[i];

		m_pReport->SetCurrentValidatorObject( pObject );

		pObject->Validate( m_pReport );

		CUsedResources rs;
		pObject->GatherUsedResources( rs );
		rs.Validate( m_pReport );

		m_pReport->SetCurrentValidatorObject( NULL );
	}

	CLogFile::WriteLine( "Validating Duplicate Objects..." );
	//////////////////////////////////////////////////////////////////////////
	// Find duplicate objects, Same objects with same transform.
	// Use simple grid parition for speed up check.
	//////////////////////////////////////////////////////////////////////////
	int gridSize = 256;

	SSectorInfo si;
	GetIEditor()->GetHeightmap()->GetSectorsInfo( si );
	float worldSize = si.numSectors * si.sectorSize;
	float fGridToWorld = worldSize / gridSize;

	// Put all objects into parition grid.
	std::vector<std::list<CBaseObject*> > grid;
	grid.resize( gridSize*gridSize );
	// Put objects to grid.
	for (i = 0; i < objects.size(); i++)
	{
		CBaseObject *pObject = objects[i];
		Vec3 pos = pObject->GetWorldPos();
		int px = ftoi( pos.x/fGridToWorld );
		int py = ftoi( pos.y/fGridToWorld );
		if (px < 0) px = 0;
		if (py < 0) py = 0;
		if (px >= gridSize) px = gridSize-1;
		if (py >= gridSize) py = gridSize-1;
		grid[py*gridSize + px].push_back( pObject );
	}

	std::list<CBaseObject*>::iterator it1,it2;
	// Check objects in grid.
	for (i = 0; i < gridSize*gridSize; i++)
	{
		std::list<CBaseObject*>::iterator first = grid[i].begin();
		std::list<CBaseObject*>::iterator last = grid[i].end();
		for (it1 = first; it1 != last; ++it1)
			for (it2 = first; it2 != last; ++it2)
			{
				if (it1 != it2)
				{
					// Check if same object.
					CBaseObject *p1 = *it1;
					CBaseObject *p2 = *it2;
					if (p1->GetClassDesc() == p2->GetClassDesc())
					{
						// Same class.
						if (p1->GetWorldPos() == p2->GetWorldPos() && p1->GetAngles() == p2->GetAngles() && p1->GetScale() == p2->GetScale())
						{
							// Same transformation
							// Check if objects are really same.
							if (p1->IsSimilarObject(p2))
							{
								// Report duplicate object.
								CErrorRecord err;
								err.error.Format( "Duplicate object detected %s",(const char*)p1->GetName() );
								err.pObject = p1;
								err.severity = CErrorRecord::ESEVERITY_WARNING;
								m_pReport->ReportError( err );
							}
						}
					}
				}
			}
	}
}

//////////////////////////////////////////////////////////////////////////
void CLevelInfo::ValidateMaterials()
{
	// Validate all objects
	CBaseObjectsArray objects;
	GetIEditor()->GetMaterialManager()->Validate();
}