////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   trackgizmo.h
//  Version:     v1.00
//  Created:     2/7/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __trackgizmo_h__
#define __trackgizmo_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "Gizmo.h"

// forward declarations.
struct DisplayContext;
struct IAnimNode;

/** Gizmo of Objects animation track.
*/
class CTrackGizmo : public CGizmo
{
public:
	CTrackGizmo();
	~CTrackGizmo();

	//////////////////////////////////////////////////////////////////////////
	// Ovverides from CGizmo
	//////////////////////////////////////////////////////////////////////////
	virtual void GetWorldBounds( BBox &bbox );
	virtual void Display( DisplayContext &dc );
	virtual bool HitTest( HitContext &hc );

	//////////////////////////////////////////////////////////////////////////
	void SetAnimNode( IAnimNode *node );
	void DrawAxis( DisplayContext &dc,const Vec3 &pos );

private:
	IAnimNode *m_animNode;
	BBox m_bbox;
	bool m_keysSelected;
};

// Define CGizmoPtr smart pointer.
SMARTPTR_TYPEDEF(CTrackGizmo);

#endif // __trackgizmo_h__
