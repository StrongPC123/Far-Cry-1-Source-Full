#ifndef __axisgizmo_h__
#define __axisgizmo_h__
#pragma once

#include "BaseObject.h"
#include "Gizmo.h"

// forward declarations.
struct DisplayContext;

/** Gizmo of Objects animation track.
*/
class CAxisGizmo : public CGizmo
{
public:
	CAxisGizmo( CBaseObject *object );
	~CAxisGizmo();

	//////////////////////////////////////////////////////////////////////////
	// Ovverides from CGizmo
	//////////////////////////////////////////////////////////////////////////
	virtual void GetWorldBounds( BBox &bbox );
	virtual void Display( DisplayContext &dc );
	virtual bool HitTest( HitContext &hc );

	//////////////////////////////////////////////////////////////////////////
	void DrawAxis( DisplayContext &dc );

	static GetGlobalAxisGizmoCount() { return m_axisGizmoCount; }

private:
	void OnObjectEvent( CBaseObject *object,int event );

	CBaseObjectPtr m_object;
	BBox m_bbox;

	int m_highlightAxis;

	static int m_axisGizmoCount;
};

// Define CGizmoPtr smart pointer.
SMARTPTR_TYPEDEF(CAxisGizmo);

#endif