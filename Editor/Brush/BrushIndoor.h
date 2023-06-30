////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   brushindoor.h
//  Version:     v1.00
//  Created:     4/12/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __brushindoor_h__
#define __brushindoor_h__
#pragma once

class CBrushObject;

/** Holds reference to indoor building to which brushes are added.
*/
class CBrushIndoor
{
public:
	CBrushIndoor();
	~CBrushIndoor();

	struct IIndoorBase* GetBuildMgr();

	//! Add new brush object to indoor.
	void AddBrush( CBrushObject *brushObj );
	//! Remove brush object from indoor.
	void RemoveBrush( CBrushObject *brushObj );

	void MakeIndoor();
	void ReleaseIndoor();

	void AddObject( IStatObj *object );
	void RemoveObject( IStatObj *object );
	void UpdateObject( IStatObj *object );

	//! Sound indoor bounding box in world space.
	void SetBounds( const BBox &bbox );

	//! Recalculate bounding box.
	void RecalcBounds();

	void GetObjects( std::vector<CBrushObject*> &objects );

private:
	// Id of building.
	int m_buildingId;

	typedef std::set<CBrushObject*> Brushes;
	Brushes m_brushes;
};


#endif // __brushindoor_h__
