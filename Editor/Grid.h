////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   grid.h
//  Version:     v1.00
//  Created:     8/7/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __grid_h__
#define __grid_h__

#if _MSC_VER > 1000
#pragma once
#endif

/** Definition of grid used in 2D viewports.
*/
class CGrid
{
public:
	//! Resolution of grid, it must be multiply of 2.
	float size;
	//! Draw major lines every Nth grid line.
	int majorLine;
	//! True if grid enabled.
	bool bEnabled;
	//! Meters per grid unit.
	float scale;

	//! If snap to angle.
	bool bAngleSnapEnabled;
	float angleSnap;

	//////////////////////////////////////////////////////////////////////////
	CGrid();

	//! Snap vector to this grid.
	Vec3 Snap( const Vec3 &vec ) const;

	//! Snap angle to current angle snapping value.
	float SnapAngle( float angle ) const;
	//! Snap angle to current angle snapping value.
	Vec3 SnapAngle( const Vec3 &angle ) const;

	//! Enable or disable grid.
	void Enable( bool enable ) { bEnabled = enable; }
	//! Check if grid enabled.
	bool IsEnabled() const { return bEnabled; }

	//! Enables or disable angle snapping.
	void EnableAngleSnap( bool enable ) { bAngleSnapEnabled = enable; };

	//! Return if snapping of angle is enabled.
	bool IsAngleSnapEnabled() const { return bAngleSnapEnabled; };
	//! Returns ammount of snapping for angle in degrees.
	float GetAngleSnap() const { return angleSnap; };

	void Serialize( XmlNodeRef &xmlNode,bool bLoading );
};


#endif // __grid_h__
